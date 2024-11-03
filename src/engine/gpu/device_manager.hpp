#pragma once

#include <inttypes.h>
#include <string>

#include "idevice.hpp"

namespace gpu {
	class DeviceManager {
	public:
		static DeviceManager* create();

		[[nodiscard]] IDevice* getDevice() const { return m_device; };

	private:
		bool createDevice();

	private:

		DeviceHandle m_device;
	};
}