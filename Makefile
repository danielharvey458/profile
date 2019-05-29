CXXFLAGS = --std=c++1z
CXXFLAGS += -Isrc/
CXXFLAGS += -Werror
CXXFLAGS += -Wall

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

.DEFAULT_GOAL = all

build/.obj/pt_profile/%.o : src/pt_profile/%.cpp
	@mkdir -p $(@D)
	@printf "Compiling $@\n"
	@$(CXX) $(<) $(CXXFLAGS) -c -o $(@)

objects := $(patsubst src/pt_profile/%.cpp,              \
                      build/.obj/pt_profile/%.o,         \
                      $(wildcard src/pt_profile/*.cpp))

build/tool : $(objects)
	@printf "Compiling $@\n"
	@$(CXX) $(^) $(LDFLAGS) -o $(@)

clean:
	@rm -rf build

all: build/tool
