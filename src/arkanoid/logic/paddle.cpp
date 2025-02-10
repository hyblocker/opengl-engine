#include "paddle.hpp"

#include "engine/input/input_manager.hpp"

void Paddle::start() {
    m_physicsComponent = (physics::PhysicsComponent*) getEntity()->findComponent(render::ComponentType::Physics);
}

void Paddle::sleep() {

}

void Paddle::update(float deltaTime) {

    float move = 0;

    if (engine::input::InputManager::getInstance()->keyDown(engine::input::Keycode::A) || engine::input::InputManager::getInstance()->keyDown(engine::input::Keycode::Left)) {
        move -= k_PADDLE_VELOCITY;
    }

    if (engine::input::InputManager::getInstance()->keyDown(engine::input::Keycode::D) || engine::input::InputManager::getInstance()->keyDown(engine::input::Keycode::Right)) {
        move += k_PADDLE_VELOCITY;
    }

    m_physicsComponent->addForce( { move, 0 } );
}