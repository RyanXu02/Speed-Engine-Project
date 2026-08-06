#pragma once
#include "Logger.h"

namespace SE
{
    class LoggerProxy
    {
    public:
        LoggerProxy(Logger& logger, std::string_view context)
            : m_logger(logger), m_context(context) {}

        template <typename... Args>
        void verbose(const std::string& message, Args&&... args) const
        {
            std::string prefixed = fmt::format("[{}] {}", m_context, message);
            m_logger.verbose(prefixed, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void debug(const std::string& message, Args&&... args) const
        {
            std::string prefixed = fmt::format("[{}] {}", m_context, message);
            m_logger.debug(prefixed, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void info(const std::string& message, Args&&... args) const
        {
            std::string prefixed = fmt::format("[{}] {}", m_context, message);
            m_logger.info(prefixed, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void warn(const std::string& message, Args&&... args) const
        {
            std::string prefixed = fmt::format("[{}] {}", m_context, message);
            m_logger.warn(prefixed, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void critical(const std::string& message, Args&&... args) const
        {
            std::string prefixed = fmt::format("[{}] {}", m_context, message);
            m_logger.critical(prefixed, std::forward<Args>(args)...);
        }

    private:
        Logger& m_logger;
        std::string m_context;
    };
}