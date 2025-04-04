/*
 * Copyright (c) 2025 Jakub Buczynski <KubaTaba1uga>
 * SPDX-License-Identifier: MIT
 * See LICENSE file in the project root for full license information.
 */
#ifndef C_MINILIB_ERROR_H
#define C_MINILIB_ERROR_H

struct cme_Error {
  int code;
  char *msg;
  char *source_file;
  char *source_func;
  int source_line;
  int stack_size;
  char **stack_symbols;
};

// Create error
struct cme_Error *cme_error_create(int code, char *source_file,
                                   char *source_func, int source_line,
                                   char *fmt, ...);

// Destroy error
void cme_error_destroy(struct cme_Error *);

#define cme_errorf(code, fmt, ...)                                             \
  cme_error_create((code), __FILE__, (char *)__func__, __LINE__,               \
                   (char *)(fmt), ##__VA_ARGS__)

// Dump error to file
int cme_error_dump(struct cme_Error *, char *);

#endif // C_MINILIB_CONFIG_ERROR_H
