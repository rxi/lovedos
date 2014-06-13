/**
 * Copyright (c) 2013, rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dmt.h"

#ifdef DMT_STACK_TRACE
#include <execinfo.h>
#ifndef DMT_STACK_TRACE_MAX
#define DMT_STACK_TRACE_MAX 32
#endif
#endif


typedef struct dmt_node_t {
  struct dmt_node_t *prev, *next;
  const char *file;
  size_t line;
  size_t size;
#ifdef DMT_STACK_TRACE
  void  *stacktrace[DMT_STACK_TRACE_MAX];
  size_t stacktrace_sz;
#endif
} dmt_node_t;


dmt_node_t *dmt_head;



int _dmt_has_node(dmt_node_t *n) {
  dmt_node_t *node = dmt_head;
  while (node != NULL) {
    if (node == n) return 1;
    node = node->next;
  }
  return 0;
}



void _dmt_abort(void) {
#ifdef DMT_STACK_TRACE
  void *array[DMT_STACK_TRACE_MAX];
  size_t sz = backtrace(array, DMT_STACK_TRACE_MAX);
  backtrace_symbols_fd(array, sz, fileno(stderr));
#endif
  abort();
}



void *_dmt_alloc(size_t sz, int zeroset, const char *file, unsigned line) {
  dmt_node_t *node = NULL;
  
  if (zeroset) {
    node = calloc(sizeof(*node) + sz, 1);
  } else {
    node = malloc(sizeof(*node) + sz);
    if (node != NULL) {
      memset(node, 0, sizeof(*node));
    }
  }

  if (node == NULL) {
#ifdef DMT_ABORT_NULL
    fprintf(stderr, "Couldn't allocate: %s, line %u\n", file, line);
    _dmt_abort();
#else
    return NULL;
#endif
  }

  node->line = line;
  node->file = file;
  node->size = sz;

#ifdef DMT_STACK_TRACE
  node->stacktrace_sz = backtrace(node->stacktrace, DMT_STACK_TRACE_MAX);
#endif

  if (dmt_head) {
    dmt_head->prev = node;
    node->next = dmt_head;
  }
  dmt_head = node;

  return (char*)node + sizeof(*node);
}



void *_dmt_realloc(void *ptr, size_t sz, const char *file, unsigned line) {
  dmt_node_t *node = (dmt_node_t*)((char*)ptr - sizeof(*node));
  dmt_node_t *old_node = node;

  if (ptr == NULL) return _dmt_alloc(sz, 0, file, line);

#ifndef DMT_UNSAFE
  if (!_dmt_has_node(node)) {
    fprintf(stderr, "Bad realloc: %p %s, line %u\n", ptr, file, line);
    _dmt_abort();
  }
#endif

  node = realloc(node, sizeof(*node) + sz);

  if (node == NULL) {
#ifdef DMT_ABORT_NULL
    fprintf(stderr, "Couldn't reallocate: %s, line %u\n", file, line);
    _dmt_abort();
#else
    return NULL;
#endif
  }

  node->size = sz;
  if (dmt_head == old_node) dmt_head = node;
  if (node->prev) node->prev->next = node;
  if (node->next) node->next->prev = node;

  return (char*)node + sizeof(*node);
}



void _dmt_free(void *ptr, const char *file, unsigned line) {
  dmt_node_t *node = (dmt_node_t*)((char*)ptr - sizeof(*node));

  if (ptr == NULL) return;

#ifndef DMT_UNSAFE
  if (!_dmt_has_node(node)) {
    fprintf(stderr, "Bad free: %p %s, line %u\n", ptr, file, line);
    _dmt_abort();
  }
#endif

  if (node == dmt_head) dmt_head = node->next;
  if (node->prev) node->prev->next = node->next;
  if (node->next) node->next->prev = node->prev;

  free(node);
}



void dmt_dump(FILE *fp) {
  dmt_node_t *node = dmt_head;
  size_t total = 0;

  if (!fp) fp = stdout;

  while (node != NULL) {
    fprintf(fp, "Unfreed: %p %s, line %lu (%lu bytes)\n", 
            (char*)node + sizeof(*node), node->file,
            (unsigned long)node->line, (unsigned long)node->size);

#ifdef DMT_STACK_TRACE
    backtrace_symbols_fd(node->stacktrace, node->stacktrace_sz, fileno(fp));
    fprintf(fp, "\n");
#endif

    total += node->size;
    node = node->next;
  }

  fprintf(fp, "Total unfreed: %lu bytes\n", (unsigned long)total);
}



size_t _dmt_size(void *ptr, const char* file, unsigned line) {
  dmt_node_t *node = (dmt_node_t*)((char*)ptr - sizeof(*node));

#ifndef DMT_UNSAFE
  if (!_dmt_has_node(node)) {
    fprintf(stderr, "Bad pointer: %p %s, line %u\n", ptr, file, line);
    _dmt_abort();
  }
#endif

  return node->size;
}



size_t dmt_usage(void) {
  dmt_node_t *node = dmt_head;
  size_t total = 0;

  while (node != NULL) {
    total += node->size;
    node = node->next;
  } 

  return total;
}



int dmt_has(void *ptr) {
  dmt_node_t *node = (dmt_node_t*)((char*)ptr - sizeof(*node));
  return _dmt_has_node(node);
}
