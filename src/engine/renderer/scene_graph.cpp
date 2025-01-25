#include "scene_graph.hpp"

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
            m_model = mul(translation, mul(rotation, scale));
            m_isDirty = false;
        }

        return m_model;
    }

    void Entity::push_back(Entity& entity) {
        entity.parent = this;
        this->children.push_back(entity);
    }

    void Entity::push_back(IComponent& component){
        if (component.getEntity() != this) {
            component.setParent(this);
        }
        this->components.push_back(component);
    }

    Entity* Entity::findEntityWithType(ComponentType type) const {

        // Check components attached to this entity
        for (const IComponent& component : components) {
            if (component.componentType == type) {
                return const_cast<Entity*>(this);
            }
        }
        
        // Entity didn't have any components we wanted attached to itself, check children
        for (const Entity& entity : children) {
            // may return null, if not null its what we're after anyway
            Entity* childEntity = entity.findEntityWithType(type);
            if (childEntity != nullptr) {
                return childEntity;
            }
        }

        return nullptr;
    }

    IComponent* Entity::findComponent(ComponentType type) const {

        // Check components attached to this entity
        for (const IComponent& component : components) {
            if (component.componentType == type) {
                return const_cast<IComponent*>(&component);
            }
        }

        // Entity didn't have any components we wanted attached to itself, check children
        for (const Entity& entity : children) {
            // may return null, if not null its what we're after anyway
            IComponent* childComponent = entity.findComponent(type);
            if (childComponent != nullptr) {
                return childComponent;
            }
        }

        return nullptr;
    }

    void Scene::push_back(Entity& entity) {
        entity.parent = nullptr;
        this->entities.push_back(entity);
    }

    Entity* Scene::findEntityWithType(ComponentType type) const {
        for (const Entity& entity : entities) {
            // may return null, if not null its what we're after anyway
            Entity* childEntity = entity.findEntityWithType(type);
            if (childEntity != nullptr) {
                return childEntity;
            }
        }

        return nullptr;
    }

    IComponent* Scene::findComponent(ComponentType type) const {
        for (const Entity& entity : entities) {
            // may return null, if not null its what we're after anyway
            IComponent* childComponent = entity.findComponent(type);
            if (childComponent != nullptr) {
                return childComponent;
            }
        }

        return nullptr;
    }

}