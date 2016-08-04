#!/bin/bash

# Got this from stack overflow
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [ -n "${LIBMIIL_DIR}" ]; then
    if [ -d ${LIBMIIL_DIR} ]; then
        # remove the previously defined LIBMIIL_DIR and it's leading ':'
        LD_LIBRARY_PATH=`echo $LD_LIBRARY_PATH | sed -e 's#:'"${LIBMIIL_DIR}"'/lib##g'`
        # remove the previously defined LIBMIIL_DIR without a leading ':'
        # couldn't get a \? escape on the : to work for some reason.
        LD_LIBRARY_PATH=`echo $LD_LIBRARY_PATH | sed -e 's#'"${LIBMIIL_DIR}"'/lib##g'`
    fi
fi


LIBMIIL_DIR=$DIR
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$LIBMIIL_DIR/lib

export LIBMIIL_DIR
export LD_LIBRARY_PATH

