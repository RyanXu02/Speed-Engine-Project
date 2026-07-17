#pragma once

namespace SE
{
	class Widget
	{
	public:
		Widget(std::string_view name) : m_name(std::string(name)) {};

		virtual void render() = 0;
		virtual void shutdown() = 0;

		const std::string& getMessage() const { return m_logmesg; }
		const std::string& getName() const { return m_name; }
	protected:
		void _setMessage(std::string_view message) { m_logmesg = std::string(message); }
	private:
		std::string m_logmesg { "" };
		std::string m_name{ "unnamed widget" };
	};
}

