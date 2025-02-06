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
                        // pPhysicsComponent->update(deltaTime);
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
        // Do a step of the physics sim after
        for (const std::shared_ptr<Entity> entity : scene.root.children) {
            physicsTick(entity, deltaTime, scene.physicsParams);
        }
    }
}