#include "TeacherController.h"
#include "UIHelper.h"
#include <iostream>
#include <iomanip>

using namespace std;

TeacherController::TeacherController(Database* db, int id, const string& name, const string& type)
    : BaseController(db), teacherId(id), teacherName(name), teacherType(type) {}

void TeacherController::markAttendance() {
    UIHelper::clearScreen();
    UIHelper::printHeader("Mark Attendance");
    
    if (teacherType == "ClassTeacher") {
        // Get teacher's assigned class
        auto assignment = db->getTeacherClassAssignment(teacherId);
        if (assignment.empty()) {
            cout << "You are not assigned to any class." << endl;
            UIHelper::pause();
            return;
        }
        
        int classId = stoi(assignment["class_id"]);
        cout << "Your Class: " << assignment["class_name"] << endl;
        
        // Get subjects that this teacher is assigned to teach (as SubjectTeacher) in their class
        auto teacherSubjects = db->getTeacherSubjectAssignments(teacherId);
        
        // Filter to only subjects in this specific class
        vector<map<string, string>> assignedSubjects;
        for (const auto& subj : teacherSubjects) {
            if (stoi(subj.at("class_id")) == classId) {
                assignedSubjects.push_back(subj);
            }
        }
        
        if (assignedSubjects.empty()) {
            cout << "\nYou are not assigned to teach any subjects in this class." << endl;
            cout << "ClassTeacher can only mark attendance for subjects they are assigned to teach." << endl;
            UIHelper::pause();
            return;
        }
        
        // Get students in this class
        auto students = db->getStudentsByClass(classId);
        if (students.empty()) {
            cout << "No students in this class." << endl;
            UIHelper::pause();
            return;
        }
        
        cout << "\nYour Assigned Subjects:" << endl;
        UIHelper::printSeparator(40);
        for (const auto& subject : assignedSubjects) {
            cout << "ID: " << subject.at("subject_id") << " - " << subject.at("subject_name") << endl;
        }
        UIHelper::printSeparator(40);
        
        int subjectId = getIntInput("\nEnter subject ID: ");
        
        // Validate teacher is assigned to this subject
        bool isAssigned = false;
        for (const auto& subj : assignedSubjects) {
            if (stoi(subj.at("subject_id")) == subjectId) {
                isAssigned = true;
                break;
            }
        }
        
        if (!isAssigned) {
            cout << "\nError: You are not assigned to teach this subject!" << endl;
            UIHelper::pause();
            return;
        }
        
        string date = getDate("Enter attendance date:");
        
        cout << "\nStudents in class:" << endl;
        UIHelper::printSeparator(60);
        
        for (const auto& student : students) {
            cout << "\nStudent: " << student.at("name") << " (ID: " << student.at("student_id") << ")" << endl;
            cout << "Mark as (1-Present / 2-Absent): ";
            int statusChoice;
            cin >> statusChoice;
            clearInputBuffer();
            
            string status = (statusChoice == 1) ? "Present" : "Absent";
            int studentId = stoi(student.at("student_id"));
            
            if (db->markAttendance(studentId, subjectId, classId, date, status)) {
                cout << "Marked " << status << endl;
            } else {
                cout << "Failed to mark attendance (may already be marked)" << endl;
            }
        }
        
    } else if (teacherType == "SubjectTeacher") {
        // Get teacher's subject-class assignments
        auto assignments = db->getTeacherSubjectAssignments(teacherId);
        if (assignments.empty()) {
            cout << "You are not assigned to any subject-class combination." << endl;
            UIHelper::pause();
            return;
        }
        
        cout << "Your Assignments:" << endl;
        UIHelper::printSeparator(60);
        for (size_t i = 0; i < assignments.size(); ++i) {
            cout << (i + 1) << ". " << assignments[i].at("subject_name") 
                      << " - " << assignments[i].at("class_name") << endl;
        }
        UIHelper::printSeparator(60);
        
        int choice = getIntInput("\nSelect assignment (number): ");
        if (choice < 1 || choice > static_cast<int>(assignments.size())) {
            cout << "Invalid choice." << endl;
            UIHelper::pause();
            return;
        }
        
        auto selectedAssignment = assignments[choice - 1];
        int subjectId = stoi(selectedAssignment["subject_id"]);
        int classId = stoi(selectedAssignment["class_id"]);
        
        // Get students in this class
        auto students = db->getStudentsByClass(classId);
        if (students.empty()) {
            cout << "No students in this class." << endl;
            UIHelper::pause();
            return;
        }
        
        string date = getDate("Enter attendance date:");
        
        cout << "\nStudents in class:" << endl;
        UIHelper::printSeparator(60);
        
        for (const auto& student : students) {
            cout << "\nStudent: " << student.at("name") << " (ID: " << student.at("student_id") << ")" << endl;
            cout << "Mark as (1-Present / 2-Absent): ";
            int statusChoice;
            cin >> statusChoice;
            clearInputBuffer();
            
            string status = (statusChoice == 1) ? "Present" : "Absent";
            int studentId = stoi(student.at("student_id"));
            
            if (db->markAttendance(studentId, subjectId, classId, date, status)) {
                cout << "Marked " << status << endl;
            } else {
                cout << "Failed to mark attendance (may already be marked)" << endl;
            }
        }
    }
    
    cout << "\nAttendance marking completed!" << endl;
    UIHelper::pause();
}

void TeacherController::viewStudentAttendance() {
    UIHelper::clearScreen();
    UIHelper::printHeader("View Student Attendance");
    
    int studentId = getIntInput("Enter student ID: ");
    
    auto records = db->getStudentAttendance(studentId);
    
    if (records.empty()) {
        cout << "No attendance records found for this student." << endl;
    } else {
        cout << left << setw(15) << "Date" 
                  << setw(30) << "Subject" 
                  << setw(15) << "Status" << endl;
        UIHelper::printSeparator(60);
        
        for (const auto& record : records) {
            cout << left << setw(15) << record.at("date")
                      << setw(30) << record.at("subject")
                      << setw(15) << record.at("status") << endl;
        }
    }
    
    UIHelper::pause();
}

void TeacherController::viewClassAttendance() {
    UIHelper::clearScreen();
    UIHelper::printHeader("View Class Attendance");
    
    int classId;
    
    if (teacherType == "ClassTeacher") {
        auto assignment = db->getTeacherClassAssignment(teacherId);
        if (assignment.empty()) {
            cout << "You are not assigned to any class." << endl;
            UIHelper::pause();
            return;
        }
        classId = stoi(assignment["class_id"]);
        cout << "Viewing attendance for: " << assignment["class_name"] << endl;
    } else {
        auto assignments = db->getTeacherSubjectAssignments(teacherId);
        if (assignments.empty()) {
            cout << "You are not assigned to any class." << endl;
            UIHelper::pause();
            return;
        }
        
        cout << "Your Classes:" << endl;
        UIHelper::printSeparator(40);
        for (size_t i = 0; i < assignments.size(); ++i) {
            cout << (i + 1) << ". " << assignments[i].at("class_name") << endl;
        }
        UIHelper::printSeparator(40);
        
        int choice = getIntInput("\nSelect class (number): ");
        if (choice < 1 || choice > static_cast<int>(assignments.size())) {
            cout << "Invalid choice." << endl;
            UIHelper::pause();
            return;
        }
        
        classId = stoi(assignments[choice - 1]["class_id"]);
    }
    
    auto records = db->getClassAttendance(classId);
    
    if (records.empty()) {
        cout << "\nNo attendance records found for this class." << endl;
    } else {
        cout << "\n" << left << setw(15) << "Date" 
                  << setw(25) << "Student" 
                  << setw(25) << "Subject"
                  << setw(15) << "Status" << endl;
        UIHelper::printSeparator(80);
        
        for (const auto& record : records) {
            cout << left << setw(15) << record.at("date")
                      << setw(25) << record.at("student")
                      << setw(25) << record.at("subject")
                      << setw(15) << record.at("status") << endl;
        }
    }
    
    UIHelper::pause();
}

void TeacherController::addStudentToClass() {
    UIHelper::clearScreen();
    UIHelper::printHeader("Add Student to Class");
    
    if (teacherType != "ClassTeacher") {
        cout << "Only Class Teachers can add students to their class." << endl;
        UIHelper::pause();
        return;
    }
    
    auto assignment = db->getTeacherClassAssignment(teacherId);
    if (assignment.empty()) {
        cout << "You are not assigned to any class." << endl;
        UIHelper::pause();
        return;
    }
    
    int classId = stoi(assignment["class_id"]);
    cout << "Adding student to: " << assignment["class_name"] << endl;
    
    string name = getInput("\nEnter student name: ");
    
    int id = db->createStudent(name, classId);
    if (id > 0) {
        cout << "\nStudent added successfully with ID: " << id << endl;
    } else {
        cout << "\nFailed to add student." << endl;
    }
    
    UIHelper::pause();
}

void TeacherController::showMenu() {
    int choice;
    do {
        UIHelper::clearScreen();
        UIHelper::printHeader("TEACHER MENU");
        cout << "Welcome, " << teacherName << " (" << teacherType << ")" << endl;
        UIHelper::printSeparator(60);
        
        cout << "1. Mark Attendance" << endl;
        cout << "2. View Student Attendance" << endl;
        cout << "3. View Class Attendance" << endl;
        
        if (teacherType == "ClassTeacher") {
            cout << "4. Add Student to Class" << endl;
        }
        
        cout << "0. Logout" << endl;
        UIHelper::printSeparator(60);
        
        choice = getIntInput("Enter your choice: ");
        
        switch (choice) {
            case 1: markAttendance(); break;
            case 2: viewStudentAttendance(); break;
            case 3: viewClassAttendance(); break;
            case 4:
                if (teacherType == "ClassTeacher") {
                    addStudentToClass();
                } else {
                    cout << "Invalid choice!" << endl;
                    UIHelper::pause();
                }
                break;
            case 0: cout << "Logging out..." << endl; break;
            default:
                cout << "Invalid choice!" << endl;
                UIHelper::pause();
        }
    } while (choice != 0);
}
