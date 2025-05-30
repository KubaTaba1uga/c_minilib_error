/* dynamic.c */
#include <errno.h>

#include "common.h"
#include "method_multiple.h"

static struct cme_DynamicError generic_error = {
    .code = -1,
    .msg = "Generic dynamic error",
    .source_file = (char *)__FILE__,
    .source_func = NULL,
    .source_line = 0,
    .stack_length = 0,
    .stack = NULL,
};

#define CREATE_GENERIC_ERROR(code_, msg_)                                      \
  do {                                                                         \
    generic_error.code = (code_);                                              \
    generic_error.msg = (msg_);                                                \
    generic_error.source_func = (char *)__func__;                              \
    generic_error.source_line = __LINE__;                                      \
  } while (0)

cme_dynamic_error_t cme_dynamic_error_create(int code, const char *file,
                                             const char *func, int line,
                                             const char *fmt, ...) {
  cme_dynamic_error_t err = malloc(sizeof(*err));
  if (!err) {
    CREATE_GENERIC_ERROR(ENOMEM, "Alloc struct failed");
    return &generic_error;
  }

  err->code = code;
  err->source_line = line;
  err->source_file = strdup(file);
  err->source_func = strdup(func);
  err->stack_length = 0;
  err->stack = NULL;
#ifdef CME_ENABLE_BACKTRACE
  err->stack = malloc(CME_STACK_MAX * sizeof(struct cme_StackSymbol));
#endif

  if (fmt) {
    va_list ap;
    va_start(ap, fmt);
    int len = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);

    if (len < 0) {
      free(err);
      CREATE_GENERIC_ERROR(EINVAL, "Bad fmt args");
      return &generic_error;
    }

    err->msg = malloc(len + 1);
    if (!err->msg) {
      free(err);
      CREATE_GENERIC_ERROR(ENOMEM, "Alloc msg failed");
      return &generic_error;
    }

    va_start(ap, fmt);
    cme_format_message_va(err->msg, len + 1, fmt, ap);
    va_end(ap);
  } else {
    err->msg = strdup("No message");
  }

  return err;
}

void cme_dynamic_error_destroy(cme_dynamic_error_t err) {
  if (!err || err == &generic_error)
    return;
  free(err->msg);
  free(err->source_file);
  free(err->source_func);
#ifdef CME_ENABLE_BACKTRACE
  free(err->stack);
#endif
  free(err);
}

int cme_dynamic_error_dump(cme_dynamic_error_t err, const char *path) {
  if (!err || !path)
    return EINVAL;

  FILE *f = fopen(path, "w");
  if (!f)
    return errno;

  CME_DUMP_COMMON_FIELDS(f, err);

#ifdef CME_ENABLE_BACKTRACE
  cme_dump_backtrace(f, err->stack_length, err->stack);
#endif

  fclose(f);
  return 0;
}
