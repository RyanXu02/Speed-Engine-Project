#pragma once

#include "Core/features/Serializable.h"

#include <glm/glm.hpp>
#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>

namespace SE
{
	// @brief Represents a RigidBody physics component that can be attached to an entity
	class RigidBody
	{
	public:
		RigidBody();
		~RigidBody();

		JPH::BodyID getBodyID() const;
		bool isCreated() const;
		void createBody(); 

		JPH::EMotionType getMotionType() const { return m_motionType; };
		void setMotionType(JPH::EMotionType motionType);
		
		JPH::Ref<JPH::Shape> getShape() const { return m_shape; }
		void setShape(JPH::Ref<JPH::Shape> shape);
		
		glm::vec3 getOffset() const { return m_offset; }
		void setOffset(const glm::vec3& offset) { m_offset = offset; };

		glm::vec3 getLinearVelocity() const;
		void setLinearVelocity(const glm::vec3& velocity);
		glm::vec3 getAngularVelocity() const;
		void setAngularVelocity(const glm::vec3& velocity);
		glm::vec3 getPosition() const;
		void setPosition(const glm::vec3& position);
		glm::quat getRotation() const;
		void setRotation(const glm::quat& rotation);
		glm::mat4 getTransform() const;

		void addForce(const glm::vec3& force);
		void addTorque(const glm::vec3& torque);
		void addImpulse(const glm::vec3& impulse);
		void addAngularImpulse(const glm::vec3& impulse);

		void activateBody();
		void deactivateBody();
		bool isActive() const;

		float getMaxLinearVelocity() const { return m_maxLinearVelocity; }
		void setMaxLinearVelocity(float maxVelocity);
		float getMaxAngularVelocity() const { return m_maxAngularVelocity; }
		void setMaxAngularVelocity(float maxVelocity);

		float getRestitution() const { return m_restitution; }
		void setRestitution(float restitution);
		float getFriction() const { return m_friction; }
		void setFriction(float friction);
		float getGravityFactor() const { return m_gravityFactor; }
		void setGravityFactor(float factor);

		DEFINE_TYPE(RigidBody,
			member("motionType", &RigidBody::m_motionType),
			//member("offset", &RigidBody::m_offset), 
			member("isActive", &RigidBody::m_isActive),
			member("maxLinearVelocity", &RigidBody::m_maxLinearVelocity),
		    member("maxAngularVelocity", &RigidBody::m_maxAngularVelocity),
			member("restitution", &RigidBody::m_restitution),
			member("friction", &RigidBody::m_friction),
			member("gravityFactor", &RigidBody::m_gravityFactor));

	private:
		JPH::Ref<JPH::Shape> m_shape{ nullptr };
		JPH::BodyInterface* m_bodyInterface{ nullptr };
		JPH::Body* m_body{ nullptr };

		// Properties
		JPH::EMotionType m_motionType{ JPH::EMotionType::Dynamic };
		glm::vec3 m_offset{ 0.0f, 0.0f, 0.0f };
		bool m_isActive{ false };
		float m_maxLinearVelocity{ 500.0f };
		float m_maxAngularVelocity{ 15.0f * 3.14159f };
		float m_restitution{ 0.0f };
		float m_friction{ 0.2f };
		float m_gravityFactor{ 1.0f };
	};
}
