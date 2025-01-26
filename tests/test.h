#pragma once
#include "../JUCE/modules/juce_core/juce_core.h"

class JUCEUnitTest  : public juce::UnitTest
{
public:
    JUCEUnitTest()  : UnitTest ("JUCE Unit Test!") {}

    void runTest() override
    {
        beginTest ("Test JUCE Unit Test");

        expect (true);
    }
};
static JUCEUnitTest JUCETest;
