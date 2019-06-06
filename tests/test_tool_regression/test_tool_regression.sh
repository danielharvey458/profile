#!/bin/bash

set -eu -o pipefail

# Setup local variables and temporary directory
TOOL=${BIN_DIR}/tool
TEMP=$(mktemp -d)
trap 'rm -rf "${TEMP}"' exit

CONFIG=${TEMP}/config_file

ADDRESS_SUBSTITUTION="0x[0-9a-f]\+--0x[0-9a-f]\+"
REPORT_QUANTITY="[0-9]\+$"

echo 'perf_count_hw_instructions hello_world' > ${CONFIG}
echo 'perf_count_hw_cpu_cycles hello_world' >> ${CONFIG}
${TOOL} ${CONFIG} ${TEST_EXECUTABLES}/hello_world 2>&1 > /dev/null | \
  sed -e "s/${ADDRESS_SUBSTITUTION}/ADDRESS_RANGE/g" \
      -e "s/${REPORT_QUANTITY}/REPORT_QUANTITY/g"
