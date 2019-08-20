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

set -eu -o pipefail

# Setup local variables and temporary directory
TOOL=${BIN_DIR}/tool

REPORT_QUANTITY="[0-9]\+$"

CONFIG="-e perf_count_hw_instructions:hello_world"
CONFIG+=" -e perf_count_hw_cpu_cycles:hello_world"
${TOOL} ${CONFIG} -- ${TEST_EXECUTABLES}/hello_world 2>&1 > /dev/null | \
  sed -e "s/${REPORT_QUANTITY}/REPORT_QUANTITY/g"

CONFIG="-e perf_count_hw_instructions:f1"
CONFIG+=" -e perf_count_hw_cpu_cycles:f1"
CONFIG+=" -e perf_count_hw_instructions:f2"
CONFIG+=" -e perf_count_hw_cpu_cycles:f2"
${TOOL} ${CONFIG} -- ${TEST_EXECUTABLES}/nested_call 2>&1 > /dev/null | \
  sed -e "s/${REPORT_QUANTITY}/REPORT_QUANTITY/g"

CONFIG="-e perf_count_hw_cache_misses:cache_friendly"
CONFIG+=" -e perf_count_hw_cache_misses:cache_missy"
${TOOL} ${CONFIG} -- ${TEST_EXECUTABLES}/cache_misses 2>&1 > /dev/null | \
  sed -e "s/${REPORT_QUANTITY}/REPORT_QUANTITY/g"

CONFIG="-e perf_count_hw_instructions:namespaced"
${TOOL} ${CONFIG} -- ${TEST_EXECUTABLES}/find_function 2>&1 > /dev/null | \
  sed -e "s/${REPORT_QUANTITY}/REPORT_QUANTITY/g"
