#include "Database.h"
#include "Config.h"
#include "UIHelper.h"
#include "AdminController.h"
#include "TeacherController.h"
#include "StudentController.h"
#include <iostream>
#include <memory>
#include <limits>

using namespace std;

void showMainMenu() {
    UIHelper::printHeader("ATTENDANCE MANAGEMENT SYSTEM");
    cout << "1. Admin Login" << endl;
    cout << "2. Teacher Login" << endl;
    cout << "3. Student Login" << endl;
    cout << "0. Exit" << endl;
    UIHelper::printSeparator(60);
}

string getInput(const string& prompt) {
    string input;
    cout << prompt;
    getline(cin, input);
    return input;
}

int getIntInput(const string& prompt) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        } else {
            cout << "Invalid input. Please enter a number." << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
}

void adminLogin(Database* db) {
    UIHelper::clearScreen();
    UIHelper::printHeader("Admin Login");
    
    string email = getInput("Email: ");
    string password = getInput("Password: ");
    
    if (db->authenticateAdmin(email, password)) {
        cout << "\nLogin successful!" << endl;
        UIHelper::pause();
        
        AdminController adminController(db);
        adminController.showMenu();
    } else {
        cout << "\nInvalid credentials!" << endl;
        UIHelper::pause();
    }
}

void teacherLogin(Database* db) {
    UIHelper::clearScreen();
    UIHelper::printHeader("Teacher Login");
    
    string email = getInput("Email: ");
    string password = getInput("Password: ");
    
    auto teacherData = db->authenticateTeacher(email, password);
    
    if (!teacherData.empty()) {
        cout << "\nLogin successful!" << endl;
        UIHelper::pause();
        
        int teacherId = stoi(teacherData["teacher_id"]);
        string name = teacherData["name"];
        string type = teacherData["teacher_type"];
        
        TeacherController teacherController(db, teacherId, name, type);
        teacherController.showMenu();
    } else {
        cout << "\nInvalid credentials!" << endl;
        UIHelper::pause();
    }
}

void studentLogin(Database* db) {
    UIHelper::clearScreen();
    UIHelper::printHeader("Student Login");
    
    int studentId = getIntInput("Enter your Student ID: ");
    
    auto studentData = db->getStudentById(studentId);
    
    if (!studentData.empty()) {
        cout << "\nLogin successful!" << endl;
        cout << "Welcome, " << studentData["name"] << "!" << endl;
        UIHelper::pause();
        
        StudentController studentController(db, studentId, studentData["name"]);
        studentController.showMenu();
    } else {
        cout << "\nStudent not found!" << endl;
        UIHelper::pause();
    }
}

int main() {
    // Load configuration
    auto config = Config::loadConfig("config.txt");
    
    if (config.empty()) {
        cerr << "Failed to load configuration file!" << endl;
        cerr << "Please ensure config.txt exists with proper MySQL connection details." << endl;
        return 1;
    }
    
    // Connect to database
    Database db(config);
    
    if (!db.isConnected()) {
        cerr << "Failed to connect to database!" << endl;
        cerr << "Please check your configuration and ensure MySQL is running." << endl;
        return 1;
    }
    
    cout << "Database connection successful!" << endl;
    UIHelper::pause();
    
    int choice;
    do {
        UIHelper::clearScreen();
        showMainMenu();
        choice = getIntInput("Enter your choice: ");
        
        switch (choice) {
            case 1:
                adminLogin(&db);
                break;
            case 2:
                teacherLogin(&db);
                break;
            case 3:
                studentLogin(&db);
                break;
            case 0:
                cout << "Thank you for using Attendance Management System!" << endl;
                break;
            default:
                cout << "Invalid choice!" << endl;
                UIHelper::pause();
        }
    } while (choice != 0);
    
    return 0;
}
