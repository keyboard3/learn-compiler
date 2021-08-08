#include <iostream>

extern "C" {
    double fib(double);
}

int main() {
    std::cout << "fib: " << fib(6) << std::endl;
}