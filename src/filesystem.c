/**
 * Copyright (c) 2016 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

#include "luaobj.h"
#include "lib/microtar/microtar.h"
#include "lib/dmt/dmt.h"

#include "filesystem.h"

#define MAX_MOUNTS  8
#define MAX_PATH    256

enum {
  FILESYSTEM_TNONE,
  FILESYSTEM_TREG,
  FILESYSTEM_TDIR,
};

typedef struct Mount Mount;

struct Mount {
  void (*unmount)(Mount *mnt);
  int (*exists)(Mount *mnt, const char *filename);
  int (*isFile)(Mount *mnt, const char *filename);
  int (*isDirectory)(Mount *mnt, const char *filename);
  void *(*read)(Mount *mnt, const char *filename, int *size);
  void *udata;
  char path[MAX_PATH];
};

int filesystem_mountIdx;
Mount filesystem_mounts[MAX_MOUNTS];

#define FOREACH_MOUNT(var)\
  for (Mount *var = &filesystem_mounts[filesystem_mountIdx - 1];\
       var >= filesystem_mounts;\
       var--)


static int get_file_type(const char *filename) {
  /* The use of `stat` is intentionally avoided here, a stat call seems to
   * block for a long time on DOS -- over 500ms in Dosbox at 26800 cycles */
  DIR *dir = opendir(filename);
  if (dir) {
    closedir(dir);
    return FILESYSTEM_TDIR;
  }
  FILE *fp = fopen(filename, "rb");
  if (fp) {
    fclose(fp);
    return FILESYSTEM_TREG;
  }
  return FILESYSTEM_TNONE;
}


static int concat_path(char *dst, const char *dir, const char *filename) {
  int dirlen = strlen(dir);
  int filenamelen = strlen(filename);

  /* Fail if the resultant path would overflow buffer */
  if (dirlen + filenamelen + 2 > MAX_PATH) {
    return FILESYSTEM_EFAILURE;
  }

  /* Write full name to buffer and return ok */
  if ( dir[dirlen - 1] == '/' ) {
    sprintf(dst, "%s%s", dir, filename);
  } else {
    sprintf(dst, "%s/%s", dir, filename);
  }
  return FILESYSTEM_ESUCCESS;
}


static int concat_and_get_file_type(const char *dir, const char *filename) {
  char buf[MAX_PATH];
  /* Make fullpath */
  int err = concat_path(buf, dir, filename);
  if (err) {
    return err;
  }
  /* Stat */
  return get_file_type(buf);
}


/*==================*/
/* Directory Mount  */
/*==================*/

static void dir_unmount(Mount *mnt) {
  /* Intentionally empty */
}


static int dir_exists(Mount *mnt, const char *filename) {
  return concat_and_get_file_type(mnt->path, filename) != FILESYSTEM_TNONE;
}


static int dir_isFile(Mount *mnt, const char *filename) {
  return concat_and_get_file_type(mnt->path, filename) == FILESYSTEM_TREG;
}


static int dir_isDirectory(Mount *mnt, const char *filename) {
  return concat_and_get_file_type(mnt->path, filename) == FILESYSTEM_TDIR;
}


static void* dir_read(Mount *mnt, const char *filename, int *size) {
  char buf[MAX_PATH];
  /* Make fullpath */
  int err = concat_path(buf, mnt->path, filename);
  if (err) {
    return NULL;
  }
  /* Open file */
  FILE *fp = fopen(buf, "rb");
  if (!fp) {
    return NULL;
  }
  /* Get size */
  fseek(fp, 0, SEEK_END);
  *size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  /* Load data */
  void *p = dmt_malloc(*size);
  if (!p) {
    return NULL;
  }
  fread(p, 1, *size, fp);
  return p;
}


static int dir_mount(Mount *mnt, const char *path) {
  /* Check the path is actually a directory */
  if ( get_file_type(path) != FILESYSTEM_TDIR ) {
    return FILESYSTEM_EFAILURE;
  }

  /* Init mount */
  mnt->udata = NULL;
  mnt->unmount = dir_unmount;
  mnt->exists = dir_exists;
  mnt->isFile = dir_isFile;
  mnt->isDirectory = dir_isDirectory;
  mnt->read = dir_read;

  /* Return ok */
  return FILESYSTEM_ESUCCESS;
}


/*==================*/
/* Tar Mount        */
/*==================*/

static int tar_find(mtar_t *tar, const char *filename, mtar_header_t *h) {
  int err;
  char buf[MAX_PATH + 1];
  /* Try to match */
  err = mtar_find(tar, filename, h);
  if (err != MTAR_ENOTFOUND) {
    return err;
  }
  /* Try to match with "/" at the end */
  strcpy(buf, filename);
  strcat(buf, "/");
  return mtar_find(tar, buf, h);
}


static void tar_unmount(Mount *mnt) {
  mtar_t *tar = mnt->udata;
  mtar_close(tar);
  dmt_free(tar);
}


static int tar_exists(Mount *mnt, const char *filename) {
  mtar_t *tar = mnt->udata;
  return tar_find(tar, filename, NULL) == MTAR_ESUCCESS;
}


static int tar_isFile(Mount *mnt, const char *filename) {
  mtar_t *tar = mnt->udata;
  int err;
  mtar_header_t h;
  err = mtar_find(tar, filename, &h);
  if (err) {
    return 0;
  }
  return h.type == MTAR_TREG;
}


static int tar_isDirectory(Mount *mnt, const char *filename) {
  mtar_t *tar = mnt->udata;
  int err;
  mtar_header_t h;
  err = tar_find(tar, filename, &h);
  if (err) {
    return 0;
  }
  return h.type == MTAR_TDIR;
}


static void* tar_read(Mount *mnt, const char *filename, int *size) {
  mtar_t *tar = mnt->udata;
  int err;
  mtar_header_t h;

  /* Find and load header for file */
  err = mtar_find(tar, filename, &h);
  if (err) {
    return 0;
  }

  /* Allocate and read data, set size and return */
  char *p = dmt_malloc(h.size);
  err = mtar_read_data(tar, p, h.size);
  if (err) {
    dmt_free(p);
    return NULL;
  }
  *size = h.size;
  return p;
}


typedef struct { FILE *fp; int offset; } TarStream;

static int tar_stream_read(mtar_t *tar, void *data, unsigned size) {
  TarStream *t = tar->stream;
  unsigned res = fread(data, 1, size, t->fp);
  return (res == size) ? MTAR_ESUCCESS : MTAR_EREADFAIL;
}

static int tar_stream_seek(mtar_t *tar, unsigned offset) {
  TarStream *t = tar->stream;
  int res = fseek(t->fp, t->offset + offset, SEEK_SET);
  return (res == 0) ? MTAR_ESUCCESS : MTAR_ESEEKFAIL;
}

static int tar_stream_close(mtar_t *tar) {
  TarStream *t = tar->stream;
  fclose(t->fp);
  dmt_free(t);
  return MTAR_ESUCCESS;
}


static int tar_mount(Mount *mnt, const char *path) {
  FILE *fp = NULL;
  TarStream *stream = NULL;
  mtar_t *tar = NULL;


  /* Try to open file */
  fp = fopen(path, "rb");
  if (!fp) {
    goto fail;
  }

  /* Init tar stream */
  stream = dmt_calloc(1, sizeof(*stream));
  if (!stream) {
    goto fail;
  }
  stream->fp = fp;

  /* Init tar */
  tar = dmt_calloc(1, sizeof(*tar));
  if (!tar) {
    goto fail;
  }
  tar->read = tar_stream_read;
  tar->seek = tar_stream_seek;
  tar->close = tar_stream_close;
  tar->stream = stream;

  /* Check start of file for valid tar header */
  mtar_header_t h;
  int err = mtar_read_header(tar, &h);

  /* If checking the start of the file failed then check the end of file for a
   * "TAR\0" tag and offset, this would have been added when packaging (see
   * `package.c`) to indicate the offset of the tar archive's beginning from the
   * file's end */
  if (err) {
    int offset;
    char buf[4] = "";
    fseek(fp, -8, SEEK_END);
    fread(buf, 1, 4, fp);
    fread(&offset, 1, 4, fp);
    if ( !memcmp(buf, "TAR\0", 4) ) {
      fseek(fp, -offset, SEEK_END);
      stream->offset = ftell(fp);
    }
    mtar_rewind(tar);
    err = mtar_read_header(tar, &h);
    if (err) {
      goto fail;
    }
  }

  /* Init mount */
  mnt->udata = tar;
  mnt->unmount = tar_unmount;
  mnt->exists = tar_exists;
  mnt->isFile = tar_isFile;
  mnt->isDirectory = tar_isDirectory;
  mnt->read = tar_read;

  /* Return ok */
  return FILESYSTEM_ESUCCESS;

fail:
  if (fp) fclose(fp);
  dmt_free(tar);
  dmt_free(stream);
  return FILESYSTEM_EFAILURE;
}


/*==================*/
/* Filesystem       */
/*==================*/

const char* filesystem_strerror(int err) {
  switch (err) {
    case FILESYSTEM_ESUCCESS: return "success";
    case FILESYSTEM_EFAILURE: return "failure";
  }
  return "unknown error";
}


void filesystem_deinit(void) {
  FOREACH_MOUNT(mnt) {
    mnt->unmount(mnt);
  }
  filesystem_mountIdx = 0;
}


int filesystem_mount(const char *path) {
  /* Check path length is ok */
  if ( strlen(path) >= MAX_PATH ) {
    return FILESYSTEM_EFAILURE;
  }
  /* Check path isn't already mounted */
  FOREACH_MOUNT(m) {
    if ( !strcmp(m->path, path) ) {
      return FILESYSTEM_EFAILURE;
    }
  }

  /* Get mount slot */
  if (filesystem_mountIdx >= MAX_MOUNTS) {
    return FILESYSTEM_EFAILURE;
  }
  Mount *mnt = &filesystem_mounts[filesystem_mountIdx++];

  /* Copy path name */
  strcpy(mnt->path, path);

  /* Try to mount path */
  if ( tar_mount(mnt, path) == FILESYSTEM_ESUCCESS ) goto success;
  if ( dir_mount(mnt, path) == FILESYSTEM_ESUCCESS ) goto success;

  /* Fail */
  filesystem_mountIdx--;
  return FILESYSTEM_EFAILURE;

success:
  return FILESYSTEM_ESUCCESS;
}


int filesystem_unmount(const char *path) {
  FOREACH_MOUNT(mnt) {
    if ( !strcmp(mnt->path, path) ) {
      /* Unmount */
      mnt->unmount(mnt);
      /* Shift remaining mounts to fill gap and decrement idx */
      int idx = mnt - filesystem_mounts;
      memmove(mnt, mnt + 1, (filesystem_mountIdx - idx - 1) * sizeof(Mount));
      filesystem_mountIdx--;
      return FILESYSTEM_ESUCCESS;
    }
  }
  return FILESYSTEM_EFAILURE;
}


int filesystem_exists(const char *filename) {
  FOREACH_MOUNT(mnt) {
    if ( mnt->exists(mnt, filename) ) {
      return 1;
    }
  }
  return 0;
}


int filesystem_isFile(const char *filename) {
  FOREACH_MOUNT(mnt) {
    if ( mnt->exists(mnt, filename) ) {
      return mnt->isFile(mnt, filename);
    }
  }
  return 0;
}


int filesystem_isDirectory(const char *filename) {
  FOREACH_MOUNT(mnt) {
    if ( mnt->exists(mnt, filename) ) {
      return mnt->isDirectory(mnt, filename);
    }
  }
  return 0;
}


void* filesystem_read(const char *filename, int *size) {
  FOREACH_MOUNT(mnt) {
    if ( mnt->exists(mnt, filename) && mnt->isFile(mnt, filename) ) {
      return mnt->read(mnt, filename, size);
    }
  }
  return NULL;
}


void filesystem_free(void *ptr) {
  dmt_free(ptr);
}


/*==================*/
/* Lua Binds        */
/*==================*/

int l_filesystem_mount(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  int err = filesystem_mount(path);
  if (err) {
    lua_pushnil(L);
    lua_pushstring(L, filesystem_strerror(err));
    return 2;
  }
  lua_pushboolean(L, 1);
  return 1;
}


int l_filesystem_unmount(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  int err = filesystem_unmount(path);
  if (err) {
    lua_pushnil(L);
    lua_pushstring(L, filesystem_strerror(err));
    return 2;
  }
  lua_pushboolean(L, 1);
  return 1;
}


int l_filesystem_exists(lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  lua_pushboolean( L, filesystem_exists(filename) );
  return 1;
}


int l_filesystem_isFile(lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  lua_pushboolean( L, filesystem_isFile(filename) );
  return 1;
}


int l_filesystem_isDirectory(lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  lua_pushboolean( L, filesystem_isDirectory(filename) );
  return 1;
}


int l_filesystem_read(lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  int size;
  void *data = filesystem_read(filename, &size);
  if (!data) {
    luaL_error(L, "could not read file");
  }
  lua_pushlstring(L, data, size);
  filesystem_free(data);
  return 1;
}


int luaopen_filesystem(lua_State *L) {
  luaL_Reg reg[] = {
    { "mount",        l_filesystem_mount        },
    { "unmount",      l_filesystem_unmount      },
    { "exists",       l_filesystem_exists       },
    { "isFile",       l_filesystem_isFile       },
    { "isDirectory",  l_filesystem_isDirectory  },
    { "read",         l_filesystem_read         },
    { 0, 0 },
  };
  luaL_newlib(L, reg);
  return 1;
}
