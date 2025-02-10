#include "physics_components.hpp"
#include <box2d/box2d.h>

namespace physics {
    void PhysicsComponent::addForce(const hlslpp::float2 force) {
        if (B2_ID_EQUALS(m_physicsId, b2_nullBodyId)) {
            // object wasn't added to the physics sim, queue it's force creation into a buffer and let the scene updater handle it
            if (m_forceQueueSize < k_MAX_FORCES_PER_COMPONENT_PER_FRAME) {
                m_pendingForces[m_forceQueueSize] = force;
                m_forceQueueSize++;
            }
        } else {
            if (bodyType == PhysicsBodyType::Rigidbody) {
                b2Body_ApplyLinearImpulseToCenter(m_physicsId, { force.x ,force.y }, true);
            }
            else if (bodyType == PhysicsBodyType::Kinematic) {
                b2Body_SetLinearVelocity(m_physicsId, { force.x ,force.y } );
            }
        }
    }
}