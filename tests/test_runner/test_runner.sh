#!/bin/bash

# Copyright 2019 Daniel Harvey
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

TEST_SCRIPT=$1
EXPECTED=$2

TEMP=$(mktemp -d)
trap "rm -rf ${TEMP}" exit

OUTPUT=${TEMP}/expected.log

${TEST_SCRIPT} &> ${OUTPUT}
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
