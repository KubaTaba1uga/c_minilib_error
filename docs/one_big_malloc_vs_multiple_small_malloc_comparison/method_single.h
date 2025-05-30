#ifndef C_MINILIB_STATIC_ERROR_H
#define C_MINILIB_STATIC_ERROR_H

#include <inttypes.h>
#include <stdio.h>

#define CME_STACK_MAX 6
#define CME_STR_MAX 256

#include "common.h"

// Because we want to hit cpu hot cache when operating on error
// error should be less than 32kib, as this is most common
// 64bit system L1 cache these days.
struct __attribute__((aligned(8))) cme_StaticError {
  uint32_t code;                 // 32/8 = 4 bytes
  char msg[CME_STR_MAX];         // 1*256 = 256 bytes
  char source_file[CME_STR_MAX]; // 1*256 = 256 bytes
  char source_func[CME_STR_MAX]; // 1*256 = 256 bytes
  uint32_t source_line;          // 32/8 = 4 bytes
#ifdef CME_ENABLE_BACKTRACE
  int32_t stack_length;                        // 32/8 = 4 bytes
  struct cme_StackSymbol stack[CME_STACK_MAX]; // 64/8 = 8 bytes
#else
  // This is not filled, we just want to keep API happy.
  uint32_t stack_length; // 32/8 = 4 bytes
  struct cme_StackSymbol *stack;
#endif
};
// If CME_ENABLED_BACKTRACE defined we have 4+256*3+8+4+4+128 = 916 bytes.
// If not we have 4+256*3+8+8+4+4+8 = 804 bytes.
// Because 916%8!=0 nor 804%8!=0 we need to align our struct to 8 bytes
//  via compiler `aligned` attribute.

typedef struct cme_StaticError *cme_static_error_t;

// Allocate and create error
cme_static_error_t cme_static_error_create(int code, const char *file,
                                           const char *func, int line,
                                           const char *fmt, ...);

// Destroy and free
void cme_static_error_destroy(cme_static_error_t err);

// Dump error to file
int cme_static_error_dump(cme_static_error_t err, const char *path);

#define cme_static_errorf(code, fmt, ...)                                      \
  cme_static_error_create((code), __FILE__, __func__, __LINE__, (fmt),         \
                          ##__VA_ARGS__)
static inline cme_static_error_t cme_static_push_frame(cme_static_error_t err,
                                                       const char *file,
                                                       const char *func,
                                                       int line) {
#ifndef CME_ENABLE_BACKTRACE
  (void)file;
  (void)func;
  (void)line;
  return err;
#else
  if (!err)
    return err;
  if (err->stack_length >= CME_STACK_MAX)
    return err;

  struct cme_StackSymbol *f = &err->stack[err->stack_length++];
  f->source_file = file;
  f->source_func = func;
  f->source_line = line;
  return err;
#endif
}

/* ── user-facing macro ───────────────────────────────────────────── */
#define cme_return(ERR)                                                        \
  cme_static_push_frame((ERR), __FILE__, __func__, __LINE__)

#endif // C_MINILIB_STATIC_ERROR_H
