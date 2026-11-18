#pragma once

#include <spdlog/sinks/base_sink.h>

namespace SE
{
	class ConsoleSink : public spdlog::sinks::base_sink <std::mutex>
	{
	public:
		struct Record
		{
			std::string text;
			spdlog::level::level_enum level;
		};

		std::vector<Record> snapshot()
		{
			std::lock_guard lock(mutex_);
			return { m_records.begin(), m_records.end() };
		}

		void clear()
		{
			std::lock_guard lock(mutex_);
			m_records.clear();
		}

	protected:
		void sink_it_(const spdlog::details::log_msg& msg) override
		{
			spdlog::memory_buf_t formatted;
			spdlog::sinks::base_sink<std::mutex>::formatter_->format(msg, formatted);
			std::string text(formatted.data());

			while (!text.empty() && (text.back() == '\n' || text.back() == '\r'))
			{
				text.pop_back();
			}

			if (m_records.size() == 4096)
				m_records.pop_front();
			
			m_records.push_back({ std::move(text), msg.level });
		}

		void flush_() override 
		{
			std::cout << std::flush;
		}

	private:
		std::deque<Record> m_records;
	};
}
