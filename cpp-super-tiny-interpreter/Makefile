cmd:
	mkdir -p .dist && g++ -std=c++17 models.cpp lexer.cpp parser.cpp scripts.cpp cmd.cpp -o .dist/cmd.out && .dist/cmd.out
test:
	mkdir -p .dist && g++ -g -std=c++17 models.cpp lexer.cpp parser.cpp scripts.cpp test.cpp -o .dist/test.out && .dist/test.out