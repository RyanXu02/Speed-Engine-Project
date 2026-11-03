#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace SE
{
    struct Particle
    {
        // Transform variables
        glm::vec3 position{0.0f, 0.0f, 0.f};
        glm::vec3 scale{0.0f, 0.0f, 0.f};
        float rotation{0.0f};

        // Physics variables
        glm::vec3 velocity{0.0f, 0.0f, 0.f};
        glm::vec3 acceleration{0.0f, 0.0f, 0.f};
        float rotationSpeed{0.0f};

        // Particle specific variables
        float lifetime{0.0f};
        glm::vec2 uvOffset{0.0f, 0.0f};
    };


    class ParticleContainer
    {
       public:
        ParticleContainer(unsigned _maxParticles, bool _oneShot);

        void Update(std::function<void(Particle& particle, float dt)> const& lambda, float dt);

        //do we have no active particles?
        bool IsEmpty() const;

        //do we have no free particles?
        bool IsFull() const;

        Particle* AllocateParticle();

        void KillParticle(Particle& particle);

       private:
        std::vector<Particle> mParticles;

        unsigned mNumActiveParticles{0};
        unsigned mNumFreeParticles{0};

        bool mOneShot{false};
    };

}  // namespace SE