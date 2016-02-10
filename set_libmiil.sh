#!/bin/bash

# Got this from stack overflow
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [ -n "${LIBMIIL_DIR}" ]; then
  if [ -d ${LIBMIIL_DIR} ]; then
      # ANDIR PREVIOUSLY DEFINED, REMOVE FROM $LD_LIBRARY_PATH
      export LD_LIBRARY_PATH=`echo $LD_LIBRARY_PATH | sed -e 's#:'"${LIBMIIL_DIR}"'/lib##'`
  fi
fi

LIBMIIL_DIR=$DIR

export LIBMIIL_DIR

if [[ $LD_LIBRARY_PATH != ?(*:)${LIBMIIL_DIR}/lib?(:*) ]]; then
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$LIBMIIL_DIR/lib
fi
