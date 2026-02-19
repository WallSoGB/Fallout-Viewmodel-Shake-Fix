#pragma once

#pragma warning(disable: 4100 4201 4244 4324 4389 5054)

#include <Windows.Foundation.h>
#include <wrl\wrappers\corewrappers.h>
#include <wrl\client.h>
#include <stdint.h>
#include <stdio.h>
#include <cmath>
#include <string>
#include <cassert>

#include "SafeWrite/SafeWrite.hpp"

template <typename T_Ret = void, typename ...Args>
__forceinline T_Ret ThisCall(uint32_t _addr, const void* _this, Args ...args) {
	return ((T_Ret(__thiscall*)(const void*, Args...))_addr)(_this, std::forward<Args>(args)...);
}

template <typename T_Ret = void, typename ...Args>
__forceinline T_Ret StdCall(uint32_t _addr, Args ...args) {
	return ((T_Ret(__stdcall*)(Args...))_addr)(std::forward<Args>(args)...);
}

template <typename T_Ret = void, typename ...Args>
__forceinline T_Ret CdeclCall(uint32_t _addr, Args ...args) {
	return ((T_Ret(__cdecl*)(Args...))_addr)(std::forward<Args>(args)...);
}

template <typename T_Ret = void, typename ...Args>
__forceinline T_Ret FastCall(uint32_t _addr, Args ...args) {
	return ((T_Ret(__fastcall*)(Args...))_addr)(std::forward<Args>(args)...);
}

#pragma region Macros
#define EXTERN_DLL_EXPORT extern "C" __declspec(dllexport)

#define ALIGN1	alignas(1)
#define ALIGN2	alignas(2)
#define ALIGN4	alignas(4)
#define ALIGN8	alignas(8)
#define ALIGN16 alignas(16)

// KiB from bytes
#define B_KiB(x) (x##u << 10u)

// MiB from bytes
#define B_MiB(x) (x##u << 20u)

// GiB from bytes
#define B_GiB(x) (x##u << 30u)

#ifdef NDEBUG
#define ASSUME_ASSERT(x) __assume(x)
#else
#define ASSUME_ASSERT(x) assert(x)
#endif

#define ASSERT_SIZE(a, b) static_assert(sizeof(a) == b, "Wrong structure size!");
#define ASSERT_OFFSET(a, b, c) static_assert(offsetof(a, b) == c, "Wrong member offset!");

#define CREATE_OBJECT(CLASS, ADDRESS) static CLASS* CreateObject() { return CdeclCall<CLASS*>(ADDRESS); };

#ifdef GAME 
#define FUNCTION_UNKOWN static_assert(false, "Find \"" __FUNCSIG__ "\" in the game" );
#define FUNCTION_NOT_IMPLEMENTED static_assert(false, "Implement \"" __FUNCSIG__ "\" in the game" );
#else
#define FUNCTION_UNKOWN static_assert(false, "Find \"" __FUNCSIG__ "\" in GECK" );
#define FUNCTION_NOT_IMPLEMENTED static_assert(false, "Implement \"" __FUNCSIG__ "\" in GECK" );
#endif

#define SPEC_RESTRICT		__declspec(restrict)
#define SPEC_NOINLINE		__declspec(noinline)
#define SPEC_INLINE			__forceinline
#define SPEC_NORETURN		__declspec(noreturn)
#define SPEC_NOALIAS		__declspec(noalias)

#define _HELPER_COMBINE1(X,Y) X##Y
#define _HELPER_COMBINE(X,Y) _HELPER_COMBINE1(X,Y)
#pragma endregion