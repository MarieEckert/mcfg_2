#!/bin/bash

# MCFG/2 testing script

CC="clang"
CFLAGS="-std=gnu17 -gdwarf-4 -Wextra -Wall -Iinclude/ -Isrc/"
LDFLAGS="-lm -L. -lmcfg_2"

TESTS="tests/src/parse.c tests/src/serialize.c"

err() {
    printf "\x1b[1m\x1b[31m==>\x1b[0m\x1b[1m $1\x1b[0m\n"
    return 1;
}

info() {
    printf "\x1b[1m\x1b[32m==>\x1b[0m\x1b[1m $1\x1b[0m\n"
}

subinfo() {
    printf "\x1b[1m\x1b[34m  ->\x1b[0m\x1b[1m $1\x1b[0m\n"
}

warn() {
    printf "\x1b[1m\x1b[93m!!!\x1b[0m\x1b[1m $1\x1b[0m\n"
}

ask() {
    printf "\x1b[1m\x1b[34m  ?\x1b[0m\x1b[1m $1\x1b[0m "
}

build_test() {
    EXEC_NAME="${1%.c}.test_exec"

    if [ ! "$1" -nt "$EXEC_NAME" ]; then
        return 0;
    fi

    subinfo "building..."
    $CC $CFLAGS "$1" -o "$EXEC_NAME" $LDFLAGS || err "failed to build"
}

run_test() {
    info "running test "'"'"$1"'"'
    build_test "$1" || return 1;

    subinfo "running $EXEC_NAME"

    $EXEC_NAME > /dev/null
    return $?;
}

let "failed=0"
let "passed=0"

for test in $TESTS; do
    if run_test "$test"; then
        let "passed+=1"
    else
        let "failed+=1"
    fi
done

info "$failed tests \x1b[1m\x1b[31mfailed\x1b[0m, $passed tests \x1b[1m\x1b[32mpassed\x1b[0m"

exit $failed
