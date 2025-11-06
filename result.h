#pragma once

#include <stdio.h>
#include <stdlib.h>

#define __RESULT_EAT_PARENS(...) __VA_ARGS__

typedef enum ResultKind
{
    OK,
    ERR
} ResultKind;

#define DEFINE_RESULT(ok_type, err_type)         \
    typedef struct Result_##ok_type##_##err_type \
    {                                            \
        ResultKind kind;                         \
        union                                    \
        {                                        \
            ok_type ok;                          \
            err_type err;                        \
        } value;                                 \
    } Result_##ok_type##_##err_type;

#define __RESULT_OK_PASTE(ok_type, err_type, ...) \
    (Result_##ok_type##_##err_type)               \
    {                                             \
        .kind = OK, .value = {.ok = __VA_ARGS__ } \
    }

#define __RESULT_OK_IMPL(ok_type, err_type, ...) \
    __RESULT_OK_PASTE(ok_type, err_type, __VA_ARGS__)

#define __RESULT_ERR_PASTE(ok_type, err_type, ...)  \
    (Result_##ok_type##_##err_type)                 \
    {                                               \
        .kind = ERR, .value = {.err = __VA_ARGS__ } \
    }

#define __RESULT_ERR_IMPL(ok_type, err_type, ...) \
    __RESULT_ERR_PASTE(ok_type, err_type, __VA_ARGS__)

#define Ok(types, ...) \
    __RESULT_OK_IMPL(__RESULT_EAT_PARENS types, (__VA_ARGS__))

#define Err(types, ...) \
    __RESULT_ERR_IMPL(__RESULT_EAT_PARENS types, (__VA_ARGS__))

#define is_ok(res) \
    ((res).kind == OK)

#define is_err(res) \
    ((res).kind == ERR)

#define expect(res, msg)                                                                                        \
    ({                                                                                                          \
        __auto_type __res_tmp = (res);                                                                          \
        (is_err(__res_tmp))                                                                                     \
            ? (fprintf(stderr, "Panic at %s:%d: %s\n", __FILE__, __LINE__, (msg)), exit(1), __res_tmp.value.ok) \
            : __res_tmp.value.ok;                                                                               \
    })

#define expect_err(res, msg)                                                                                     \
    ({                                                                                                           \
        __auto_type __res_tmp = (res);                                                                           \
        (is_ok(__res_tmp))                                                                                       \
            ? (fprintf(stderr, "Panic at %s:%d: %s\n", __FILE__, __LINE__, (msg)), exit(1), __res_tmp.value.err) \
            : __res_tmp.value.err;                                                                               \
    })

#define unwrap_or(res, default_val)                            \
    ({                                                         \
        __auto_type __res_tmp = (res);                         \
        is_ok(__res_tmp) ? __res_tmp.value.ok : (default_val); \
    })

#define unwrap_or_else(res, func)                         \
    ({                                                    \
        __auto_type __res_tmp = (res);                    \
        is_ok(__res_tmp) ? __res_tmp.value.ok : (func)(); \
    })

#define __RESULT_MAP_PASTE(out_ok_type, err_type, res_in, var, ...)         \
    ({                                                                      \
        __auto_type __res_tmp = (res_in);                                   \
        (is_err(__res_tmp))                                                 \
            ? __RESULT_ERR_IMPL(out_ok_type, err_type, __res_tmp.value.err) \
            : ({                                                            \
                  typeof(__res_tmp.value.ok) var = __res_tmp.value.ok;      \
                  __RESULT_OK_IMPL(out_ok_type, err_type, __VA_ARGS__);     \
              });                                                           \
    })

#define __RESULT_MAP_IMPL(out_ok_type, err_type, res_in, var, ...) \
    __RESULT_MAP_PASTE(out_ok_type, err_type, res_in, var, __VA_ARGS__)

#define map(types, res_in, var, ...) \
    __RESULT_MAP_IMPL(__RESULT_EAT_PARENS types, res_in, var, (__VA_ARGS__))

#define __RESULT_MAP_ERR_PASTE(ok_type, out_err_type, res_in, var, ...)   \
    ({                                                                    \
        __auto_type __res_tmp = (res_in);                                 \
        (is_ok(__res_tmp))                                                \
            ? __RESULT_OK_IMPL(ok_type, out_err_type, __res_tmp.value.ok) \
            : ({                                                          \
                  typeof(__res_tmp.value.err) var = __res_tmp.value.err;  \
                  __RESULT_ERR_IMPL(ok_type, out_err_type, __VA_ARGS__);  \
              });                                                         \
    })

#define __RESULT_MAP_ERR_IMPL(ok_type, out_err_type, res_in, var, ...) \
    __RESULT_MAP_ERR_PASTE(ok_type, out_err_type, res_in, var, __VA_ARGS__)

#define map_err(types, res_in, var, ...) \
    __RESULT_MAP_ERR_IMPL(__RESULT_EAT_PARENS types, res_in, var, (__VA_ARGS__))

#define __RESULT_AND_THEN_PASTE(out_ok_type, err_type, res_in, var, ...)    \
    ({                                                                      \
        __auto_type __res_tmp = (res_in);                                   \
        (is_err(__res_tmp))                                                 \
            ? __RESULT_ERR_IMPL(out_ok_type, err_type, __res_tmp.value.err) \
            : ({                                                            \
                  typeof(__res_tmp.value.ok) var = __res_tmp.value.ok;      \
                  __VA_ARGS__;                                              \
              });                                                           \
    })

#define __RESULT_AND_THEN_IMPL(out_ok_type, err_type, res_in, var, ...) \
    __RESULT_AND_THEN_PASTE(out_ok_type, err_type, res_in, var, __VA_ARGS__)

#define __RESULT_AND_THEN_CALL(...) __RESULT_AND_THEN_IMPL(__VA_ARGS__)

#define and_then(types, res_in, var, ...) \
    __RESULT_AND_THEN_CALL(__RESULT_EAT_PARENS types, res_in, var, (__VA_ARGS__))