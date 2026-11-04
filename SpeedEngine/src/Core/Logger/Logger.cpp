#include "pch.h"

#include "Logger.h"
#include "ConsoleSink.h"

namespace SE
{
	#ifdef _DEBUG
	std::shared_ptr<spdlog::sinks::basic_file_sink_mt> Logger::s_file_sinkd = nullptr;
	std::shared_ptr<spdlog::sinks::basic_file_sink_mt> Logger::s_file_sinkv = nullptr;
	std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> Logger::s_terminal_sink = nullptr;
	#endif
	std::shared_ptr<ConsoleSink> Logger::s_console_sink = nullptr;
	std::once_flag Logger::s_sinks_init_flag;

	void Logger::initSharedSinks()
	{
#ifdef _DEBUG
		Logger::s_file_sinkd = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/debuglog.txt", true);
		Logger::s_file_sinkd->set_level(spdlog::level::debug);

		Logger::s_file_sinkv = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/verboselog.txt", true);
		Logger::s_file_sinkv->set_level(spdlog::level::trace);

		Logger::s_terminal_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		Logger::s_terminal_sink->set_level(spdlog::level::info);
#endif

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
#ifdef _DEBUG
		auto logger = std::make_shared<spdlog::logger>(std::string(name), spdlog::sinks_init_list
			{ s_terminal_sink, s_console_sink, s_file_sinkd, s_file_sinkv });
		logger->set_level(spdlog::level::trace);
#endif
#ifdef NDEBUG
		auto logger = std::make_shared<spdlog::logger>(std::string(name), spdlog::sinks_init_list
			{ s_console_sink });
		logger->set_level(spdlog::level::info);
#endif
		spdlog::register_logger(logger);

		m_spdlogger = spdlog::get(name.data());
		assert(m_spdlogger != nullptr);
	}

	void Logger::shutdown()
	{
		m_spdlogger.reset();
	}
} // namespace SE
