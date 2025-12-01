#include "StudentController.h"
#include "UIHelper.h"
#include <iostream>
#include <iomanip>

using namespace std;

StudentController::StudentController(Database* db, int id, const string& name)
    : BaseController(db), studentId(id), studentName(name) {}

void StudentController::viewMyAttendance() {
    UIHelper::clearScreen();
    UIHelper::printHeader("My Attendance Records");
    
    auto records = db->getStudentAttendance(studentId);
    
    if (records.empty()) {
        cout << "No attendance records found." << endl;
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

void StudentController::viewAttendancePercentage() {
    UIHelper::clearScreen();
    UIHelper::printHeader("Attendance Percentage");
    
    // Overall percentage
    double overallPercentage = db->getAttendancePercentage(studentId, 0);
    
    cout << "Overall Attendance: " << fixed << setprecision(2) 
              << overallPercentage << "%" << endl;
    
    // Get all subjects for subject-wise percentage
    auto subjects = db->getAllSubjects();
    
    if (!subjects.empty()) {
        cout << "\nSubject-wise Attendance:" << endl;
        UIHelper::printSeparator(50);
        cout << left << setw(30) << "Subject" 
                  << setw(20) << "Percentage" << endl;
        UIHelper::printSeparator(50);
        
        for (const auto& subject : subjects) {
            int subjectId = stoi(subject.at("subject_id"));
            double percentage = db->getAttendancePercentage(studentId, subjectId);
            
            if (percentage > 0) {  // Only show subjects with attendance records
                cout << left << setw(30) << subject.at("name")
                          << setw(20) << (to_string(static_cast<int>(percentage)) + "%") 
                          << endl;
            }
        }
    }
    
    UIHelper::pause();
}

void StudentController::showMenu() {
    int choice;
    do {
        UIHelper::clearScreen();
        UIHelper::printHeader("STUDENT MENU");
        cout << "Welcome, " << studentName << endl;
        UIHelper::printSeparator(60);
        
        cout << "1. View My Attendance" << endl;
        cout << "2. View Attendance Percentage" << endl;
        cout << "3. Update Profile" << endl;
        cout << "0. Logout" << endl;
        UIHelper::printSeparator(60);
        
        choice = getIntInput("Enter your choice: ");
        
        switch (choice) {
            case 1: viewMyAttendance(); break;
            case 2: viewAttendancePercentage(); break;
            case 3: updateProfile(); break;
            case 0: cout << "Logging out..." << endl; break;
            default:
                cout << "Invalid choice!" << endl;
                UIHelper::pause();
        }
    } while (choice != 0);
}

void StudentController::updateProfile() {
    UIHelper::clearScreen();
    UIHelper::printHeader("Update Profile");
    
    cout << "Current Name: " << studentName << endl;
    cout << "\n1. Update Name" << endl;
    cout << "0. Cancel" << endl;
    UIHelper::printSeparator(40);
    
    int choice = getIntInput("Enter your choice: ");
    
    if (choice == 1) {
        string newName = getInput("Enter new name: ");
        
        if (db->updateStudentName(studentId, newName)) {
            studentName = newName;
            cout << "\nProfile updated successfully!" << endl;
        } else {
            cout << "\nFailed to update profile." << endl;
        }
    }
    
    UIHelper::pause();
}
