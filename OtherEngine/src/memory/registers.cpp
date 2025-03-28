/**
 * \file memory/registers.cpp
 **/
#include "memory/registers.hpp"

#include <mutex>

namespace other {

  ValueReference::ValueReference(Value& value)
      : value(value) {
    value.mutex.lock();
  }

  ValueReference::~ValueReference() {
    value.mutex.unlock();
  }

  Registers::Registers() {
    pages.reserve(kNumPages);
    /// first page is reserved for core registers
    pages.emplace_back() = NewRef<MemoryPool<Value>>(NUM_CORE_REGISTERS);
    /// second page is the 'stack' and the rest are the 'heap'
    for (size_t i = 1; i < kNumPages; i++) {
      pages.emplace_back() = NewRef<MemoryPool<Value>>(256);
    }
  }

  bool Registers::AddressValid(address_t addr) const {
    if (addr == null_address || addr.segment >= pages.size()) {
      return false;
    }

    OE_ASSERT(pages[addr.segment] != nullptr, "Page is null!");
    return addr.index < pages[addr.segment]->MaxObjects();
  }

  // address_t Registers::Push(Value& value) {
  //   OE_ASSERT(stack_cursor < pages[1]->MaxObjects(), "Stack Overflow!");
  //   address_t addr = { 1, stack_cursor++ };
  //   WriteTo(addr, value);
  //   return addr;
  // }

  // Value& Registers::Pop(address_t addr) {
  //   OE_ASSERT(AddressValid(addr), "Invalid address!");
  //   OE_ASSERT(addr.segment == 1, "Invalid page for stack!");
  //   OE_ASSERT(stack_cursor > 0, "Stack Underflow!");

  //   return (*pages[addr.segment])[addr.index];
  // }

  address_t Registers::GetCoreRegisterAddress(CoreRegisters reg) const {
    OE_ASSERT(reg < CoreRegisters::NUM_CORE_REGISTERS, "Invalid core register!");
    return { 0, static_cast<uint32_t>(reg) };
  }

  ValueReference Registers::GetCoreRegister(CoreRegisters reg) {
    address_t addr = GetCoreRegisterAddress(reg);
    return Get(addr);
  }

  void Registers::ClearCoreRegister(CoreRegisters reg) {
    address_t addr = GetCoreRegisterAddress(reg);
    Clear(addr);
  }

  void Registers::SetCoreRegister(CoreRegisters reg, Value& value) {
    address_t addr = GetCoreRegisterAddress(reg);
    WriteTo(addr, value);
  }

  address_t Registers::GetNextAddress() {
    if (current_index >= pages[page_cursor]->MaxObjects()) {
      current_index = 0;
      page_cursor = (page_cursor + 1) % pages.size();
    }
    OE_ASSERT(page_cursor < pages.size(), "Page cursor out of bounds!");
    OE_ASSERT(pages[page_cursor] != nullptr, "Page is null!");

    return { page_cursor, current_index++ };
  }

  ValueReference Registers::Get(address_t addr) {
    OE_ASSERT(AddressValid(addr), "Invalid address!");
    return { (*pages[addr.segment])[addr.index] };
  }

  address_t Registers::Set(Value& value) {
    address_t addr = GetNextAddress();
    WriteTo(addr, value);
    return addr;
  }

  void Registers::WriteTo(address_t addr, Value& value) {
    OE_ASSERT(AddressValid(addr), "Invalid address!");
    std::unique_lock lock(value.mutex);
    (*pages[addr.segment])[addr.index] = std::move(value);
  }

  void Registers::Clear(address_t addr) {
    OE_ASSERT(AddressValid(addr), "Invalid address!");
    (*pages[addr.segment])[addr.index] = Value{};
  }

}  // namespace other