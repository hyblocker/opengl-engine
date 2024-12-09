#pragma once

#include <inttypes.h>
#include <string>
#include <memory>

#include <dexode/EventBus.hpp>

#include "engine/window.hpp"
#include "engine/events.hpp"
#include "engine/gpu/device_manager.hpp"
#include "engine/ilayer.hpp"
#include "engine/log.hpp"

struct AppDesc {
	// Window props
	WindowDesc window;
	// OpenGL
	int32_t openglMajor = 3;
	int32_t openglMinor = 3;
};

class App {
public:
	App(AppDesc desc);
	~App();
	
	static constexpr double k_FIXED_DELTA_TIME = 1.0 / 60.0;

	void run();

	void pushLayer(engine::ILayer* layer);

	// Getters
	[[nodiscard]] inline Window* getWindow() { return m_window.get(); };
	[[nodiscard]] inline gpu::DeviceManager* getDeviceManager() { return m_graphicsDeviceManager; };
	[[nodiscard]] inline dexode::EventBus* getEventBus() { return m_eventBus.get(); };

	[[nodiscard]] inline static App* getInstance() { return s_instance; };
private:
	void windowResizeEventHandler(const engine::events::EventWindowResize& evt);
private:
	AppDesc m_appProps;
	std::unique_ptr<Window> m_window;
	std::shared_ptr<dexode::EventBus> m_eventBus = std::make_shared<dexode::EventBus>();
	dexode::EventBus::Listener m_listener{ m_eventBus };

	// Graphics
	gpu::DeviceManager* m_graphicsDeviceManager = nullptr;
	gpu::IDevice* m_graphicsDevice = nullptr;

	// Layer system
	std::vector<engine::ILayer*> m_layerStack;

	static App* s_instance;
};