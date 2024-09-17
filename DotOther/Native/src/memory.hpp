/**
 * \file memory.hpp 
 **/
#ifndef DOTOTHER_NATIVE_MEMORY_HPP
#define DOTOTHER_NATIVE_MEMORY_HPP

namespace dotother {

class Memory {
  public:
	  static void* AllocHGlobal(size_t size);
	  static void FreeHGlobal(void* ptr);

// 	  static wchar_t* StringToCoTaskMemAuto(std::string_view InString);

	  static void FreeCoTaskMem(void* handle);
};

} // namespace dotother

#endif // !DOTOTHER_NATIVE_MEMORY_HPP