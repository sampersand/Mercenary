// courtesy of NeGate

#pragma once

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef dyn_array_throw
#define dyn_array_throw(msg, file, line) \
  { abort(); }
#endif

#ifndef arr_err_none
#define arr_err_none 0
#endif

#ifndef arr_err_null_ptr
#define arr_err_null_ptr 1
#endif

#ifndef arr_err_size_mismatch
#define arr_err_size_mismatch 2
#endif

#ifndef arr_err_out_of_mem
#define arr_err_out_of_mem 3
#endif

#ifndef arr_err_out_of_bounds
#define arr_err_out_of_bounds 4
#endif

#ifndef arr_default_cap
#define arr_default_cap 16
#endif

#define slice_decl(type) \
  struct {               \
    size_t length;       \
    struct type* data;   \
  }

#define arr_decl(name, ...) \
  struct name##_struct {    \
    size_t cap;             \
    size_t siz;             \
    __VA_ARGS__ dat[];      \
  };
#define arr_forward_decl(name) \
  struct name##_struct;        \
  typedef struct name##_struct* name;

// #define arr_decl(...) struct { size_t cap; size_t siz; __VA_ARGS__ dat[]; }*
arr_forward_decl(arr_unit_array) arr_decl(arr_unit_array, char)
#define arr_data_size(name) (((name)->siz) * (sizeof(*(name)->dat)))
#define arr_elem_size(name) (sizeof(*(name)->dat))
#define arr_get_cap(name) ((name)->cap)
#define arr_get_size(name) ((name)->siz)
#define arr_get_data(name) ((name)->dat)
#define arr_clear(name) \
  { (name)->siz = 0; }
#define arr_clone(dst, name)                            \
  ({                                                    \
    dst = malloc(sizeof(*name) + arr_data_size(name));  \
    dst->siz = dst->cap = (name)->siz;                  \
    memcpy(dst->dat, (name)->dat, arr_data_size(name)); \
  })
#define arr_copy(dst, src)                              \
  do {                                                  \
    if ((dst)->siz != (src)->siz)                       \
      dyn_array_throw(arr_err_size_mismatch, __FILE__, __LINE__); \
    memcpy((dst)->dat, (src)->dat, arr_data_size(src)); \
  } while (0)
#define arr_shrink(arr)                                    \
  do {                                                     \
    arr = realloc(arr, sizeof(*arr) + arr_data_size(arr)); \
    arr->cap = arr->siz;                                   \
  } while (0)
#define arr_free(name) \
  {                    \
    free(name);        \
    name = NULL;       \
  }

#define arr_for(it, name)                       \
  for (typeof((name)->dat[0])*it = (name)->dat, \
      *end__ = (name)->dat + (name)->siz;       \
       it != end__; ++it)

    inline static size_t
    ACB__(size_t idx, size_t limit, const char* file, int line) {
  if (idx >= limit) {
    dyn_array_throw(array_err_out_of_bounds, file, line);
  }
  return idx;
}

#ifdef arr_bounds_checking
#define arr_at(name, at) \
  (((name)->dat[ACB__(at, (name)->siz, __FILE__, __LINE__)]))
#else
#define arr_at(name, at) (((name)->dat[(size_t)at]))
#endif

#ifdef _DEBUG
#define arr_chk_alloc(new_arr, arr)                \
  if (new_arr == NULL) {                           \
    free(arr);                                     \
    dyn_array_throw(arr_err_out_of_mem, __FILE__, __LINE__); \
  }
#else
#define arr_chk_alloc(new_arr, arr) assert(new_arr)
#endif

#define arr_alloc(arr)                                                      \
  do {                                                                      \
    (arr) =                                                                 \
        malloc(sizeof(*(arr)) + (arr_default_cap * sizeof((arr)->dat[0]))); \
    (arr)->cap = arr_default_cap;                                           \
    (arr)->siz = 0;                                                         \
  } while (0)

#define arr_append(arr)                                                      \
  (*({                                                                       \
    (arr) = arr_reserve_cap__((arr_unit_array)(arr), sizeof((arr)->dat[0])); \
    &(arr)->dat[(arr)->siz++];                                               \
  }))

#define arr_indexof(arr, val)                                            \
  ({                                                                     \
    typeof(arr->dat[0]) search_val = (val);                              \
    arr_search__((arr_unit_array)arr, &search_val, sizeof(arr->dat[0])); \
  })

// typedef char[__builtin_types_compatible_p(typeof(dst->dat[0]),
// typeof(src->dat[0])) ? 1 : -1] type_check_test__;

#define arr_pop(arr)                                                       \
  do {                                                                     \
    if ((arr)->siz == 0) dyn_array_throw(arr_err_out_of_bounds, __FILE__, __LINE__); \
    arr->siz--;                                                            \
  } while (0)

#define arr_remove(name, at)                                                 \
  do {                                                                       \
    size_t idx = (at);                                                       \
    size_t* deref = (size_t*)(name);                                         \
    if (deref == 0) dyn_array_throw(array_err_null_ptr, __FILE__, __LINE__);           \
    if (idx >= deref[1]) dyn_array_throw(array_err_out_of_bounds, __FILE__, __LINE__); \
                                                                             \
    if (deref[1] > 1) {                                                      \
      memcpy(((char*)&deref[2]) + (arr_elem_size(name) * idx),               \
             ((char*)&deref[2]) + (arr_elem_size(name) * (deref[1] - 1)),    \
             arr_elem_size(name));                                           \
    }                                                                        \
    deref[1]--;                                                              \
  } while (0)

inline static void* arr_reserve_cap__(arr_unit_array arr, size_t elem_size) {
  if (arr->siz + 1 >= arr->cap) {
    arr->cap = (arr->siz + 1) * 2;
    void* new_arr = realloc(arr, sizeof(*arr) + (arr->cap * elem_size));
    arr_chk_alloc(new_arr, arr);
    return new_arr;
  }
  return (void*)arr;
}

inline static size_t arr_search__(arr_unit_array arr, const void* baseline,
                                  size_t elem_size) {
  for (size_t i = 0; i < arr->siz; i++) {
    if (memcmp(&arr->dat[i * elem_size], baseline, elem_size) == 0) return i;
  }

  return SIZE_MAX;
}