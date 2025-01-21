/**
 * \file steam/steam_manager.cpp
 **/
#include "steam/steam_manager.hpp"

#include <steam/steam_api.h>

#include "core/logger.hpp"

namespace other {

  SteamManager* SteamManager::instance = nullptr;
  ArenaAllocator<SteamManager> SteamManager::steam_manager_allocator;

  SteamManager* SteamManager::Instance() {
    OE_ASSERT(instance != nullptr, "SteamManager instance is null!");
    return instance;
  }

  void SteamManager::Initialize() {
    OE_ASSERT(instance == nullptr, "SteamManager instance already exists!");
    instance = steam_manager_allocator.Allocate();
    OE_ASSERT(instance != nullptr, "Failed to allocate SteamManager instance!");
  }

  void SteamManager::Shutdown() {
    OE_ASSERT(instance != nullptr, "SteamManager instance is null!");
    steam_manager_allocator.Free(instance);
    instance = nullptr;
  }

  bool SteamManager::CheckForRestart(int32_t app_id) {
    if (SteamAPI_RestartAppIfNecessary(app_id)) {
      return true;
    }
    return false;
  }

  bool SteamManager::SteamInitialize() {
    OE_ASSERT(instance != nullptr, "SteamManager instance is null!");

    SteamErrMsg err_msg = { 0 };
    switch (SteamAPI_InitEx(&err_msg)) {
      case k_ESteamAPIInitResult_OK:
        OE_DEBUG("SteamAPI_Init() succeeded");
        return true;

      case k_ESteamAPIInitResult_FailedGeneric:
        OE_ERROR("Unknown SteamAPI init failure!");
        break;
      case k_ESteamAPIInitResult_NoSteamClient:
        OE_ERROR("No Steam client found!");
        break;
      case k_ESteamAPIInitResult_VersionMismatch:
        OE_ERROR("Steam client version mismatch!");
        break;
      default:
        OE_ASSERT(false, "Unknown SteamAPI init result!");
        break;
    }
    std::string msg = err_msg;
    OE_ERROR(" > Steam Error Message : {}", msg);

    return false;
  }

  void SteamManager::SteamShutdown() {
    OE_ASSERT(instance != nullptr, "SteamManager instance is null!");
    SteamAPI_Shutdown();
  }

  SteamManager::SteamManager() {
  }

  SteamManager::~SteamManager() {
  }

  // void SteamManager::SetSteamManagerInstance(SteamManager* manager) {
  //   instance = manager;
  // }

}  // namespace other