#ifndef DATABASE_H
#define DATABASE_H

#include <mysql/mysql.h>
#include <string>
#include <vector>
#include <map>

using namespace std;

class Database {
private:
    MYSQL* conn;
    map<string, string> connectionConfig;
    
    // Helper method for connection management
    bool reconnect();
    bool ensureConnection();
    
public:
    Database(const map<string, string>& config);
    ~Database();
    
    bool isConnected() const;
    
    // Authentication
    bool authenticateAdmin(const string& email, const string& password);
    map<string, string> authenticateTeacher(const string& email, const string& password);
    
    // CRUD - Subjects
    int createSubject(const string& name, int maxMarks);
    vector<map<string, string>> getAllSubjects();
    map<string, string> getSubjectById(int id);
    
    // CRUD - Classes
    int createClass(const string& className);
    vector<map<string, string>> getAllClasses();
    map<string, string> getClassById(int id);
    
    // CRUD - Teachers
    int createTeacher(const string& name, const string& email, 
                     const string& password, double salary, 
                     const string& joinDate, const string& type);
    vector<map<string, string>> getAllTeachers();
    map<string, string> getTeacherById(int id);
    map<string, string> getTeacherClassAssignment(int teacherId);
    vector<map<string, string>> getTeacherSubjectAssignments(int teacherId);
    
    // Teacher assignments
    bool assignClassTeacher(int classId, int teacherId);
    bool assignSubjectTeacher(int teacherId, int subjectId, int classId);
    
    // CRUD - Students
    int createStudent(const string& name, int classId);
    vector<map<string, string>> getAllStudents();
    map<string, string> getStudentById(int id);
    vector<map<string, string>> getStudentsByClass(int classId);
    bool updateStudentName(int studentId, const string& newName);
    
    // Delete operations
    bool deleteSubject(int subjectId);
    bool deleteClass(int classId);
    bool deleteTeacher(int teacherId);
    bool deleteStudent(int studentId);
    
    // Validation operations
    bool classExists(int classId);
    bool subjectExists(int subjectId);
    bool teacherExists(int teacherId);
    bool studentExists(int studentId);
    bool isClassNameUnique(const string& className);
    bool isSubjectNameUnique(const string& subjectName);
    bool isAttendanceMarked(int studentId, int subjectId, const string& date);
    bool isSubjectInClass(int subjectId, int classId);
    
    // Attendance operations
    bool markAttendance(int studentId, int subjectId, int classId, 
                       const string& date, const string& status);
    vector<map<string, string>> getStudentAttendance(int studentId);
    vector<map<string, string>> getClassAttendance(int classId);
    double getAttendancePercentage(int studentId, int subjectId);
    
    // Class-Subject operations
    bool addSubjectToClass(int classId, int subjectId);
    vector<map<string, string>> getClassSubjects(int classId);
    
    // Helper methods
    string escapeString(const string& str);
};

#endif // DATABASE_H
