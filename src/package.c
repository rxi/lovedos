/**
 * Copyright (c) 2016 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdarg.h>

#include "lib/microtar/microtar.h"

#include "package.h"


static void error(const char *fmt, ...) {
  va_list argp;
  printf("Package error: ");
  va_start(argp, fmt);
  vprintf(fmt, argp);
  va_end(argp);
  printf("\n");
  exit(EXIT_FAILURE);
}


static int tar_stream_write(mtar_t *tar, const void *data, unsigned size) {
  unsigned res = fwrite(data, 1, size, tar->stream);
  if (res != size) {
    error("failed when writing to output file");
  }
  return MTAR_ESUCCESS;
}


static void concat(char *dst, int dstsz, ...) {
  const char *s;
  va_list argp;
  int i = 0;
  va_start(argp, dstsz);
  while ( (s = va_arg(argp, const char*)) ) {
    while (*s) {
      dst[i++] = *s++;
      if (i == dstsz) {
        error("string length exceeds destination buffer");
      }
    }
  }
  dst[i] = '\0';
  va_end(argp);
}


static
void concat_path(char *dst, int dstsz, const char *dir, const char *filename) {
  int dirlen = strlen(dir);
  if ( dir[dirlen - 1] == '/' || *dir == '\0' ) {
    concat(dst, dstsz, dir, filename, NULL);
  } else {
    concat(dst, dstsz, dir, "/", filename, NULL);
  }
}


static void write_file(mtar_t *tar, const char *inname, const char *outname) {
  FILE *fp = fopen(inname, "rb");
  if (!fp) {
    error("couldn't open input file '%s'", inname);
  }

  /* Get size */
  fseek(fp, 0, SEEK_END);
  int size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  /* Write header */
  mtar_header_t h;
  memset(&h, 0, sizeof(h));
  concat(h.name, sizeof(h.name), outname, NULL);
  h.size = size;
  h.mode = 0664;
  mtar_write_header(tar, &h);

  /* Write file */
  int chr;
  while ( (chr = fgetc(fp)) != EOF ) {
    unsigned char byte = chr;
    mtar_write_data(tar, &byte, 1);
  }

  /* Close file and return ok */
  fclose(fp);
}


static void write_dir(mtar_t *tar, const char *indir, const char *outdir) {
  char inbuf[256];
  char outbuf[256];
  struct dirent *ep;

  DIR *dir = opendir(indir);
  if (!dir) {
    error("couldn't open input dir '%s'", indir);
  }

  /* Write dir */
  if (*outdir) {
    mtar_header_t h;
    memset(&h, 0, sizeof(h));
    concat(h.name, sizeof(h.name), outdir, NULL);
    h.type = MTAR_TDIR;
    h.mode = 0775;
    mtar_write_header(tar, &h);
  }

  /* Write files */
  while ( (ep = readdir(dir)) ) {
    /* Skip `.` and `..` */
    if (!strcmp(ep->d_name, ".") || !strcmp(ep->d_name, "..")) {
      continue;
    }
    /* Get full input name and full output name */
    concat_path(inbuf, sizeof(inbuf), indir, ep->d_name);
    concat_path(outbuf, sizeof(outbuf), outdir, ep->d_name);
    /* Write */
    DIR *d = opendir(inbuf);
    if (d) {
      closedir(d);
      write_dir(tar, inbuf, outbuf);
    } else {
      write_file(tar, inbuf, outbuf);
    }
  }

  closedir(dir);
}


void package_make(const char *indir, const char *outfile, const char *exefile, int type) {
  /* Open file */
  FILE *fp = fopen(outfile, "wb");
  if (!fp) {
    error("couldn't open output file");
  }

  /* Copy .exe to file if exe type is set */
  if (type == PACKAGE_TEXE) {
    FILE *exefp = fopen(exefile, "rb");
    if (!exefp) {
      error("couldn't open .exe file");
    }
    int chr;
    while ( (chr = fgetc(exefp)) != EOF ) {
      fputc(chr, fp);
    }
    fclose(exefp);
  }

  /* Get start of file (used for offset) */
  int start = ftell(fp);

  /* Init tar writer */
  mtar_t tar;
  memset(&tar, 0, sizeof(tar));
  tar.write = tar_stream_write;
  tar.stream = fp;

  /* Write package data to file and finalize tar */
  write_dir(&tar, indir, "");
  mtar_finalize(&tar);

  /* Write "TAR\0" and offset int so we can find the .tar if it was appended
   * onto the end of the .exe */
  int offset = ftell(fp) - start + 8;
  fwrite("TAR\0", 1, 4, fp);
  fwrite(&offset, 1, 4, fp);

  /* Done */
  fclose(fp);
}


int package_run(int argc, char **argv) {
  /* Check for `--pack` argument; return failure if it isn't present */
  if (argc < 2) {
    return PACKAGE_EFAILURE;
  }
  if ( strcmp(argv[1], "--pack") != 0 && strcmp(argv[1], "--PACK") != 0 ) {
    return PACKAGE_EFAILURE;
  }

  /* Check arguments */
  if (argc < 4) {
    error("expected arguments: %s DIRNAME OUTFILE", argv[1]);
  }

  /* Set package type based on file extension */
  int type = PACKAGE_TTAR;
  if ( strstr(argv[3], ".exe") || strstr(argv[3], ".EXE") ) {
    type = PACKAGE_TEXE;
  }

  /* Make package and return success*/
  package_make(argv[2], argv[3], argv[0], type);
  return PACKAGE_ESUCCESS;
}
