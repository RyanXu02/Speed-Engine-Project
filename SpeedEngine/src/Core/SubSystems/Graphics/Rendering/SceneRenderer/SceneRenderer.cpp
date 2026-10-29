#include "pch.h"

#include "SceneRenderer.h"

#include "Core/ResourceManagers/ResourceManager.h"
#include "Core/ResourceManagers/Shader/Shader.h"

#include "Core/Scene/Entity/Component/Transform.h"
#include "Core/Scene/Entity/Entity.h"
#include "Core/Scene/Scene.h"

#include "Core/SubSystems/Input/InputSystem.h"

#include "Core/SubSystems/Graphics/Rendering/RenderContext.h"

#include "Engine.h"
#include "EngineStates.h"

#include <glm/gtc/matrix_transform.hpp>

namespace SE
{
	void SceneRenderer::init()
	{
		m_logger.info("Initializing");
	}
	void SceneRenderer::shutdown()
	{
		m_logger.info("Shutting down");
	}
	void SceneRenderer::update(double deltaTime, RenderContext& context)
	{
		if (EngineStates::isEditorCameraActive())
		{
			_updateCamera(deltaTime, context.viewport.getCameraFrustum());
		}
	}

	void SceneRenderer::render(RenderContext& context)
	{
		if (!context.scene)
			return;

		const CameraFrustum& camera = context.viewport.getCameraFrustum();

		// Get default shader
		Shader* defaultShader = context.resources.getResource<Shader>(m_defaultShaderId);
		if (!defaultShader)
		{
			m_logger.critical("Default shader not found, cannot render");
			return;
		}
		// Get default material
		Material* defaultMaterial = context.resources.getResource<Material>(m_defaultMaterialId);

		// per frame uniforms
		defaultShader->bind();
		defaultShader->setMat4("projectionView", camera.viewProjectionMatrix);
		defaultShader->setVec3("camPos", camera.getPosition());
		_setLightUniforms(context, *defaultShader);

		auto entities = context.scene->getEntitiesWith<Transform, NameComponent, MeshComponent>();
		for (auto& entity : entities)
		{
			MeshComponent* mesh = entity.getComponent<MeshComponent>();
			Transform* transform = entity.getComponent<Transform>();
			NameComponent* name = entity.getComponent<NameComponent>();


			uint32_t meshResourceId = mesh->meshResourceID;
			if (meshResourceId == 0)
			{
				continue;
			}

			MeshResource* meshResource = context.resources.getResource<MeshResource>(meshResourceId);
			if (!meshResource)
			{
				m_logger.warn("Mesh resource with ID {} not found, skipping entity {}", meshResourceId, name->name);
				continue;
			}

			defaultShader->setMat4("model", transform->getTransform());	// Set transform

			// each submesh can have different set of material so get material for each mesh
			const std::vector<uint32_t>& meshMaterials = _getMeshMaterials(context, meshResourceId, *meshResource);
			const std::vector<SubMesh>& subMeshes = meshResource->getSubMeshes();
			for (size_t i = 0; i < subMeshes.size(); ++i)
			{
				const int32_t materialIndex = subMeshes[i].materialIndex;
				const bool hasPBR = materialIndex >= 0 && static_cast<size_t>(materialIndex) < meshMaterials.size();
				const PBRMaterial* pbr = hasPBR ? &meshResource->getMaterials()[materialIndex] : nullptr;

				Material* material = hasPBR ? context.resources.getResource<Material>(meshMaterials[materialIndex]) : nullptr;
				if (!material)
					material = defaultMaterial;

				// Set material properties
				defaultShader->setVec4("albedoFactor", pbr ? pbr->albedoFactor : glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
				defaultShader->setVec3("emissiveFactor", pbr ? pbr->emissiveFactor : glm::vec3(0.0f));
				defaultShader->setFloat("roughnessFactor", pbr ? pbr->roughnessFactor : 1.0f);
				defaultShader->setFloat("metallicFactor", pbr ? pbr->metallicFactor : 0.0f);
				defaultShader->setFloat("aoStrength", pbr ? pbr->aoStrength : 1.0f);
				defaultShader->setFloat("normalScale", pbr ? pbr->normalScale : 1.0f);

				if (material) {
					material->bind();
				}

				context.meshes.draw(meshResourceId, *meshResource, i);

				if (material) {
					material->unbind();
				}
			}
		}

		defaultShader->unbind();
	}

	void SceneRenderer::_setLightUniforms(const RenderContext& context, Shader& shader)
	{
		const DirectionalLight& sun = context.sun;
		shader.setBool("useDirLight", sun.enabled);
		shader.setVec3("dirLightDirection", glm::normalize(sun.direction));
		shader.setVec3("dirLightColor", sun.color * sun.intensity);

		auto pointLights = context.scene->getEntitiesWith<Transform, LightComponent>();
		const size_t numLights = std::min(pointLights.size(), MAX_LIGHTS);
		shader.setInt("numLights", static_cast<int>(numLights));
		for (size_t i = 0; i < numLights; ++i)
		{
			Transform* transform = pointLights[i].getComponent<Transform>();
			LightComponent* light = pointLights[i].getComponent<LightComponent>();
			shader.setVec3(fmt::format("lightPositions[{}]", i), transform->getTranslation());
			shader.setVec3(fmt::format("lightColors[{}]", i), light->color * light->intensity);
		}

		shader.setFloat("ambientStrength", m_lightingSettings.ambientStrength);
		shader.setFloat("gamma", m_lightingSettings.gamma);
	}

	const std::vector<uint32_t>& SceneRenderer::_getMeshMaterials(RenderContext& context, uint32_t meshResourceId, const MeshResource& meshResource)
	{
		auto [it, inserted] = m_meshMaterials.try_emplace(meshResourceId);
		if (!inserted) {
			return it->second;
		}

		const std::vector<PBRMaterial>& pbrMaterials = meshResource.getMaterials();
		it->second.reserve(pbrMaterials.size());
		for (size_t i = 0; i < pbrMaterials.size(); ++i)
		{
			const PBRMaterial& pbr = pbrMaterials[i];

			std::vector<std::pair<TextureType, std::string_view>> textures;
			const std::pair<TextureType, const std::string&> candidates[] = {
				{ TextureType::Albedo, pbr.albedoPath },
				{ TextureType::Normal, pbr.normalPath },
				{ TextureType::AmbOcc, pbr.aoPath },
				{ TextureType::Emissive, pbr.emissivePath },
				{ TextureType::Roughness, pbr.roughnessPath },
				{ TextureType::Metallic, pbr.metallicPath },
			};
			for (const auto& [type, path] : candidates)
			{
				if (!path.empty()) {
					textures.emplace_back(type, path);
				}
			}

			uint32_t materialId = 0;
			if (!textures.empty())
			{
				std::string materialName = fmt::format("{}_{}_{}", meshResource.getResourceName(), i, pbr.name);
				materialId = context.resources.addResource<ResourceType::Material>(m_defaultShaderId, std::span<const std::pair<TextureType, std::string_view>>(textures), materialName);
			}
			it->second.push_back(materialId);
		}

		m_logger.info("Created {} materials for mesh resource {}", it->second.size(), meshResource.getResourceName());
		return it->second;
	}

	void SceneRenderer::_updateCameraPosition(double deltaTime, CameraFrustum& camera)
	{
		auto IS = Engine::Instance().getSubSystem<InputSystem>();

		// x-y plane movement
		float movespeed = m_cameraMoveSpeed * static_cast<float>(deltaTime);
		glm::vec3 cameraLookDir = glm::normalize(camera.getTarget() - camera.getPosition());
		cameraLookDir.y = 0.0f;
		glm::vec3 cameraRight = glm::normalize(glm::cross(cameraLookDir, WORLD_UP));
		glm::vec3 CameraMoveDir(0.0f);
		if (EngineStates::isEditorCameraActive())
		{
			if (IS->isKeyDown(KeyCodes::KEY_W))
				CameraMoveDir += cameraLookDir;
			if (IS->isKeyDown(KeyCodes::KEY_S))
				CameraMoveDir += -cameraLookDir;
			if (IS->isKeyDown(KeyCodes::KEY_A))
				CameraMoveDir += -cameraRight;
			if (IS->isKeyDown(KeyCodes::KEY_D))
				CameraMoveDir += cameraRight;
		}

		if (glm::length(CameraMoveDir) > 0.0f)
		{
			CameraMoveDir = glm::normalize(CameraMoveDir);
			camera.move(CameraMoveDir * movespeed);
		}

		// camera dolly movement (to and from target)
		float dollySpeed = m_cameraDollySpeed * static_cast<float>(deltaTime);
		glm::vec3 lookDir = glm::normalize(camera.getTarget() - camera.getPosition());
		if (IS->getMouseScroll().yoffset > 0.0)
		{
			camera.move(lookDir * dollySpeed);
		}
		if (IS->getMouseScroll().yoffset < 0.0)
		{
			camera.move(-lookDir * dollySpeed);
		}

		// camera y-axis movement
		if (IS->isKeyDown(KeyCodes::KEY_SPACE))
			camera.move(WORLD_UP * movespeed);
		if (IS->isKeyDown(KeyCodes::KEY_LEFT_CONTROL))
			camera.move(-WORLD_UP * movespeed);
	}

	void SceneRenderer::_updateCameraTarget(double deltaTime, CameraFrustum& camera)
	{
		auto IS = Engine::Instance().getSubSystem<InputSystem>();

		if (IS->isMouseDown(MouseCodes::MOUSE_BUTTON_3))
		{
			glm::vec2 mouseDelta = glm::vec2{ IS->getMouseDelta().x, IS->getMouseDelta().y };

			glm::vec3 cameraDirection = glm::normalize(camera.getTarget() - camera.getPosition());
			glm::vec3 cameraRight = glm::normalize(glm::cross(cameraDirection, WORLD_UP));
			glm::vec3 cameraUp = glm::normalize(glm::cross(cameraRight, cameraDirection));

			float yawAngle = -mouseDelta.x * m_sensitivity;
			glm::mat4 yawRotation = glm::rotate(glm::mat4(1.0f), yawAngle, cameraUp);
			cameraDirection = glm::vec3(yawRotation * glm::vec4(cameraDirection, 0.0f));
			// Limit pitch avoid gimbal lock
			float pitchAngle = -mouseDelta.y * m_sensitivity;
			glm::vec3 newDirection = glm::vec3(glm::rotate(glm::mat4(1.0f), pitchAngle, cameraRight) * glm::vec4(cameraDirection, 0.0f));
			float upDot = glm::dot(newDirection, WORLD_UP);
			if (upDot < 0.999f && upDot > -0.999f)
			{
				cameraDirection = newDirection;
			}

			float distance = glm::length(camera.getTarget() - camera.getPosition());
			glm::vec3 cameraTarget = camera.getPosition() + cameraDirection * distance;
			camera.setTarget(cameraTarget);
		}
	}

	void SceneRenderer::_updateCamera(double deltaTime, CameraFrustum& camera)
	{
		auto IS = Engine::Instance().getSubSystem<InputSystem>();
		if (IS->isKeyRising(KeyCodes::KEY_LEFT_SHIFT))
		{
			m_cameraMoveSpeed *= 2.0f;
			m_cameraDollySpeed *= 2.0f;
		}
		else if (IS->isKeyFalling(KeyCodes::KEY_LEFT_SHIFT))
		{
			m_cameraMoveSpeed /= 2.0f;
			m_cameraDollySpeed /= 2.0f;
		}
		_updateCameraPosition(deltaTime, camera);
		_updateCameraTarget(deltaTime, camera);
	}
} // namespace SE
