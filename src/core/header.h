//
// Created by Zero on 04/09/2022.
//

#pragma once

#include "ext/nlohmann/json.hpp"

using DataWrap = nlohmann::json;

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

#define VS_MAKE_CLASS(Class)                                                                      \
    VS_EXPORT_API Class *create(vision::Description *desc) {                                      \
        return ocarina::new_with_allocator<vision::BoxFilter>(dynamic_cast<Class::Desc *>(desc)); \
    }                                                                                             \
    OC_EXPORT_API void destroy(Class *filter) {                                                   \
        ocarina::delete_with_allocator(filter);                                                   \
    }