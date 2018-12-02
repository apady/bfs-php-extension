#!/usr/bin/env bash
set -x -e

########################################
# build php extension for bfs-php-sdk
########################################

SRC_DIR=`pwd`/src

cd ${SRC_DIR}

phpize

./configure

sudo make && make install

cd -
