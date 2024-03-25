/**
 * \file core/logger.hpp
*/
#ifndef OTHER_ENGINE_LOGGER_HPP
#define OTHER_ENGINE_LOGGER_HPP

#include <map>
#include <source_location>
#include <thread>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

#include "core/config.hpp"

#define VA_ARGS(...) , ##__VA_ARGS__

#define LOG_ARGS(level , fmt) other::Logger::Level::level , fmt , std::source_location::current() , std::this_thread::get_id()
#define LOG_INSTANCE() other::Logger::Instance()
#define OE_LOG(level , fmt , ...) \
  do { \
    LOG_INSTANCE()->Log(LOG_ARGS(level , fmt) VA_ARGS(__VA_ARGS__)); \
  } while(false)

#define OE_TRACE(fmt , ...) OE_LOG(TRACE , fmt , __VA_ARGS__)
#define OE_DEBUG(fmt , ...) OE_LOG(DEBUG , fmt , __VA_ARGS__)
#define OE_INFO(fmt , ...) OE_LOG(INFO , fmt , __VA_ARGS__)
#define OE_WARN(fmt , ...) OE_LOG(WARN , fmt , __VA_ARGS__)
#define OE_ERROR(fmt , ...) OE_LOG(ERR , fmt , __VA_ARGS__)
#define OE_CRITICAL(fmt , ...) OE_LOG(CRITICAL , fmt , __VA_ARGS__)

#define OE_ASSERT(x , fmt , ...) \
  do { \
    if ((x)) {} else { \
      OE_CRITICAL(fmt , __VA_ARGS__); \
      std::abort(); \
    } \
  } while(false)

namespace other {
  
  constexpr static std::string_view kConsoleFmt = "%^[ OTHER ENGINE -- %l ]%$ %v";
  constexpr static std::string_view kFileFmt = "%^[ %l ]%$ %v [%Y/%m/%d %H:%M:%S %z]";

  class Logger {
    public:
      enum CoreTarget : uint64_t {
        CONSOLE  = 0, 
        FILE , 

        NUM_TARGETS
      };

      enum Level {
        TRACE , 
        DEBUG ,
        INFO ,
        WARN ,
        ERR ,
        CRITICAL
      };

      static Logger* Open(const ConfigTable& config);

      /// not typical 'get' function, this will throw if Open has not been called
      static Logger* Instance();

      StdRef<spdlog::logger> GetLogger() { return logger; }

      void Configure(const ConfigTable& config);
      
      template <typename... Args>
      void Log(Level l , const std::string_view format, std::source_location src_pos , std::thread::id thread_id , Args&&... args) {
        std::string file = src_pos.file_name();

        std::string func = src_pos.function_name();
        int line = src_pos.line();
        int col = src_pos.column();

        file = file.substr(file.find_last_of("/\\") + 1);

        std::string src_str = fmt::format(std::string_view{ "[{} - {}:{}]" }, file , line , col);

        std::string thread_name = "";
        auto it = thread_names.find(thread_id);

        if (it == thread_names.end()) {
          logger->error("Logging from unregistered thread");
          thread_name = "Unknown";
        } else {
          thread_name = it->second;
        }
        std::string str = fmt::format(fmt::runtime(format) , std::forward<Args>(args)...);
        str = fmt::format(std::string_view{ "{} | {} [{}]" } , str , src_str , thread_name);

        logger->log(LevelFromLevel(l) , str);
      }

      void RegisterThread(const std::string& name); 

      void SetCorePattern(CoreTarget target , const std::string& pattern);

      void SetCoreLevel(CoreTarget target , const std::string& l);
      void SetCoreLevel(CoreTarget target , Level l);

      bool ChangeFiles(const std::string& path);

      static void Shutdown();

    private:
      Logger();
      Logger(const ConfigTable& config);
      ~Logger() {}

      Logger(const Logger&) = delete;
      Logger(Logger&&) = delete;
      Logger& operator=(Logger&&) = delete;
      Logger& operator=(const Logger&) = delete;

      static Logger* instance;
      
      constexpr static uint32_t kNumCoreTargets = 2;
      constexpr static std::array<Level , kNumCoreTargets> kCoreTargetLevels = { 
        Level::INFO , Level::TRACE 
      };
      constexpr static std::array<std::string_view , kNumCoreTargets> kTargetStrings = {
        "console" , "file"
      }; 
      constexpr static std::array<uint64_t , kNumCoreTargets> kSinkHashes = {
        FNV("CONSOLE") , FNV("FILE")
      };

      std::vector<spdlog::sink_ptr> sinks = { nullptr , nullptr };
      std::vector<spdlog::level::level_enum> sink_levels = { spdlog::level::info , spdlog::level::trace };
      std::vector<std::string> sink_patterns = { kConsoleFmt.data() , kFileFmt.data() };

      StdRef<spdlog::logger> logger;

      std::mutex thread_map_mutex;
      std::map<std::thread::id , std::string> thread_names;

      spdlog::level::level_enum LevelFromString(const std::string& l);
      spdlog::level::level_enum LevelFromLevel(Level l);
  };

} // namespace other

#endif // !OTHER_ENGINE_LOGGER_HPP
