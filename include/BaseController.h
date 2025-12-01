#ifndef BASECONTROLLER_H
#define BASECONTROLLER_H

#include "Database.h"
#include <string>

class BaseController {
protected:
    Database* db;
    
    void clearInputBuffer();
    string getInput(const string& prompt);
    int getIntInput(const string& prompt);
    double getDoubleInput(const string& prompt);
    string getDate(const string& prompt);
    
public:
    BaseController(Database* database);
    virtual ~BaseController() = default;
    virtual void showMenu() = 0;
};

#endif // BASECONTROLLER_H
