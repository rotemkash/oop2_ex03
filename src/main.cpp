#include <iostream>
#include <string>

#include "FunctionCalculator.h"

int main() {
  try {
    FunctionCalculator(std::cin, std::cout).run();
  } catch (const std::exception &e) {
    std::cout << e.what();
  }
}
