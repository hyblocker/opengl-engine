#pragma once

#include <quill/Logger.h>
#include <quill/LogMacros.h>
#include <quill/bundled/fmt/format.h>

namespace engine::log {
	extern ::quill::Logger* s_logger;
	void init();
}

#define LOG_DEBUG(fmt, ...)					QUILL_LOG_DEBUG(::engine::log::s_logger, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)					QUILL_LOG_INFO(::engine::log::s_logger, fmt, ##__VA_ARGS__)
#define LOG_NOTICE(fmt, ...)				QUILL_LOG_NOTICE(::engine::log::s_logger, fmt, ##__VA_ARGS__)
#define LOG_WARNING(fmt, ...)				QUILL_LOG_WARNING(::engine::log::s_logger, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)					QUILL_LOG_WARNING(::engine::log::s_logger, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)					QUILL_LOG_ERROR(::engine::log::s_logger, fmt, ##__VA_ARGS__)
#define LOG_CRITICAL(fmt, ...)				QUILL_LOG_CRITICAL(::engine::log::s_logger, fmt, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...)					QUILL_LOG_CRITICAL(::engine::log::s_logger, fmt, ##__VA_ARGS__)
#define LOG_DYNAMIC(log_level, fmt, ...)	QUILL_LOG_DYNAMIC(::engine::log::s_logger, log_level, fmt, ##__VA_ARGS__)