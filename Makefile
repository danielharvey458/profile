.DEFAULT_GOAL = all

CXXFLAGS = --std=c++1z
CXXFLAGS += -Isrc/
CXXFLAGS += -Werror
CXXFLAGS += -Wall
CXXFLAGS += -g

# Depends on libelf and libdwarf
LIBELFIN_VERSION=libelfin-v1
LIBELFIN_LOCATION=/home/harvey/software/libelfin/$(LIBELFIN_VERSION)

CXXFLAGS += -I$(LIBELFIN_LOCATION)/elf
LDFLAGS  += -I$(LIBELFIN_LOCATION)/elf
LDFLAGS  += -L$(LIBELFIN_LOCATION)/elf
LDFLAGS  += -lelf++

CXXFLAGS += -I$(LIBELFIN_LOCATION)/dwarf
LDFLAGS  += -I$(LIBELFIN_LOCATION)/dwarf
LDFLAGS  += -L$(LIBELFIN_LOCATION)/dwarf
LDFLAGS  += -ldwarf++
LDFLAGS  += -Wl,-rpath,$(LIBELFIN_LOCATION)/elf
LDFLAGS  += -Wl,-rpath,$(LIBELFIN_LOCATION)/dwarf

build/.obj/profile/%.o : src/profile/%.cpp src/profile/%.h
	@mkdir -p $(@D)
	@printf "Compiling $@\n"
	@$(CXX) $(<) $(CXXFLAGS) -c -o $(@)

objects := $(patsubst src/profile/%.cpp,              \
                      build/.obj/profile/%.o,         \
                      $(wildcard src/profile/*.cpp))

build/tool : $(objects)
	@printf "Compiling $@\n"
	@$(CXX) $(^) $(LDFLAGS) -o $(@)

## Regression tests
TEST_DIR=tests/test_tool_regression
TEST_EXECUTABLES_DIR=$(TEST_DIR)/test_executables
TEST_RUNNER=$(TEST_DIR)/test_runner.sh
TEST_SCRIPT=$(TEST_DIR)/test_tool_regression.sh
TEST_EXPECTED=$(TEST_DIR)/expected.log

# Compile test executables
build/tests/test_executables/% : $(TEST_EXECUTABLES_DIR)/%.cpp
	@mkdir -p $(@D)
	@printf "Compiling $@\n"
	@$(CXX) $(<) $(CXXFLAGS) -o $(@)

TEST_EXECUTABLES := \
  $(patsubst $(TEST_EXECUTABLES_DIR)/%.cpp, \
             build/tests/test_executables/%, \
             $(wildcard $(TEST_EXECUTABLES_DIR)/*.cpp))

build/tests/result.log : $(TEST_EXECUTABLES) build/tool $(TEST_SCRIPT)
	@BIN_DIR=build                                     \
	TEST_EXECUTABLES=build/tests/test_executables      \
	./$(TEST_RUNNER) $(TEST_SCRIPT) $(TEST_EXPECTED)
	@touch $(@)

test : build/tests/result.log

clean:
	@rm -rf build

all: build/tool

.PHONY: clean test all
