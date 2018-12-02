#!/usr/bin/env bash
set -x -e

########################################
# build php extension for bfs-php-sdk
########################################
phpize

./configure

sudo make && make install
