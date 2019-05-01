debug_tool:
	mkdir -p build 
	mkdir -p build/.obj
	g++ src/pt_profile/breakpoint.cpp --std=c++1z -Isrc/ -c -o build/.obj/breakpoint.o
	g++ src/pt_profile/tokenizer.cpp --std=c++1z -Isrc/ -c -o build/.obj/tokenizer.o
	g++ src/pt_profile/registers.cpp --std=c++1z -Isrc/ -c -o build/.obj/registers.o
	g++ src/pt_profile/debugger.cpp --std=c++1z -I/home/harvey/software/linenoise/linenoise-v1 -Isrc/ -c -o build/.obj/debugger.o
	g++ src/pt_profile/tool.cpp --std=c++1z build/.obj/breakpoint.o build/.obj/tokenizer.o build/.obj/registers.o build/.obj/debugger.o -Isrc/ -I/home/harvey/software/linenoise/linenoise-v1 -L/home/harvey/software/linenoise/linenoise-v1/lib -llinenoise -o build/tool
clean:
	rm -rf build
