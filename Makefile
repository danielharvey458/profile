.DEFAULT_GOAL = all

##########################################################################
# Core compiler flags
##########################################################################

CXXFLAGS = --std=c++1z
CXXFLAGS += -Isrc/
CXXFLAGS += -Werror
CXXFLAGS += -Wall
CXXFLAGS += -g
CXXFLAGS += -fpic

##########################################################################
# Thirdparty
##########################################################################

LIBELFIN_VERSION=libelfin-v1
LIBELFIN_LOCATION=/home/harvey/software/libelfin/$(LIBELFIN_VERSION)

ELF_CXXFLAGS += -I$(LIBELFIN_LOCATION)/elf
ELF_LDFLAGS  += -I$(LIBELFIN_LOCATION)/elf
ELF_LDFLAGS  += -L$(LIBELFIN_LOCATION)/elf
ELF_LDFLAGS  += -lelf++

DWARF_CXXFLAGS += -I$(LIBELFIN_LOCATION)/dwarf
DWARF_LDFLAGS  += -I$(LIBELFIN_LOCATION)/dwarf
DWARF_LDFLAGS  += -L$(LIBELFIN_LOCATION)/dwarf
DWARF_LDFLAGS  += -ldwarf++
DWARF_LDFLAGS  += -Wl,-rpath,$(LIBELFIN_LOCATION)/elf
DWARF_LDFLAGS  += -Wl,-rpath,$(LIBELFIN_LOCATION)/dwarf

##########################################################################
# libprofile
##########################################################################
PROFILE_LIB_CXXFLAGS:=$(CXXFLAGS) $(DWARF_CXXFLAGS) $(ELF_CXXFLAGS) -Isrc
PROFILE_LIB_LDFLAGS:=$(LDFLAGS) $(DWARF_LDFLAGS) $(ELF_LDFLAGS)

build/.obj/profile/%.o : src/profile/%.cpp src/profile/%.h
	@mkdir -p $(@D)
	@printf "Compiling $@\n"
	@$(CXX) $(<) $(PROFILE_LIB_CXXFLAGS) -c -o $(@)

objects := $(patsubst src/profile/%.cpp,              \
                      build/.obj/profile/%.o,         \
                      $(wildcard src/profile/*.cpp))

build/lib/libprofile.so : $(objects)
	@mkdir -p $(@D)
	@printf "Linking $(@)\n"
	@$(CXX) -shared -o $(@) $(PROFILE_LIB_LDFLAGS) $(objects)

##########################################################################
# Commandline tool
##########################################################################

PROFILE_TOOL_CXXFLAGS := $(PROFILE_LIB_CXXFLAGS) -Isrc
PROFILE_TOOL_LDFLAGS := $(PROFILE_LIB_LDFLAGS)
PROFILE_TOOL_LDFLAGS += -Isrc
PROFILE_TOOL_LDFLAGS += -Lbuild/lib
PROFILE_TOOL_LDFLAGS += -lprofile
PROFILE_TOOL_LDFLAGS += -Wl,-rpath,build/lib

build/tool : tools/profile/profile.cpp build/lib/libprofile.so
	@printf "Compiling $@\n"
	@$(CXX) $(<) $(PROFILE_TOOL_CXXFLAGS) $(PROFILE_TOOL_LDFLAGS) -o $(@)

##########################################################################
# Tests
##########################################################################

REGRESSION_TEST_DIR=tests/test_tool_regression
REGRESSION_TEST_EXECUTABLES_DIR=$(REGRESSION_TEST_DIR)/test_executables
REGRESSION_TEST_RUNNER=$(REGRESSION_TEST_DIR)/test_runner.sh
REGRESSION_TEST_SCRIPT=$(REGRESSION_TEST_DIR)/test_tool_regression.sh
REGRESSION_TEST_EXPECTED=$(REGRESSION_TEST_DIR)/expected.log
REGRESSION_TEST_BIN_DIR=build/tests/test_executables

# Compile test executables
$(REGRESSION_TEST_BIN_DIR)/% : $(REGRESSION_TEST_EXECUTABLES_DIR)/%.cpp
	@mkdir -p $(@D)
	@printf "Compiling $@\n"
	@$(CXX) $(<) $(CXXFLAGS) -o $(@)

REGRESSION_TEST_EXECUTABLES := \
  $(patsubst $(REGRESSION_TEST_EXECUTABLES_DIR)/%.cpp,   \
             $(REGRESSION_TEST_BIN_DIR)/%,             \
             $(wildcard $(REGRESSION_TEST_EXECUTABLES_DIR)/*.cpp))

build/tests/result.log : $(REGRESSION_TEST_EXECUTABLES)       \
	                       build/tool $(REGRESSION_TEST_SCRIPT)
	@mkdir -p $(@D)
	@BIN_DIR=build TEST_EXECUTABLES=$(REGRESSION_TEST_BIN_DIR) \
     $(REGRESSION_TEST_RUNNER) $(REGRESSION_TEST_SCRIPT)     \
     $(REGRESSION_TEST_EXPECTED)
	@touch $(@)

## C++ tests
TEST_DIR=tests/test_profiler
build/tests/% : $(TEST_DIR)/%.cpp
	@mkdir -p $(@D)
	printf "Compiling $@\n"
	$(CXX) $(<) $(CXXFLAGS) $(LDFLAGS) -o $(@)

##########################################################################
# Tests
##########################################################################

test : build/tests/result.log

clean:
	@rm -rf build

all: build/tool

.PHONY: clean test all
