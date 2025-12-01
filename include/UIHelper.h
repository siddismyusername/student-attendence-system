#ifndef UIHELPER_H
#define UIHELPER_H

#include <string>
#include <limits>

class UIHelper {
public:
    static void clearScreen();
    static void pause();
    static void printHeader(const std::string& title);
    static void printSeparator(int width = 60, char ch = '-');
};

#endif // UIHELPER_H
