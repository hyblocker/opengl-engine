#include "scene_updater.hpp"
#include "engine/log.hpp"
#include "engine/physics/physics_components.hpp"
#include "particle_system.hpp"

#include "b2debug/debug_draw.hpp"

namespace render {

    void SceneUpdater::init() {
#if _DEBUG
        g_draw.Create();
#endif
    }

    void SceneUpdater::shutdown() {
#if _DEBUG
        g_draw.Destroy();
#endif
    }

    void SceneUpdater::start(const std::shared_ptr<Entity> entity) {
        if (entity->enabled) {
            for (const std::shared_ptr<IComponent> component : entity->components) {
                if (component->getComponentType() == render::ComponentType::UserBehaviour) {
                    IBehaviour* pBehaviour = (IBehaviour*)component.get();
                    if (pBehaviour->enabled) {
                        pBehaviour->start();
                    }
                }
            }

            // Entity didn't have any components we wanted attached to itself, check children
            for (const std::shared_ptr<Entity> childEntity : entity->children) {
                // may return null, if not null its what we're after anyway
                if (childEntity->enabled) {
                    start(childEntity);
                }
            }
        }
    }

    void SceneUpdater::sleep(const std::shared_ptr<Entity> entity) {
        if (entity->enabled) {
            for (const std::shared_ptr<IComponent> component : entity->components) {
                if (component->getComponentType() == render::ComponentType::UserBehaviour) {
                    IBehaviour* pBehaviour = (IBehaviour*)component.get();
                    if (pBehaviour->enabled) {
                        pBehaviour->sleep();
                    }
                }
            }

            // Entity didn't have any components we wanted attached to itself, check children
            for (const std::shared_ptr<Entity> childEntity : entity->children) {
                // may return null, if not null its what we're after anyway
                if (childEntity->enabled) {
                    sleep(childEntity);
                }
            }
        }
    }

    void SceneUpdater::render(const std::shared_ptr<Entity> entity) {
        if (entity->enabled) {
            for (const std::shared_ptr<IComponent> component : entity->components) {
                if (component->getComponentType() == render::ComponentType::UserBehaviour) {
                    IBehaviour* pBehaviour = (IBehaviour*)component.get();
                    if (pBehaviour->enabled) {
                        pBehaviour->render();
                    }
                }
            }

            // Entity didn't have any components we wanted attached to itself, check children
            for (const std::shared_ptr<Entity> childEntity : entity->children) {
                // may return null, if not null its what we're after anyway
                if (childEntity->enabled) {
                    render(childEntity);
                }
            }
        }
    }
    
    void SceneUpdater::imgui(const std::shared_ptr<Entity> entity) {
        if (entity->enabled) {
            for (const std::shared_ptr<IComponent> component : entity->components) {
                if (component->getComponentType() == render::ComponentType::UserBehaviour) {
                    IBehaviour* pBehaviour = (IBehaviour*)component.get();
                    if (pBehaviour->enabled) {
                        pBehaviour->imgui();
                    }
                }
            }

            // Entity didn't have any components we wanted attached to itself, check children
            for (const std::shared_ptr<Entity> childEntity : entity->children) {
                // may return null, if not null its what we're after anyway
                if (childEntity->enabled) {
                    imgui(childEntity);
                }
            }
        }
    }

    void SceneUpdater::update(const std::shared_ptr<Entity> entity, const float deltaTime) {
        if (entity->enabled) {
            for (const std::shared_ptr<IComponent> component : entity->components) {
                if (component->getComponentType() == render::ComponentType::UserBehaviour) {
                    IBehaviour* pBehaviour = (IBehaviour*)component.get();
                    if (pBehaviour->enabled) {
                        if (pBehaviour->m_lastFrameEnabled != pBehaviour->enabled || entity->_lastFrameEnabled != entity->enabled) {
                            pBehaviour->start();
                        }
                        pBehaviour->update(deltaTime);
                        pBehaviour->m_lastFrameEnabled = pBehaviour->enabled;
                    }
                }
                if (component->getComponentType() == render::ComponentType::ParticleSystem) {
                    ParticleSystem* pParticleSystem = (ParticleSystem*)component.get();
                    if (pParticleSystem->enabled) {
                        pParticleSystem->update(deltaTime);
                    }
                }
            }

            entity->_lastFrameEnabled = entity->enabled;

            // Entity didn't have any components we wanted attached to itself, check children
            for (const std::shared_ptr<Entity> childEntity : entity->children) {
                // may return null, if not null its what we're after anyway
                if (childEntity->enabled) {
                    update(childEntity, deltaTime);
                }
            }
        }
    }

    void SceneUpdater::physicsTick(const std::shared_ptr<Entity> entity, const float deltaTime, Scene::PhysicsParameters physicsParams) {
        if (entity->enabled) {
            for (const std::shared_ptr<IComponent> component : entity->components) {
                if (component->getComponentType() == render::ComponentType::Physics) {
                    physics::PhysicsComponent* pPhysicsComponent = (physics::PhysicsComponent*)component.get();
                    if (pPhysicsComponent->enabled) {
                        if (B2_ID_EQUALS(pPhysicsComponent->m_physicsId, b2_nullBodyId)) {
                            // if internal physics stuff isn't setup, create it
                            
                            b2BodyDef bodyDef = b2DefaultBodyDef();
                            b2ShapeDef shapeDef = b2DefaultShapeDef();
                            shapeDef.friction = pPhysicsComponent->friction;
                            shapeDef.density = pPhysicsComponent->density;
                            shapeDef.restitution = pPhysicsComponent->bounciness;
                            switch (pPhysicsComponent->bodyType) {
                                case physics::PhysicsBodyType::Static:
                                {
                                    bodyDef.type = b2_staticBody;
                                    break;
                                }
                                case physics::PhysicsBodyType::Kinematic:
                                {
                                    bodyDef.type = b2_kinematicBody;
                                    break;
                                }
                                case physics::PhysicsBodyType::Rigidbody:
                                {
                                    bodyDef.type = b2_dynamicBody;
                                    break;
                                }
                            }
                            bodyDef.position = { entity->transform.getPosition().x * physics::k_PHYSICS_SCALE, entity->transform.getPosition().y * physics::k_PHYSICS_SCALE };
                            bodyDef.gravityScale = pPhysicsComponent->gravityScale;
                            bodyDef.fixedRotation = pPhysicsComponent->fixedRotation;
                            bodyDef.userData = pPhysicsComponent;
                            // @TODO: Decompose quat to euler
                            //        need helper func
                            pPhysicsComponent->m_physicsId = b2CreateBody(physicsParams.m_box2Dworld, &bodyDef);

                            // define the shape of the collider
                            switch (pPhysicsComponent->shape.shape) {
                            case physics::PhysicsShape::Box: {

                                b2Polygon boxCollider = b2MakeBox(pPhysicsComponent->shape.box.size.x, pPhysicsComponent->shape.box.size.y);
                                b2CreatePolygonShape(pPhysicsComponent->m_physicsId, &shapeDef, &boxCollider);
                                break;
                            }
                            case physics::PhysicsShape::Circle: {
                                b2Circle circleCollider = {
                                    .center = { -pPhysicsComponent->shape.circle.radius * physics::k_PHYSICS_SCALE / 2.0f, -pPhysicsComponent->shape.circle.radius * physics::k_PHYSICS_SCALE / 2.0f },
                                    .radius = pPhysicsComponent->shape.circle.radius * physics::k_PHYSICS_SCALE
                                };
                                b2CreateCircleShape(pPhysicsComponent->m_physicsId, &shapeDef, &circleCollider);

                                break;
                            }
                            case physics::PhysicsShape::Capsule: {
                                b2Capsule capsuleCollider = {
                                    .center1 = {pPhysicsComponent->shape.capsule.p1.x * physics::k_PHYSICS_SCALE, pPhysicsComponent->shape.capsule.p1.y * physics::k_PHYSICS_SCALE },
                                    .center2 = {pPhysicsComponent->shape.capsule.p2.x * physics::k_PHYSICS_SCALE, pPhysicsComponent->shape.capsule.p2.y * physics::k_PHYSICS_SCALE },
                                    .radius = pPhysicsComponent->shape.capsule.radius
                                };
                                b2CreateCapsuleShape(pPhysicsComponent->m_physicsId, &shapeDef, &capsuleCollider);
                                break;
                            }
                            }

                            b2Body_SetLinearDamping(pPhysicsComponent->m_physicsId, 1.0f);
                        }

                        // simulate current state
                        for (int i = 0; i < pPhysicsComponent->m_forceQueueSize; i++) {
                            if (pPhysicsComponent->bodyType == physics::PhysicsBodyType::Rigidbody) {
                                b2Body_ApplyLinearImpulseToCenter(pPhysicsComponent->m_physicsId, { pPhysicsComponent->m_pendingForces[i].x ,pPhysicsComponent->m_pendingForces[i].y }, true);
                            } else if (pPhysicsComponent->bodyType == physics::PhysicsBodyType::Kinematic) {
                                b2Body_SetLinearVelocity(pPhysicsComponent->m_physicsId, { pPhysicsComponent->m_pendingForces[i].x ,pPhysicsComponent->m_pendingForces[i].y });
                            }
                        }
                        // clear queue
                        pPhysicsComponent->m_forceQueueSize = 0;
                    } else {
                        // Physics should pause if the component is disabled
                        b2Body_SetAwake(pPhysicsComponent->m_physicsId, false);
                    }
                }
            }

            // Entity didn't have any components we wanted attached to itself, check children
            for (const std::shared_ptr<Entity> childEntity : entity->children) {
                // may return null, if not null its what we're after anyway
                if (childEntity->enabled) {
                    physicsTick(childEntity, deltaTime, physicsParams);
                }
            }
        }
    }

    void SceneUpdater::physicsTickPost(const std::shared_ptr<Entity> entity, const float deltaTime, Scene::PhysicsParameters physicsParams) {
        if (entity->enabled) {
            for (const std::shared_ptr<IComponent> component : entity->components) {
                if (component->getComponentType() == render::ComponentType::Physics) {
                    physics::PhysicsComponent* pPhysicsComponent = (physics::PhysicsComponent*)component.get();
                    if (pPhysicsComponent->enabled) {
                        
                        b2Vec2 newPos = b2Body_GetPosition(pPhysicsComponent->m_physicsId);
                        float newRot = b2Rot_GetAngle(b2Body_GetRotation(pPhysicsComponent->m_physicsId));

                        hlslpp::float3 oldPos = entity->transform.getPosition();
                        entity->transform.setPosition({ newPos.x, newPos.y, oldPos.z });
                        entity->transform.setRotation(hlslpp::quaternion::rotation_euler_zxy({ 0, newRot, 0 }));
                    }
                }
            }

            // Entity didn't have any components we wanted attached to itself, check children
            for (const std::shared_ptr<Entity> childEntity : entity->children) {
                // may return null, if not null its what we're after anyway
                if (childEntity->enabled) {
                    physicsTickPost(childEntity, deltaTime, physicsParams);
                }
            }
        }
    }

    void SceneUpdater::start(const Scene& scene) {

        // We could do a more complex scene graph to optimise searching for entities but it doesn't harm performance enough to matter

        for (const std::shared_ptr<Entity> entity : scene.root.children) {
            start(entity);
        }
    }

    void SceneUpdater::sleep(const Scene& scene) {

        // We could do a more complex scene graph to optimise searching for entities but it doesn't harm performance enough to matter

        for (const std::shared_ptr<Entity> entity : scene.root.children) {
            sleep(entity);
        }

        // b2DestroyWorld(scene.physicsParams.m_box2Dworld);
    }

    void SceneUpdater::render(const Scene& scene) {

        // We could do a more complex scene graph to optimise searching for entities but it doesn't harm performance enough to matter

        for (const std::shared_ptr<Entity> entity : scene.root.children) {
            render(entity);
        }
    }
    

    void SceneUpdater::imgui(const Scene& scene) {

        // We could do a more complex scene graph to optimise searching for entities but it doesn't harm performance enough to matter

        for (const std::shared_ptr<Entity> entity : scene.root.children) {
            imgui(entity);
        }
    }

    void SceneUpdater::update(Scene& scene, const float deltaTime) {

        // We could do a more complex scene graph to optimise searching for entities but it doesn't harm performance enough to matter

        for (const std::shared_ptr<Entity> entity : scene.root.children) {
            update(entity, deltaTime);
        }

        /*
        if (!scene.physicsParams.m_initialised) {
            b2WorldDef worldDef = b2DefaultWorldDef();
            worldDef.gravity = { scene.physicsParams.gravity.x, scene.physicsParams.gravity.y };
            scene.physicsParams.m_box2Dworld = b2CreateWorld(&worldDef);
            scene.physicsParams.m_initialised = true;
        }

        // Do a step of the physics sim after
        for (const std::shared_ptr<Entity> entity : scene.root.children) {
            physicsTick(entity, deltaTime, scene.physicsParams);
        }

        // actually simulate physics
        b2World_Step(scene.physicsParams.m_box2Dworld, deltaTime, physics::k_PHYSICS_SUBSTEP_COUNT);

        // update the entity's position in the world after sim
        for (const std::shared_ptr<Entity> entity : scene.root.children) {
            physicsTickPost(entity, deltaTime, scene.physicsParams);
        }
        */
    }
}