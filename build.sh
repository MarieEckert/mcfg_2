#!/bin/sh

CC="clang"
SRCDIR="src/"
OBJDIR="out/"

LIB_BASENAME="mcfg_2"
LIBNAME="lib$LIB_BASENAME.a"

CFLAGS="-ggdb"
LDFLAGS="-lm -L. -l$LIB_BASENAME"

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
  echo "  LD -o $TEST_BIN ${COMPILED_OBJECTS[@]} $LDFLAGS"
  $CC $CFLAGS -o $TEST_BIN ${COMPILED_OBJECTS[@]} $LDFLAGS
}

function build_all() {
  build_lib
  build_test
}

echo "MCFG/2 build script. "
if command -v mb &> /dev/null; then
  echo "Hint: You have mariebuild installed. If the version is compatible, it is"
  echo "      recommended to use mariebuild instead. (Usage 'mb')"
fi
echo ""

if [ "$1" = "--lib-only" ]; then
  build_lib
else
  build_all
fi

echo "==> Finished build!"
