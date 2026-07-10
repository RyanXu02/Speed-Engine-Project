#pragma once
#include "../SubSystems/SubSystem.h"

#include "spdlog.h"
#include "sinks/stdout_color_sinks.h"
#include "sinks/basic_file_sink.h"

namespace SE
{
    class Logger
    {
    public:
		Logger();

		//@brief creates logger and its channels
		//@param name name of this logger
        void init(std::string_view name);

		//@brief shuts down logger
		void shutdown();

		//@brief critical level logging (literally everything)
		//@param message message to log
		//@param args variables to log in message
		template <typename... Args>
		void verbose(const std::string& message, Args&&... args) 
		{
			m_spdlogger->trace(fmt::runtime(message), std::forward<Args>(args)...);
		};

		//@brief debug level logging (fine details)
		//@param message message to log
		//@param args variables to log in message
		template <typename... Args>
		void debug(const std::string& message, Args&&... args) 
		{
			m_spdlogger->debug(fmt::runtime(message), std::forward<Args>(args)...);
		};

		//@brief info level logging (regular level)
		//@param message message to log
		//@param args variables to log in message
		template <typename... Args>
		void info(const std::string& message, Args&&... args) 
		{
			m_spdlogger->info(fmt::runtime(message), std::forward<Args>(args)...);
		};

		//@brief warning level logging (address asap)
		//@param message message to log
		//@param args variables to log in message
		template <typename... Args>
		void warn(const std::string& message, Args&&... args) 
		{
			m_spdlogger->warn(fmt::runtime(message), std::forward<Args>(args)...);
		};

		//@brief critical level logging (crashworthy)
		//@param message message to log
		//@param args variables to log in message
		template <typename... Args>
		void critical(const std::string& message, Args&&... args) 
		{
			m_spdlogger->critical(fmt::runtime(message), std::forward<Args>(args)...);
		};


    private:
		std::shared_ptr<spdlog::logger> m_spdlogger;

		// Static shared sinks reused by all loggers
		static std::shared_ptr<spdlog::sinks::basic_file_sink_mt> s_file_sinkd;
		static std::shared_ptr<spdlog::sinks::basic_file_sink_mt> s_file_sinkv;
		static std::once_flag s_sinks_init_flag; //for use with std::call_once

		//inits 2 static shared sinks
		static void initSharedSinks();
    };
}


