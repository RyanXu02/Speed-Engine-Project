#include "pch.h"
#include "RigidBody.h"

#include <glm/gtc/quaternion.hpp>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/Body.h>

inline glm::vec3 toGLM(const JPH::Vec3& vec) {
	return glm::vec3(vec.GetX(), vec.GetY(), vec.GetZ());
}
inline glm::quat toGLM(const JPH::Quat& quat) {
	return glm::quat(quat.GetW(), quat.GetX(), quat.GetY(), quat.GetZ());
}
inline glm::mat4 toGLM(const JPH::Mat44& mat) {
	glm::mat4 newMat;
	mat.StoreFloat4x4(reinterpret_cast<JPH::Float4*>(&newMat[0][0]));
	return newMat;
}
inline JPH::Vec3 toJPH(const glm::vec3& vec) {
	return JPH::Vec3(vec.x, vec.y, vec.z);
}
inline JPH::Quat toJPH(const glm::quat& quat) {
	return JPH::Quat(quat.x, quat.y, quat.z, quat.w);
}
inline JPH::Mat44 toJPH(const glm::mat4& mat) {
	return JPH::Mat44(
	    JPH::Vec4(mat[0][0], mat[0][1], mat[0][2], mat[0][3]), 
		JPH::Vec4(mat[1][0], mat[1][1], mat[1][2], mat[1][3]), 
		JPH::Vec4(mat[2][0], mat[2][1], mat[2][2], mat[2][3]), 
		JPH::Vec4(mat[3][0], mat[3][1], mat[3][2], mat[3][3]));
}

namespace SE
{

	void RigidBody::initialize(JPH::BodyInterface* bodyInterface)
	{
		m_bodyInterface = bodyInterface;
	}

	RigidBody::~RigidBody()
	{
		if (m_body)
		{
			m_bodyInterface->RemoveBody(getBodyID());
			m_bodyInterface->DestroyBody(getBodyID());
			m_body = nullptr;
		}
	}

	JPH::BodyID RigidBody::getBodyID() const
	{
		if (m_body)
			return m_body->GetID();
		else
			return JPH::BodyID();
	}

	bool RigidBody::isCreated() const {
		return m_body != nullptr;
	}

	void RigidBody::createBody() { 
		if (!m_shape || m_body)
			return;
		
		JPH::BodyCreationSettings bodySettings;

		bodySettings.SetShape(m_shape);
		bodySettings.mMotionType = m_motionType;
		if (m_motionType == JPH::EMotionType::Static)
			bodySettings.mObjectLayer = 0;
		else
			bodySettings.mObjectLayer = 1;
		bodySettings.mMotionQuality = JPH::EMotionQuality::Discrete;
		bodySettings.mFriction = m_friction;
		bodySettings.mRestitution = m_restitution;
		bodySettings.mMaxLinearVelocity = m_maxLinearVelocity;
		bodySettings.mMaxAngularVelocity = m_maxAngularVelocity;
		bodySettings.mGravityFactor = m_gravityFactor;
		bodySettings.mAllowDynamicOrKinematic = true;
		if (m_needsMassOverride)
		{
			bodySettings.mOverrideMassProperties = JPH::EOverrideMassProperties::MassAndInertiaProvided;
			bodySettings.mMassPropertiesOverride.mMass = 1.0f;
			bodySettings.mMassPropertiesOverride.mInertia = JPH::Mat44::sIdentity();
		}

		m_body = m_bodyInterface->CreateBody(bodySettings);
	}

	void RigidBody::setMotionType(JPH::EMotionType motionType) {
		m_motionType = motionType;
		if (m_body)
		{
			m_bodyInterface->SetMotionType(getBodyID(), motionType, JPH::EActivation::DontActivate);
			if (m_motionType == JPH::EMotionType::Static)
				m_bodyInterface->SetObjectLayer(getBodyID(), 0);
			else
				m_bodyInterface->SetObjectLayer(getBodyID(), 1);
		}
	}

	void RigidBody::setShape(JPH::Ref<JPH::Shape> shape, bool needsMassOverride)
	{
		if (shape == m_shape)
			return;

		m_shape = shape;
		bool massOverrideChanged = m_needsMassOverride != needsMassOverride;
		m_needsMassOverride = needsMassOverride;
		if (m_body)
		{
			if (massOverrideChanged)
			{
				m_bodyInterface->RemoveBody(getBodyID());
				m_bodyInterface->DestroyBody(getBodyID());
				m_body = nullptr;
			}
			else
			{
				m_bodyInterface->SetShape(getBodyID(), shape, true, JPH::EActivation::DontActivate);
			}
		}
	}

	glm::vec3 RigidBody::getLinearVelocity() const {
		if (m_body)
			return toGLM(m_bodyInterface->GetLinearVelocity(getBodyID()));
		else
			return glm::vec3(0.0f);
	}
	void RigidBody::setLinearVelocity(const glm::vec3& velocity) {
		if (m_body)
			m_bodyInterface->SetLinearVelocity(getBodyID(), toJPH(velocity));
	}

	glm::vec3 RigidBody::getAngularVelocity() const {
		if (m_body)
			return toGLM(m_bodyInterface->GetAngularVelocity(getBodyID()));
		else
			return glm::vec3(0.0f);
	}
	void RigidBody::setAngularVelocity(const glm::vec3& velocity) {
		if (m_body)
			m_bodyInterface->SetAngularVelocity(getBodyID(), toJPH(velocity));
	}

	glm::vec3 RigidBody::getPosition() const {
		if (m_body)
			return toGLM(m_bodyInterface->GetPosition(getBodyID()));
		else
			return glm::vec3(0.0f);
	}
	void RigidBody::setPosition(const glm::vec3& position) {
		if (m_body)
			m_bodyInterface->SetPosition(getBodyID(), toJPH(position), JPH::EActivation::DontActivate);
	}

	glm::quat RigidBody::getRotation() const {
		if (m_body)
			return toGLM(m_bodyInterface->GetRotation(getBodyID()));
		else
			return glm::quat();
	}
	void RigidBody::setRotation(const glm::quat& rotation) {
		if (m_body)
			m_bodyInterface->SetRotation(getBodyID(), toJPH(rotation), JPH::EActivation::DontActivate);
	}

	glm::mat4 RigidBody::getTransform() const {
		if (m_body)
			return toGLM(m_bodyInterface->GetWorldTransform(getBodyID()));
		else
			return glm::mat4(1.0f);
	}

	void RigidBody::addForce(const glm::vec3& force) {
		if (m_body)
			m_bodyInterface->AddForce(getBodyID(), toJPH(force));
	}

	void RigidBody::addTorque(const glm::vec3& torque) {
		if (m_body)
			m_bodyInterface->AddTorque(getBodyID(), toJPH(torque));
	}

	void RigidBody::addImpulse(const glm::vec3& impulse) {
		if (m_body)
			m_bodyInterface->AddImpulse(getBodyID(), toJPH(impulse));
	}

	void RigidBody::addAngularImpulse(const glm::vec3& impulse) {
		if (m_body)
			m_bodyInterface->AddAngularImpulse(getBodyID(), toJPH(impulse));
	}

	void RigidBody::activateBody() {
		m_isActive = true;
		if (m_body)
			m_bodyInterface->ActivateBody(getBodyID());
	}

	void RigidBody::deactivateBody() {
		m_isActive = false;
		if (m_body)
			m_bodyInterface->DeactivateBody(getBodyID());
	}

	bool RigidBody::isActive() const {
		if (m_body)
			return m_bodyInterface->IsActive(getBodyID());
		else
			return m_isActive;
	}

	void RigidBody::setMaxLinearVelocity(float maxVelocity)
	{
		m_maxLinearVelocity = maxVelocity;
		if (m_body)
			m_bodyInterface->SetMaxLinearVelocity(getBodyID(), maxVelocity);
	}

	void RigidBody::setMaxAngularVelocity(float maxVelocity)
	{
		m_maxAngularVelocity = maxVelocity;
		if (m_body)
			m_bodyInterface->SetMaxAngularVelocity(getBodyID(), maxVelocity);
	}

	void RigidBody::setRestitution(float restitution) {
		m_restitution = restitution;
		if (m_body)
			m_bodyInterface->SetRestitution(getBodyID(), restitution);
	}

	void RigidBody::setFriction(float friction) {
		m_friction = friction;
		if (m_body)
			m_bodyInterface->SetFriction(getBodyID(), friction);
	}

	void RigidBody::setGravityFactor(float factor) {
		m_gravityFactor = factor;
		if (m_body)
			m_bodyInterface->SetGravityFactor(getBodyID(), factor);
	}
}
