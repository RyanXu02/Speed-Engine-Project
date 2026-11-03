#pragma once
#include "Core/Features/Serializable.h"
#include "Core/Scene/Entity/Component/Transform.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace SE
{
    // Forward Declaration
    struct Particle;
    class ParticleContainer;
    class Texture;

    class ParticleEmitter
    {
    public:
        ParticleEmitter() = default;
		~ParticleEmitter() = default;

        void update(float dt);

        void SetFrame(int frameIndex, Particle* particle);
        void EmitParticle();
        void ForEachParticle(std::function<void(Particle& particle, float dt)> const& lambda, float dt);

        //@brief This exists because the container has know max particles to hold as well as if its a oneshot system.
        //@brief With our serialization, this gets called from the hook function when a PE is added.
        //@brief lowkenuinely horrible that its public so anyone could call it but idk how else to solve this
		void finalize();
		void acquireTransform(const Transform* tr) { myTransform = tr; }

        glm::vec2 GetAcceleration() const { return mAcceleration; }
        void SetAcceleration(glm::vec2 v) { mAcceleration = v; }

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
        glm::vec2 GetEmitterScale() const { return mEmitterScale; }
        void SetEmitterScale(glm::vec2 v) { mEmitterScale = v; }
        glm::vec2 GetScaleMin() const { return mScaleMin; }
        void SetScaleMin(glm::vec2 v) { mScaleMin = v; }
        glm::vec2 GetScaleMax() const { return mScaleMax; }
        void SetScaleMax(glm::vec2 v) { mScaleMax = v; }
        glm::vec2 GetDirectionRange() const { return mDirectionRange; }
        void SetDirectionRange(glm::vec2 v) { mDirectionRange = v; }
        float GetEmitRate() const { return mEmitRate; }
        void SetEmitRate(float v) { mEmitRate = v; }
        unsigned GetEmitMax() const { return mEmitMax; }
        void SetEmitMax(unsigned v) { mEmitMax = v; }
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
        void SetOneShot(bool v) { mOneShot = v; }
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
            member("directionRange", &ParticleEmitter::mDirectionRange),
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
            member("oneShot", &ParticleEmitter::mOneShot),
            member("isRunning", &ParticleEmitter::mIsRunning));
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
		//for choosing random velocity direction. ex:(0,360) = emitted in any 2d direction from point of emission
        glm::vec3 mDirectionRange{0.0f, 6.28f};

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
        const Transform* myTransform;
		bool mIsDone{ false };

        std::unique_ptr<ParticleContainer> mContainer{};
    };
}  // namespace SE