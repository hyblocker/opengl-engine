#pragma once

#include <inttypes.h>
#include "gpu/device_manager.hpp"
#include "events/event.hpp"

namespace engine {
    class ILayer {
    public:
        explicit ILayer(gpu::DeviceManager* deviceManager, const std::string& debugName)
            : m_deviceManager(deviceManager),
            m_debugName(debugName)
        {}
        virtual ~ILayer() = default;

        // layer events
        virtual void attach() { } // called when a layer is attached to the layer stack
        virtual void detach() { } // called when a layer is popped from the layer stack

        // game loop events
        virtual void update(double timeElapsed, double deltaTime) { } // called every frame before updating anything
        virtual void render(double deltaTime) { } // called every frame for rendering
        virtual void event(events::Event& event) { } // called every frame for processing events from the engine

        // debug
        virtual void imguiDraw() { } // optional: called every frame during the imgui render pass

        inline const std::string& getDebugName() const { return m_debugName; }

        [[nodiscard]] gpu::DeviceManager* getDeviceManager() const { return m_deviceManager; }
        [[nodiscard]] gpu::IDevice* getDevice() const { return m_deviceManager->getDevice(); }
    private:
        gpu::DeviceManager* m_deviceManager = nullptr;
        std::string m_debugName;
    };
}