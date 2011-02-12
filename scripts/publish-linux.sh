#!/bin/bash

cd $CC_BASE

. $CC_FUNCTIONS

cd $CC_PACKAGING

RELEASE_NAME="ctrl-cut-$CC_VERSION"
FILES="`cat included`"
TAR="tar -X excluded -chjf $RELEASE_NAME.tar.bz2 $RELEASE_NAME"

function  cleanup() {
    [ -d "$RELEASE_NAME/" ] && rm -r "$RELEASE_NAME"
}

function populate() {
    mkdir $RELEASE_NAME

    
    echo "$FILES" | while read incl; do
        cp -r "$CC_BASE/$incl" "$RELEASE_NAME/"
    done
}

cleanup
populate
$TAR
cleanup
build

