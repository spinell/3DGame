#pragma once
#include <spdlog/spdlog.h>

#include <memory>

namespace Engine::Log {

/// @brief
void Initialize();

/// @brief
void Shutdown();

std::shared_ptr<spdlog::logger> getCoreLogger();
std::shared_ptr<spdlog::logger> getClientLogger();
}; // namespace Engine::Log

#define ENGINE_CORE_INFO(...) Engine::Log::getCoreLogger()->info(__VA_ARGS__)
#define ENGINE_CORE_DEBUG(...) Engine::Log::getCoreLogger()->debug(__VA_ARGS__)
#define ENGINE_CORE_WARNING(...) Engine::Log::getCoreLogger()->warn(__VA_ARGS__)
#define ENGINE_CORE_ERROR(...) Engine::Log::getCoreLogger()->error(__VA_ARGS__)
#define ENGINE_CORE_CRITICAL(...) Engine::Log::getCoreLogger()->critical(__VA_ARGS__)
#define ENGINE_CORE_TRACE(...) Engine::Log::getCoreLogger()->trace(__VA_ARGS__)

#define ENGINE_INFO(...) Engine::Log::getClientLogger()->info(__VA_ARGS__)
#define ENGINE_DEBUG(...) Engine::Log::getClientLogger()->debug(__VA_ARGS__)
#define ENGINE_WARNING(...) Engine::Log::getClientLogger()->warn(__VA_ARGS__)
#define ENGINE_ERROR(...) Engine::Log::getClientLogger()->error(__VA_ARGS__)
#define ENGINE_CRITICAL(...) Engine::Log::getClientLogger()->critical(__VA_ARGS__)
#define ENGINE_TRACE(...) Engine::Log::getClientLogger()->trace(__VA_ARGS__)
