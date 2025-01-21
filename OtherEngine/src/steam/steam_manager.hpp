/**
 * \file steam/steam_manager.hpp
 **/
#ifndef OTHER_ENGINE_STEAM_MANAGER_HPP
#define OTHER_ENGINE_STEAM_MANAGER_HPP

#include "memory/arena_allocator.hpp"

namespace other {

  class SteamManager {
   public:
    static SteamManager* Instance();

    static void Initialize();
    static void Shutdown();

    static bool CheckForRestart(int32_t app_id);

    static bool SteamInitialize();
    static void SteamShutdown();

   private:
    friend class ArenaAllocator<SteamManager>;
    friend class ModuleRegistry;

    static SteamManager* instance;
    static ArenaAllocator<SteamManager> steam_manager_allocator;

    SteamManager();
    ~SteamManager();

    SteamManager(SteamManager&&) = delete;
    SteamManager(const SteamManager&) = delete;
    SteamManager& operator=(SteamManager&&) = delete;
    SteamManager& operator=(const SteamManager&) = delete;

    bool InitializeSteam();
    void ShutdownSteam();

    // static void SetSteamManagerInstance(SteamManager* manager);
  };

}  // namespace other

#endif  // !OTHER_ENGINE_STEAM_MANAGER_HPP