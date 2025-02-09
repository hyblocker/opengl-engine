#include "scene_updater.hpp"
#include "engine/log.hpp"
#include "engine/physics/physics_components.hpp"

namespace render {

    void SceneUpdater::init() {}

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

    void SceneUpdater::update(const std::shared_ptr<Entity> entity, const float deltaTime) {
        if (entity->enabled) {
            for (const std::shared_ptr<IComponent> component : entity->components) {
                if (component->getComponentType() == render::ComponentType::UserBehaviour) {
                    IBehaviour* pBehaviour = (IBehaviour*)component.get();
                    if (pBehaviour->enabled) {
                        pBehaviour->update(deltaTime);
                    }
                }
            }

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
                            bodyDef.position = { entity->transform.getPosition().x, entity->transform.getPosition().y };
                            // @TODO: Decompose quat to euler
                            //        need helper func
                            pPhysicsComponent->m_physicsId = b2CreateBody(physicsParams.m_box2Dworld, &bodyDef);

                            // define the shape of the collider
                            switch (pPhysicsComponent->shape.shape) {
                            case physics::PhysicsShape::Box: {

                                b2Polygon boxCollider = b2MakeBox(50.0f, 10.0f);
                                b2ShapeDef shapeDef = b2DefaultShapeDef();
                                b2CreatePolygonShape(pPhysicsComponent->m_physicsId, &shapeDef, &boxCollider);
                                break;
                            }
                            case physics::PhysicsShape::Circle: {
                                b2Circle circleCollider = {
                                    .center = { pPhysicsComponent->shape.circle.centre.x, pPhysicsComponent->shape.circle.centre.y },
                                    .radius = pPhysicsComponent->shape.circle.radius
                                };
                                b2CreateCircleShape(pPhysicsComponent->m_physicsId, &shapeDef, &circleCollider);

                                break;
                            }
                            case physics::PhysicsShape::Capsule: {
                                b2Capsule capsuleCollider = {
                                    .center1 = {pPhysicsComponent->shape.capsule.p1.x, pPhysicsComponent->shape.capsule.p1.y},
                                    .center2 = {pPhysicsComponent->shape.capsule.p2.x, pPhysicsComponent->shape.capsule.p2.y},
                                    .radius = pPhysicsComponent->shape.capsule.radius
                                };
                                b2ShapeDef shapeDef = b2DefaultShapeDef();
                                b2CreateCapsuleShape(pPhysicsComponent->m_physicsId, &shapeDef, &capsuleCollider);
                                break;
                            }
                            }


                        }

                        // simulate current state
                        b2Body_SetAwake(pPhysicsComponent->m_physicsId, true);
                        for (int i = 0; i < pPhysicsComponent->m_forceQueueSize; i++) {
                            b2Body_ApplyForceToCenter(pPhysicsComponent->m_physicsId, { pPhysicsComponent->m_pendingForces[i].x ,pPhysicsComponent->m_pendingForces[i].y }, true);
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

        // @TODO: We could do a more complex scene graph to optimise searching for entities but it doesn't harm performance enough to matter

        for (const std::shared_ptr<Entity> entity : scene.root.children) {
            start(entity);
        }
    }

    void SceneUpdater::sleep(const Scene& scene) {

        // @TODO: We could do a more complex scene graph to optimise searching for entities but it doesn't harm performance enough to matter

        for (const std::shared_ptr<Entity> entity : scene.root.children) {
            sleep(entity);
        }

        b2DestroyWorld(scene.physicsParams.m_box2Dworld);
    }

    void SceneUpdater::render(const Scene& scene) {

        // @TODO: We could do a more complex scene graph to optimise searching for entities but it doesn't harm performance enough to matter

        for (const std::shared_ptr<Entity> entity : scene.root.children) {
            render(entity);
        }
    }

    void SceneUpdater::update(Scene& scene, const float deltaTime) {

        // @TODO: We could do a more complex scene graph to optimise searching for entities but it doesn't harm performance enough to matter

        for (const std::shared_ptr<Entity> entity : scene.root.children) {
            update(entity, deltaTime);
        }

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
    }
}