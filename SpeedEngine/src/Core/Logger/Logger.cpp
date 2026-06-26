#include "pch.h"
#include "Logger.h"

namespace SE
{
	Logger::Logger() : m_spdlogger(nullptr) {}

	void Logger::init(std::string_view name)
	{
		auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		console_sink->set_level(spdlog::level::info);

		auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/log.txt", true);
		file_sink->set_level(spdlog::level::trace);

		auto logger = std::make_shared<spdlog::logger>(std::string(name), spdlog::sinks_init_list{ console_sink, file_sink });
		spdlog::register_logger(logger);

		m_spdlogger = spdlog::get(name.data());
		assert(m_spdlogger != nullptr);
	}

	void Logger::shutdown()
	{
		spdlog::shutdown();
	}
}