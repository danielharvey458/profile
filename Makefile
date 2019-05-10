
CXXFLAGS = --std=c++1z
CXXFLAGS += -Isrc/
CXXFLAGS += -I/home/harvey/software/linenoise/linenoise-v1
CXXFLAGS += -Werror
CXXFLAGS += -Wall

LDFLAGS += -I/home/harvey/software/linenoise/linenoise-v1
LDFLAGS += -L/home/harvey/software/linenoise/linenoise-v1/lib
LDFLAGS += -Lbuild/lib/
LDFLAGS += -llinenoise
LDFLAGS += -lperfevent

build_directory:
	mkdir -p build 
	mkdir -p build/lib
	mkdir -p build/.obj/pt_profile
	mkdir -p build/.obj/perf_event

build/.obj/pt_profile/breakpoint.o: build_directory
	$(CXX) src/pt_profile/breakpoint.cpp $(CXXFLAGS) -c -o $(@)

build/.obj/pt_profile/tokenizer.o: build_directory
	$(CXX) src/pt_profile/tokenizer.cpp $(CXXFLAGS) -c -o $(@)

build/.obj/pt_profile/registers.o: build_directory
	$(CXX) src/pt_profile/registers.cpp $(CXXFLAGS) -c -o $(@)

build/.obj/pt_profile/debugger.o: build_directory
	$(CXX) src/pt_profile/debugger.cpp $(CXXFLAGS) -c -o $(@)

build/tool: build/.obj/pt_profile/breakpoint.o build/.obj/pt_profile/tokenizer.o build/.obj/pt_profile/registers.o build/.obj/pt_profile/debugger.o build/lib/libperfevent.so
	$(CXX) src/pt_profile/tool.cpp $(CXXFLAGS) $(^) $(LDFLAGS) -o $(@)

build/.obj/perf_event/perf_event.o: build_directory
	$(CXX) src/perf_event/perf_event.cpp $(CXXFLAGS) -fpic -c -o $(@)

build/lib/libperfevent.so: build/.obj/perf_event/perf_event.o
	$(CXX) -shared -o $(@) $(^)

clean:
	rm -rf build
