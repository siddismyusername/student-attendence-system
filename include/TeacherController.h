#ifndef TEACHERCONTROLLER_H
#define TEACHERCONTROLLER_H

#include "BaseController.h"
#include <string>

class TeacherController : public BaseController {
private:
    int teacherId;
    std::string teacherName;
    std::string teacherType;
    
public:
    TeacherController(Database* db, int id, const std::string& name, const std::string& type);
    
    void markAttendance();
    void viewStudentAttendance();
    void viewClassAttendance();
    void addStudentToClass();
    
    void showMenu() override;
};

#endif // TEACHERCONTROLLER_H
