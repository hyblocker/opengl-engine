#include "app.hpp"
#include "engine/log.hpp"
#include "engine/core.hpp"
#include "engine/gpu/device_manager.hpp"
#include "engine/events/application_event.hpp"

#include <chrono>
#include <thread>
#include <algorithm>

namespace engine {
	App* App::s_instance = nullptr;

	App::App(AppDesc desc) {
		ASSERT(s_instance == nullptr);
		s_instance = this;
		::engine::log::init();

		m_maxFrameRate = desc.maxFramerate;
		m_maxFrameTime = 1.0 / desc.maxFramerate;

		m_inputManager = new input::InputManager;
		m_inputManager->init();
		m_window = std::make_unique<Window>(desc.window, desc.openglMajor, desc.openglMinor);
		m_window->createNativeWindow();

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

		// initialise asset pipeline
		m_assetManager = new managers::AssetManager(m_graphicsDevice);

		// make imgui layer
		m_imguiLayer = new ImguiLayer(m_graphicsDeviceManager, m_assetManager);
		m_imguiLayer->blockEvents(true);
		pushOverlay(m_imguiLayer);
	}

	App::~App() {
		delete m_assetManager;
		delete m_inputManager;
	}

	void App::run() {
		LOG_INFO("Starting execution loop...");
		m_isRunning = true;

		auto lastTime = std::chrono::high_resolution_clock::now();
		double timeElapsed = 0.0;
		double physicsAccumulator = 0.0;
		double sleepTime = 0.0;

		while (!m_window->shouldCloseWindow() && m_isRunning) {

			// @NOTE: deltaTime is 0 for the first frame of the app's lifetime
			auto currentTime = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - lastTime).count();
			double frameTime = duration * 0.000000001;
			lastTime = currentTime;

			// Based on https://www.gafferongames.com/post/fix_your_timestep/
			physicsAccumulator += frameTime;
			while (physicsAccumulator > k_FIXED_DELTA_TIME) {
				for (engine::ILayer* layer : m_layerStack) {
					layer->update(timeElapsed, k_FIXED_DELTA_TIME);
				}
				m_inputManager->update();
				physicsAccumulator -= k_FIXED_DELTA_TIME;
				timeElapsed += k_FIXED_DELTA_TIME;
			}

			if (m_maxFrameTime > 0) {
				sleepTime += m_maxFrameTime - frameTime;
				sleepTime = std::max(sleepTime, 0.0);
				std::this_thread::sleep_for(std::chrono::nanoseconds((long long)(1000000000 * sleepTime)));
			}

			// don't issue draw calls while minimised
			if (!m_minimised) {
				// draw events
				for (engine::ILayer* layer : m_layerStack) {
					layer->render(frameTime);
				}

				// draw imgui data
				m_imguiLayer->begin();
				for (engine::ILayer* layer : m_layerStack) {
					layer->imguiDraw();
				}
				m_imguiLayer->end();

				// Present
				m_graphicsDevice->present();
				m_window->windowPresent();
			}

		}
		m_window->close();
	}

	void App::pushLayer(engine::ILayer* layer) {
		ASSERT(layer != nullptr);
		LOG_INFO("Pushing layer \"{}\"...", layer->getDebugName());
		m_layerStack.pushLayer(layer);
	}

	void App::pushOverlay(engine::ILayer* overlay) {
		ASSERT(overlay != nullptr);
		LOG_INFO("Pushing overlay \"{}\"...", overlay->getDebugName());
		m_layerStack.pushOverlay(overlay);
	}

	void App::event(events::Event& event) {
		events::EventDispatcher dispatcher(event);
		dispatcher.dispatch<events::WindowCloseEvent>(EVENT_BIND_FUNC(App::onWindowClose));
		dispatcher.dispatch<events::WindowResizeEvent>(EVENT_BIND_FUNC(App::onWindowResize));

		for (auto it = m_layerStack.rbegin(); it != m_layerStack.rend(); ++it) {
			if (event.handled)
				break;
			(*it)->event(event);
		}
		m_inputManager->event(event);
	}

	bool App::onWindowClose(const events::WindowCloseEvent& event) {
		m_isRunning = false;
		return true;
	}

	bool App::onWindowResize(const events::WindowResizeEvent& event) {

		if (event.width == 0 || event.height == 0) {
			m_minimised = true;
			return false;
		}

		m_minimised = false;

		return false;
	}
}