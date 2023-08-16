//
// Created by Zero on 04/09/2022.
//

#pragma once

#include "ext/nlohmann/json.hpp"

#ifdef __cplusplus
#define VS_EXTERN_C extern "C"
#define VS_NOEXCEPT noexcept
#else
#define VS_EXTERN_C
#define VS_NOEXCEPT
#endif

#ifdef _MSC_VER
#define VS_FORCE_INLINE inline
#define VS_NEVER_INLINE __declspec(noinline)
#define VS_DLL
#define VS_EXPORT_API VS_EXTERN_C __declspec(dllexport)
#define VS_IMPORT_API VS_EXTERN_C __declspec(dllimport)
#else
#define VS_FORCE_INLINE [[gnu::always_inline, gnu::hot]] inline
#define VS_NEVER_INLINE [[gnu::noinline]]
#define VS_DLL
#define VS_EXPORT_API VS_EXTERN_C [[gnu::visibility("default")]]
#define VS_IMPORT_API VS_EXTERN_C
#endif

#define VS_MAKE_CALLABLE(func)                          \
    template<EPort p = EPort::D, typename... Args>      \
    [[nodiscard]] auto func(Args &&...args) {           \
        if constexpr (p == EPort::D) {                  \
            static Callable impl = func##_impl<p>;      \
            impl.function()->set_description(#func);    \
            return impl(OC_FORWARD(args)...);           \
        } else {                                        \
            return func##_impl<p>(OC_FORWARD(args)...); \
        }                                               \
    }

#define CALLABLE_TYPE Callable

//#define VS_MAKE_CALLABLE(func)                      \
//    template<EPort p = EPort::D, typename... Args>  \
//    [[nodiscard]] auto func(Args &&...args) {       \
//        return func##_impl<p>(OC_FORWARD(args)...); \
//    }
//
// #define CALLABLE_TYPE auto
