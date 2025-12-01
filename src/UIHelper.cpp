#include "UIHelper.h"
#include <iostream>
#include <cstdlib>

void UIHelper::clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void UIHelper::pause() {
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void UIHelper::printHeader(const std::string& title) {
    printSeparator(60, '=');
    std::cout << "  " << title << std::endl;
    printSeparator(60, '=');
}

void UIHelper::printSeparator(int width, char ch) {
    for (int i = 0; i < width; ++i) {
        std::cout << ch;
    }
    std::cout << std::endl;
}
