/**
 * \file environment/command_executor.cpp
 **/
#include "environment/command_executor.hpp"

#include "core/logger.hpp"
#include "application/app_state.hpp"
#include "environment/command.hpp"

namespace other {

  ExitCode Instructions::Execute(Executor& exe) {
    ExitCode ec = ExitCode::SUCCESS;
    for (auto& [hash,b] : blocks) {
      ec = Execute(exe, b);
      if (ec == ExitCode::FAILURE) {
        return ec;
      }
    } 

    return ec;
  }
    
  ExitCode Instructions::Execute(Executor& exe, const uint64_t block_hash) {
    auto itr = blocks.find(block_hash);
    if (itr == blocks.end()) {
      OE_ERROR("Failed to execute block : [{}]", block_hash); 
      return ExitCode::FAILURE;
    }

    auto [_,block] = *itr;
    return Execute(exe, block);
  }

  ExitCode Instructions::Execute(Executor& exe, const std::string_view block_name) {
    return Execute(exe, FNV(block_name));
  }
    
  ExitCode Instructions::Execute(Executor& exe, CommandBlock& block) {
    if (block.command_queue.empty()) {
      return ExitCode::SUCCESS;
    }

    try {
      do {
        Command c = block.command_queue.front();
        block.command_queue.pop();
        switch (c.command) {
          case CommandType::LOAD_COMMAND: {
            if (block.command_queue.empty()) {
              OE_ERROR("load command missing argument");
              return ExitCode::FAILURE;
            }

            Command arg = block.command_queue.front();
            address_t address = arg.argument_address;
            exe.Execute(c.command, c.category, c.priority, address);
          } break;

          case CommandType::SAVE_COMMAND:
            OE_WARN("save command unimplemented");
            return ExitCode::FAILURE;

          case CommandType::CREATE_COMMAND:
            OE_WARN("create command unimplemented");
            return ExitCode::FAILURE;

          case CommandType::DELETE_COMMAND:
            OE_WARN("delte command unimplemented");
            return ExitCode::FAILURE;

          case CommandType::NO_OP_COMMAND:
          default:
            break;
        }
      } while (!block.command_queue.empty());
    } catch (ExecutorError error) {
      OE_DEBUG("Failed to execute block {} : error [ {} ]", block.name, error.type);
      return ExitCode::FAILURE;
    } catch (...) {
      OE_ERROR("Block {} failed to execute", block.name);
      return ExitCode::FAILURE;
    }

    return ExitCode::SUCCESS;
  }

  ExitCode Executor::Execute() {
    return instruction_set.Execute(*this);
  }

  ExitCode Executor::Execute(const uint64_t hash) {
    return instruction_set.Execute(*this, hash);
  }

  ExitCode Executor::Execute(const std::string_view block) {
    return instruction_set.Execute(*this, block);
  }
      
  ExitCode Executor::Execute(CommandBlock& block) {
    return instruction_set.Execute(*this, block);
  }
      
  void Executor::Execute(CommandType command, CommandCategory category, CommandPriority prio, Opt<address_t> address) {
    if (address.has_value()) {
      OE_DEBUG("[{}:{}] : {}", category, command, *address);
    }

    switch (category) {
      case CommandCategory::SCENE_COMMAND:
        HandleSceneCommand(command, address);
        break;

      case CommandCategory::PROJECT_COMMAND:
        OE_WARN("project command unimplemented");
        break;

      case CommandCategory::INVALID_COMMAND:
      case CommandCategory::EMPTY_COMMAND:
      default:
        break;
    }
  }
      
  void Executor::HandleSceneCommand(CommandType command, Opt<address_t> address) {
    switch (command) {
      case CommandType::LOAD_COMMAND: {
        if (!address.has_value()) {
          throw ExecutorError(COMMAND_MISSING_ARGUMENT);
        }
        if (memory.CheckValueType(*address) != ValueType::STRING) {
          throw ExecutorError(COMMAND_INVALID_ARGUMENT);
        }
        
        std::string name = memory.GetString(*address);
        AppState::Scenes()->LoadScene(name);
      } break;

      default:
        break;
    }
  }

} // namespace other
