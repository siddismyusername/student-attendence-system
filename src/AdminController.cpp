#include "AdminController.h"
#include "UIHelper.h"
#include <iostream>
#include <iomanip>

using namespace std;

AdminController::AdminController(Database* db) : BaseController(db) {}

void AdminController::createSubject() {
    UIHelper::clearScreen();
    UIHelper::printHeader("Create New Subject");
    
    string name = getInput("Enter subject name: ");
    
    // FR-19: Prevent duplicate subject names
    if (!db->isSubjectNameUnique(name)) {
        cout << "\nError: A subject with this name already exists!" << endl;
        UIHelper::pause();
        return;
    }
    
    int maxMarks = getIntInput("Enter maximum marks: ");
    
    int id = db->createSubject(name, maxMarks);
    if (id > 0) {
        cout << "\nSubject created successfully with ID: " << id << endl;
    } else {
        cout << "\nFailed to create subject." << endl;
    }
    
    UIHelper::pause();
}

void AdminController::createClass() {
    UIHelper::clearScreen();
    UIHelper::printHeader("Create New Class");
    
    string className = getInput("Enter class name: ");
    
    // FR-19: Prevent duplicate class names
    if (!db->isClassNameUnique(className)) {
        cout << "\nError: A class with this name already exists!" << endl;
        UIHelper::pause();
        return;
    }
    
    int id = db->createClass(className);
    if (id > 0) {
        cout << "\nClass created successfully with ID: " << id << endl;
    } else {
        cout << "\nFailed to create class." << endl;
    }
    
    UIHelper::pause();
}

void AdminController::createTeacher() {
    UIHelper::clearScreen();
    UIHelper::printHeader("Create New Teacher");
    
    string name = getInput("Enter teacher name: ");
    string email = getInput("Enter email: ");
    string password = getInput("Enter password: ");
    double salary = getDoubleInput("Enter salary: ");
    string joinDate = getDate("Enter join date:");
    
    cout << "\nSelect teacher type:" << endl;
    cout << "1. ClassTeacher" << endl;
    cout << "2. SubjectTeacher" << endl;
    int typeChoice = getIntInput("Enter choice: ");
    
    string type = (typeChoice == 1) ? "ClassTeacher" : "SubjectTeacher";
    
    int id = db->createTeacher(name, email, password, salary, joinDate, type);
    if (id > 0) {
        cout << "\nTeacher created successfully with ID: " << id << endl;
    } else {
        cout << "\nFailed to create teacher." << endl;
    }
    
    UIHelper::pause();
}

void AdminController::createStudent() {
    UIHelper::clearScreen();
    UIHelper::printHeader("Create New Student");
    
    // Show available classes
    auto classes = db->getAllClasses();
    if (classes.empty()) {
        cout << "No classes available. Please create a class first." << endl;
        UIHelper::pause();
        return;
    }
    
    cout << "\nAvailable Classes:" << endl;
    UIHelper::printSeparator(40);
    for (const auto& cls : classes) {
        cout << "ID: " << cls.at("class_id") << " - " << cls.at("class_name") << endl;
    }
    UIHelper::printSeparator(40);
    
    string name = getInput("\nEnter student name: ");
    int classId = getIntInput("Enter class ID: ");
    
    // FR-20: Validate class exists before adding student
    if (!db->classExists(classId)) {
        cout << "\nError: Class does not exist!" << endl;
        UIHelper::pause();
        return;
    }
    
    int id = db->createStudent(name, classId);
    if (id > 0) {
        cout << "\nStudent created successfully with ID: " << id << endl;
    } else {
        cout << "\nFailed to create student." << endl;
    }
    
    UIHelper::pause();
}

void AdminController::assignClassTeacher() {
    UIHelper::clearScreen();
    UIHelper::printHeader("Assign Class Teacher");
    
    // Show available classes
    auto classes = db->getAllClasses();
    if (classes.empty()) {
        cout << "No classes available." << endl;
        UIHelper::pause();
        return;
    }
    
    cout << "\nAvailable Classes:" << endl;
    UIHelper::printSeparator(40);
    for (const auto& cls : classes) {
        cout << "ID: " << cls.at("class_id") << " - " << cls.at("class_name") << endl;
    }
    
    // Show available teachers
    auto teachers = db->getAllTeachers();
    if (teachers.empty()) {
        cout << "\nNo teachers available." << endl;
        UIHelper::pause();
        return;
    }
    
    cout << "\nAvailable Teachers:" << endl;
    UIHelper::printSeparator(40);
    for (const auto& teacher : teachers) {
        cout << "ID: " << teacher.at("teacher_id") << " - " << teacher.at("name") 
                  << " (" << teacher.at("teacher_type") << ")" << endl;
    }
    UIHelper::printSeparator(40);
    
    int classId = getIntInput("\nEnter class ID: ");
    int teacherId = getIntInput("Enter teacher ID: ");
    
    // FR-20: Validate entities exist
    if (!db->classExists(classId)) {
        cout << "\nError: Class does not exist!" << endl;
        UIHelper::pause();
        return;
    }
    
    if (!db->teacherExists(teacherId)) {
        cout << "\nError: Teacher does not exist!" << endl;
        UIHelper::pause();
        return;
    }
    
    if (db->assignClassTeacher(classId, teacherId)) {
        cout << "\nClass teacher assigned successfully!" << endl;
    } else {
        cout << "\nFailed to assign class teacher." << endl;
    }
    
    UIHelper::pause();
}

void AdminController::assignSubjectTeacher() {
    UIHelper::clearScreen();
    UIHelper::printHeader("Assign Subject Teacher");
    
    // Show available teachers
    auto teachers = db->getAllTeachers();
    if (teachers.empty()) {
        cout << "No teachers available." << endl;
        UIHelper::pause();
        return;
    }
    
    cout << "\nAvailable Teachers:" << endl;
    UIHelper::printSeparator(40);
    for (const auto& teacher : teachers) {
        cout << "ID: " << teacher.at("teacher_id") << " - " << teacher.at("name") << endl;
    }
    
    // Show available subjects
    auto subjects = db->getAllSubjects();
    if (subjects.empty()) {
        cout << "\nNo subjects available." << endl;
        UIHelper::pause();
        return;
    }
    
    cout << "\nAvailable Subjects:" << endl;
    UIHelper::printSeparator(40);
    for (const auto& subject : subjects) {
        cout << "ID: " << subject.at("subject_id") << " - " << subject.at("name") << endl;
    }
    
    // Show available classes
    auto classes = db->getAllClasses();
    if (classes.empty()) {
        cout << "\nNo classes available." << endl;
        UIHelper::pause();
        return;
    }
    
    cout << "\nAvailable Classes:" << endl;
    UIHelper::printSeparator(40);
    for (const auto& cls : classes) {
        cout << "ID: " << cls.at("class_id") << " - " << cls.at("class_name") << endl;
    }
    UIHelper::printSeparator(40);
    
    int teacherId = getIntInput("\nEnter teacher ID: ");
    int subjectId = getIntInput("Enter subject ID: ");
    int classId = getIntInput("Enter class ID: ");
    
    // FR-20: Validate all entities exist
    if (!db->teacherExists(teacherId)) {
        cout << "\nError: Teacher does not exist!" << endl;
        UIHelper::pause();
        return;
    }
    
    if (!db->subjectExists(subjectId)) {
        cout << "\nError: Subject does not exist!" << endl;
        UIHelper::pause();
        return;
    }
    
    if (!db->classExists(classId)) {
        cout << "\nError: Class does not exist!" << endl;
        UIHelper::pause();
        return;
    }
    
    if (db->assignSubjectTeacher(teacherId, subjectId, classId)) {
        cout << "\nSubject teacher assigned successfully!" << endl;
    } else {
        cout << "\nFailed to assign subject teacher. May already be assigned." << endl;
    }
    
    UIHelper::pause();
}

void AdminController::addSubjectToClass() {
    UIHelper::clearScreen();
    UIHelper::printHeader("Add Subject to Class");
    
    // Show available classes
    auto classes = db->getAllClasses();
    if (classes.empty()) {
        cout << "No classes available." << endl;
        UIHelper::pause();
        return;
    }
    
    cout << "\nAvailable Classes:" << endl;
    UIHelper::printSeparator(40);
    for (const auto& cls : classes) {
        cout << "ID: " << cls.at("class_id") << " - " << cls.at("class_name") << endl;
    }
    
    // Show available subjects
    auto subjects = db->getAllSubjects();
    if (subjects.empty()) {
        cout << "\nNo subjects available." << endl;
        UIHelper::pause();
        return;
    }
    
    cout << "\nAvailable Subjects:" << endl;
    UIHelper::printSeparator(40);
    for (const auto& subject : subjects) {
        cout << "ID: " << subject.at("subject_id") << " - " << subject.at("name") << endl;
    }
    UIHelper::printSeparator(40);
    
    int classId = getIntInput("\nEnter class ID: ");
    int subjectId = getIntInput("Enter subject ID: ");
    
    // FR-20: Validate entities exist
    if (!db->classExists(classId)) {
        cout << "\nError: Class does not exist!" << endl;
        UIHelper::pause();
        return;
    }
    
    if (!db->subjectExists(subjectId)) {
        cout << "\nError: Subject does not exist!" << endl;
        UIHelper::pause();
        return;
    }
    
    // FR-19: Prevent duplicate assignments
    if (db->isSubjectInClass(subjectId, classId)) {
        cout << "\nError: This subject is already assigned to this class!" << endl;
        UIHelper::pause();
        return;
    }
    
    if (db->addSubjectToClass(classId, subjectId)) {
        cout << "\nSubject added to class successfully!" << endl;
    } else {
        cout << "\nFailed to add subject to class." << endl;
    }
    
    UIHelper::pause();
}

void AdminController::viewAllSubjects() {
    UIHelper::clearScreen();
    UIHelper::printHeader("All Subjects");
    
    auto subjects = db->getAllSubjects();
    
    if (subjects.empty()) {
        cout << "No subjects found." << endl;
    } else {
        cout << left << setw(10) << "ID" 
                  << setw(30) << "Name" 
                  << setw(15) << "Max Marks" << endl;
        UIHelper::printSeparator(55);
        
        for (const auto& subject : subjects) {
            cout << left << setw(10) << subject.at("subject_id")
                      << setw(30) << subject.at("name")
                      << setw(15) << subject.at("max_marks") << endl;
        }
    }
    
    UIHelper::pause();
}

void AdminController::viewAllClasses() {
    UIHelper::clearScreen();
    UIHelper::printHeader("All Classes");
    
    auto classes = db->getAllClasses();
    
    if (classes.empty()) {
        cout << "No classes found." << endl;
    } else {
        cout << left << setw(10) << "ID" 
                  << setw(30) << "Class Name" << endl;
        UIHelper::printSeparator(40);
        
        for (const auto& cls : classes) {
            cout << left << setw(10) << cls.at("class_id")
                      << setw(30) << cls.at("class_name") << endl;
        }
    }
    
    UIHelper::pause();
}

void AdminController::viewAllTeachers() {
    UIHelper::clearScreen();
    UIHelper::printHeader("All Teachers");
    
    auto teachers = db->getAllTeachers();
    
    if (teachers.empty()) {
        cout << "No teachers found." << endl;
    } else {
        cout << left << setw(10) << "ID" 
                  << setw(25) << "Name" 
                  << setw(30) << "Email"
                  << setw(20) << "Type" << endl;
        UIHelper::printSeparator(85);
        
        for (const auto& teacher : teachers) {
            cout << left << setw(10) << teacher.at("teacher_id")
                      << setw(25) << teacher.at("name")
                      << setw(30) << teacher.at("email")
                      << setw(20) << teacher.at("teacher_type") << endl;
        }
    }
    
    UIHelper::pause();
}

void AdminController::viewAllStudents() {
    UIHelper::clearScreen();
    UIHelper::printHeader("All Students");
    
    auto students = db->getAllStudents();
    
    if (students.empty()) {
        cout << "No students found." << endl;
    } else {
        cout << left << setw(10) << "ID" 
                  << setw(30) << "Name" 
                  << setw(20) << "Class" << endl;
        UIHelper::printSeparator(60);
        
        for (const auto& student : students) {
            cout << left << setw(10) << student.at("student_id")
                      << setw(30) << student.at("name")
                      << setw(20) << student.at("class_name") << endl;
        }
    }
    
    UIHelper::pause();
}

void AdminController::showMenu() {
    int choice;
    do {
        UIHelper::clearScreen();
        UIHelper::printHeader("ADMIN MENU");
        
        cout << "1.  Create Subject" << endl;
        cout << "2.  Create Class" << endl;
        cout << "3.  Create Teacher" << endl;
        cout << "4.  Create Student" << endl;
        cout << "5.  Assign Class Teacher" << endl;
        cout << "6.  Assign Subject Teacher" << endl;
        cout << "7.  Add Subject to Class" << endl;
        cout << "8.  View All Subjects" << endl;
        cout << "9.  View All Classes" << endl;
        cout << "10. View All Teachers" << endl;
        cout << "11. View All Students" << endl;
        cout << "12. Delete Subject" << endl;
        cout << "13. Delete Class" << endl;
        cout << "14. Delete Teacher" << endl;
        cout << "15. Delete Student" << endl;
        cout << "0.  Logout" << endl;
        UIHelper::printSeparator(60);
        
        choice = getIntInput("Enter your choice: ");
        
        switch (choice) {
            case 1: createSubject(); break;
            case 2: createClass(); break;
            case 3: createTeacher(); break;
            case 4: createStudent(); break;
            case 5: assignClassTeacher(); break;
            case 6: assignSubjectTeacher(); break;
            case 7: addSubjectToClass(); break;
            case 8: viewAllSubjects(); break;
            case 9: viewAllClasses(); break;
            case 10: viewAllTeachers(); break;
            case 11: viewAllStudents(); break;
            case 12: deleteSubject(); break;
            case 13: deleteClass(); break;
            case 14: deleteTeacher(); break;
            case 15: deleteStudent(); break;
            case 0: cout << "Logging out..." << endl; break;
            default: 
                cout << "Invalid choice!" << endl;
                UIHelper::pause();
        }
    } while (choice != 0);
}

void AdminController::deleteSubject() {
    UIHelper::clearScreen();
    UIHelper::printHeader("Delete Subject");
    
    viewAllSubjects();
    
    int subjectId = getIntInput("\nEnter subject ID to delete (0 to cancel): ");
    if (subjectId == 0) return;
    
    if (!db->subjectExists(subjectId)) {
        cout << "\nError: Subject does not exist!" << endl;
        UIHelper::pause();
        return;
    }
    
    cout << "Are you sure you want to delete this subject? (1-Yes / 2-No): ";
    int confirm = getIntInput("");
    
    if (confirm == 1) {
        if (db->deleteSubject(subjectId)) {
            cout << "\nSubject deleted successfully!" << endl;
        } else {
            cout << "\nFailed to delete subject. It may be referenced by other records." << endl;
        }
    } else {
        cout << "\nDeletion cancelled." << endl;
    }
    
    UIHelper::pause();
}

void AdminController::deleteClass() {
    UIHelper::clearScreen();
    UIHelper::printHeader("Delete Class");
    
    viewAllClasses();
    
    int classId = getIntInput("\nEnter class ID to delete (0 to cancel): ");
    if (classId == 0) return;
    
    if (!db->classExists(classId)) {
        cout << "\nError: Class does not exist!" << endl;
        UIHelper::pause();
        return;
    }
    
    cout << "Are you sure you want to delete this class? (1-Yes / 2-No): ";
    int confirm = getIntInput("");
    
    if (confirm == 1) {
        if (db->deleteClass(classId)) {
            cout << "\nClass deleted successfully!" << endl;
        } else {
            cout << "\nFailed to delete class. It may be referenced by other records." << endl;
        }
    } else {
        cout << "\nDeletion cancelled." << endl;
    }
    
    UIHelper::pause();
}

void AdminController::deleteTeacher() {
    UIHelper::clearScreen();
    UIHelper::printHeader("Delete Teacher");
    
    viewAllTeachers();
    
    int teacherId = getIntInput("\nEnter teacher ID to delete (0 to cancel): ");
    if (teacherId == 0) return;
    
    if (!db->teacherExists(teacherId)) {
        cout << "\nError: Teacher does not exist!" << endl;
        UIHelper::pause();
        return;
    }
    
    cout << "Are you sure you want to delete this teacher? (1-Yes / 2-No): ";
    int confirm = getIntInput("");
    
    if (confirm == 1) {
        if (db->deleteTeacher(teacherId)) {
            cout << "\nTeacher deleted successfully!" << endl;
        } else {
            cout << "\nFailed to delete teacher. It may be referenced by other records." << endl;
        }
    } else {
        cout << "\nDeletion cancelled." << endl;
    }
    
    UIHelper::pause();
}

void AdminController::deleteStudent() {
    UIHelper::clearScreen();
    UIHelper::printHeader("Delete Student");
    
    viewAllStudents();
    
    int studentId = getIntInput("\nEnter student ID to delete (0 to cancel): ");
    if (studentId == 0) return;
    
    if (!db->studentExists(studentId)) {
        cout << "\nError: Student does not exist!" << endl;
        UIHelper::pause();
        return;
    }
    
    cout << "Are you sure you want to delete this student? (1-Yes / 2-No): ";
    int confirm = getIntInput("");
    
    if (confirm == 1) {
        if (db->deleteStudent(studentId)) {
            cout << "\nStudent deleted successfully!" << endl;
        } else {
            cout << "\nFailed to delete student." << endl;
        }
    } else {
        cout << "\nDeletion cancelled." << endl;
    }
    
    UIHelper::pause();
}
