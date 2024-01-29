#!/bin/sh

CC="clang"
SRCDIR="src/"
OBJDIR="out/"

CFLAGS="-ggdb"
LDFLAGS="-lm -L. -lmcfg_2"

LIB_BASENAME="mcfg_2"
LIBNAME="lib$LIB_BASENAME.a"

TEST_BIN="mcfg_test"

function build_objs() {
  COMPILED_OBJECTS=()

  for i in $1
  do
    OUTNAME="$OBJDIR$i.o"
    INNAME="$SRCDIR$i.c"
    
    echo "  CC $INNAME"

    $CC $CFLAGS -c -o $OUTNAME $INNAME || exit

    COMPILED_OBJECTS+=("${OUTNAME}")
  done
}

function build_lib() {
  OBJECTS=("mcfg mcfg_util")

  echo "==> Compiling sources for \"$LIBNAME\""
  build_objs "${OBJECTS[@]}"

  echo "==> Creating static library archive"
  echo "  AR -rc $LIBNAME ${COMPILED_OBJECTS[@]}"
  ar -rc $LIBNAME ${COMPILED_OBJECTS[@]} || exit

  echo "  RANLIB $LIBNAME"
  ranlib $LIBNAME || exit
}

function build_test() {
  OBJECTS=("main")

  echo "==> Compiling Sources for \"$TEST_BIN\""
  build_objs "${OBJECTS[@]}"

  echo "==> Linking \"$TEST_BIN\""
  echo "  LD -o $TEST_BIN ${COMPILED_OBJECTS[@]}"
  $CC $CFLAGS -o $TEST_BIN ${COMPILED_OBJECTS[@]} $LDFLAGS
}

function build_all() {
  build_lib
  build_test
}

if [ "$1" = "--lib-only" ]; then
  build_lib
else
  build_all
fi

echo "==> Finished build!"
