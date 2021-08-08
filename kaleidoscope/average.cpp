#include <iostream>

extern "C" {
    double average(double,double);
}

int main() {
    std::cout << "average: " << average(5,4) << std::endl;
}