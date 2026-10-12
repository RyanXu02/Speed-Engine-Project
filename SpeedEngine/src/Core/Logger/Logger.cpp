#include "pch.h"

#include "Logger.h"
#include "ConsoleSink.h"

namespace SE
{
	std::shared_ptr<spdlog::sinks::basic_file_sink_mt> Logger::s_file_sinkd = nullptr;
	std::shared_ptr<spdlog::sinks::basic_file_sink_mt> Logger::s_file_sinkv = nullptr;
	std::shared_ptr<ConsoleSink> Logger::s_console_sink = nullptr;
	std::once_flag Logger::s_sinks_init_flag;

	void Logger::initSharedSinks()
	{
		Logger::s_file_sinkd = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/debuglog.txt", true);
		Logger::s_file_sinkd->set_level(spdlog::level::debug);

		Logger::s_file_sinkv = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/verboselog.txt", true);
		Logger::s_file_sinkv->set_level(spdlog::level::trace);

		Logger::s_console_sink = std::make_shared<ConsoleSink>();
		Logger::s_console_sink->set_level(spdlog::level::info);
	}

	Logger::Logger()
	    : m_spdlogger(nullptr)
	{}

	void Logger::init(std::string_view name)
	{
		std::call_once(s_sinks_init_flag,
		    &Logger::initSharedSinks); // init all sinks across all threads

		auto logger = std::make_shared<spdlog::logger>(std::string(name), spdlog::sinks_init_list{ s_console_sink, s_file_sinkd, s_file_sinkv });
		logger->set_level(spdlog::level::trace);
		spdlog::register_logger(logger);

		m_spdlogger = spdlog::get(name.data());
		assert(m_spdlogger != nullptr);
	}

	void Logger::shutdown()
	{
		spdlog::shutdown();
	}
} // namespace SE
