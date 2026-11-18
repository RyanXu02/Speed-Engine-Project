#include "pch.h"

#include "UISystem.h"
#include "UILayout.h"
#include "Core/Engine.h"
#include "Core/EngineStates.h"
#include "Core/SubSystems/Events/EventSystem.h"
#include "Core/SubSystems/Input/InputSystem.h"
#include "Core/SubSystems/Scenes/SceneSystem.h"

namespace SE
{
	namespace // Helper functions for UISystem
	{
		static bool isEligibleCanvas(Scene & scene, Entity entity)
		{
			const auto* canvas = entity.getComponent<UICanvasComponent>();
			return canvas && canvas->visible && canvas->inputEnabled && !scene.isEntityPendingRemoval(entity.id());
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
				    if (element.canvasKey == canvas.canvasKey && element.visible && element.enabled && rect.size.x > 0.0f && rect.size.y > 0.0f && !scene.isEntityPendingRemoval(entity.id()))
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

		static uint32_t hitTest(Entity canvasEntity, const std::vector<Entity>& candidates, const InputSystem& input, bool buttonsOnly = false)
		{
			if (!input.isPointerInInputRegion())
				return 0;
			const auto pointer = input.getPointerPositionInInputRegion();
			const auto& canvas = *canvasEntity.getComponent<UICanvasComponent>();
			if (!pointer || canvas.referenceResolution.x <= 0.0f || canvas.referenceResolution.y <= 0.0f)
				return 0;
			const auto layout = resolveUICanvasLayout(glm::vec2(input.getInputRegion().renderTargetSize), canvas.referenceResolution);
			uint32_t hit = 0;
			int topOrder = std::numeric_limits<int>::min();
			for (auto entity : candidates)
			{
				if (buttonsOnly && !entity.getComponent<UIButtonComponent>())
					continue;
				const auto rect = resolveUIRectLayout(*entity.getComponent<UIRectComponent>(), layout);
				const auto max = rect.pixelTopLeft + rect.pixelSize;
				const int order = entity.getComponent<UIElementComponent>()->drawOrder;
				if (pointer->x >= rect.pixelTopLeft.x && pointer->y >= rect.pixelTopLeft.y && pointer->x < max.x && pointer->y < max.y &&
					(order > topOrder || (order == topOrder && entity.id() > hit)))
				{
					hit = entity.id();
					topOrder = order;
				}
			}
			return hit;
		}

		static void publishActivations(std::vector<ButtonActivated>& activations)
		{
			auto* events = Engine::Instance().getSubSystem<EventSystem>();
			for (auto& activation : activations)
				events->publish(std::make_unique<ButtonActivated>(std::move(activation)));
		}

		static void setFlag(Scene& scene, uint32_t id, bool UISelectableComponent::*flag, bool value)
		{
			if (scene.hasEntity(id))
				if (auto* selectable = scene.getEntity(id).getComponent<UISelectableComponent>(); selectable && selectable->*flag != value)
					selectable->*flag = value;
		}

	} // Helper functions for UISystem


	void UISystem::_changeFocus(Scene& scene, uint32_t elementId)
	{
		if (m_capturedElement != 0 && m_captureSource == CaptureSource::Command && m_capturedElement != elementId)
			_cancelCapture(scene);
		if (m_focusedElement == elementId)
			return;
		setFlag(scene, m_focusedElement, &UISelectableComponent::focused, false);
		m_focusedElement = elementId;
		setFlag(scene, elementId, &UISelectableComponent::focused, true);
	}

	void UISystem::_beginPress(Scene& scene, uint32_t elementId, CaptureSource source)
	{
		if (m_capturedElement != 0 || elementId == 0 || !scene.getEntity(elementId).getComponent<UIButtonComponent>())
			return;
		m_capturedElement = elementId;
		m_captureSource = source;
		setFlag(scene, elementId, &UISelectableComponent::pressed, true);
	}

	void UISystem::_releasePress(Scene& scene, uint32_t elementId, CaptureSource source, std::vector<ButtonActivated>& activations)
	{
		if (m_capturedElement == 0 || m_captureSource != source)
			return;
		if (m_capturedElement == elementId)
		{
			const auto& element = *scene.getEntity(m_capturedElement).getComponent<UIElementComponent>();
			activations.emplace_back(m_capturedElement, element.canvasKey, element.elementKey);
		}
		_cancelCapture(scene);
	}

	void UISystem::_cancelCapture(Scene& scene)
	{
		setFlag(scene, m_capturedElement, &UISelectableComponent::pressed, false);
		m_capturedElement = 0;
	}

	Scene* UISystem::_refreshFocus()
	{
		auto* scenes = Engine::Instance().getSubSystem<SceneSystem>();
		auto* scene = scenes->getCurrentScene();
		if (m_sceneRevision != scenes->getSceneRevision())
		{
			m_activeCanvas = 0;
			m_focusedElement = 0;
			m_capturedElement = 0;
			m_sceneRevision = scenes->getSceneRevision();
		}
		if (!scene)
		{
			m_activeCanvas = 0;
			m_focusedElement = 0;
			m_capturedElement = 0;
			return nullptr;
		}

		if (!scene->hasEntity(m_activeCanvas) || !isEligibleCanvas(*scene, scene->getEntity(m_activeCanvas)))
		{
			m_activeCanvas = 0;
			_changeFocus(*scene, 0);
			_cancelCapture(*scene);
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
			const auto eligible = [&](uint32_t id)
			{
				return std::ranges::any_of(candidates, [id](Entity entity) { return entity.id() == id; });
			};
			_changeFocus(*scene, eligible(m_focusedElement) ? m_focusedElement : 0);
			if (!eligible(m_capturedElement) || !scene->getEntity(m_capturedElement).getComponent<UIButtonComponent>())
				_cancelCapture(*scene);
		}
		if (!isAcceptingCommands())
			_cancelCapture(*scene);

		return scene;
	}

	bool UISystem::activateCanvas(uint32_t canvasId)
	{
		auto* scene = _refreshFocus();
		if (!scene || !scene->hasEntity(canvasId) || !isEligibleCanvas(*scene, scene->getEntity(canvasId)))
			return false;

		if (m_activeCanvas != canvasId)
			_cancelCapture(*scene);
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
		std::vector<ButtonActivated> activations;
		_processCommand(command, activations);
		publishActivations(activations);
	}

	void UISystem::_processCommand(UICommand command, std::vector<ButtonActivated>& activations)
	{
		auto* scene = _refreshFocus();
		if (!scene || !isAcceptingCommands())
			return;

		auto canvasEntity = scene->getEntity(m_activeCanvas);
		const auto candidates = getCandidates(*scene, canvasEntity);
		if (m_focusedElement == 0)
		{
			switch (command)
			{
			case UICommand::NavigateUp:
			case UICommand::NavigateDown:
			case UICommand::NavigateLeft:
			case UICommand::NavigateRight:
			case UICommand::Next:
			case UICommand::Previous:
				_changeFocus(*scene, defaultFocus(canvasEntity, candidates));
				break;
			case UICommand::Cancel:
				_cancelCapture(*scene);
				break;
			default:
				break;
			}
			return;
		}
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

		case UICommand::SubmitPressed:
			_beginPress(*scene, m_focusedElement, CaptureSource::Command);
			return;
		case UICommand::SubmitReleased:
			_releasePress(*scene, m_focusedElement, CaptureSource::Command, activations);
			return;
		case UICommand::Cancel:
			_cancelCapture(*scene);
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

	void UISystem::_processPointer(Scene& scene, const InputSystem& input, std::vector<ButtonActivated>& activations)
	{
		auto canvas = scene.getEntity(m_activeCanvas);
		const auto candidates = getCandidates(scene, canvas);
		const uint32_t hit = hitTest(canvas, candidates, input);
		const auto& mouseDelta = input.getMouseDelta();
		if (mouseDelta.x != 0.0 || mouseDelta.y != 0.0)
			_changeFocus(scene, hit);

		const uint32_t buttonHit = hitTest(canvas, candidates, input, true);
		if (input.isMouseRising(MOUSE_BUTTON_LEFT) && m_capturedElement == 0)
		{
			if (buttonHit != 0 || hit != 0)
				_changeFocus(scene, buttonHit != 0 ? buttonHit : hit);
			_beginPress(scene, buttonHit, CaptureSource::Pointer);
		}
		if (m_capturedElement != 0 && m_captureSource == CaptureSource::Pointer)
		{
			setFlag(scene, m_capturedElement, &UISelectableComponent::pressed, buttonHit == m_capturedElement);
			if (input.isMouseFalling(MOUSE_BUTTON_LEFT))
				_releasePress(scene, buttonHit, CaptureSource::Pointer, activations);
		}
	}

	void UISystem::_processKeyboard(const InputSystem& input, std::vector<ButtonActivated>& activations)
	{
		if (input.isKeyRising(KEY_UP)) _processCommand(UICommand::NavigateUp, activations);
		if (input.isKeyRising(KEY_DOWN)) _processCommand(UICommand::NavigateDown, activations);
		if (input.isKeyRising(KEY_LEFT)) _processCommand(UICommand::NavigateLeft, activations);
		if (input.isKeyRising(KEY_RIGHT)) _processCommand(UICommand::NavigateRight, activations);

		const bool shiftDown = input.isKeyDown(KEY_LEFT_SHIFT) || input.isKeyDown(KEY_RIGHT_SHIFT);
		if (input.isKeyRising(KEY_TAB)) _processCommand(shiftDown ? UICommand::Previous : UICommand::Next, activations);

		const bool submitDown = input.isKeyDown(KEY_ENTER) || input.isKeyDown(KEY_SPACE);
		const bool submitRising = input.isKeyRising(KEY_ENTER) || input.isKeyRising(KEY_SPACE);
		if (!m_submitDown && submitRising) _processCommand(UICommand::SubmitPressed, activations);
		if ((m_submitDown || submitRising) && !submitDown) _processCommand(UICommand::SubmitReleased, activations);
		m_submitDown = submitDown;

		if (input.isKeyRising(KEY_ESCAPE)) _processCommand(UICommand::Cancel, activations);
	}

	void UISystem::update(double)
	{
		auto* scene = _refreshFocus();
		const auto& input = *Engine::Instance().getSubSystem<InputSystem>();
		if (!isAcceptingCommands())
		{
			m_submitDown = input.isKeyDown(KEY_ENTER) || input.isKeyDown(KEY_SPACE);
			return;
		}
		std::vector<ButtonActivated> activations;
		_processPointer(*scene, input, activations);
		_processKeyboard(input, activations);

		publishActivations(activations);
	}
}
