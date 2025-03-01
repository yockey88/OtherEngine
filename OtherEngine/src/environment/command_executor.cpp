/**
 * \file termianl/command_executor.cpp
 **/
#include "environment/command_executor.hpp"

#include "core/directory.hpp"
#include "core/filesystem.hpp"
#include "core/logger.hpp"
#include "core/platform.hpp"
// #include "thread/thread_manager.hpp"

#include "event/core_events.hpp"
#include "event/event_queue.hpp"

// #include "modules/module_registry.hpp"
#include "environment/terminal.hpp"

namespace other {

  CommandExecutor::CommandExecutor(Terminal* terminal)
      : terminal(terminal) {
    OE_ASSERT(terminal != nullptr, "Terminal is null!");
  }

  ExitCode CommandExecutor::Execute(CommandBlock& block) {
    current_block = &block;

    ExitCode ec = ExitCode::FAILURE;
    while (!block.command_queue.empty()) {
      ec = ExecuteCommand(block.command_queue.front());
      block.command_queue.pop();

      if (ec != ExitCode::SUCCESS) {
        break;
      }
    }

    current_block = nullptr;
    return ec;
  }

  ExitCode CommandExecutor::ExecuteCommand(const Command& command) {
    OE_ASSERT(terminal != nullptr, "Terminal is null!");
    OE_ASSERT(current_block != nullptr, "Current block is null!");

    current_command = &command;

    if (command.opcode == OpCode::NO_OP) {
      current_command = nullptr;
      return ExitCode::SUCCESS;
    }

    OE_ASSERT(command.num_args <= current_block->argument_queue.size(), "Argument queue size mismatch");

    /// FIXME: handle commands better, maybe have some sort of command handler class or dynamic command registration?
    ///       that would allow users to replace default handlers
    switch (CATEGORY(command.opcode)) {
      case CommandCategory::CONTROL_CMD:
        HandleControl(VALUE(command.opcode));
        break;

      case CommandCategory::FILE_CMD:
        HandleFile(VALUE(command.opcode));
        break;

      case CommandCategory::MODULE_CMD:
        HandleModule(VALUE(command.opcode));
        break;

      case CommandCategory::NET_CMD:
        HandleNet(VALUE(command.opcode));
        break;

      case CommandCategory::DEBUG_CMD:
        HandleDebug(VALUE(command.opcode));
        break;

      case CommandCategory::OTHER_CMD:
        HandleOther(VALUE(command.opcode));
        break;

      default:
        OE_ERROR("Invalid opcode [{}], command not recognized", command.opcode);
        current_command = nullptr;
        return ExitCode::FAILURE;
    }

    current_command = nullptr;
    return ExitCode::SUCCESS;
  }

  void CommandExecutor::HandleControl(uint8_t value) {
    switch (value) {
      case HELP_CMD:
        PrintHelp();
        break;

      case CLEAR_CMD:
        terminal->Clear();
        break;

      case EXIT_CMD:
        EventQueue::PushEvent<ShutdownEvent>({ ExitCode::SUCCESS });
        break;

      case CALL_CMD:
        HandleCall();
        break;

      case LUA_CALL_CMD:
        HandleLuaCall();
        break;

      default:
        PushErrorMessage(fmtstr("Invalid control command : {}", value));
        break;
    }
  }

  void CommandExecutor::HandleFile(uint8_t value) {
    switch (value) {
      case LS_CMD:
        HandleLs();
        break;

      case PWD_CMD:
        HandlePwd();
        break;

      case SOURCE_CMD:
        HandleSource();
        break;

      case MOUNT_CMD:
        HandleMount();
        break;

      default:
        PushErrorMessage(fmtstr("Invalid file command : {}", value));
        break;
    }
  }

  void CommandExecutor::HandleModule(uint8_t value) {
    switch (value) {
      case LOAD_CMD:
        HandleLoad();
        break;

        // case UNLOAD:
        // HandleUnload();
        // break;

      default:
        PushErrorMessage(fmtstr("Invalid module command : {}", value));
        break;
    }
  }

  void CommandExecutor::HandleNet(uint8_t value) {
    switch (value) {
      case LISTEN_CMD:
        HandleListen();
        break;

      case CONNECT_CMD:
        HandleConnect();
        break;

      default:
        PushErrorMessage(fmtstr("Invalid net command : {}", value));
        break;
    }
  }

  void CommandExecutor::HandleDebug(uint8_t value) {
    switch (value) {
      case ECHO_CMD:
        HandleEcho();
        break;

      default:
        PushErrorMessage(fmtstr("Invalid debug command : {}", value));
        break;
    }
  }

  void CommandExecutor::HandleOther(uint8_t value) {
    switch (value) {
      case CREATE_CMD:
        // HandleCreate();
        break;

      default:
        PushErrorMessage(fmtstr("Invalid other command : {}", value));
        break;
    }
  }

  void CommandExecutor::PrintHelp() {
    for (const auto& command : kAvailableCommands) {
      if (command.num_args.max == 0) {
        terminal->PushMessage({ TerminalFilter::INFO_FILTER, fmtstr("  - {} : {}", command.name, command.description) }, false);
      } else if (command.num_args.min == command.num_args.max) {
        terminal->PushMessage({ TerminalFilter::INFO_FILTER, fmtstr("  - {} <{}> : {}", command.name, command.meta_var_name, command.description) }, false);
      } else if (command.num_args.min == 0 && command.num_args.max == 1) {
        terminal->PushMessage({ TerminalFilter::INFO_FILTER, fmtstr("  - {} <{}>? : {}", command.name, command.meta_var_name, command.description) }, false);
      } else {
        terminal->PushMessage({ TerminalFilter::INFO_FILTER, fmtstr("  - {} <{}>... : {}", command.name, command.meta_var_name, command.description) }, false);
      }
    }
  }

  void CommandExecutor::HandleCall() {
    std::string function = GetArgument<std::string>();
    OE_ASSERT(!function.empty(), "Function argument is empty");
  }

  void CommandExecutor::HandleLuaCall() {
    std::string function = GetArgument<std::string>();
    OE_ASSERT(!function.empty(), "Function argument is empty");

    sol::state& lua_state = terminal->lua_state;
    try {
      sol::protected_function func = lua_state[function];
      if (!func.valid()) {
        PushErrorMessage(fmtstr("Failed to find lua function : {}", function));
        return;
      }

      func();
    } catch (const sol::error& e) {
      PushErrorMessage(fmtstr("Failed to call lua function : {}", e.what()));
    }
  }

  void CommandExecutor::HandleLs() {
    std::vector<Path> dirs = Filesystem::MountedDirectories();
    std::vector<Path> files = Filesystem::MountedFiles();

    terminal->PushMessage({ TerminalFilter::INFO_FILTER, "Mounted Directories : " }, false);
    for (const auto& dir : dirs) {
      terminal->PushMessage({ TerminalFilter::DEBUG_FILTER, fmtstr("  - {}", dir.string()) }, false);
    }

    terminal->PushMessage({ TerminalFilter::INFO_FILTER, "cwd :" }, false);

    using namespace std::string_literals;
    try {
      for (auto& entry : std::filesystem::directory_iterator(std::filesystem::current_path())) {
        std::string msg_str = " - "s + (entry.is_directory() ? "./" : "") + entry.path().string();
        terminal->PushMessage({ TerminalFilter::DEBUG_FILTER, msg_str }, false);
      }
    } catch (std::filesystem::filesystem_error& e) {
      terminal->PushMessage({ TerminalFilter::ERROR_FILTER, fmtstr("Failed to list files in current directory : {}", e.what()) }, false);
    } catch (...) {
      terminal->PushMessage({ TerminalFilter::ERROR_FILTER, "Unknown filesystem error" }, false);
    }
  }

  void CommandExecutor::HandlePwd() {
    terminal->PushMessage({ TerminalFilter::INFO_FILTER, "cwd : " + std::filesystem::current_path().string() }, false);
  }

  void CommandExecutor::HandleSource() {
    std::string source = GetArgument<std::string>();
    if (source.empty()) {
      return;
    }

    Path path{ source };
    if (!std::filesystem::exists(path)) {
      terminal->PushMessage({ TerminalFilter::ERROR_FILTER, "source file does not exist" }, false);
      return;
    }

    terminal->SourceFile(source);
  }

  void CommandExecutor::HandleMount() {
    std::string source = GetArgument<std::string>();
    if (source.empty()) {
      terminal->PushMessage({ TerminalFilter::ERROR_FILTER, "mount source argument is empty" });
      return;
    }

    Path path{ source };
    if (!std::filesystem::exists(path)) {
      Ref<Directory> project_root = Filesystem::GetDirectory("project-root");
      OE_ASSERT(project_root != nullptr, "Failed to get project root directory");

      path = project_root->AbsolutePath() / path;
      if (!std::filesystem::exists(path)) {
        terminal->PushMessage({ TerminalFilter::ERROR_FILTER, fmtstr("  > '{}' does not exist!", path) });
        return;
      }
    }

    if (!Filesystem::IsDirectory(path)) {
      terminal->PushMessage({ TerminalFilter::WARNING_FILTER, fmtstr(" > '{}' is not a directory", path) });
      return;
    }

    if (Filesystem::IsMounted(path)) {
      terminal->PushMessage({ TerminalFilter::WARNING_FILTER, fmtstr(" > '{}' is already mounted", path) });
      return;
    }

    Ref<Directory> dir = Filesystem::MountDirectory(path.filename().string(), path);
    if (dir == nullptr) {
      terminal->PushMessage({ TerminalFilter::ERROR_FILTER, fmtstr("Failed to mount directory : '{}'", path) });
      return;
    }

    terminal->PushMessage({ TerminalFilter::INFO_FILTER, fmtstr("Mounted directory : '{}'", path) });
  }

  void CommandExecutor::HandleLoad() {
    std::string module = GetArgument<std::string>();
    if (module.empty()) {
      terminal->PushMessage({ TerminalFilter::ERROR_FILTER, "module argument is empty" });
      return;
    }

    std::vector<std::string> path_components = Directory::SplitPath(module);

    OE_ASSERT(path_components.size() > 0, "Failed to split path");
    Ref<FileHandle> file = nullptr;
    if (path_components.size() == 1) {
      file = Filesystem::FindFileByName(path_components[0], PlatformLayer::ModuleExtension());
    } else {
      Ref<Directory> dir = Filesystem::GetDirectory(path_components[0]);
      if (dir == nullptr) {
        terminal->PushMessage({ TerminalFilter::ERROR_FILTER, fmtstr("Failed to load module [{}] could not find directory {}", module, path_components[0]) });
        return;
      }

      for (size_t i = 1; i < path_components.size() - 1; ++i) {
        Ref<Directory> child = dir->GetChildDirectory(path_components[i]);
        if (child == nullptr) {
          terminal->PushMessage({ TerminalFilter::ERROR_FILTER, fmtstr("Failed to load module [{}] could not find directory {}", module, path_components[i]) });
          return;
        }
        dir = child;
      }

      file = dir->GetFileHandleByName(path_components.back(), PlatformLayer::ModuleExtension());
    }

    if (file == nullptr) {
      terminal->PushMessage({ TerminalFilter::ERROR_FILTER, fmtstr("Failed to load module [{}] could not find file {}", module, path_components.back()) });
      return;
    }

    if (!file->Exists()) {
      terminal->PushMessage({ TerminalFilter::ERROR_FILTER, fmtstr("Failed to load module [{}] file does not exist", module) });
      return;
    }
    OE_ASSERT(file->Extension() == PlatformLayer::ModuleExtension(), "Invalid module extension");

    terminal->PushMessage({ TerminalFilter::INFO_FILTER, fmtstr("Loading module [{}]", module) });
    OE_DEBUG("Attempting to load module : {}", file->AbsolutePath());

    // Ref<Module> mod = PlatformLayer::LoadModule(file);
    // if (mod == nullptr) {
    //   terminal->PushMessage({ TerminalFilter::ERROR_FILTER, fmtstr("Failed to load module [{}]", module) });
    //   return;
    // }

    // ModuleRegistry::StoreLoadedModule(file->FileName(), mod);
    // terminal->PushMessage({ TerminalFilter::INFO_FILTER, fmtstr("Module [{}] loaded successfully", module) });
    terminal->PushMessage({ TerminalFilter::ERROR_FILTER, "Module loading not implemented" });
  }

  void CommandExecutor::HandleListen() {
    //   terminal->PushMessage({ TerminalFilter::INFO_FILTER, "Attempting to listen on port 8080" });

    //   Registers& registers = Arena::GetRegisters();
    //   {
    //     uint16_t port = GetArgument<uint16_t>();
    //     if (port == 0) {
    //       terminal->PushMessage({ TerminalFilter::ERROR_FILTER, "Invalid port number" });
    //       return;
    //     }

    //     address_t addr = registers.Write(port);
    //     ThreadManager::SendMessage("IO-Thread", { ThreadMessageType::LISTEN_COMMAND, addr });
    //   }
  }

  void CommandExecutor::HandleConnect() {
    terminal->PushMessage({ TerminalFilter::ERROR_FILTER, "Connect command not implemented" });
  }

  void CommandExecutor::HandleEcho() {
    std::string message = GetArgument<std::string>();
    if (message.empty()) {
      return;
    }

    terminal->PushMessage({ TerminalFilter::INFO_FILTER, message });
  }

  void CommandExecutor::PushErrorMessage(const std::string_view message) {
    OE_ASSERT(terminal != nullptr, "Terminal is null!");
    terminal->PushMessage({ TerminalFilter::ERROR_FILTER, std::string{ message } });
  }

}  // namespace other