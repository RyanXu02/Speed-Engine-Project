#include "pch.h"
#include "ParticleEmitter.h"
#include "Core/Scene/Entity/Component/ComponentStructs.h"
#include "Core/Features/RNG.h"
#include "Core/Features/Math.h"

namespace SE
{
    void ParticleEmitter::update(float dt)
    {
        // the added-hook only runs for deserialized components, so make sure the container exists
        finalize();

        // lambda function to update a particle
        auto lambda = [this](Particle& particle, float dt)
        {
            if (particle.lifetime > 0)
            {
                particle.lifetime -= dt;

                if (particle.lifetime > 0)
                {
                    // Update velocity and rotation
                    particle.velocity += particle.acceleration * dt;
                    particle.position += particle.velocity * dt;
                    particle.rotation += particle.rotationSpeed * dt;
                }
                else
                {
                    mContainer->KillParticle(particle);
                }
            }
        };

        // pass lambda function to container update
        mContainer->Update(lambda, dt);

        mEmitAccumulator += mEmitRate * dt;
        while (mEmitAccumulator > 0.0f)
        {
            if (!mContainer->IsFull())
            {
                EmitParticle();
            }

            mEmitAccumulator -= 1.0f;
        }

        //if there are 0 active particles and 0 free particles then kill parent entity. this will be exclusively true for oneshot emitters
        if (mContainer->IsEmpty() && mContainer->IsFull())
        {
            //replace below line with a bool + getter so scene::update can access it and remove the associated entity
            //GetGameObject()->Destroy();
			mIsDone = true;
        }
    }

    void ParticleEmitter::SetFrame(int frameIndex, Particle* particle)
    {
        // Calculate texture offset
        particle->uvOffset = {
            (1.0f / static_cast<float>(mNumCols)) * static_cast<float>(frameIndex % mNumCols),
            (1.0f / static_cast<float>(mNumRows)) * static_cast<int>(frameIndex / mNumCols)};
    }

    void ParticleEmitter::EmitParticle()
    {
        if (!mIsRunning)
            return;
        Particle* particle = mContainer->AllocateParticle();
        if (particle)
        {
            if (mFrameIndex >= 0 && mFrameIndex < mNumRows * mNumCols)
            {
                SetFrame(mFrameIndex, particle);
            }
            else
            {
                SetFrame(RNG::Range(0, mNumRows * mNumCols - 1), particle);    
            }

            // Set random lifetime between min and max
            particle->lifetime = RNG::Range(mLifetimeMin, mLifetimeMax);

            // Set position of particle randomly within the bounds of the emitter (Point, Line,
            // Square all work here)
            //Transform* transform = GetGameObject()->GetComponent<Transform>();
            glm::vec3 translation = myTransform->translate;
            float xMin = translation.x - (mEmitterScale.x / 2);
            float xMax = translation.x + (mEmitterScale.x / 2);
            float yMin = translation.y - (mEmitterScale.y / 2);
            float yMax = translation.y + (mEmitterScale.y / 2);
			float zMin = translation.z - (mEmitterScale.z / 2);
			float zMax = translation.z + (mEmitterScale.z / 2);
            particle->position = glm::vec3(RNG::Range(xMin, xMax), RNG::Range(yMin, yMax), RNG::Range(zMin, zMax));

            // Set random particle scale between min and max. Uniform scaling
			float t = RNG::Range(0.f, 1.f);
			particle->scale = glm::mix(mScaleMin, mScaleMax, t);

            // Set random starting rotation of particle
			particle->rotation = RNG::Range(mRotationMin, mRotationMax);
            
            // Set random rotation speed between min and max, also randomize rotation direction (+ or -)
			float rotationSpeed = RNG::Range(mRotationSpeedMin, mRotationSpeedMax);
			if (RNG::Range(0, 1) == 1)
                particle->rotationSpeed = rotationSpeed;
            else
                particle->rotationSpeed = -rotationSpeed;

            //==========VELOCITY DIRECTION CALCULATION==========================
            // Set velocity direction, if direction < 0, set to random direction
			float minA = glm::radians(mConeAngleRange.x);
			float maxA = glm::radians(mConeAngleRange.y);

			// Sample cos(theta) so the distribution is uniform over the sphere surface
			float cosTheta = RNG::Range(glm::cos(maxA), glm::cos(minA));
			float sinTheta = glm::sqrt(glm::max(0.f, 1.f - cosTheta * cosTheta));
			float phi = glm::radians(RNG::Range(mAzimuthRange.x, mAzimuthRange.y));

			// Direction in a local frame where the axis is +Z
			glm::vec3 local(sinTheta * glm::cos(phi), sinTheta * glm::sin(phi), cosTheta);

			// Build an orthonormal basis around the emission axis
			glm::vec3 axis = glm::normalize(mDirectionAxis);
			glm::vec3 up = (glm::abs(axis.y) < 0.99f) ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
			glm::vec3 right = glm::normalize(glm::cross(up, axis));
			glm::vec3 fwd = glm::cross(axis, right);

			glm::vec3 dir = right * local.x + fwd * local.y + axis * local.z;
            //===========END VELOCITY DIRECTION CALCULATION===================
            // Set random speed between min and max
			float speed = RNG::Range(mSpeedMin, mSpeedMax);
            dir *= speed;
            particle->velocity = dir;
            
            // Set particle initial acceleration
            particle->acceleration = mAcceleration;
        }
    }

    void ParticleEmitter::ForEachParticle(std::function<void(Particle& particle, float dt)> const& lambda, float dt)
    {
        finalize();
        mContainer->Update(lambda, dt);
    }

    void ParticleEmitter::finalize() {
        //holy jank lowkey. when PE constructed, mContainer should be null so this func will only do anything in that case.
        //if its non-null return out immediately
		if (mContainer != nullptr)
			return;

		mContainer = std::make_unique<ParticleContainer>(mEmitMax, mOneShot);
    }

}  // namespace SE