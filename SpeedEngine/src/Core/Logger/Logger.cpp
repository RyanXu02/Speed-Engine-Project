#include "pch.h"
#include "Logger.h"

namespace SE
{
	Logger::Logger() : m_spdlogger(nullptr) {}

	void Logger::init(std::string_view name)
	{
		auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		console_sink->set_level(spdlog::level::info);

		auto file_sinkd = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/debuglog.txt", true);
		file_sinkd->set_level(spdlog::level::debug);

		auto file_sinkv = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/verboselog.txt", true);
		file_sinkv->set_level(spdlog::level::trace);

		auto logger = std::make_shared<spdlog::logger>(std::string(name), spdlog::sinks_init_list{ console_sink, file_sinkd, file_sinkv });
		logger->set_level(spdlog::level::trace);
		spdlog::register_logger(logger);

		m_spdlogger = spdlog::get(name.data());
		assert(m_spdlogger != nullptr);
	}

	void Logger::shutdown()
	{
		spdlog::shutdown();
	}
}