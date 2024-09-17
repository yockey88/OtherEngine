/**
 * \file defines.hpp 
 **/
#ifndef DOTOTHER_NATIVE_DEFINES_HPP
#define DOTOTHER_NATIVE_DEFINES_HPP

#include <ios>
#include <iostream>
#include <string_view>
#include <filesystem>

#include <spdlog/fmt/fmt.h>

#include "coreclr_delegates.h"
#include "hostfxr.h"
#include "nethost.h"

#ifdef DOTOTHER_WINDOWS
  #include <Windows.h>
  #include <ShlObj_core.h>
  #define DOTOTHER_CALLTYPE __cdecl

	#ifdef _WCHAR_T_DEFINED
  	#define DOTOTHER_HOSTFXR_NAME "hostfxr.dll"
		#define DO_STR(s) L##s
		#define DOTOTHER_WIDE_CHARS
	#else 
		#define DOTOTHER_HOSTFXR_NAME "hostfxr.dll"
		#define DO_STR(s) s
	#endif // _WCHAR_T_DEFINED
#else
	#define DOTOTHER_CALLTYPE
	#define DODOTOTHER_STR(s) s
	#define DO_HOSTFXR_NAME "libhostfxr.so"
#endif

#define DOTOTHER_DOTNET_TARGET_VERSION_MAJOR 8
#define DOTOTHER_DOTNET_TARGET_VERSION_MAJOR_STR '8'
#define DOTNET_UNMANAGED_FUNCTION UNMANAGEDCALLERSONLY_METHOD

namespace dotother {

	using dochar = char_t;
	using dostring_view = std::basic_string_view<dochar>;
	using dostring_t = std::basic_string<dochar>;

	using dotypeid = int32_t;
	using dohandle = int32_t;
	using dobool = uint32_t;

	enum class MessageLevel {
		Info = 1 << 0,
		Warning = 1 << 1,
		Error = 1 << 2,
		All = Info | Warning | Error
	};

	template<typename T>
	constexpr auto ValOf(T InValue) {
		return static_cast<std::underlying_type_t<T>>(InValue);
	}

	constexpr MessageLevel operator|(const MessageLevel lhs, const MessageLevel rhs) noexcept {
		return static_cast<MessageLevel>(ValOf(lhs) | ValOf(rhs));
	}

	constexpr bool operator&(const MessageLevel lhs, const MessageLevel rhs) noexcept {
		return (ValOf(lhs) & ValOf(rhs)) != 0;
	}
	
	constexpr MessageLevel operator~(const MessageLevel InValue) noexcept {
		return static_cast<MessageLevel>(~ValOf(InValue));
	}
	
	constexpr MessageLevel& operator|=(MessageLevel& lhs, const MessageLevel& rhs) noexcept {
		return (lhs = (lhs | rhs));
	}

	enum class TypeAccessibility {
		PUBLIC,
		PRIVATE,
		PROTECTED,
		INTERNAL,
		PROTECTED_PUBLIC,
		PRIVATE_PROTECTED
	};

	struct InternalCall {
		const dochar* Name;
		void* NativeFunctionPtr;
	};

	template <typename... Args>
	constexpr void print(const std::string_view fmt, Args&&... args) {
		std::cout << fmt::format(fmt, std::forward<Args>(args)...) << std::endl;
	}
	
	template <typename... Args>
	constexpr void print(const dostring_view fmt, MessageLevel level, Args&&... args) {
		if (level & MessageLevel::Info) {
			print(fmt, std::forward<Args>(args)...);
		}
	}

} // namespace dotother

#endif // !DOTOTHER_NATIVE_DEFINES_HPP