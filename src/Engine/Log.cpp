#include "Log.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/msvc_sink.h>
#include <cassert>

namespace {
    std::shared_ptr<spdlog::logger> sCoreLogger;
    std::shared_ptr<spdlog::logger> sClientLogger;
}


/// TODO: [Logging] Write log file next the the executable.
/// TODO: [Logging] Support enabling custom log level from spdlog config.
/// TODO: [Logging] Only create std::stdout_color_sink_mt if there is a console (Windows /Subsystem:CONSOLE vs Subsystem:WINDOWS)
void Engine::Log::Initialize() {
    
    // Core logger
    {
        sCoreLogger = std::make_shared<spdlog::logger>("Engine");
        sCoreLogger->set_level(spdlog::level::trace);

        auto coreFileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("engine.log", true);
        sCoreLogger->sinks().push_back(coreFileSink);

        auto coreConsoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        sCoreLogger->sinks().push_back(coreConsoleSink);

#ifdef _WIN32
        auto coreMSVCSink    = std::make_shared<spdlog::sinks::msvc_sink_mt>(true);
        coreMSVCSink->set_level(spdlog::level::err);
        sCoreLogger->sinks().push_back(coreMSVCSink);
#endif
}


    // Client logger
    {
        sClientLogger = std::make_shared<spdlog::logger>("App");
        sClientLogger->set_level(spdlog::level::trace);

        auto clienFileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("App.log", true);
        sClientLogger->sinks().push_back(clienFileSink);

        auto clientConsoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        sClientLogger->sinks().push_back(clientConsoleSink);

#ifdef _WIN32
        auto clientMSVCSink    = std::make_shared<spdlog::sinks::msvc_sink_mt>(true);
        clientMSVCSink->set_level(spdlog::level::err);
        sClientLogger->sinks().push_back(clientMSVCSink);
#endif
    }
}

void Engine::Log::Shutdown() {
    sCoreLogger.reset();
    sClientLogger.reset();
}

std::shared_ptr<spdlog::logger> Engine::Log::getCoreLogger() {
    assert(sCoreLogger);
    return sCoreLogger;
}

std::shared_ptr<spdlog::logger> Engine::Log::getClientLogger() {
    assert(sClientLogger);
    return sClientLogger;
}
