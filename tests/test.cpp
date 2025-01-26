#include "test.h"
#include <iostream>

int main()
{
    juce::UnitTestRunner runner;
    runner.runAllTests();

    int number = runner.getNumResults();

    bool allPass = true;

    for (int i = 0; i < number; ++i) {
        auto res = runner.getResult(i);
        if (res->failures > 0) {
            std::cout << res->unitTestName << " - " << res->subcategoryName << " " << "FAILED" << std::endl;
            allPass = false;
        }
    }

    if (allPass) {
        std::cout << "ALL " << number << " TESTS PASSED!" << std::endl;
        return 0;
    }
    return -1;
}
