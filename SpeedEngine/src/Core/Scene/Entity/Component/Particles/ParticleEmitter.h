#pragma once
#include "Core/Features/Serializable.h"
#include "Core/Features/MathToJSONBindings.h"
#include "ParticleContainer.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace SE
{
    // Forward Declaration
    struct Transform;

    class ParticleEmitter
    {
    public:
		ParticleEmitter() = default;

		void update(double dt);

        void SetFrame(int frameIndex, Particle* particle);
        void EmitParticle();
        void ForEachParticle(std::function<void(Particle& particle, double dt)> const& lambda, double dt);

        //@brief This exists because the container has know max particles to hold as well as if its a oneshot system.
        //@brief With our serialization, this gets called from the hook function when a PE is added.
        //@brief lowkenuinely horrible that its public so anyone could call it but idk how else to solve this
		void finalize();
		void acquireTransform(const Transform* tr) { myTransform = tr; }

        glm::vec3 GetAcceleration() const { return mAcceleration; }
        void SetAcceleration(glm::vec3 v) { mAcceleration = v; }

        void ToggleEmitter() { mIsRunning ^= true; }
        void StartEmitter() { mIsRunning = true; }
        void StopEmitter() { mIsRunning = false; }

        /*Texture* GetTexture() const { return mTexture; }
        void SetTexture(Texture* v) { mTexture = v; }
        Shader* GetShader() const { return mShader; }
        void SetShader(Shader* v) { mShader = v; }*/
        float GetAlpha() const { return mAlpha; }
        void SetAlpha(float v) { mAlpha = v; }
        glm::vec3 GetTintColor() const { return mTintColor; }
        void SetTintColor(glm::vec3 v) { mTintColor = v; }
        int GetNumRows() const { return mNumRows; }
        void SetNumRows(int v) { mNumRows = v; }
        int GetNumCols() const { return mNumCols; }
        void SetNumCols(int v) { mNumCols = v; }
        glm::vec3 GetEmitterScale() const { return mEmitterScale; }
        void SetEmitterScale(glm::vec3 v) { mEmitterScale = v; }
        glm::vec3 GetScaleMin() const { return mScaleMin; }
        void SetScaleMin(glm::vec3 v) { mScaleMin = v; }
        glm::vec3 GetScaleMax() const { return mScaleMax; }
        void SetScaleMax(glm::vec3 v) { mScaleMax = v; }
        glm::vec3 GetDirectionAxis() const { return mDirectionAxis; }
        void SetDirectionAxis(glm::vec3 v) { mDirectionAxis = v; }
		glm::vec2 GetConeAngleRange() const { return mConeAngleRange; }
		void SetConeAngleRange(glm::vec2 v) { mConeAngleRange = v; }
		glm::vec2 GetAzimuthRange() const { return mAzimuthRange; }
		void SetAzimuthRange(glm::vec2 v) { mAzimuthRange = v; }
        float GetEmitRate() const { return mEmitRate; }
        void SetEmitRate(float v) { mEmitRate = v; }
        unsigned GetEmitMax() const { return mEmitMax; }
        void SetEmitMax(unsigned v)
        {
            mEmitMax = v;
            mContainer.reset();  // update() recreates it with the new size via finalize()
        }
        float GetLifetimeMin() const { return mLifetimeMin; }
        void SetLifetimeMin(float v) { mLifetimeMin = v; }
        float GetLifetimeMax() const { return mLifetimeMax; }
        void SetLifetimeMax(float v) { mLifetimeMax = v; }
        float GetSpeedMin() const { return mSpeedMin; }
        void SetSpeedMin(float v) { mSpeedMin = v; }
        float GetSpeedMax() const { return mSpeedMax; }
        void SetSpeedMax(float v) { mSpeedMax = v; }
        float GetRotationSpeedMin() const { return mRotationSpeedMin; }
        void SetRotationSpeedMin(float v) { mRotationSpeedMin = v; }
        float GetRotationSpeedMax() const { return mRotationSpeedMax; }
        void SetRotationSpeedMax(float v) { mRotationSpeedMax = v; }
        float GetRotationMin() const { return mRotationMin; }
        void SetRotationMin(float v) { mRotationMin = v; }
        float GetRotationMax() const { return mRotationMax; }
        void SetRotationMax(float v) { mRotationMax = v; }
        bool GetOneShot() const { return mOneShot; }
        void SetOneShot(bool v)
		{
            mOneShot = v;
            mContainer.reset();  // the container bakes in the one-shot flag
        }
        bool GetIsRunning() const { return mIsRunning; }

        bool AmIDone() const { return mIsDone; }

        DEFINE_TYPE(ParticleEmitter, 
            member("tintColor", &ParticleEmitter::mTintColor),
            member("alpha", &ParticleEmitter::mAlpha),
            //member("texture", &ParticleEmitter::),
            //member("shader", &ParticleEmitter::mTintColor),       //probably dont need shader, as all billboard particles will use same billboard shader. idk maybe lol
	        member("frameIndex", &ParticleEmitter::mFrameIndex), 
            member("sheetRows", &ParticleEmitter::mNumRows), 
            member("sheetColumns", &ParticleEmitter::mNumCols),
            member("scale", &ParticleEmitter::mEmitterScale),
            member("particleScaleMin", &ParticleEmitter::mScaleMin),
            member("particleScaleMax", &ParticleEmitter::mScaleMax),
            member("acceleration", &ParticleEmitter::mAcceleration),
            member("directionAxis", &ParticleEmitter::mDirectionAxis),
            member("coneAngleRange", &ParticleEmitter::mConeAngleRange),
            member("azimuthRange", &ParticleEmitter::mAzimuthRange),
            member("emitRate", &ParticleEmitter::mEmitRate),
            member("emitMax", &ParticleEmitter::mEmitMax),
            member("lifetimeMin", &ParticleEmitter::mLifetimeMin),
            member("lifetimeMax", &ParticleEmitter::mLifetimeMax),
            member("speedMin", &ParticleEmitter::mSpeedMin),
            member("speedMax", &ParticleEmitter::mSpeedMax),
            member("rotationSpeedMin", &ParticleEmitter::mRotationSpeedMin),
            member("rotationSpeedMax", &ParticleEmitter::mRotationSpeedMax),
            member("rotationMin", &ParticleEmitter::mRotationMin),
            member("rotationMax", &ParticleEmitter::mRotationMax),
            member("oneShot", &ParticleEmitter::mOneShot));
            //could also serialize mIsRunning but this seems to be for internal use as Stop/StartEmitter change its value.
    private:
        //Texture* mTexture{nullptr};
        //Shader* mShader{nullptr};
        float mAlpha{1.0f};
        glm::vec3 mTintColor{1.0f, 1.0f, 1.0f};
        int mNumRows = 1;
        int mNumCols = 1;
        int mFrameIndex = 0;

        glm::vec3 mEmitterScale{0.0f, 0.0f, 0.f};
        glm::vec3 mScaleMin{1.0f, 1.0f, 1.f};
        glm::vec3 mScaleMax{1.0f, 1.0f, 1.f};
        glm::vec3 mAcceleration{0.0f, 0.0f, 0.f};
		
		glm::vec3 mDirectionAxis{ 0.f, 1.f, 0.f }; // emission axis (cone center)
		glm::vec2 mConeAngleRange{ 0.f, 180.f }; // min/max angle away from the axis, degrees
		glm::vec2 mAzimuthRange{ 0.f, 360.f }; // optional: spin around the axis, degrees

        float mEmitAccumulator{0.0f};
        float mEmitRate{1.0f};
        unsigned mEmitMax{1000}; //must be able to emit @ least 1000 particles at once with no fps drop
        float mLifetimeMin{1.0f};
        float mLifetimeMax{1.0f};
        float mSpeedMin{1.0f};
        float mSpeedMax{1.0f};
        float mRotationSpeedMin{0.0f};
        float mRotationSpeedMax{0.0f};
        float mRotationMin{0.0f};
        float mRotationMax{0.0f};
        bool mOneShot{false}; //whether particle system loops (e.g. campfire sparks: false)
        bool mIsRunning{true};

        //replaces GetGameObj()->getComponent<Transform> from mitosis engine code in conjunction with SE::ParticleEmitter::acquireTransform
		const Transform* myTransform{ nullptr };
		bool mIsDone{ false };

        // component storage needs copy-constructible types. this is a unique_ptr that is copy-constructible
        // but copies are empty. No need to copy over the particles themselves.
        // Doing this also means we dont have to write the big 4 for the ParticleEmitter, which has like 20+ goddam fields
        struct ContainerPtr : std::unique_ptr<ParticleContainer>
        {
            using std::unique_ptr<ParticleContainer>::unique_ptr;
            using std::unique_ptr<ParticleContainer>::operator=;

            ContainerPtr() = default;
            ContainerPtr(const ContainerPtr&) {}
            ContainerPtr& operator=(const ContainerPtr& other)
            {
                if (this != &other)
                    reset();
                return *this;
            }
            ContainerPtr(ContainerPtr&&) = default;
            ContainerPtr& operator=(ContainerPtr&&) = default;
        };

        ContainerPtr mContainer{};
    };
}  // namespace SE