#!/bin/bash

# Got this from stack overflow
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [ -n "${LIBMIIL_DIR}" ]; then
    if [ -d ${LIBMIIL_DIR} ]; then
        # PREVIOUSLY DEFINED, REMOVE FROM $LD_LIBRARY_PATH
        LD_LIBRARY_PATH=`echo $LD_LIBRARY_PATH | sed -e 's#:'"${LIBMIIL_DIR}"'/lib##'`
    fi
fi

if [ -n "${LIBMIIL_DIR}" ]; then
    if [ -d ${LIBMIIL_DIR} ]; then
        # PREVIOUSLY DEFINED, REMOVE FROM $LD_LIBRARY_PATH
        PYTHONPATH=`echo $PYTHONPATH| sed -e 's#:'"${LIBMIIL_DIR}"'/python##'`
    fi
fi

LIBMIIL_DIR=$DIR
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$LIBMIIL_DIR/lib
PYTHONPATH=$PYTHONPATH:$LIBMIIL_DIR/python

export LIBMIIL_DIR
export LD_LIBRARY_PATH
export PYTHONPATH
