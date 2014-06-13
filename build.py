#!/usr/bin/python2.7
import os, sys

COMPILER  = "i586-pc-msdosdjgpp-gcc"
SRC_DIR   = "src"
BIN_DIR   = "bin"
BIN_NAME  = "love.exe"

CFLAGS    = ["-Ofast", "-Wall"]
DLIBS     = ["m"]
DEFINES   = ["DMT_ABORT_NULL"]


def strformat(fmt, var):
  for k in var:
    fmt = fmt.replace("{%s}" % str(k), var[k])
  return fmt


def listdir(path):
  return [os.path.join(dp, f) for dp, dn, fn in os.walk(path) for f in fn]


def main():
  os.chdir(sys.path[0])

  if not os.path.exists(BIN_DIR):
    os.makedirs(BIN_DIR)

  cfiles = filter(lambda x:x.endswith((".c", ".C")), listdir(SRC_DIR))

  cmd = strformat(
    "{compiler} {flags} {defines} -o {outfile} {srcfiles} {libs} {argv}",
    {
      "compiler"  : COMPILER,
      "flags"     : " ".join(CFLAGS),
      "defines"   : " ".join(map(lambda x: "-D " + x, DEFINES)),
      "outfile"   : BIN_DIR + "/" + BIN_NAME,
      "srcfiles"  : " ".join(cfiles),
      "libs"      : " ".join(map(lambda x: "-l" + x, DLIBS)),
      "argv"      : " ".join(sys.argv[1:])
    })

  print "compiling..."
  res = os.system(cmd)

  print("done" + (" with errors" if res else ""))



if __name__ == "__main__":
  main()
