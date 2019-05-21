CXXFLAGS = --std=c++1z
CXXFLAGS += -Isrc/
CXXFLAGS += -Werror
CXXFLAGS += -Wall

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
