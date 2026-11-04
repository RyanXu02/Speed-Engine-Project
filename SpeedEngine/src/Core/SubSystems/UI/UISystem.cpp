#include "pch.h"

#include "UISystem.h"
#include "UILayout.h"
#include "Core/Engine.h"
#include "Core/EngineStates.h"
#include "Core/SubSystems/Input/InputSystem.h"
#include "Core/SubSystems/Scenes/SceneSystem.h"

namespace SE
{
	namespace // Helper functions for UISystem
	{
		static bool isAlive(Scene & scene, Entity entity)
		{
			const auto* life = entity.getComponent<LifeComponent>();
			return !scene.isEntityPendingRemoval(entity.id()) && (!life || life->isAlive);
		}

		static bool isEligibleCanvas(Scene & scene, Entity entity)
		{
			const auto* canvas = entity.getComponent<UICanvasComponent>();
			return canvas && canvas->visible && canvas->inputEnabled && isAlive(scene, entity);
		}

		static std::vector<Entity> getCandidates(Scene & scene, Entity canvasEntity)
		{
			const auto& canvas = *canvasEntity.getComponent<UICanvasComponent>();
			std::vector<Entity> candidates;
			scene.forEntitiesWith<UIElementComponent, UIRectComponent, UISelectableComponent>(
			    [&](Entity entity)
			    {
				    const auto& element = *entity.getComponent<UIElementComponent>();
				    const auto& rect = *entity.getComponent<UIRectComponent>();
				    if (element.canvasKey == canvas.canvasKey && element.visible && element.enabled && rect.size.x > 0.0f && rect.size.y > 0.0f && isAlive(scene, entity))
					    candidates.push_back(entity);
			    });
			std::ranges::sort(candidates,
			    [](Entity a, Entity b)
			    {
				    const int aOrder = a.getComponent<UISelectableComponent>()->navigationOrder;
				    const int bOrder = b.getComponent<UISelectableComponent>()->navigationOrder;
				    return aOrder != bOrder ? aOrder < bOrder : a.id() < b.id();
			    });
			return candidates;
		}

		static uint32_t defaultFocus(Entity canvasEntity, const std::vector<Entity>& candidates)
		{
			const auto& key = canvasEntity.getComponent<UICanvasComponent>()->defaultFocusElementKey;
			if (!key.empty())
				for (auto entity : candidates)
					if (entity.getComponent<UIElementComponent>()->elementKey == key)
						return entity.id();
			return candidates.empty() ? 0 : candidates.front().id();
		}

		static float navigationScore(glm::vec2 delta, glm::vec2 direction)
		{
			const float forward = glm::dot(delta, direction);
			const float perpendicular = std::abs(delta.x * direction.y - delta.y * direction.x);
			return forward > 0.0f ? forward + 2.0f * perpendicular : std::numeric_limits<float>::infinity();
		}
	}

	void UISystem::_changeFocus(Scene& scene, uint32_t elementId)
	{
		if (scene.hasEntity(m_focusedElement))
			if (auto* selectable = scene.getEntity(m_focusedElement).getComponent<UISelectableComponent>())
				selectable->focused = false;
		m_focusedElement = elementId;
		if (elementId != 0)
			scene.getEntity(elementId).getComponent<UISelectableComponent>()->focused = true;
	}

	Scene* UISystem::_refreshFocus()
	{
		auto* scenes = Engine::Instance().getSubSystem<SceneSystem>();
		auto* scene = scenes->getCurrentScene();
		if (m_sceneRevision != scenes->getSceneRevision())
		{
			m_activeCanvas = 0;
			m_focusedElement = 0;
			m_sceneRevision = scenes->getSceneRevision();
			if (scene)
				scene->forEntitiesWith<UISelectableComponent>([](Entity entity)
				{
					entity.getComponent<UISelectableComponent>()->focused = false;
				});
		}
		if (!scene)
		{
			m_activeCanvas = 0;
			m_focusedElement = 0;
			return nullptr;
		}

		if (!scene->hasEntity(m_activeCanvas) || !isEligibleCanvas(*scene, scene->getEntity(m_activeCanvas)))
		{
			m_activeCanvas = 0;
			_changeFocus(*scene, 0);
			scene->forEntitiesWith<UICanvasComponent>([&](Entity entity)
			{
				if (isEligibleCanvas(*scene, entity) && (m_activeCanvas == 0 || entity.id() < m_activeCanvas))
					m_activeCanvas = entity.id();
			});
		}

		if (m_activeCanvas != 0)
		{
			auto canvas = scene->getEntity(m_activeCanvas);
			const auto candidates = getCandidates(*scene, canvas);
			const bool validFocus = std::ranges::any_of(candidates, [this](Entity entity)
			{
				return entity.id() == m_focusedElement;
			});
			_changeFocus(*scene, validFocus ? m_focusedElement : defaultFocus(canvas, candidates));
		}

		return scene;
	}

	bool UISystem::activateCanvas(uint32_t canvasId)
	{
		auto* scene = _refreshFocus();
		if (!scene || !scene->hasEntity(canvasId) || !isEligibleCanvas(*scene, scene->getEntity(canvasId)))
			return false;

		m_activeCanvas = canvasId;
		auto canvas = scene->getEntity(canvasId);
		_changeFocus(*scene, defaultFocus(canvas, getCandidates(*scene, canvas)));
		return true;
	}

	bool UISystem::setFocus(uint32_t elementId)
	{
		auto* scene = _refreshFocus();
		if (!scene || m_activeCanvas == 0)
			return false;
		for (auto entity : getCandidates(*scene, scene->getEntity(m_activeCanvas)))
			if (entity.id() == elementId)
			{
				_changeFocus(*scene, elementId);
				return true;
			}
		return false;
	}

	bool UISystem::isAcceptingCommands() const
	{
		return m_activeCanvas != 0 && EngineStates::isGameInputActive() &&
			Engine::Instance().getSubSystem<InputSystem>()->isAcceptingInput();
	}

	void UISystem::processCommand(UICommand command)
	{
		auto* scene = _refreshFocus();
		if (!scene || !isAcceptingCommands() || m_focusedElement == 0)
			return;

		auto canvasEntity = scene->getEntity(m_activeCanvas);
		const auto candidates = getCandidates(*scene, canvasEntity);
		auto current = std::ranges::find_if(candidates, [this](Entity entity) { return entity.id() == m_focusedElement; });
		auto currentEntity = *current;
		const auto& selectable = *currentEntity.getComponent<UISelectableComponent>();

		std::string_view link;
		glm::vec2 direction{ 0.0f };
		switch (command)
		{
		case UICommand::NavigateUp: link = selectable.upElementKey; direction.y = -1.0f; break;
		case UICommand::NavigateDown: link = selectable.downElementKey; direction.y = 1.0f; break;
		case UICommand::NavigateLeft: link = selectable.leftElementKey; direction.x = -1.0f; break;
		case UICommand::NavigateRight: link = selectable.rightElementKey; direction.x = 1.0f; break;

		case UICommand::Next:
			_changeFocus(*scene, std::next(current) == candidates.end() ? candidates.front().id() : std::next(current)->id());
			return;
		case UICommand::Previous:
			_changeFocus(*scene, current == candidates.begin() ? candidates.back().id() : std::prev(current)->id());
			return;

		case UICommand::Submit:
		case UICommand::Cancel:
			return;
		}

		// try to find linked element first
		if (!link.empty())
			for (auto entity : candidates)
				if (entity.getComponent<UIElementComponent>()->elementKey == link)
				{
					_changeFocus(*scene, entity.id());
					return;
				}

		const auto& canvas = *canvasEntity.getComponent<UICanvasComponent>();
		const glm::vec2 viewport = Engine::Instance().getSubSystem<InputSystem>()->getInputRegion().renderTargetSize;
		if (viewport.x <= 0.0f || viewport.y <= 0.0f || canvas.referenceResolution.x <= 0.0f || canvas.referenceResolution.y <= 0.0f)
			return;

		const auto layout = resolveUICanvasLayout(viewport, canvas.referenceResolution);
		const auto center = [&](Entity entity)
		{
			const auto rect = resolveUIRectLayout(*entity.getComponent<UIRectComponent>(), layout);
			return rect.topLeft + rect.size * 0.5f;
		};
		const auto origin = center(*current);

		// find the closest element in the given direction
		float bestScore = std::numeric_limits<float>::infinity();
		uint32_t best = m_focusedElement;
		for (auto entity : candidates)
		{
			const float score = navigationScore(center(entity) - origin, direction);
			if (score < bestScore)
			{
				bestScore = score;
				best = entity.id();
			}
		}
		_changeFocus(*scene, best);
	}

	void UISystem::_processKeyboard(const InputSystem& input)
	{
		if (input.isKeyRising(KEY_UP)) processCommand(UICommand::NavigateUp);
		if (input.isKeyRising(KEY_DOWN)) processCommand(UICommand::NavigateDown);
		if (input.isKeyRising(KEY_LEFT)) processCommand(UICommand::NavigateLeft);
		if (input.isKeyRising(KEY_RIGHT)) processCommand(UICommand::NavigateRight);

		const bool shiftDown = input.isKeyDown(KEY_LEFT_SHIFT) || input.isKeyDown(KEY_RIGHT_SHIFT);
		if (input.isKeyRising(KEY_TAB)) processCommand(shiftDown ? UICommand::Previous : UICommand::Next);

		if (input.isKeyRising(KEY_ENTER) || input.isKeyRising(KEY_SPACE)) processCommand(UICommand::Submit);

		if (input.isKeyRising(KEY_ESCAPE)) processCommand(UICommand::Cancel);
	}

	void UISystem::update(double deltaTime)
	{
		_refreshFocus();
		if (isAcceptingCommands())
			_processKeyboard(*Engine::Instance().getSubSystem<InputSystem>());
	}
}
