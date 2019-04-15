debug_tool:
	mkdir -p build 
	g++ src/tool.cpp --std=c++1z -I/home/harvey/software/linenoise/linenoise-v1 -L/home/harvey/software/linenoise/linenoise-v1/lib -llinenoise -o build/tool

clean:
	rm -rf build
