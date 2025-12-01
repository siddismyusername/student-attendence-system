#include "BaseController.h"
#include <iostream>
#include <limits>
#include <sstream>
#include <iomanip>

using namespace std;

BaseController::BaseController(Database* database) : db(database) {}

void BaseController::clearInputBuffer() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

string BaseController::getInput(const string& prompt) {
    string input;
    cout << prompt;
    getline(cin, input);
    return input;
}

int BaseController::getIntInput(const string& prompt) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            clearInputBuffer();
            return value;
        } else {
            cout << "Invalid input. Please enter a number." << endl;
            clearInputBuffer();
        }
    }
}

double BaseController::getDoubleInput(const string& prompt) {
    double value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            clearInputBuffer();
            return value;
        } else {
            cout << "Invalid input. Please enter a number." << endl;
            clearInputBuffer();
        }
    }
}

string BaseController::getDate(const string& prompt) {
    int day, month, year;
    cout << prompt << endl;
    day = getIntInput("Day (1-31): ");
    month = getIntInput("Month (1-12): ");
    year = getIntInput("Year (e.g., 2025): ");
    
    ostringstream oss;
    oss << year << "-" << setfill('0') << setw(2) << month << "-" 
        << setfill('0') << setw(2) << day;
    
    return oss.str();
}
