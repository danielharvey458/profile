#!/bin/bash

TEST_SCRIPT=$1
EXPECTED=$2

TEMP=$(mktemp -d)
trap "rm -rf ${TEMP}" exit

OUTPUT=${TEMP}/expected.log

${TEST_SCRIPT} > ${OUTPUT}
DIFF=$(diff ${OUTPUT} ${EXPECTED})

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

if [ "$DIFF" != "" ]
then
  echo -e "Test $(basename ${TEST_SCRIPT}) [${RED}Failed${NC}]"
  PERM=$(mktemp -d)
  mv ${OUTPUT} ${PERM}/unexpected.log
  echo ${DIFF}
  echo -e "Compare output using:"
  echo -e "    diff ${PERM}/unexpected.log ${EXPECTED}"
  exit 1
else
  echo -e "Test $(basename ${TEST_SCRIPT}) [${GREEN}Passed${NC}]"
fi
