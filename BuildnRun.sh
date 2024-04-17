#!/bin/bash

set -eux

# path to rpcs3 executable; calling rpcs3 directly doesnt seem to work 
RPCS3=/home/louis/Documents/ps3/rpcs3_build/bin/rpcs3

function build() {
    make clean 
    make -j $(nproc --all)
}

function run() {
    $RPCS3 3d-engine-ps3.self
}

function build_pipeline() {
    build
    run
}

"${@:-build_pipeline}"