/**
 * \file memory/registers.hpp
 **/
#ifndef OTHER_ENGINE_REGISTERS_HPP
#define OTHER_ENGINE_REGISTERS_HPP

#include <cstdint>

#include "core/ref.hpp"
#include "core/value.hpp"

#include "memory/memory_pool.hpp"


namespace other {

#pragma pack(push, 1)
  struct address_t {
    uint8_t segment = 0;
    uint32_t index = 0;

    address_t() = default;
    address_t(uint8_t segment, uint32_t index)
        : segment(segment), index(index) {}

    constexpr auto operator<=>(const address_t&) const = default;

    operator uint64_t() const {
      return (static_cast<uint64_t>(segment) << 32) | index;
    }
  };
#pragma pack(pop)

  struct ValueReference {
    ValueReference(Value& value);
    ~ValueReference();

    Value& value;
  };

  class Registers {
   public:
    Registers();

    static inline address_t null_address = { 0u, 0u };
    enum CoreRegisters {
      /// 0 is unused to allow for null_address
      CONFIG_TABLE = 1,

      WINDOW_CONTEXT,
      GPU_CONTEXT,
      IO_CONTEXT,

      RETURN_VALUE,  // RAX
      ARGUMENT_0,    // RDI
      ARGUMENT_1,    // RSI
      ARGUMENT_2,    // RDX
      ARGUMENT_3,    // RCX
      ARGUMENT_4,    // R8
      ARGUMENT_5,    // R9

      NUM_CORE_REGISTERS,
      INVALID_REGISTER = NUM_CORE_REGISTERS,
    };

    bool AddressValid(address_t addr) const;

    // address_t Push(Value& value);
    // Value Pop(address_t addr);

    template <typename T>
    address_t Write(const T& value) {
      Value v(value);
      address_t addr = GetNextAddress();
      OE_ASSERT(AddressValid(addr), "Invalid address!");
      WriteTo(addr, value);
      return addr;
    }

    template <typename T>
    address_t Write(T&& value) {
      Value v(value);
      address_t addr = GetNextAddress();
      OE_ASSERT(AddressValid(addr), "Invalid address!");
      WriteTo(addr, value);
      return addr;
    }

    address_t GetCoreRegisterAddress(CoreRegisters reg) const;
    ValueReference GetCoreRegister(CoreRegisters reg);
    void ClearCoreRegister(CoreRegisters reg);

    void SetCoreRegister(CoreRegisters reg, Value& value);

    address_t GetNextAddress();

    ValueReference Get(address_t addr);

    address_t Set(Value& value);
    void WriteTo(address_t addr, Value& value);

    template <typename T>
    void WriteTo(address_t addr, const T& value) {
      Value v(value);
      WriteTo(addr, v);
    }

    void Clear(address_t addr);

    static constexpr uint8_t kNumPages = 16;

   private:
    std::mutex mutex;

    address_t core_registers[NUM_CORE_REGISTERS];

    /// first page is reserved for core registers, the second page is the 'stack'
    uint32_t stack_cursor = 0;

    uint8_t page_cursor = 2;
    uint32_t current_index = 0;

    std::vector<Ref<MemoryPool<Value>>> pages;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_REGISTERS_HPP