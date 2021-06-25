models:
	mkdir -p .dist && g++ -g -std=c++17 models.cpp -o .dist/models.out && .dist/models.out
test:
	mkdir -p .dist && g++ -g -std=c++17 models.cpp lexer.cpp parser.cpp script.cpp test.cpp -o .dist/test.out && .dist/test.out