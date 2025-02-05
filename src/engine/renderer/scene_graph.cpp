#include "scene_graph.hpp"
#include "scene_composer.hpp"

namespace render {

    using namespace ::hlslpp;

    void Transform::setPosition(float3 newPosition) {
        m_position = newPosition;
        m_isDirty = true;
    }
    void Transform::setRotation(quaternion newRotation) {
        m_rotation = newRotation;
        m_isDirty = true;
    }
    void Transform::setScale(float3 newScale) {
        m_scale = newScale;
        m_isDirty = true;
    }

    const float4x4 Transform::getModel() {
        if (m_isDirty) {

            float4x4 translation = float4x4::translation(m_position);
            float4x4 rotation = float4x4(m_rotation);
            float4x4 scale = float4x4::scale(m_scale);

            // SRT matrix composition
            m_model = mul(rotation, scale);
            m_model = mul(translation, m_model);

            m_isDirty = false;
        }

        return m_model;
    }

    void Entity::push_back(std::shared_ptr<Entity> entity) {
        this->children.push_back(entity);
        this->children.back()->parent = this;
    }

    void Entity::push_back(std::shared_ptr<IComponent> component){
        if (component->getEntity() != this) {
            component->setParent(this);
        }
        this->components.push_back(component);
    }

    Entity* Entity::findEntityWithType(ComponentType type, bool ignoreDisabled /* = false */) const {

        // Check components attached to this entity
        for (const std::shared_ptr<IComponent> component : components) {
            if (component->componentType == type) {
                return const_cast<Entity*>(this);
            }
        }
        
        // Entity didn't have any components we wanted attached to itself, check children
        for (const std::shared_ptr<Entity> entity : children) {
            // may return null, if not null its what we're after anyway
            if (entity->enabled) {
                if (ignoreDisabled || (!ignoreDisabled && entity->enabled)) {
                    Entity* childEntity = entity->findEntityWithType(type);
                    if (childEntity != nullptr) {
                        return childEntity;
                    }
                }
            }
        }

        return nullptr;
    }

    IComponent* Entity::findComponent(ComponentType type, bool traverseChildren /* = false */, bool ignoreDisabled /* = false */) const {

        // Check components attached to this entity
        for (const std::shared_ptr<IComponent> component : components) {
            if (component->componentType == type) {
                return const_cast<IComponent*>(component.get());
            }
        }

        // Entity didn't have any components we wanted attached to itself, check children
        if (traverseChildren) {
            for (const std::shared_ptr<Entity> entity : children) {
                // may return null, if not null its what we're after anyway
                if (ignoreDisabled || (!ignoreDisabled && entity->enabled)) {
                    IComponent* childComponent = entity->findComponent(type);
                    if (childComponent != nullptr) {
                        return childComponent;
                    }
                }
            }
        }

        return nullptr;
    }

    void Scene::push_back(EntityBuilder& entityBuilder) {
        std::shared_ptr<Entity> entity = entityBuilder.build();
        root.push_back(entity);
    }

    Entity* Scene::findEntityWithType(ComponentType type, bool ignoreDisabled /* = false */) const {
        return root.findEntityWithType(type, ignoreDisabled);;
    }

    IComponent* Scene::findComponent(ComponentType type, bool ignoreDisabled /* = false */) const {
        return root.findComponent(type, true, ignoreDisabled);;
    }
}