#ifndef ADMINCONTROLLER_H
#define ADMINCONTROLLER_H

#include "BaseController.h"

class AdminController : public BaseController {
public:
    AdminController(Database* db);
    
    void createSubject();
    void createClass();
    void createTeacher();
    void createStudent();
    void assignClassTeacher();
    void assignSubjectTeacher();
    void addSubjectToClass();
    void viewAllSubjects();
    void viewAllClasses();
    void viewAllTeachers();
    void viewAllStudents();
    void deleteSubject();
    void deleteClass();
    void deleteTeacher();
    void deleteStudent();
    
    void showMenu() override;
};

#endif // ADMINCONTROLLER_H
