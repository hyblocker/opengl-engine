#include "app.hpp"
#include "engine/log.hpp"
#include "engine/core.hpp"
#include "engine/gpu/device_manager.hpp"

#include <chrono>

App* App::s_instance = nullptr;

App::App(AppDesc desc) {
	ASSERT(s_instance == nullptr);
	s_instance = this;
	::engine::log::init();

	m_window = std::make_unique<Window>(desc.window, desc.openglMajor, desc.openglMinor, m_eventBus);
	m_window->createNativeWindow();

	m_listener.listen<engine::events::EventWindowResize>(std::bind(&App::windowResizeEventHandler, this, std::placeholders::_1));

	m_graphicsDeviceManager = gpu::DeviceManager::create();
	m_graphicsDevice = m_graphicsDeviceManager->getDevice();

	// Set initial viewport to the window size
	gpu::Rect viewport = {
		.left = 0,
		.right = desc.window.width,
		.top = 0,
		.bottom = desc.window.height,
	};
	m_graphicsDevice->setViewport(viewport);
}

App::~App() {
}

void App::run() {
	LOG_INFO("Starting execution loop...");
	
	auto lastTime = std::chrono::high_resolution_clock::now();
	double timeElapsed = 0.0;
	double physicsAccumulator = 0.0;

	while (!m_window->shouldCloseWindow()) {

		// Propagate queued events
		m_eventBus->process();

		// @NOTE: deltaTime is 0 for the first frame of the app's lifetime
		auto currentTime = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastTime).count();
		double frameTime = static_cast<double>(duration);
		lastTime = currentTime;
		
		// Based on https://www.gafferongames.com/post/fix_your_timestep/
		physicsAccumulator += frameTime;
		while (physicsAccumulator > k_FIXED_DELTA_TIME) {
			for (auto layer : m_layerStack) {
				layer->update(timeElapsed, k_FIXED_DELTA_TIME);
				physicsAccumulator -= k_FIXED_DELTA_TIME;
				timeElapsed += k_FIXED_DELTA_TIME;
			}
		}
		for (auto layer : m_layerStack) {
			layer->render(frameTime);
		}

		m_window->windowPresent();
	}
	m_window->close();

}

void App::pushLayer(engine::ILayer* layer) {
	LOG_INFO("Push layer...");
	m_layerStack.push_back(layer);
}

void App::windowResizeEventHandler(const engine::events::EventWindowResize& evt) {
	for (auto layer : m_layerStack) {
		layer->backBufferResizing();
		layer->backBufferResized(evt.newWidth, evt.newHeight, 1);
	}
}