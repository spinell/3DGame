#pragma once

#if defined(_WIN32) && !defined(VK_USE_PLATFORM_WIN32_KHR)
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#ifdef WANT_VOLK
#	include "volk/volk.h"
#else
#	include <vulkan/vulkan.h>
#endif

constexpr unsigned MAX_FRAME_IN_FLIGHT = 3u;
