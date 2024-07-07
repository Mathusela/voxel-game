module;

#include "voxel_game/debug_macros.hpp"

#include <format>
#include <string_view>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

export module voxel_game.logging:logger;

namespace vxg::logging {

	class ConsoleMixin {
	protected:
		void console_log(std::string_view msg) noexcept {
			std::cout << msg;
		}
	
	public:
		virtual ~ConsoleMixin() {}
	};

	class FileMixin {
		std::ofstream m_file;

	protected:
		void file_log(std::string_view msg) noexcept {
			m_file << msg;
		}

	public:
		FileMixin(std::string_view filePath)
			: m_file(std::string(filePath)) {}

		virtual ~FileMixin() {}
	};

}; // namespace vxg::logging

// EXPORTED
export namespace vxg::logging {

	enum class LogType {
		INFO,
		WARNING,
		ERROR
	};

	class Logger {
	protected:
		virtual void log_impl(std::string_view msg) noexcept = 0;
	
		[[nodiscard]]
		std::string format_log(LogType logType, std::string msg) noexcept {
			switch (logType) {
				case LogType::INFO:
					return "[INFO] " + msg;
				case LogType::WARNING:
					return "[WARNING] " + msg;
				case LogType::ERROR:
					return "[ERROR] " + msg;
				default:
					return "[UNKNOWN] " + msg;
			}
		}

		virtual void log_raw(std::string_view msg) noexcept {
			log_impl(std::string(msg));
		}

	public:
		virtual ~Logger() {}

		template <typename... Args>
		void log(LogType logType, std::format_string<Args...> fmt, Args&&... args) {
			log(logType, std::format(fmt, std::forward<Args>(args)...));
		}

		virtual void log(LogType logType, std::string_view msg) noexcept {
			log_impl(format_log(logType, std::string(msg)));
		}


		friend class CombinedLogger;
	};

	class DebugOnlyLogger : public Logger {
	protected:
		void log_raw([[maybe_unused]] std::string_view msg) noexcept override {
			#ifdef DEBUG
			log_impl(std::string(msg));
			#endif // DEBUG
		}

	public:
		virtual ~DebugOnlyLogger() {}

		void log([[maybe_unused]] LogType logType, [[maybe_unused]] std::string_view msg) noexcept override {
			#ifdef DEBUG
			log_impl(format_log(logType, std::string(msg)));
			#endif // DEBUG
		}
	};

	class NullLogger final : public Logger {
		void log_impl([[maybe_unused]] std::string_view msg) noexcept override {}
	};

	class ConsoleDebugLogger final : public DebugOnlyLogger, ConsoleMixin {
		void log_impl(std::string_view msg) noexcept override {
			console_log(msg);
		}
	};

	class ConsoleReleaseLogger final : public Logger, ConsoleMixin {
		void log_impl(std::string_view msg) noexcept override {
			console_log(msg);
		}
	};

	class FileDebugLogger final : public DebugOnlyLogger, FileMixin {
		void log_impl(std::string_view msg) noexcept override {
			file_log(msg);
		}

	public:
		FileDebugLogger(std::string_view filePath)
			: FileMixin(filePath) {}
	};

	class FileReleaseLogger final : public Logger, FileMixin {
		void log_impl(std::string_view msg) noexcept override {
			file_log(msg);
		}

	public:
		FileReleaseLogger(std::string_view filePath)
			: FileMixin(filePath) {}
	};
	
	class CombinedLogger final : public Logger {
		std::vector<std::unique_ptr<Logger>> m_loggers;
		
		void log_impl(std::string_view msg) noexcept override {
			for (auto& logger : m_loggers)
				logger->log_raw(msg);
		}

	public:
		template <typename... Loggers>
		CombinedLogger(std::unique_ptr<Loggers>... loggers) {
			(m_loggers.push_back(std::move(loggers)), ...);
		}
	};

}; // namespace vxg::logging