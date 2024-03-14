#!/bin/bash

OBJ_DIR="obj/"

function directory_setup() {
  if [ -d $OBJ_DIR ]; then
    rm -rf $OBJ_DIR
  fi

  mkdir $OBJ_DIR
}

echo "MCFG/2 setup script"
echo "==> Setting up directory structure"
directory_setup

echo "==> DONE!"
