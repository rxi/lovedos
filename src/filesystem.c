/**
 * Copyright (c) 2017 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

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

typedef struct mount_t mount_t;

struct mount_t {
  void (*unmount)(mount_t *mnt);
  int (*exists)(mount_t *mnt, const char *filename);
  int (*isFile)(mount_t *mnt, const char *filename);
  int (*isDirectory)(mount_t *mnt, const char *filename);
  void *(*read)(mount_t *mnt, const char *filename, int *size);
  void *udata;
  char path[MAX_PATH];
};

int filesystem_mountIdx;
mount_t filesystem_mounts[MAX_MOUNTS];
char filesystem_writeDir[MAX_PATH];

#define FOREACH_MOUNT(var)\
  for (mount_t *var = &filesystem_mounts[filesystem_mountIdx - 1];\
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
    return FILESYSTEM_ETOOLONG;
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


static unsigned hash_string(const char *str) {
  unsigned hash = 5381;
  while (*str) {
    hash = ((hash << 5) + hash) ^ *str++;
  }
  return hash;
}


static void strip_trailing_slash(char *str) {
  int len = strlen(str);
  if (len > 0 && str[len - 1] == '/') {
    str[len - 1] = '\0';
  }
}


static int is_separator(int chr) {
  return (chr == '/' || chr == '\\');
}


static int make_dirs(const char *path) {
  char str[MAX_PATH];
  char *p = str;
  int err = concat_path(str, path, "");
  if (err) {
    return err;
  }
  if (p[0] == '/') p++;
  if (p[0] && p[1] == ':' && p[2] == '\\') p += 3;
  while (*p) {
    if (is_separator(*p)) {
      *p = '\0';
      if (get_file_type(str) != FILESYSTEM_TDIR) {
        if (mkdir(str, S_IRWXU) == -1) {
          return FILESYSTEM_EMKDIRFAIL;
        }
      }
      *p = '/';
    }
    p++;
  }
  return FILESYSTEM_ESUCCESS;
}


/*==================*/
/* Directory mount_t  */
/*==================*/

static void dir_unmount(mount_t *mnt) {
  /* Intentionally empty */
}


static int dir_exists(mount_t *mnt, const char *filename) {
  return concat_and_get_file_type(mnt->path, filename) != FILESYSTEM_TNONE;
}


static int dir_isFile(mount_t *mnt, const char *filename) {
  return concat_and_get_file_type(mnt->path, filename) == FILESYSTEM_TREG;
}


static int dir_isDirectory(mount_t *mnt, const char *filename) {
  return concat_and_get_file_type(mnt->path, filename) == FILESYSTEM_TDIR;
}


static void* dir_read(mount_t *mnt, const char *filename, int *size) {
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
  /* Close file and return data */
  fclose(fp);
  return p;
}


static int dir_mount(mount_t *mnt, const char *path) {
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
/* Tar mount_t        */
/*==================*/

typedef struct { unsigned hash, pos; } tar_file_ref_t;

typedef struct {
  mtar_t tar;
  FILE *fp;
  int offset;
  tar_file_ref_t *map;
  int nfiles;
} tar_mount_t;


static int tar_find(mount_t *mnt, const char *filename, mtar_header_t *h) {
  /* Hash filename and linear search map for matching hash, read header and
   * check against filename if the hashes match */
  tar_mount_t *tm = mnt->udata;
  unsigned hash = hash_string(filename);
  int i;
  for (i = 0; i < tm->nfiles; i++) {

    if (tm->map[i].hash == hash) {
      /* Seek to and load header */
      mtar_seek(&tm->tar, tm->map[i].pos);
      mtar_read_header(&tm->tar, h);
      /* Compare names */
      strip_trailing_slash(h->name);
      if ( !strcmp(h->name, filename) ) {
        return FILESYSTEM_ESUCCESS;
      }
    }

  }
  return FILESYSTEM_EFAILURE;
}


static void tar_unmount(mount_t *mnt) {
  tar_mount_t *tm = mnt->udata;
  mtar_close(&tm->tar);
  dmt_free(tm->map);
  dmt_free(tm);
}


static int tar_exists(mount_t *mnt, const char *filename) {
  mtar_header_t h;
  return tar_find(mnt, filename, &h) == FILESYSTEM_ESUCCESS;
}


static int tar_isFile(mount_t *mnt, const char *filename) {
  mtar_header_t h;
  int err = tar_find(mnt, filename, &h);
  if (err) {
    return 0;
  }
  return h.type == MTAR_TREG;
}


static int tar_isDirectory(mount_t *mnt, const char *filename) {
  mtar_header_t h;
  int err = tar_find(mnt, filename, &h);
  if (err) {
    return 0;
  }
  return h.type == MTAR_TDIR;
}


static void* tar_read(mount_t *mnt, const char *filename, int *size) {
  mtar_t *tar = mnt->udata;
  int err;
  mtar_header_t h;

  /* Find and load header for file */
  err = tar_find(mnt, filename, &h);
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


static int tar_stream_read(mtar_t *tar, void *data, unsigned size) {
  tar_mount_t *tm = tar->stream;
  unsigned res = fread(data, 1, size, tm->fp);
  return (res == size) ? MTAR_ESUCCESS : MTAR_EREADFAIL;
}


static int tar_stream_seek(mtar_t *tar, unsigned offset) {
  tar_mount_t *tm = tar->stream;
  int res = fseek(tm->fp, tm->offset + offset, SEEK_SET);
  return (res == 0) ? MTAR_ESUCCESS : MTAR_ESEEKFAIL;
}


static int tar_stream_close(mtar_t *tar) {
  tar_mount_t *tm = tar->stream;
  fclose(tm->fp);
  return MTAR_ESUCCESS;
}


static int tar_mount(mount_t *mnt, const char *path) {
  tar_mount_t *tm = NULL;
  FILE *fp = NULL;

  /* Try to open file */
  fp = fopen(path, "rb");
  if (!fp) {
    goto fail;
  }

  /* Init tar_mount_t */
  tm = dmt_calloc(1, sizeof(*tm));
  tm->fp = fp;

  /* Init tar */
  mtar_t *tar = &tm->tar;
  tar->read = tar_stream_read;
  tar->seek = tar_stream_seek;
  tar->close = tar_stream_close;
  tar->stream = tm;

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
      tm->offset = ftell(fp);
    }
    mtar_rewind(tar);
    err = mtar_read_header(tar, &h);
    if (err) {
      goto fail;
    }
  }

  /* Iterate all files and store [namehash:position] pairs; this is used by
   * tar_find() */
  mtar_rewind(tar);
  int n = 0;
  int cap = 0;
  while ( (mtar_read_header(tar, &h)) == MTAR_ESUCCESS ) {
    /* Realloc if map capacity was reached */
    if (n >= cap) {
      cap = cap ? (cap << 1) : 16;
      tm->map = dmt_realloc(tm->map, cap * sizeof(*tm->map));
    }
    /* Store entry */
    strip_trailing_slash(h.name);
    tm->map[n].hash = hash_string(h.name);
    tm->map[n].pos = tar->pos;
    /* Next */
    mtar_next(tar);
    n++;
  }
  tm->nfiles = n;

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
  if (tm) {
    dmt_free(tm->map);
    dmt_free(tm);
  }
  return FILESYSTEM_EFAILURE;
}


/*==================*/
/* Filesystem       */
/*==================*/

const char* filesystem_strerror(int err) {
  switch (err) {
    case FILESYSTEM_ESUCCESS    : return "success";
    case FILESYSTEM_EFAILURE    : return "failure";
    case FILESYSTEM_ETOOLONG    : return "path too long";
    case FILESYSTEM_EMOUNTED    : return "path already mounted";
    case FILESYSTEM_ENOMOUNT    : return "path is not mounted";
    case FILESYSTEM_EMOUNTFAIL  : return "could not mount path";
    case FILESYSTEM_ENOWRITEDIR : return "no write directory set";
    case FILESYSTEM_EWRITEFAIL  : return "could not write file";
    case FILESYSTEM_EMKDIRFAIL  : return "could not make directory";
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
    return FILESYSTEM_ETOOLONG;
  }
  /* Check path isn't already mounted */
  FOREACH_MOUNT(m) {
    if ( !strcmp(m->path, path) ) {
      return FILESYSTEM_EMOUNTED;
    }
  }

  /* Get mount slot */
  if (filesystem_mountIdx >= MAX_MOUNTS) {
    return FILESYSTEM_EFAILURE;
  }
  mount_t *mnt = &filesystem_mounts[filesystem_mountIdx++];

  /* Copy path name */
  strcpy(mnt->path, path);

  /* Try to mount path */
  if ( tar_mount(mnt, path) == FILESYSTEM_ESUCCESS ) goto success;
  if ( dir_mount(mnt, path) == FILESYSTEM_ESUCCESS ) goto success;

  /* Fail */
  filesystem_mountIdx--;
  return FILESYSTEM_EMOUNTFAIL;

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
      memmove(mnt, mnt + 1, (filesystem_mountIdx - idx - 1) * sizeof(mount_t));
      filesystem_mountIdx--;
      return FILESYSTEM_ESUCCESS;
    }
  }
  return FILESYSTEM_ENOMOUNT;
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


int filesystem_setWriteDir(const char *path) {
  if (strlen(path) >= MAX_PATH) {
    return FILESYSTEM_ETOOLONG;
  }
  int err = make_dirs(path);
  if (err) {
    return err;
  }
  strcpy(filesystem_writeDir, path);
  return FILESYSTEM_ESUCCESS;
}


int filesystem_write(const char *filename, const void *data, int size) {
  int err, n;
  char buf[MAX_PATH];
  if (!*filesystem_writeDir) {
    return FILESYSTEM_ENOWRITEDIR;
  }
  err = concat_path(buf, filesystem_writeDir, filename);
  if (err) {
    return err;
  }
  FILE *fp = fopen(buf, "wb");
  if (!fp) {
    return FILESYSTEM_EWRITEFAIL;
  }
  n = fwrite(data, 1, size, fp);
  fclose(fp);
  return n == size ? FILESYSTEM_ESUCCESS : FILESYSTEM_EWRITEFAIL;
}
