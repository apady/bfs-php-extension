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
touch /etc/php.d/bfs.ini
echo -e "extension=bfs.so" > /etc/php.d/bfs.ini
cd -
