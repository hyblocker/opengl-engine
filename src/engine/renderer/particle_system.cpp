#include "particle_system.hpp"
#include "engine/app.hpp"

namespace render {
    void ParticleSystem::start() {}

    void ParticleSystem::update(float deltaTime) {
        for (auto& particle : m_particlePool) {
            if (!particle.alive) {
                continue;
            }

            if (particle.lifeRemaining <= 0.0f) {
                particle.alive = false;
                m_particleCount--;
                continue;
            }

            particle.lifeRemaining -= deltaTime;
            particle.position += particle.velocity * deltaTime;
        }
    }

    void ParticleSystem::clear() {
        m_poolIndex = m_particlePool.size() - 1;
        for (auto& particle : m_particlePool) {
            particle.alive = false;
            m_particleCount = 0;
        }
    }

    void ParticleSystem::emit(ParticleParams params) {
        
        ParticleInstance& particle = m_particlePool[m_poolIndex];
        if (particle.alive == false) {
            m_particleCount++;
        }
        particle.alive = true;
        particle.position = params.position;
        particle.velocity = params.velocity;
        particle.velocity.x += params.velocityVariation.x * (engine::RandomNumberGenerator::getFloat() - 0.5f);
        particle.velocity.y += params.velocityVariation.y * (engine::RandomNumberGenerator::getFloat() - 0.5f);

        particle.colourBegin = params.colourBegin;
        particle.colourEnd = params.colourEnd;

        particle.lifeTime = params.lifeTime;
        particle.lifeRemaining = params.lifeTime;
        particle.sizeBegin = params.sizeBegin + params.sizeVariation * (engine::RandomNumberGenerator::getFloat() - 0.5f);
        particle.sizeEnd = params.sizeEnd;

        m_poolIndex = (m_poolIndex - 1) % m_particlePool.size();
    }
}