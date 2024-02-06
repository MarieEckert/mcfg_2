#!/bin/bash

function directory_setup() {
  if [ -d out/ ]; then
    rm -rf out
  fi

  mkdir out/
}

echo "MCFG/2 setup script"
echo "==> Setting up directory structure"
directory_setup

echo "==> DONE!"
