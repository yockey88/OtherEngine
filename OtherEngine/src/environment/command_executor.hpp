/**
 * \file terminal/command_executor.hpp
 **/
#ifndef OTHERENV_TERMINAL_COMMAND_EXECUTOR_HPP
#define OTHERENV_TERMINAL_COMMAND_EXECUTOR_HPP

#include "core/command.hpp"
#include "core/defines.hpp"

namespace other {

  class Terminal;

  class CommandExecutor {
   public:
    CommandExecutor(Terminal* terminal);
    ~CommandExecutor() = default;

    ExitCode Execute(CommandBlock& block);

   private:
    Terminal* terminal = nullptr;

    CommandBlock* current_block = nullptr;
    const Command* current_command = nullptr;

    ExitCode ExecuteCommand(const Command& command);

    void HandleControl(uint8_t value);
    void HandleFile(uint8_t value);
    void HandleModule(uint8_t value);
    void HandleNet(uint8_t value);
    void HandleDebug(uint8_t value);
    void HandleOther(uint8_t value);

    void PrintHelp();
    void HandleCall();
    void HandleLuaCall();

    void HandleLs();
    void HandlePwd();
    void HandleSource();
    void HandleMount();

    void HandleLoad();

    void HandleListen();
    void HandleConnect();

    void HandleEcho();

    void PushErrorMessage(const std::string_view message);

    template <typename T>
    T GetArgument() {
      OE_ASSERT(current_block != nullptr, "Current block is null!");
      OE_ASSERT(!current_block->argument_queue.empty(), "Argument queue is empty!");

      address_t addr = current_block->argument_queue.front();
      current_block->argument_queue.pop();

      Registers& registers = Arena::GetRegisters();
      T value;
      {
        ValueReference val = registers.Get(addr);
        OE_ASSERT(val.value.GetType() == ValueType::STRING, "Argument is not a string");

        std::string str_val = val.value.Get<std::string>();
        try {
          if constexpr (std::is_same_v<T, std::string>) {
            value = str_val;
          } else if constexpr (std::is_same_v<T, uint8_t>) {
            value = static_cast<uint8_t>(std::stoi(str_val));
          } else if constexpr (std::is_same_v<T, uint16_t>) {
            value = static_cast<uint16_t>(std::stoi(str_val));
          } else if constexpr (std::is_same_v<T, uint32_t>) {
            value = static_cast<uint32_t>(std::stoi(str_val));
          } else if constexpr (std::is_same_v<T, uint64_t>) {
            value = static_cast<uint64_t>(std::stoull(str_val));
          } else {
            OE_ASSERT(false, "Invalid argument type");
          }
        } catch (const std::exception& e) {
          PushErrorMessage(fmtstr("Failed to parse argument : {}", e.what()));
          return T{};
        }

        registers.Clear(addr);
      }

      return value;
    }
  };

}  // namespace other

#endif  // !OTHERENV_TERMINAL_COMMAND_EXECUTOR_HPP