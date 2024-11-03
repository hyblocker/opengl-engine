#include "quill/Backend.h"
#include "quill/Frontend.h"
#include "quill/LogMacros.h"
#include "quill/Logger.h"
#include "quill/sinks/ConsoleSink.h"
#include "quill/sinks/FileSink.h"

#include "log.hpp"
#include "core.hpp"

namespace engine::log {
    ::quill::Logger* s_logger = nullptr;

    void init() {
        ASSERT(s_logger == nullptr);

        quill::Backend::start();

        s_logger = quill::Frontend::create_or_get_logger("engine", {
            quill::Frontend::create_or_get_sink<quill::ConsoleSink>("console"),
            quill::Frontend::create_or_get_sink<quill::FileSink>("engine.log"),
        });
    }
}