#include "Database.h"
#include <iostream>
#include <sstream>
#include <cstring>

using namespace std;

Database::Database(const map<string, string>& config) : connectionConfig(config) {
    conn = mysql_init(nullptr);
    
    if (conn == nullptr) {
        cerr << "MySQL initialization failed" << endl;
        return;
    }
    
    // Enable auto-reconnect
    bool auto_reconnect = true;
    mysql_options(conn, MYSQL_OPT_RECONNECT, &auto_reconnect);
    
    // Set connection timeout
    unsigned int timeout = 28800; // 8 hours
    mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
    mysql_options(conn, MYSQL_OPT_READ_TIMEOUT, &timeout);
    mysql_options(conn, MYSQL_OPT_WRITE_TIMEOUT, &timeout);
    
    string host = config.count("host") ? config.at("host") : "localhost";
    string user = config.count("user") ? config.at("user") : "root";
    string password = config.count("password") ? config.at("password") : "";
    string database = config.count("database") ? config.at("database") : "attendance_system";
    int port = config.count("port") ? stoi(config.at("port")) : 3306;
    
    if (mysql_real_connect(conn, host.c_str(), user.c_str(), password.c_str(), 
                           database.c_str(), port, nullptr, 0) == nullptr) {
        cerr << "Connection failed: " << mysql_error(conn) << endl;
        mysql_close(conn);
        conn = nullptr;
    }
}

Database::~Database() {
    if (conn != nullptr) {
        mysql_close(conn);
    }
}

bool Database::isConnected() const {
    return conn != nullptr;
}

bool Database::reconnect() {
    // Close existing connection only if it's valid
    if (conn != nullptr) {
        try {
            mysql_close(conn);
        } catch (...) {
            cerr << "Error closing connection, continuing with reconnect..." << endl;
        }
        conn = nullptr;
    }
    
    conn = mysql_init(nullptr);
    if (conn == nullptr) {
        cerr << "MySQL re-initialization failed" << endl;
        return false;
    }
    
    // Enable auto-reconnect
    bool auto_reconnect = true;
    mysql_options(conn, MYSQL_OPT_RECONNECT, &auto_reconnect);
    
    // Set connection timeout
    unsigned int timeout = 28800;
    mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
    mysql_options(conn, MYSQL_OPT_READ_TIMEOUT, &timeout);
    mysql_options(conn, MYSQL_OPT_WRITE_TIMEOUT, &timeout);
    
    if (connectionConfig.empty()) {
        cerr << "Connection configuration is empty" << endl;
        return false;
    }
    
    string host = connectionConfig.count("host") ? connectionConfig.at("host") : "localhost";
    string user = connectionConfig.count("user") ? connectionConfig.at("user") : "root";
    string password = connectionConfig.count("password") ? connectionConfig.at("password") : "";
    string database = connectionConfig.count("database") ? connectionConfig.at("database") : "attendance_system";
    int port = connectionConfig.count("port") ? stoi(connectionConfig.at("port")) : 3306;
    
    if (mysql_real_connect(conn, host.c_str(), user.c_str(), password.c_str(),
                           database.c_str(), port, nullptr, 0) == nullptr) {
        cerr << "Reconnection failed: " << mysql_error(conn) << endl;
        mysql_close(conn);
        conn = nullptr;
        return false;
    }
    
    cout << "Database reconnected successfully" << endl;
    return true;
}

bool Database::ensureConnection() {
    if (conn == nullptr) {
        return reconnect();
    }
    
    // Ping the connection to check if it's alive
    if (mysql_ping(conn) != 0) {
        cerr << "Connection lost, attempting to reconnect..." << endl;
        return reconnect();
    }
    
    return true;
}

string Database::escapeString(const string& str) {
    // Return input unchanged if no connection (prevent segfault)
    if (conn == nullptr) {
        return str;
    }
    
    char* escaped = new char[str.length() * 2 + 1];
    mysql_real_escape_string(conn, escaped, str.c_str(), str.length());
    string result(escaped);
    delete[] escaped;
    return result;
}

// Authentication
bool Database::authenticateAdmin(const string& email, const string& password) {
    if (!ensureConnection()) {
        cerr << "Database connection unavailable" << endl;
        return false;
    }
    
    string query = "SELECT * FROM admins WHERE email='" + escapeString(email) + 
                       "' AND password='" + escapeString(password) + "'";
    
    if (mysql_query(conn, query.c_str())) {
        cerr << "Query failed: " << mysql_error(conn) << endl;
        return false;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    bool authenticated = (mysql_num_rows(result) > 0);
    mysql_free_result(result);
    
    return authenticated;
}

map<string, string> Database::authenticateTeacher(const string& email, const string& password) {
    map<string, string> teacherData;
    
    if (!ensureConnection()) {
        cerr << "Database connection unavailable" << endl;
        return teacherData;
    }
    
    string query = "SELECT teacher_id, name, teacher_type FROM teachers WHERE email='" + 
                       escapeString(email) + "' AND password='" + escapeString(password) + "'";
    
    if (mysql_query(conn, query.c_str())) {
        cerr << "Query failed: " << mysql_error(conn) << endl;
        return teacherData;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (mysql_num_rows(result) > 0) {
        MYSQL_ROW row = mysql_fetch_row(result);
        teacherData["teacher_id"] = row[0] ? row[0] : "";
        teacherData["name"] = row[1] ? row[1] : "";
        teacherData["teacher_type"] = row[2] ? row[2] : "";
    }
    mysql_free_result(result);
    
    return teacherData;
}

// CRUD - Subjects
int Database::createSubject(const string& name, int maxMarks) {
    if (!ensureConnection()) return -1;
    
    string query = "INSERT INTO subjects (name, max_marks) VALUES ('" + 
                       escapeString(name) + "', " + to_string(maxMarks) + ")";
    
    if (mysql_query(conn, query.c_str())) {
        cerr << "Insert failed: " << mysql_error(conn) << endl;
        return -1;
    }
    
    return mysql_insert_id(conn);
}

vector<map<string, string>> Database::getAllSubjects() {
    vector<map<string, string>> subjects;
    if (!ensureConnection()) return subjects;
    
    if (mysql_query(conn, "SELECT * FROM subjects")) {
        cerr << "Query failed: " << mysql_error(conn) << endl;
        return subjects;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    MYSQL_ROW row;
    
    while ((row = mysql_fetch_row(result))) {
        map<string, string> subject;
        subject["subject_id"] = row[0] ? row[0] : "";
        subject["name"] = row[1] ? row[1] : "";
        subject["max_marks"] = row[2] ? row[2] : "";
        subjects.push_back(subject);
    }
    
    mysql_free_result(result);
    return subjects;
}

map<string, string> Database::getSubjectById(int id) {
    map<string, string> subject;
    if (!ensureConnection()) return subject;
    
    string query = "SELECT * FROM subjects WHERE subject_id=" + to_string(id);
    
    if (mysql_query(conn, query.c_str())) {
        cerr << "Query failed: " << mysql_error(conn) << endl;
        return subject;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (mysql_num_rows(result) > 0) {
        MYSQL_ROW row = mysql_fetch_row(result);
        subject["subject_id"] = row[0] ? row[0] : "";
        subject["name"] = row[1] ? row[1] : "";
        subject["max_marks"] = row[2] ? row[2] : "";
    }
    mysql_free_result(result);
    
    return subject;
}

// CRUD - Classes
int Database::createClass(const string& className) {
    if (!ensureConnection()) return -1;
    
    string query = "INSERT INTO classes (class_name) VALUES ('" + 
                       escapeString(className) + "')";
    
    if (mysql_query(conn, query.c_str())) {
        cerr << "Insert failed: " << mysql_error(conn) << endl;
        return -1;
    }
    
    return mysql_insert_id(conn);
}

vector<map<string, string>> Database::getAllClasses() {
    vector<map<string, string>> classes;
    if (!ensureConnection()) return classes;
    
    if (mysql_query(conn, "SELECT * FROM classes")) {
        cerr << "Query failed: " << mysql_error(conn) << endl;
        return classes;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    MYSQL_ROW row;
    
    while ((row = mysql_fetch_row(result))) {
        map<string, string> cls;
        cls["class_id"] = row[0] ? row[0] : "";
        cls["class_name"] = row[1] ? row[1] : "";
        classes.push_back(cls);
    }
    
    mysql_free_result(result);
    return classes;
}

map<string, string> Database::getClassById(int id) {
    map<string, string> cls;
    if (!ensureConnection()) return cls;
    
    string query = "SELECT * FROM classes WHERE class_id=" + to_string(id);
    
    if (mysql_query(conn, query.c_str())) {
        cerr << "Query failed: " << mysql_error(conn) << endl;
        return cls;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (mysql_num_rows(result) > 0) {
        MYSQL_ROW row = mysql_fetch_row(result);
        cls["class_id"] = row[0] ? row[0] : "";
        cls["class_name"] = row[1] ? row[1] : "";
    }
    mysql_free_result(result);
    
    return cls;
}

// CRUD - Teachers
int Database::createTeacher(const string& name, const string& email, 
                           const string& password, double salary, 
                           const string& joinDate, const string& type) {
    if (!ensureConnection()) return -1;
    
    string query = string("INSERT INTO teachers (name, email, password, salary, join_date, teacher_type) VALUES ('") +
                       escapeString(name) + "', '" + escapeString(email) + "', '" +
                       escapeString(password) + "', " + to_string(salary) + ", '" + 
                       joinDate + "', '" + escapeString(type) + "')";
    
    if (mysql_query(conn, query.c_str())) {
        cerr << "Insert failed: " << mysql_error(conn) << endl;
        return -1;
    }
    
    return mysql_insert_id(conn);
}

vector<map<string, string>> Database::getAllTeachers() {
    vector<map<string, string>> teachers;
    if (!ensureConnection()) return teachers;
    
    if (mysql_query(conn, "SELECT * FROM teachers")) {
        cerr << "Query failed: " << mysql_error(conn) << endl;
        return teachers;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    MYSQL_ROW row;
    
    while ((row = mysql_fetch_row(result))) {
        map<string, string> teacher;
        teacher["teacher_id"] = row[0] ? row[0] : "";
        teacher["name"] = row[1] ? row[1] : "";
        teacher["email"] = row[2] ? row[2] : "";
        teacher["salary"] = row[4] ? row[4] : "";
        teacher["join_date"] = row[5] ? row[5] : "";
        teacher["teacher_type"] = row[6] ? row[6] : "";
        teachers.push_back(teacher);
    }
    
    mysql_free_result(result);
    return teachers;
}

map<string, string> Database::getTeacherById(int id) {
    map<string, string> teacher;
    if (!ensureConnection()) return teacher;
    
    string query = "SELECT * FROM teachers WHERE teacher_id=" + to_string(id);
    
    if (mysql_query(conn, query.c_str())) {
        cerr << "Query failed: " << mysql_error(conn) << endl;
        return teacher;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (mysql_num_rows(result) > 0) {
        MYSQL_ROW row = mysql_fetch_row(result);
        teacher["teacher_id"] = row[0] ? row[0] : "";
        teacher["name"] = row[1] ? row[1] : "";
        teacher["email"] = row[2] ? row[2] : "";
        teacher["teacher_type"] = row[6] ? row[6] : "";
    }
    mysql_free_result(result);
    
    return teacher;
}

map<string, string> Database::getTeacherClassAssignment(int teacherId) {
    map<string, string> assignment;
    
    string query = "SELECT tca.class_id, c.class_name FROM teacher_class_assignments tca "
                       "JOIN classes c ON tca.class_id = c.class_id "
                       "WHERE tca.teacher_id=" + to_string(teacherId);
    
    if (mysql_query(conn, query.c_str())) {
        cerr << "Query failed: " << mysql_error(conn) << endl;
        return assignment;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (mysql_num_rows(result) > 0) {
        MYSQL_ROW row = mysql_fetch_row(result);
        assignment["class_id"] = row[0] ? row[0] : "";
        assignment["class_name"] = row[1] ? row[1] : "";
    }
    mysql_free_result(result);
    
    return assignment;
}

vector<map<string, string>> Database::getTeacherSubjectAssignments(int teacherId) {
    vector<map<string, string>> assignments;
    
    string query = "SELECT tsa.subject_id, s.name, tsa.class_id, c.class_name "
                       "FROM teacher_subject_assignments tsa "
                       "JOIN subjects s ON tsa.subject_id = s.subject_id "
                       "JOIN classes c ON tsa.class_id = c.class_id "
                       "WHERE tsa.teacher_id=" + to_string(teacherId);
    
    if (mysql_query(conn, query.c_str())) {
        cerr << "Query failed: " << mysql_error(conn) << endl;
        return assignments;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    MYSQL_ROW row;
    
    while ((row = mysql_fetch_row(result))) {
        map<string, string> assignment;
        assignment["subject_id"] = row[0] ? row[0] : "";
        assignment["subject_name"] = row[1] ? row[1] : "";
        assignment["class_id"] = row[2] ? row[2] : "";
        assignment["class_name"] = row[3] ? row[3] : "";
        assignments.push_back(assignment);
    }
    
    mysql_free_result(result);
    return assignments;
}

bool Database::assignClassTeacher(int classId, int teacherId) {
    string query = "INSERT INTO teacher_class_assignments (class_id, teacher_id) VALUES (" + 
                       to_string(classId) + ", " + to_string(teacherId) + ") "
                       "ON DUPLICATE KEY UPDATE teacher_id=" + to_string(teacherId);
    
    if (mysql_query(conn, query.c_str())) {
        cerr << "Assignment failed: " << mysql_error(conn) << endl;
        return false;
    }
    
    return true;
}

bool Database::assignSubjectTeacher(int teacherId, int subjectId, int classId) {
    string query = "INSERT INTO teacher_subject_assignments (teacher_id, subject_id, class_id) "
                       "VALUES (" + to_string(teacherId) + ", " + to_string(subjectId) + 
                       ", " + to_string(classId) + ")";
    
    if (mysql_query(conn, query.c_str())) {
        cerr << "Assignment failed: " << mysql_error(conn) << endl;
        return false;
    }
    
    return true;
}

// CRUD - Students
int Database::createStudent(const string& name, int classId) {
    if (!ensureConnection()) return -1;
    
    string query = "INSERT INTO students (name, class_id) VALUES ('" + 
                       escapeString(name) + "', " + to_string(classId) + ")";
    
    if (mysql_query(conn, query.c_str())) {
        cerr << "Insert failed: " << mysql_error(conn) << endl;
        return -1;
    }
    
    return mysql_insert_id(conn);
}

vector<map<string, string>> Database::getAllStudents() {
    vector<map<string, string>> students;
    if (!ensureConnection()) return students;
    
    string query = string("SELECT s.student_id, s.name, s.class_id, c.class_name ") +
                       "FROM students s LEFT JOIN classes c ON s.class_id = c.class_id";
    
    if (mysql_query(conn, query.c_str())) {
        cerr << "Query failed: " << mysql_error(conn) << endl;
        return students;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    MYSQL_ROW row;
    
    while ((row = mysql_fetch_row(result))) {
        map<string, string> student;
        student["student_id"] = row[0] ? row[0] : "";
        student["name"] = row[1] ? row[1] : "";
        student["class_id"] = row[2] ? row[2] : "";
        student["class_name"] = row[3] ? row[3] : "N/A";
        students.push_back(student);
    }
    
    mysql_free_result(result);
    return students;
}

map<string, string> Database::getStudentById(int id) {
    map<string, string> student;
    if (!ensureConnection()) return student;
    
    string query = string("SELECT s.student_id, s.name, s.class_id, c.class_name ") +
                       "FROM students s LEFT JOIN classes c ON s.class_id = c.class_id " +
                       "WHERE s.student_id=" + to_string(id);
    
    if (mysql_query(conn, query.c_str())) {
        cerr << "Query failed: " << mysql_error(conn) << endl;
        return student;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    if (mysql_num_rows(result) > 0) {
        MYSQL_ROW row = mysql_fetch_row(result);
        student["student_id"] = row[0] ? row[0] : "";
        student["name"] = row[1] ? row[1] : "";
        student["class_id"] = row[2] ? row[2] : "";
        student["class_name"] = row[3] ? row[3] : "N/A";
    }
    mysql_free_result(result);
    
    return student;
}

vector<map<string, string>> Database::getStudentsByClass(int classId) {
    vector<map<string, string>> students;
    if (!ensureConnection()) return students;
    
    string query = "SELECT student_id, name, class_id FROM students WHERE class_id=" + 
                       to_string(classId);
    
    if (mysql_query(conn, query.c_str())) {
        cerr << "Query failed: " << mysql_error(conn) << endl;
        return students;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    MYSQL_ROW row;
    
    while ((row = mysql_fetch_row(result))) {
        map<string, string> student;
        student["student_id"] = row[0] ? row[0] : "";
        student["name"] = row[1] ? row[1] : "";
        student["class_id"] = row[2] ? row[2] : "";
        students.push_back(student);
    }
    
    mysql_free_result(result);
    return students;
}

// Attendance operations
bool Database::markAttendance(int studentId, int subjectId, int classId, 
                             const string& date, const string& status) {
    if (!ensureConnection()) return false;
    
    string query = "INSERT INTO attendance_records (student_id, subject_id, class_id, attendance_date, status) "
                       "VALUES (" + to_string(studentId) + ", " + to_string(subjectId) + ", " + 
                       to_string(classId) + ", '" + date + "', '" + escapeString(status) + "')";
    
    if (mysql_query(conn, query.c_str())) {
        cerr << "Insert failed: " << mysql_error(conn) << endl;
        return false;
    }
    
    return true;
}

vector<map<string, string>> Database::getStudentAttendance(int studentId) {
    vector<map<string, string>> records;
    if (!ensureConnection()) return records;
    
    string query = "SELECT ar.attendance_date, s.name as subject_name, ar.status "
                       "FROM attendance_records ar "
                       "JOIN subjects s ON ar.subject_id = s.subject_id "
                       "WHERE ar.student_id=" + to_string(studentId) + 
                       " ORDER BY ar.attendance_date";
    
    if (mysql_query(conn, query.c_str())) {
        cerr << "Query failed: " << mysql_error(conn) << endl;
        return records;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    MYSQL_ROW row;
    
    while ((row = mysql_fetch_row(result))) {
        map<string, string> record;
        record["date"] = row[0] ? row[0] : "";
        record["subject"] = row[1] ? row[1] : "";
        record["status"] = row[2] ? row[2] : "";
        records.push_back(record);
    }
    
    mysql_free_result(result);
    return records;
}

vector<map<string, string>> Database::getClassAttendance(int classId) {
    vector<map<string, string>> records;
    if (!ensureConnection()) return records;
    
    string query = "SELECT ar.attendance_date, st.name as student_name, s.name as subject_name, ar.status "
                       "FROM attendance_records ar "
                       "JOIN students st ON ar.student_id = st.student_id "
                       "JOIN subjects s ON ar.subject_id = s.subject_id "
                       "WHERE ar.class_id=" + to_string(classId) + 
                       " ORDER BY ar.attendance_date";
    
    if (mysql_query(conn, query.c_str())) {
        cerr << "Query failed: " << mysql_error(conn) << endl;
        return records;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    MYSQL_ROW row;
    
    while ((row = mysql_fetch_row(result))) {
        map<string, string> record;
        record["date"] = row[0] ? row[0] : "";
        record["student"] = row[1] ? row[1] : "";
        record["subject"] = row[2] ? row[2] : "";
        record["status"] = row[3] ? row[3] : "";
        records.push_back(record);
    }
    
    mysql_free_result(result);
    return records;
}

double Database::getAttendancePercentage(int studentId, int subjectId) {
    if (!ensureConnection()) return 0.0;
    
    string query = "SELECT COUNT(*) as total, "
                       "SUM(CASE WHEN status='Present' THEN 1 ELSE 0 END) as present "
                       "FROM attendance_records WHERE student_id=" + to_string(studentId);
    
    if (subjectId > 0) {
        query += " AND subject_id=" + to_string(subjectId);
    }
    
    if (mysql_query(conn, query.c_str())) {
        cerr << "Query failed: " << mysql_error(conn) << endl;
        return 0.0;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    double percentage = 0.0;
    
    if (mysql_num_rows(result) > 0) {
        MYSQL_ROW row = mysql_fetch_row(result);
        int total = row[0] ? stoi(row[0]) : 0;
        int present = row[1] ? stoi(row[1]) : 0;
        
        if (total > 0) {
            percentage = (present * 100.0) / total;
        }
    }
    
    mysql_free_result(result);
    return percentage;
}

bool Database::addSubjectToClass(int classId, int subjectId) {
    if (!ensureConnection()) return false;
    
    string query = "INSERT INTO class_subjects (class_id, subject_id) VALUES (" + 
                       to_string(classId) + ", " + to_string(subjectId) + ")";
    
    if (mysql_query(conn, query.c_str())) {
        cerr << "Insert failed: " << mysql_error(conn) << endl;
        return false;
    }
    
    return true;
}

vector<map<string, string>> Database::getClassSubjects(int classId) {
    vector<map<string, string>> subjects;
    if (!ensureConnection()) return subjects;
    
    string query = "SELECT s.subject_id, s.name, s.max_marks "
                       "FROM class_subjects cs "
                       "JOIN subjects s ON cs.subject_id = s.subject_id "
                       "WHERE cs.class_id=" + to_string(classId);
    
    if (mysql_query(conn, query.c_str())) {
        cerr << "Query failed: " << mysql_error(conn) << endl;
        return subjects;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    MYSQL_ROW row;
    
    while ((row = mysql_fetch_row(result))) {
        map<string, string> subject;
        subject["subject_id"] = row[0] ? row[0] : "";
        subject["name"] = row[1] ? row[1] : "";
        subject["max_marks"] = row[2] ? row[2] : "";
        subjects.push_back(subject);
    }
    
    mysql_free_result(result);
    return subjects;
}

// Delete operations
bool Database::deleteSubject(int subjectId) {
    if (!ensureConnection()) return false;
    
    string query = "DELETE FROM subjects WHERE subject_id=" + to_string(subjectId);
    
    if (mysql_query(conn, query.c_str())) {
        cerr << "Delete failed: " << mysql_error(conn) << endl;
        return false;
    }
    
    return true;
}

bool Database::deleteClass(int classId) {
    if (!ensureConnection()) return false;
    
    string query = "DELETE FROM classes WHERE class_id=" + to_string(classId);
    
    if (mysql_query(conn, query.c_str())) {
        cerr << "Delete failed: " << mysql_error(conn) << endl;
        return false;
    }
    
    return true;
}

bool Database::deleteTeacher(int teacherId) {
    if (!ensureConnection()) return false;
    
    string query = "DELETE FROM teachers WHERE teacher_id=" + to_string(teacherId);
    
    if (mysql_query(conn, query.c_str())) {
        cerr << "Delete failed: " << mysql_error(conn) << endl;
        return false;
    }
    
    return true;
}

bool Database::deleteStudent(int studentId) {
    if (!ensureConnection()) return false;
    
    string query = "DELETE FROM students WHERE student_id=" + to_string(studentId);
    
    if (mysql_query(conn, query.c_str())) {
        cerr << "Delete failed: " << mysql_error(conn) << endl;
        return false;
    }
    
    return true;
}

bool Database::updateStudentName(int studentId, const string& newName) {
    if (!ensureConnection()) return false;
    
    string query = "UPDATE students SET name='" + escapeString(newName) + 
                   "' WHERE student_id=" + to_string(studentId);
    
    if (mysql_query(conn, query.c_str())) {
        cerr << "Update failed: " << mysql_error(conn) << endl;
        return false;
    }
    
    return true;
}

// Validation operations
bool Database::classExists(int classId) {
    if (!ensureConnection()) return false;
    
    string query = "SELECT class_id FROM classes WHERE class_id=" + to_string(classId);
    
    if (mysql_query(conn, query.c_str())) {
        return false;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    bool exists = (mysql_num_rows(result) > 0);
    mysql_free_result(result);
    
    return exists;
}

bool Database::subjectExists(int subjectId) {
    if (!ensureConnection()) return false;
    
    string query = "SELECT subject_id FROM subjects WHERE subject_id=" + to_string(subjectId);
    
    if (mysql_query(conn, query.c_str())) {
        return false;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    bool exists = (mysql_num_rows(result) > 0);
    mysql_free_result(result);
    
    return exists;
}

bool Database::teacherExists(int teacherId) {
    if (!ensureConnection()) return false;
    
    string query = "SELECT teacher_id FROM teachers WHERE teacher_id=" + to_string(teacherId);
    
    if (mysql_query(conn, query.c_str())) {
        return false;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    bool exists = (mysql_num_rows(result) > 0);
    mysql_free_result(result);
    
    return exists;
}

bool Database::studentExists(int studentId) {
    if (!ensureConnection()) return false;
    
    string query = "SELECT student_id FROM students WHERE student_id=" + to_string(studentId);
    
    if (mysql_query(conn, query.c_str())) {
        return false;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    bool exists = (mysql_num_rows(result) > 0);
    mysql_free_result(result);
    
    return exists;
}

bool Database::isClassNameUnique(const string& className) {
    if (!ensureConnection()) return true;
    
    string query = "SELECT class_id FROM classes WHERE class_name='" + escapeString(className) + "'";
    
    if (mysql_query(conn, query.c_str())) {
        return true;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    bool unique = (mysql_num_rows(result) == 0);
    mysql_free_result(result);
    
    return unique;
}

bool Database::isSubjectNameUnique(const string& subjectName) {
    if (!ensureConnection()) return true;
    
    string query = "SELECT subject_id FROM subjects WHERE name='" + escapeString(subjectName) + "'";
    
    if (mysql_query(conn, query.c_str())) {
        return true;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    bool unique = (mysql_num_rows(result) == 0);
    mysql_free_result(result);
    
    return unique;
}

bool Database::isAttendanceMarked(int studentId, int subjectId, const string& date) {
    if (!ensureConnection()) return false;
    
    string query = "SELECT attendance_id FROM attendance_records WHERE student_id=" + 
                   to_string(studentId) + " AND subject_id=" + to_string(subjectId) + 
                   " AND attendance_date='" + date + "'";
    
    if (mysql_query(conn, query.c_str())) {
        return false;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    bool marked = (mysql_num_rows(result) > 0);
    mysql_free_result(result);
    
    return marked;
}

bool Database::isSubjectInClass(int subjectId, int classId) {
    if (!ensureConnection()) return false;
    
    string query = "SELECT id FROM class_subjects WHERE class_id=" + to_string(classId) + 
                   " AND subject_id=" + to_string(subjectId);
    
    if (mysql_query(conn, query.c_str())) {
        return false;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    bool exists = (mysql_num_rows(result) > 0);
    mysql_free_result(result);
    
    return exists;
}
