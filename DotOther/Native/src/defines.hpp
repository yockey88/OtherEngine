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

	using nbool32 = uint32_t;
	
namespace literals {

	constexpr dostring_view operator""_sv(const wchar_t* _Str, size_t _Len) noexcept {
			return dostring_view(_Str, _Len);
	}

} // namespace literals

	enum class 
	MessageLevel {
		TRACE = 0,
		DEBUG = 1,
		INFO = 2,
		WARNING = 3,
		ERR = 4,
		CRITICAL = 5,

		MESSAGE = 6,
	};

	enum class ManagedType {
		UNKNOWN,

		SBYTE,
		BYTE,
		SHORT,
		USHORT,
		INT,
		UINT,
		LONG,
		ULONG,

		FLOAT,
		DOUBLE,

		BOOL,

		POINTER,
	};

	enum class AssemblyLoadStatus {
    SUCCESS, 
    FILE_NOT_FOUND,
    FILE_LOAD_FAILED,
    INVALID_FILE_PATH,
    INVALID_ASSEMBLY,
    UNKNOWN_ERROR,
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


namespace util { 

#ifdef DOTOTHER_WIDE_CHARS
	std::wstring CharToWide(const std::string_view str);
	std::string WideToChar(const std::wstring_view str);
#else
	std::string CharToWide(const std::string_view str);
	std::string WideToChar(const std::wstring_view str);
#endif // DOTOTHER_WIDE_CHARS

	template <typename... Args>
	static void print(dostring_view fmt, Args&&... args) {
		std::cout << fmt::format(fmt::runtime(WideToChar(fmt)), std::forward<Args>(args)...) << std::endl;
	}
	
	template <typename... Args>
	static void print(dostring_view fmt, MessageLevel level, Args&&... args) {
		if (level & MessageLevel::INFO) {
			print(fmt, std::forward<Args>(args)...);
		}
	}

	template <>
	void print(dostring_view fmt, MessageLevel level) {
		if (level & MessageLevel::INFO) {
			print(fmt);
		}
	}

	template<typename TArg>
	constexpr ManagedType GetManagedType() {
		if constexpr (std::is_pointer_v<std::remove_reference_t<TArg>>) {
			return ManagedType::POINTER;
		} else if constexpr (std::same_as<TArg, uint8_t> || std::same_as<TArg, std::byte>) {
			return ManagedType::BYTE;
		} else if constexpr (std::same_as<TArg, uint16_t>) {
			return ManagedType::USHORT;
		} else if constexpr (std::same_as<TArg, uint32_t> || (std::same_as<TArg, unsigned long> && sizeof(TArg) == 4)) {
			return ManagedType::UINT;
		} else if constexpr (std::same_as<TArg, uint64_t> || (std::same_as<TArg, unsigned long> && sizeof(TArg) == 8)) {
			return ManagedType::ULONG;
		} else if constexpr (std::same_as<TArg, char8_t>) {
			return ManagedType::SBYTE;
		} else if constexpr (std::same_as<TArg, int16_t>) {
			return ManagedType::SHORT;
		} else if constexpr (std::same_as<TArg, int32_t> || (std::same_as<TArg, long> && sizeof(TArg) == 4)) {
			return ManagedType::INT;
		} else if constexpr (std::same_as<TArg, int64_t> || (std::same_as<TArg, long> && sizeof(TArg) == 8)){
			return ManagedType::LONG;
		} else if constexpr (std::same_as<TArg, float>) {
			return ManagedType::FLOAT;
		} else if constexpr (std::same_as<TArg, double>) {
			return ManagedType::DOUBLE;
		} else if constexpr (std::same_as<TArg, bool>) {
			return ManagedType::BOOL;
		} else {
			return ManagedType::UNKNOWN;
		}
	}

	template <typename A, size_t I>
	inline void AddToArrayAt(const void** args_arr, ManagedType* param_types, A&& InArg) {
		param_types[I] = GetManagedType<A>();
		if constexpr (std::is_pointer_v<std::remove_reference_t<A>>) {
			args_arr[I] = reinterpret_cast<const void*>(InArg);
		} else {
			args_arr[I] = reinterpret_cast<const void*>(&InArg);
		}
	}

	template <typename... Args, size_t... Is>
	inline void AddToArray(const void** InArgumentsArr, ManagedType* InParameterTypes, Args&&... InArgs, const std::index_sequence<Is...>&) {
		(AddToArrayAt<Args, Is>(InArgumentsArr, InParameterTypes, std::forward<Args>(InArgs)), ...);
	}

} // namespace util
} // namespace dotother

template <>
struct fmt::formatter<dotother::MessageLevel> : public fmt::formatter<std::string_view> {
	public:
		template <typename FormatContext>
		auto format(dotother::MessageLevel level, FormatContext& ctx) {
			std::string_view name;
			switch (level) {
				case dotother::MessageLevel::TRACE:
					name = "TRACE";
					break;
				case dotother::MessageLevel::DEBUG:
					name = "DEBUG";
					break;
				case dotother::MessageLevel::INFO:
					name = "INFO";
					break;
				case dotother::MessageLevel::WARNING:
					name = "WARNING";
					break;
				case dotother::MessageLevel::ERR:
					name = "ERR";
					break;
				case dotother::MessageLevel::CRITICAL:
					name = "CRITICAL";
					break;
				case dotother::MessageLevel::MESSAGE:
					name = "MESSAGE";
					break;
			}
			return formatter<std::string_view>::format(name, ctx);
		}
};

#endif // !DOTOTHER_NATIVE_DEFINES_HPP