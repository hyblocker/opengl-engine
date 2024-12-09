#pragma once

#include <inttypes.h>
#include "gpu/device_manager.hpp"

namespace engine {
    class ILayer {
    public:
        explicit ILayer(gpu::DeviceManager* deviceManager)
            : m_deviceManager(deviceManager)
        {}
        virtual ~ILayer() = default;

        virtual void update(double timeElapsed, double deltaTime) { }
        virtual void render(double deltaTime) { }
        // invalidate backbuffer/post-process textures
        virtual void backBufferResizing() { }
        // re-create new backbuffer/post-process textures
        virtual void backBufferResized(uint32_t width, uint32_t height, uint32_t samples) { }

        [[nodiscard]] gpu::DeviceManager* getDeviceManager() const { return m_deviceManager; }
        [[nodiscard]] gpu::IDevice* getDevice() const { return m_deviceManager->getDevice(); }
    private:
        gpu::DeviceManager* m_deviceManager = nullptr;
    };
}