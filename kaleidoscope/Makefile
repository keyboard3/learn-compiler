all:
	clang++ -g -O3 toy.cpp `llvm-config --cxxflags --ldflags --system-libs --libs all` -o compiler-toy

jit:
	clang++ -g jit-toy.cpp `llvm-config --cxxflags --ldflags --system-libs --libs core orcjit native` -O3 -o jit-toy

debug: 
	clang++ -g -O3 debug-toy.cpp `llvm-config --cxxflags --ldflags --system-libs --libs all` -o debug-toy

average:
	clang++ -g average.cpp output.o -o average && ./average

fib:
	clang++ -g fib.cpp output.o -o fib && ./fib