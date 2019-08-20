#!/bin/bash

set -eu

FILES=$(git --git-dir=${PROJECT_ROOT}/.git ls-files | grep -P '.(h|cpp|inl)$')

rc=0

for FILE in ${FILES}; do
  if ! grep "Copyright 2019" ${FILE} &> /dev/null; then
    echo ${FILE} failed
    rc=1
  fi
done

echo ${rc}
