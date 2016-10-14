/**
 * Copyright (c) 2016 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

enum {
  FILESYSTEM_ESUCCESS     =  0,
  FILESYSTEM_EFAILURE     = -1,
  FILESYSTEM_ETOOLONG     = -2,
  FILESYSTEM_EMOUNTED     = -3,
  FILESYSTEM_ENOMOUNT     = -4,
  FILESYSTEM_EMOUNTFAIL   = -5,
  FILESYSTEM_ENOWRITEDIR  = -6,
  FILESYSTEM_EWRITEFAIL   = -7,
  FILESYSTEM_EMKDIRFAIL   = -8
};

const char* filesystem_strerror(int err);
void filesystem_deinit(void);
int filesystem_mount(const char *path);
int filesystem_unmount(const char *path);
int filesystem_exists(const char *filename);
int filesystem_isFile(const char *filename);
int filesystem_isDirectory(const char *filename);
void* filesystem_read(const char *filename, int *size);
void filesystem_free(void *ptr);
int filesystem_setWriteDir(const char *path);
int filesystem_write(const char *filename, const void *data, int size);

#endif
