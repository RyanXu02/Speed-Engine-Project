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

        void init(std::string_view name);
		void shutdown();

		template <typename... Args>
		void verbose(const std::string& message, Args... args) 
		{
			m_spdlogger->trace(message, args...);
		};
		template <typename... Args>
		void debug(const std::string& message, Args... args) 
		{
			m_spdlogger->debug(message, args...);
		};
		template <typename... Args>
		void info(const std::string& message, Args... args) 
		{
			m_spdlogger->info(message, args...);
		};
		template <typename... Args>
		void warn(const std::string& message, Args... args) 
		{
			m_spdlogger->warn(message, args...);
		};
		template <typename... Args>
		void critical(const std::string& message, Args... args) 
		{
			m_spdlogger->critical(message, args...);
		};


    private:
		std::shared_ptr<spdlog::logger> m_spdlogger;
    };
}


