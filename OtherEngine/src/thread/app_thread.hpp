/**
 * \file thread/app_thread.hpp
 **/
#ifndef OTHER_ENGINE_APP_THREAD_HPP
#define OTHER_ENGINE_APP_THREAD_HPP

namespace other {

  class AppThread {
    public:
      AppThread() {}
      ~AppThread() {}

      void Launch();
      void Join();

    private:
      void Run();
  };

} // namespace other

#endif // !OTHER_ENGINE_APP_THREAD_HPP
