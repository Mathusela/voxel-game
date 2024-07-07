module;

#include "voxel_game/debug_macros.hpp"

#include <format>
#include <string_view>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <type_traits>
#include <tuple>

export module voxel_game.logging:logger;

// EXPORTED
export namespace vxg::logging {
	
	enum class LogType {
		INFO,
		WARNING,
		ERROR
	};

}; // namespace vxg::logging

namespace vxg::logging {

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

	struct ConsoleLoggerImpl {
		void initialize_impl() noexcept {}

		void log_impl(LogType logType, std::string msg) noexcept {
			std::cout << format_log(logType, msg);
		}

	};

	class FileLoggerImpl {
		std::ofstream m_file;
	
	public:
		void initialize_impl(std::string filePath) noexcept {
			m_file.open(filePath);
		}

		void log_impl(LogType logType, std::string msg) noexcept {
			m_file << format_log(logType, msg);
		}
	};

}; // namespace vxg::logging

// EXPORTED
export namespace vxg::logging {


	template <typename Derived>
	class Logger {
		Logger() noexcept = default;
		friend Derived;

		const Derived* derived_instance() const noexcept {
			return static_cast<const Derived*>(this);
		}

		Derived* derived_instance() noexcept {
			return static_cast<Derived*>(this);
		}
		
	public:
		template <typename... Args>
		void log(LogType logType, std::format_string<Args...> fmt, Args&&... args) {
			derived_instance()->log_impl(logType, fmt, std::forward<Args>(args)...);
		}
		
		void log(LogType logType, std::string_view msg) noexcept {
			derived_instance()->log_impl(logType, msg);
		}
	};

	template <typename Implementation>
	class DebugLogger : public Logger<DebugLogger<Implementation>>{
		using Base = Logger<DebugLogger>;
		friend Base;

		Implementation m_implementation;

		template <typename... Args>
		void log_impl([[maybe_unused]] LogType logType, [[maybe_unused]] std::format_string<Args...> fmt, [[maybe_unused]] Args&&... args) {
			#ifdef DEBUG
			m_implementation.log_impl(logType, std::format(fmt, std::forward<Args>(args)...));
			#endif // DEBUG
		}

		void log_impl([[maybe_unused]] LogType logType, [[maybe_unused]] std::string_view msg) noexcept {
			#ifdef DEBUG
			m_implementation.log_impl(logType, msg);
			#endif // DEBUG
		}

		template<typename... Args>
		void initialize([[maybe_unused]] Args&&... args) noexcept {
			#ifdef DEBUG
			m_implementation.initialize_impl(std::forward<Args>(args)...);
			#endif // DEBUG
		}

	public:
		template<typename... Args>
		DebugLogger(Args&&... args) noexcept {
			initialize(std::forward<Args>(args)...);
		}
	};

	template <typename Implementation>
	class ReleaseLogger : public Logger<ReleaseLogger<Implementation>> {
		using Base = Logger<ReleaseLogger>;
		friend Base;

		Implementation m_implementation;

		template <typename... Args>
		void log_impl(LogType logType, std::format_string<Args...> fmt, Args&&... args) {
			m_implementation.log_impl(logType, std::format(fmt, std::forward<Args>(args)...));
		}

		void log_impl(LogType logType, std::string_view msg) noexcept {
			m_implementation.log_impl(logType, msg);
		}

		template<typename... Args>
		void initialize(Args&&... args) noexcept {
			#ifdef DEBUG
			m_implementation.initialize_impl(std::forward<Args>(args)...);
			#endif // DEBUG
		}

	public:
		template<typename... Args>
		ReleaseLogger(Args&&... args) noexcept {
			initialize(std::forward<Args>(args)...);
		}
	};

	template <typename... Loggers>
	class CombinedLogger : public Logger<CombinedLogger<Loggers...>> {
		using Base = Logger<CombinedLogger>;
		friend Base;
	
		std::tuple<Loggers...> m_loggers;

		template <typename... Args>
		void log_impl(LogType logType, std::format_string<Args...> fmt, Args&&... args) {
			std::apply([&](auto&... loggers) {
				(loggers.log(logType, fmt, std::forward<Args>(args)...), ...);
			}, m_loggers);
		}

		void log_impl(LogType logType, std::string_view msg) noexcept {
			std::apply([&](auto&... loggers) {
				(loggers.log(logType, msg), ...);
			}, m_loggers);
		}

	public:
		template<typename... Loggers>
		CombinedLogger(Loggers&&... loggers) noexcept
			: m_loggers(std::forward<Loggers>(loggers)...) {}
	};
	template <typename... Loggers>
	CombinedLogger(Loggers&&...) -> CombinedLogger<Loggers...>;

	template <template <typename> typename Base>
	using ConsoleLogger = Base<ConsoleLoggerImpl>;

	template <template <typename> typename Base>
	using FileLogger = Base<FileLoggerImpl>;

}; // namespace vxg::logging