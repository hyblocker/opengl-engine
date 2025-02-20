#pragma once

#include <inttypes.h>
#include <string>
#include <memory>
#include <random>

#include "engine/core.hpp"
#include "engine/window.hpp"
#include "engine/gpu/device_manager.hpp"
#include "engine/layerstack.hpp"
#include "engine/events/event.hpp"
#include "engine/events/application_event.hpp"
#include "engine/debug/imgui_layer.hpp"
#include "engine/managers/asset_manager.hpp"
#include "engine/input/input_manager.hpp"
#include "engine/log.hpp"

struct AppDesc {
	// Window props
	WindowDesc window;
	// OpenGL
	int32_t openglMajor = 3;
	int32_t openglMinor = 3;

	double maxFramerate = 60.0f;
};

namespace engine {

	class App;
	class RandomNumberGenerator {
		friend class App;
	public:
		// generates a random number between 0 and 1
		static float getFloat() {
			return (float)s_distribution(s_rng) / (float)std::numeric_limits<uint32_t>::max();
		}
		// generates a random int between min and max
		static int getRangedInt(int min, int max) {
			double randomDouble = (double)s_distribution(s_rng) / (double)std::numeric_limits<uint32_t>::max();
			return min + static_cast<int>(randomDouble * (max - min + 1));
		}

		static inline std::mt19937 getRng() { return s_rng; };
	private:
		static void init() {
			s_rng.seed(std::random_device()());
		}
	private:
		static std::mt19937 s_rng;
		static std::uniform_int_distribution<std::mt19937::result_type> s_distribution;
	};

	class App {
	public:
		App(AppDesc desc);
		~App();

		static constexpr double k_FIXED_DELTA_TIME = 1.0 / 60.0;

		void run();

		void event(events::Event& event);

		void pushLayer(engine::ILayer* layer);
		void pushOverlay(engine::ILayer* overlay);

		// Getters
		[[nodiscard]] inline Window* getWindow() { return m_window.get(); };
		[[nodiscard]] inline gpu::DeviceManager* getDeviceManager() { return m_graphicsDeviceManager; };
		[[nodiscard]] inline ImguiLayer* getImguiLayer() { return m_imguiLayer; };
		[[nodiscard]] inline managers::AssetManager* getAssetManager() { return m_assetManager; };

		[[nodiscard]] inline static App* getInstance() { return s_instance; };
		
	private:
		bool onWindowClose(const events::WindowCloseEvent& event);
		bool onWindowResize(const events::WindowResizeEvent& event);
	private:
		AppDesc m_appProps;
		std::unique_ptr<Window> m_window;

		// Graphics
		gpu::DeviceManager* m_graphicsDeviceManager = nullptr;
		gpu::IDevice* m_graphicsDevice = nullptr;
		managers::AssetManager* m_assetManager = nullptr;
		input::InputManager* m_inputManager = nullptr;

		// Layer system
		ImguiLayer* m_imguiLayer = nullptr;
		LayerStack m_layerStack;

		bool m_isRunning = false;
		bool m_minimised = false;

		double m_maxFrameRate = 0;
		double m_maxFrameTime = 0;

		static App* s_instance;
	};
}