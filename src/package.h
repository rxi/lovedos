/**
 * Copyright (c) 2016 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

enum {
  PACKAGE_TTAR,
  PACKAGE_TEXE
};

enum {
  PACKAGE_ESUCCESS =  0,
  PACKAGE_EFAILURE = -1
};

void package_make(const char *indir, const char *outfile, const char *exefile, int type);
int package_run(int argc, char **argv);
