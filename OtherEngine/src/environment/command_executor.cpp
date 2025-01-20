/**
 * \file environment/command_executor.cpp
 **/
#include "environment/command_executor.hpp"

#include <winscard.h>

#include "core/filesystem.hpp"
#include "core/logger.hpp"
#include "environment/command.hpp"
#include "environment/environment.hpp"

#include "application/app_state.hpp"
#include "event/core_events.hpp"
#include "event/event_queue.hpp"

namespace other {

  ExitCode Executor::Execute(const CommandBlock& bl) {
    PROFILE_SECTION("Executor--Execute(CommandBlock)");
    block = bl;

    if (block->command_queue.empty()) {
      return ExitCode::SUCCESS;
    }

    do {
      try {
        Command c = block->command_queue.front();
        block->command_queue.pop();

        ExitCode ec = Execute(c);
        if (ec != ExitCode::SUCCESS) {
          return ec;
        }
      } catch (ExecutorError& error) {
        OE_ERROR("Failed to execute block {} : error [ {} ]", block->name, error.type);
        return ExitCode::FAILURE;
      } catch (...) {
        OE_ERROR("Block {} failed to execute", block->name);
        return ExitCode::FAILURE;
      }
    } while (!block->command_queue.empty());

    return ExitCode::SUCCESS;
  }

  ExitCode Executor::Execute(const Command& command) {
    PROFILE_SECTION("Executor--Execute(Command)");
    switch (command.command) {
      case CommandType::CLEAR_COMMAND:
        return HandleClear(command);
      case CommandType::EXIT_COMMAND:
        return HandleExit(command);

      case CommandType::CREATE_COMMAND:
        OE_WARN("create command unimplemented");
        return ExitCode::FAILURE;

      case CommandType::DELETE_COMMAND:
        OE_WARN("delete command unimplemented");
        return ExitCode::FAILURE;

      case CommandType::LOAD_COMMAND:
        return HandleLoad(command);

      case CommandType::UNLOAD_COMMAND:
        return HandleUnload(command);

      case CommandType::SAVE_COMMAND:
        OE_WARN("save command unimplemented");
        return ExitCode::FAILURE;

      case CommandType::NO_OP_COMMAND:
        return ExitCode::SUCCESS;

      default:
        OE_ERROR("Unknown command category : {}", command.category);
        throw ExecutorError(ExecutorErrorType::UNKNOWN_COMMAND_CATEGORY);
    }
  }

  ExitCode Executor::HandleLoad(const Command& command) {
    OE_TRACE("Handling LOAD command");

    if (command.num_args == 0) {
      OE_ERROR("Command {} requires an argument", command.command);
      throw ExecutorError(ExecutorErrorType::COMMAND_MISSING_ARGUMENT);
    }

    OE_ASSERT(block->command_queue.size() > 0, "Command queue is empty");

    Command address_command = block->command_queue.front();
    block->command_queue.pop();

    address_t address = address_command.argument_address;
    OE_TRACE(" > argument @[{}]", address);

    switch (command.category) {
      case CommandCategory::SCENE_COMMAND: {
        std::string scene_name = memory.GetString(address);
        OE_TRACE(" > loading scene : {}", scene_name);

        Ref<Directory> scene_dir = Filesystem::GetDirectory("scenes");
        OE_ASSERT(scene_dir != nullptr, "Failed to get scene directory!");

        Ref<FileHandle> scene_file = scene_dir->GetFileHandleByName(scene_name);
        if (scene_file == nullptr) {
          OE_ERROR("Failed to find scene : {}", scene_name);
          return ExitCode::FAILURE;
        } else if (!scene_file->Exists()) {
          OE_ERROR("Scene file does not exist : {}", scene_name);
          return ExitCode::FAILURE;
        }

        Ref<Scene> scene = AppState::Assets()->GetAsset(scene_file->handle, AssetType::SCENE);
        if (scene == nullptr) {
          return ExitCode::FAILURE;
        }

        AppState::Scenes()->Activate(scene);
        OE_TRACE(" > loaded scene : {}", scene_name);
        return ExitCode::SUCCESS;
      } break;

      default:
        OE_ERROR("Command {} does not apply to [ {} ]", command.command, command.category);
        throw ExecutorError(ExecutorErrorType::COMMAND_DOES_NOT_APPLY);
    }
  }

  ExitCode Executor::HandleUnload(const Command& command) {
    OE_TRACE("Handling UNLOAD command");

    if (command.num_args == 0) {
      switch (command.category) {
        case CommandCategory::SCENE_COMMAND:
          if (AppState::Scenes()->HasActiveScene()) {
            AppState::Scenes()->UnloadActive();
          } else {
            OE_WARN("No active scene to unload");
          }
          return ExitCode::SUCCESS;

        default:
          OE_ERROR("Command {} does not apply to [ {} ]", command.command, command.category);
          throw ExecutorError(ExecutorErrorType::COMMAND_DOES_NOT_APPLY);
      }
    }

    if (block->command_queue.empty()) {
      throw ExecutorError(ExecutorErrorType::COMMAND_MISSING_ARGUMENT);
    }

    Command address_command = block->command_queue.front();
    block->command_queue.pop();

    address_t address = address_command.argument_address;
    OE_TRACE(" > argument @[{}]", address);

    switch (command.category) {
      default:
        OE_ERROR("Command {} does not apply to [ {} ]", command.command, command.category);
        throw ExecutorError(ExecutorErrorType::COMMAND_DOES_NOT_APPLY);
    }

    return ExitCode::SUCCESS;
  }

  ExitCode Executor::HandleClear(const Command& command) {
    OE_TRACE("Handling CLEAR command");
    Environment& env = Environment::Get();
    env.terminal.terminal_history.clear();
    return ExitCode::SUCCESS;
  }

  ExitCode Executor::HandleExit(const Command& command) {
    OE_TRACE("Handling EXIT command");
    EventQueue::PushEvent<ShutdownEvent>({ ExitCode::SUCCESS });
    return ExitCode::SUCCESS;
  }

}  // namespace other
