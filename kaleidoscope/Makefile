all:
	clang++ -g -O3 toy.cpp `llvm-config --cxxflags --ldflags --system-libs --libs all` -o compiler-toy

jit:
	clang++ -g jit-toy.cpp `llvm-config --cxxflags --ldflags --system-libs --libs core orcjit native` -O3 -o jit-toy

test:
	clang++ main.cpp output.o -o main && ./main