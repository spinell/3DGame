#pragma once
#include "vulkan.h"

namespace VulkanContext {

bool Initialize();
void Shutdown();

VkInstance       getIntance();
VkPhysicalDevice getPhycalDevice();
VkDevice         getDevice();

bool isLayerSupported();
bool isInstanceExtensionSupported();
bool isDeviceExtensionSupported();

} // namespace VulkanContext
