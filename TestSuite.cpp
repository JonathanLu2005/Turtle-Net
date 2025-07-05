#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include "LogoEngine.hpp"

namespace fs = std::filesystem;

bool RunTestFile(const std::string& PathFile) {
    std::ifstream LogoFile(PathFile);

    if (!LogoFile) {
        std::cerr << "Couldn't open file" << std::endl;
        return false;
    }

    std::string LogoCode((std::istreambuf_iterator<char>(LogoFile)), std::istreambuf_iterator<char>());
    LogoEngine Engine;
    auto Lines = Engine.Run(LogoCode);
    return !Lines.empty();
}

int main() {
    std::vector<std::pair<std::string, bool>> TestCases;

    for (const auto& TestFile : fs::directory_iterator("tests/passing")) {
        if (TestFile.is_regular_file() && TestFile.path().extension() == ".logo") {
            TestCases.emplace_back(TestFile.path().string(), true);
        }
    }

    for (const auto& TestFile : fs::directory_iterator("tests/failing")) {
        if (TestFile.is_regular_file() && TestFile.path().extension() == ".logo") {
            TestCases.emplace_back(TestFile.path().string(), false);
        }
    }

    int Passed = 0;
    int Total = 0;
    for (const auto& [TestFile, Outcome] : TestCases) {
        bool Result = RunTestFile(TestFile);
        bool Ok = (Result == Outcome);

        std::cout << TestFile << ": " << (Ok ? "[PASS]" : "[FAIL]") << std::endl;

        if (Ok) {
            Passed++;
        }
        Total++;
    }
    std::cout << "\nSummary: " << std::to_string(Passed) << "/" << std::to_string(Total) << " tests passed.\n";
    return 0;
}