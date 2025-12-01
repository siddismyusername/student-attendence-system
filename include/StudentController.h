#ifndef STUDENTCONTROLLER_H
#define STUDENTCONTROLLER_H

#include "BaseController.h"
#include <string>

class StudentController : public BaseController {
private:
    int studentId;
    std::string studentName;
    
public:
    StudentController(Database* db, int id, const std::string& name);
    
    void viewMyAttendance();
    void viewAttendancePercentage();
    void updateProfile();
    
    void showMenu() override;
};

#endif // STUDENTCONTROLLER_H
