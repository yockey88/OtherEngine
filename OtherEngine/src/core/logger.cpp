/**
 * \file core/logger.cpp
*/
#include "logger.hpp"

#include <spdlog/common.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace other {
  
  constexpr static uint64_t kConsoleLevelKey = FNV("CONSOLE-LEVEL");
  constexpr static uint64_t kFileLevelKey = FNV("FILE-LEVEL");
  constexpr static uint64_t kConsoleFmtKey = FNV("CONSOLE-FMT");
  constexpr static uint64_t kFileFmtKey = FNV("FILE-FMT");
  constexpr static uint64_t kThreadFiltersKey = FNV("THREAD-FILTERS");
  constexpr static uint64_t kLogFilePathKey = FNV("PATH");

  constexpr static std::string_view kTraceStr = "TRACE";
  constexpr static std::string_view kDebugStr = "DEBUG";
  constexpr static std::string_view kInfoStr = "INFO";
  constexpr static std::string_view kWarnStr = "WARN";
  constexpr static std::string_view kErrorStr = "ERROR";
  constexpr static std::string_view kCriticalStr = "CRITICAL";

  constexpr static uint64_t kTraceKey = FNV(kTraceStr);
  constexpr static uint64_t kDebugKey = FNV(kDebugStr);
  constexpr static uint64_t kInfoKey = FNV(kInfoStr);
  constexpr static uint64_t kWarnKey = FNV(kWarnStr);
  constexpr static uint64_t kErrorKey = FNV(kErrorStr);
  constexpr static uint64_t kCriticalKey = FNV(kCriticalStr);


  constexpr static std::string_view kLoggerName = "OTHER-ENGINE-LOG";
  /// just a default
  constexpr static std::string_view kDefaultLogFilePath = "other.log";

  Logger* Logger::instance = nullptr;

  Logger* Logger::Open(const ConfigTable& config) {
    if (instance == nullptr) {
      instance = new Logger(config);
    } else {
      instance->logger->log(spdlog::level::warn , "Reinitializing logger with new configuration");
      Shutdown();
      instance = new Logger(config);
    }
    return instance;
  }
  
  Logger* Logger::Instance() {
    if (instance == nullptr) {
      /// this is really bad
      throw std::runtime_error("Logger instance not initialized");
    }
    return instance;
  }

  void Logger::Configure(const ConfigTable& config_table) {
    auto config = config_table.Get("LOG");

    auto log_file_path = config.find(kLogFilePathKey);
    auto console_level = config.find(kConsoleLevelKey);
    auto file_level = config.find(kFileLevelKey);
    auto cpattern = config.find(kConsoleFmtKey);
    auto fpattern = config.find(kFileFmtKey); 
    
    std::string log_path = kDefaultLogFilePath.data();
    
    if (log_file_path != config.end()) {
      auto real_path = log_file_path->second[0];
      std::transform(real_path.begin() , real_path.end() , real_path.begin() , ::tolower);
      log_path = real_path;
    } else {
      log_path = kDefaultLogFilePath.data();
    }

    ChangeFiles(log_path);

    if (console_level != config.end()) {
      if (console_level->second.size() == 1) {
        SetCoreLevel(CONSOLE , console_level->second[0]);
      } else {
        logger->log(spdlog::level::warn , "Invalid number of arguments for CONSOLE-LEVEL");
      }
    } 

    if (file_level != config.end()) {
      if (file_level->second.size() == 1) {
        SetCoreLevel(FILE , file_level->second[0]);
      } else {
        logger->log(spdlog::level::warn , "Invalid number of arguments for FILE-LEVEL");
      }
    }
  
    if (cpattern != config.end()) {
      if (cpattern->second.size() == 1) {
        SetCorePattern(CONSOLE , cpattern->second[0]);
      } else {
        logger->log(spdlog::level::warn , "Invalid number of arguments for CONSOLE-FMT");
      }
    }

    if (fpattern != config.end()) {
      if (fpattern->second.size() == 1) {
        SetCorePattern(FILE , fpattern->second[0]);
      } else {
        logger->log(spdlog::level::warn , "Invalid number of arguments for FILE-FMT");
      }
    }
  }

  void Logger::RegisterThread(const std::string& name) {
    std::lock_guard<std::mutex> lock(thread_map_mutex);
    if (thread_names.contains(std::this_thread::get_id())) {
      logger->log(spdlog::level::warn , std::string_view{ "Thread already registered with name: {}" } , name);
      return;
    }

    thread_names[std::this_thread::get_id()] = name;
  }

  void Logger::SetCorePattern(CoreTarget target , const std::string& pattern) {
    if (target == CoreTarget::CONSOLE) {
      sink_patterns[CONSOLE] = pattern;
      sinks[CONSOLE]->set_pattern(pattern);
    } else if (target == CoreTarget::FILE) {
      sink_patterns[FILE] = pattern;
      sinks[FILE]->set_pattern(pattern);
    }
  }

  void Logger::SetCoreLevel(CoreTarget target , const std::string& level) {
    spdlog::level::level_enum spd_level = LevelFromString(level);
    if (target == CoreTarget::CONSOLE) {
      sink_levels[CONSOLE] = spd_level;
      sinks[CONSOLE]->set_level(spd_level);
    } else if (target == CoreTarget::FILE) {
      sink_levels[FILE] = spd_level;
      sinks[FILE]->set_level(spd_level);
    }
  }

  void Logger::SetCoreLevel(CoreTarget target , Level level) {
    spdlog::level::level_enum spd_level = LevelFromLevel(level);
    if (target == CoreTarget::CONSOLE) {
      sink_levels[CONSOLE] = spd_level;
      sinks[CONSOLE]->set_level(spd_level);
    } else if (target == CoreTarget::FILE) {
      sink_levels[FILE] = spd_level;
      sinks[FILE]->set_level(spd_level);
    }
  }

  bool Logger::ChangeFiles(const std::string& path) {
    sinks[FILE] = NewStdRef<spdlog::sinks::basic_file_sink_mt>(path , true);
    sinks[FILE]->set_level(sink_levels[FILE]);
    sinks[FILE]->set_pattern(sink_patterns[FILE]);

    logger->sinks() = { sinks[CONSOLE] , sinks[FILE] };
    return true;
  }

  void Logger::Shutdown() {
    spdlog::drop_all();
    delete instance;
    instance = nullptr;
  }

  /// should never be called
  Logger::Logger() {
    sinks[CONSOLE] = NewStdRef<spdlog::sinks::stdout_color_sink_mt>();
    sinks[CONSOLE]->set_level(sink_levels[CONSOLE]);
    sinks[CONSOLE]->set_pattern(sink_patterns[CONSOLE]);

    logger = NewStdRef<spdlog::logger>(kLoggerName.data());
    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::trace);
    logger->sinks() = { sinks[CONSOLE] };

    spdlog::register_logger(logger);
  }

  Logger::Logger(const ConfigTable& config) {
    sinks[CONSOLE] = NewStdRef<spdlog::sinks::stdout_color_sink_mt>();
    sinks[CONSOLE]->set_level(sink_levels[CONSOLE]);
    sinks[CONSOLE]->set_pattern(sink_patterns[CONSOLE]);

    logger = NewStdRef<spdlog::logger>(kLoggerName.data());
    /// leave these as trace to defer to sink levels
    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::trace);
    logger->sinks() = { sinks[CONSOLE]  , nullptr };

    spdlog::register_logger(logger);

    /// have to do this last so the the console sink is set up
    Configure(config);
  }

  spdlog::level::level_enum Logger::LevelFromString(const std::string& l) {
    uint64_t hash = FNV(l);

    switch (hash) {
      case kTraceKey: return spdlog::level::trace;
      case kDebugKey: return spdlog::level::debug;
      case kInfoKey: return spdlog::level::info;
      case kWarnKey: return spdlog::level::warn;
      case kErrorKey: return spdlog::level::err;
      case kCriticalKey: return spdlog::level::critical;
      default: return spdlog::level::trace;
    }
  }

  spdlog::level::level_enum Logger::LevelFromLevel(Level l) {
    switch (l) {
      case TRACE: return spdlog::level::trace;
      case DEBUG: return spdlog::level::debug;
      case INFO: return spdlog::level::info;
      case WARN: return spdlog::level::warn;
      case ERR: return spdlog::level::err;
      case CRITICAL: return spdlog::level::critical;
      default: return spdlog::level::critical;
    }
  }

} // namespace other