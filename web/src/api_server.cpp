#include "../../include/Database.h"
#include "../../include/Config.h"
#include "../include/httplib.h"
#include "../include/json.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <mutex>
#include <filesystem>

using json = nlohmann::json;
using namespace std;

// Global database instance
Database* db = nullptr;
// Mutex to protect database access from multiple threads
mutex db_mutex;

// Thread-safe database call wrapper
#define DB_CALL(call) ({ \
    lock_guard<mutex> lock(db_mutex); \
    call; \
})

// Helper to read integer fields that may be sent as strings
static int getIntField(const json& j, const string& key, int defaultVal = 0) {
    if (!j.contains(key)) return defaultVal;
    const auto& v = j.at(key);
    if (v.is_number_integer()) return v.get<int>();
    if (v.is_number_unsigned()) return static_cast<int>(v.get<unsigned int>());
    if (v.is_string()) {
        try { return stoi(v.get<string>()); } catch (...) { return defaultVal; }
    }
    return defaultVal;
}

// Helper function to create error response
json errorResponse(const string& message) {
    return {{"success", false}, {"error", message}};
}

// Helper function to create success response
json successResponse(const json& data = json::object()) {
    json response = {{"success", true}};
    // Always include data if it's an array, even if empty
    if (data.is_array() || !data.empty()) {
        response["data"] = data;
    }
    return response;
}

// Helper function to convert map vector to json array
json mapVectorToJson(const vector<map<string, string>>& vec) {
    json arr = json::array();
    for (const auto& m : vec) {
        json obj;
        for (const auto& pair : m) {
            // Normalize ID field names to 'id'
            if (pair.first == "subject_id" || pair.first == "class_id" || 
                pair.first == "teacher_id" || pair.first == "student_id") {
                obj["id"] = pair.second;
            } 
            // Normalize class_name to name
            else if (pair.first == "class_name") {
                obj["name"] = pair.second;
            } 
            // Add all other fields as-is
            else {
                obj[pair.first] = pair.second;
            }
        }
        arr.push_back(obj);
    }
    return arr;
}

// Authentication endpoints
void setupAuthEndpoints(httplib::Server& svr) {
    // Admin login
    svr.Post("/api/login/admin", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = json::parse(req.body);
            if (!body.contains("email") || !body.contains("password")) {
                res.set_content(errorResponse("Missing email or password").dump(), "application/json");
                return;
            }
            string email = body["email"];
            string password = body["password"];
            
            if (DB_CALL(db->authenticateAdmin(email, password))) {
                res.set_content(successResponse({{"role", "admin"}, {"email", email}}).dump(), "application/json");
            } else {
                res.set_content(errorResponse("Invalid credentials").dump(), "application/json");
            }
        } catch (const exception& e) {
            res.set_content(errorResponse(string("Invalid request: ") + e.what()).dump(), "application/json");
        }
    });

    // Teacher login
    svr.Post("/api/login/teacher", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = json::parse(req.body);
            if (!body.contains("email") || !body.contains("password")) {
                res.set_content(errorResponse("Missing email or password").dump(), "application/json");
                return;
            }
            string email = body["email"];
            string password = body["password"];
            
            auto teacherData = DB_CALL(db->authenticateTeacher(email, password));
            if (!teacherData.empty() && teacherData.count("id") && teacherData["id"] != "0") {
                json data = {
                    {"role", "teacher"},
                    {"teacherId", stoi(teacherData["id"])},
                    {"name", teacherData["name"]},
                    {"email", teacherData["email"]},
                    {"type", teacherData["type"]}
                };
                
                // Get class assignment if exists
                auto classAssignment = DB_CALL(db->getTeacherClassAssignment(stoi(teacherData["id"])));
                if (!classAssignment.empty() && classAssignment.count("class_id")) {
                    data["classId"] = stoi(classAssignment["class_id"]);
                } else {
                    data["classId"] = 0;
                }
                
                res.set_content(successResponse(data).dump(), "application/json");
            } else {
                res.set_content(errorResponse("Invalid credentials").dump(), "application/json");
            }
        } catch (const exception& e) {
            res.set_content(errorResponse(string("Invalid request: ") + e.what()).dump(), "application/json");
        }
    });

    // Student login - using ID
    svr.Post("/api/login/student", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = json::parse(req.body);
            // Frontend sends ID in 'email' field for compatibility with generic login function
            if (!body.contains("email")) {
                res.set_content(errorResponse("Missing Student ID").dump(), "application/json");
                return;
            }
            
            string idStr = body["email"];
            int studentId = 0;
            try {
                studentId = stoi(idStr);
            } catch (...) {
                res.set_content(errorResponse("Invalid Student ID format").dump(), "application/json");
                return;
            }
            
            auto studentMap = DB_CALL(db->getStudentById(studentId));
            
            if (!studentMap.empty() && studentMap.count("student_id")) {
                json data = {
                    {"role", "student"},
                    {"studentId", stoi(studentMap.at("student_id"))},
                    {"name", studentMap.at("name")},
                    {"email", idStr} // Echo back ID as email
                };
                
                if (studentMap.find("class_id") != studentMap.end() && !studentMap.at("class_id").empty()) {
                    int classId = stoi(studentMap.at("class_id"));
                    data["classId"] = classId;
                    data["className"] = studentMap.count("class_name") ? studentMap.at("class_name") : "Unknown";
                } else {
                    data["classId"] = 0;
                    data["className"] = "Not Assigned";
                }
                
                res.set_content(successResponse(data).dump(), "application/json");
            } else {
                res.set_content(errorResponse("Student not found").dump(), "application/json");
            }
        } catch (const exception& e) {
            res.set_content(errorResponse(string("Invalid request: ") + e.what()).dump(), "application/json");
        }
    });
}

// Subject endpoints
void setupSubjectEndpoints(httplib::Server& svr) {
    // Get all subjects
    svr.Get("/api/subjects", [](const httplib::Request& req, httplib::Response& res) {
        auto subjects = DB_CALL(db->getAllSubjects());
        res.set_content(successResponse(mapVectorToJson(subjects)).dump(), "application/json");
    });

    // Create subject
    svr.Post("/api/subjects", [](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body);
        string name = body["name"];
        
        if (DB_CALL(db->createSubject(name, 100))) {
            res.set_content(successResponse().dump(), "application/json");
        } else {
            res.set_content(errorResponse("Failed to create subject").dump(), "application/json");
        }
    });

    // Delete subject
    svr.Delete("/api/subjects/(\\d+)", [](const httplib::Request& req, httplib::Response& res) {
        int subjectId = stoi(req.matches[1]);
        
        if (DB_CALL(db->deleteSubject(subjectId))) {
            res.set_content(successResponse().dump(), "application/json");
        } else {
            res.set_content(errorResponse("Failed to delete subject").dump(), "application/json");
        }
    });
}

// Class endpoints
void setupClassEndpoints(httplib::Server& svr) {
    // Get all classes
    svr.Get("/api/classes", [](const httplib::Request& req, httplib::Response& res) {
        auto classes = DB_CALL(db->getAllClasses());
        res.set_content(successResponse(mapVectorToJson(classes)).dump(), "application/json");
    });

    // Create class
    svr.Post("/api/classes", [](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body);
        string name = body["name"];
        
        if (DB_CALL(db->createClass(name))) {
            res.set_content(successResponse().dump(), "application/json");
        } else {
            res.set_content(errorResponse("Failed to create class").dump(), "application/json");
        }
    });

    // Delete class
    svr.Delete("/api/classes/(\\d+)", [](const httplib::Request& req, httplib::Response& res) {
        int classId = stoi(req.matches[1]);
        
        if (DB_CALL(db->deleteClass(classId))) {
            res.set_content(successResponse().dump(), "application/json");
        } else {
            res.set_content(errorResponse("Failed to delete class").dump(), "application/json");
        }
    });

    // Get subjects for a class
    svr.Get("/api/classes/(\\d+)/subjects", [](const httplib::Request& req, httplib::Response& res) {
        int classId = stoi(req.matches[1]);
        auto subjects = DB_CALL(db->getClassSubjects(classId));
        res.set_content(successResponse(mapVectorToJson(subjects)).dump(), "application/json");
    });

    // Add subject to class
    svr.Post("/api/classes/(\\d+)/subjects", [](const httplib::Request& req, httplib::Response& res) {
        int classId = stoi(req.matches[1]);
        auto body = json::parse(req.body);
        int subjectId = getIntField(body, "subjectId", 0);
        
        if (DB_CALL(db->addSubjectToClass(classId, subjectId))) {
            res.set_content(successResponse().dump(), "application/json");
        } else {
            res.set_content(errorResponse("Failed to add subject to class").dump(), "application/json");
        }
    });

    // Get students in a class
    svr.Get("/api/classes/(\\d+)/students", [](const httplib::Request& req, httplib::Response& res) {
        int classId = stoi(req.matches[1]);
        auto students = DB_CALL(db->getStudentsByClass(classId));
        res.set_content(successResponse(mapVectorToJson(students)).dump(), "application/json");
    });
}

// Teacher endpoints
void setupTeacherEndpoints(httplib::Server& svr) {
    // Get all teachers (Optimized)
    svr.Get("/api/teachers", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto teachers = DB_CALL(db->getAllTeachersWithDetails());
            json result = json::array();
            
            for (const auto& teacherMap : teachers) {
                if (!teacherMap.count("teacher_id") || !teacherMap.count("name")) {
                    continue; // Skip invalid entries
                }
                
                json teacher;
                teacher["id"] = teacherMap.at("teacher_id");
                teacher["name"] = teacherMap.at("name");
                teacher["email"] = teacherMap.count("email") ? teacherMap.at("email") : "";
                teacher["type"] = teacherMap.count("teacher_type") ? teacherMap.at("teacher_type") : "Teacher";
                
                if (teacherMap.find("salary") != teacherMap.end()) {
                    teacher["salary"] = teacherMap.at("salary");
                }
                if (teacherMap.find("join_date") != teacherMap.end()) {
                    teacher["joinDate"] = teacherMap.at("join_date");
                }
                
                // Class info is now already in the map from the JOIN
                if (teacherMap.count("class_id") && !teacherMap.at("class_id").empty()) {
                    teacher["classId"] = stoi(teacherMap.at("class_id"));
                    teacher["className"] = teacherMap.count("class_name") ? teacherMap.at("class_name") : "Unknown";
                } else {
                    teacher["classId"] = 0;
                    teacher["className"] = "Not Assigned";
                }
                
                result.push_back(teacher);
            }
            
            res.set_content(successResponse(result).dump(), "application/json");
        } catch (const exception& e) {
            res.set_content(errorResponse(string("Internal Error: ") + e.what()).dump(), "application/json");
        }
    });

    // Create teacher
    svr.Post("/api/teachers", [](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body);
        string name = body["name"];
        string email = body["email"];
        string password = body["password"];
        string type = body.contains("type") ? body["type"] : "Teacher";
        
        if (DB_CALL(db->createTeacher(name, email, password, 50000.0, "2025-12-01", type))) {
            res.set_content(successResponse().dump(), "application/json");
        } else {
            res.set_content(errorResponse("Failed to create teacher").dump(), "application/json");
        }
    });

    // Delete teacher
    svr.Delete("/api/teachers/(\\d+)", [](const httplib::Request& req, httplib::Response& res) {
        int teacherId = stoi(req.matches[1]);
        
        if (DB_CALL(db->deleteTeacher(teacherId))) {
            res.set_content(successResponse().dump(), "application/json");
        } else {
            res.set_content(errorResponse("Failed to delete teacher").dump(), "application/json");
        }
    });

    // Assign class teacher
    svr.Post("/api/teachers/(\\d+)/assign-class", [](const httplib::Request& req, httplib::Response& res) {
        int teacherId = stoi(req.matches[1]);
        auto body = json::parse(req.body);
        int classId = getIntField(body, "classId", 0);
        
        if (DB_CALL(db->assignClassTeacher(classId, teacherId))) {
            res.set_content(successResponse().dump(), "application/json");
        } else {
            res.set_content(errorResponse("Failed to assign class teacher").dump(), "application/json");
        }
    });

    // Assign subject teacher
    svr.Post("/api/teachers/(\\d+)/assign-subject", [](const httplib::Request& req, httplib::Response& res) {
        int teacherId = stoi(req.matches[1]);
        auto body = json::parse(req.body);
        int subjectId = getIntField(body, "subjectId", 0);
        int classId = getIntField(body, "classId", 0);
        
        if (classId == 0) {
            // Fallback: try to get class from teacher's class assignment
            auto classAssignment = DB_CALL(db->getTeacherClassAssignment(teacherId));
            if (!classAssignment.empty()) {
                classId = stoi(classAssignment["class_id"]);
            }
        }
        
        if (classId == 0) {
            res.set_content(errorResponse("Class ID is missing. Please select a class.").dump(), "application/json");
            return;
        }
        
        if (DB_CALL(db->assignSubjectTeacher(teacherId, subjectId, classId))) {
            res.set_content(successResponse().dump(), "application/json");
        } else {
            res.set_content(errorResponse("Failed to assign subject teacher. This assignment might already exist.").dump(), "application/json");
        }
    });

    // Get teacher's assigned subjects
    svr.Get("/api/teachers/(\\d+)/subjects", [](const httplib::Request& req, httplib::Response& res) {
        int teacherId = stoi(req.matches[1]);
        auto subjects = DB_CALL(db->getTeacherSubjectAssignments(teacherId));
        
        json result = json::array();
        for (const auto& s : subjects) {
            json subject;
            subject["id"] = s.count("subject_id") ? s.at("subject_id") : "0";
            subject["name"] = s.count("subject_name") ? s.at("subject_name") : "Unknown";
            subject["classId"] = s.count("class_id") ? s.at("class_id") : "0";
            subject["className"] = s.count("class_name") ? s.at("class_name") : "Unknown";
            result.push_back(subject);
        }
        
        res.set_content(successResponse(result).dump(), "application/json");
    });
}

// Student endpoints
void setupStudentEndpoints(httplib::Server& svr) {
    // Get all students
    svr.Get("/api/students", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto students = DB_CALL(db->getAllStudents());
            json result = json::array();
            
            for (const auto& studentMap : students) {
                if (!studentMap.count("student_id") || !studentMap.count("name")) {
                    continue; // Skip invalid entries
                }
                
                json student;
                student["id"] = studentMap.at("student_id");
                student["name"] = studentMap.at("name");
            
                // Handle class assignment
                if (studentMap.find("class_id") != studentMap.end() && !studentMap.at("class_id").empty() && studentMap.at("class_id") != "0") {
                    int classId = stoi(studentMap.at("class_id"));
                    student["classId"] = classId;
                    auto classInfo = DB_CALL(db->getClassById(classId));
                    if (!classInfo.empty() && classInfo.find("class_name") != classInfo.end()) {
                        student["className"] = classInfo["class_name"];
                    } else {
                        student["className"] = "Unknown";
                    }
                } else {
                    student["classId"] = 0;
                    student["className"] = "Not Assigned";
                }
                
                result.push_back(student);
            }
            
            res.set_content(successResponse(result).dump(), "application/json");
        } catch (const exception& e) {
            res.set_content(errorResponse(string("Error loading students: ") + e.what()).dump(), "application/json");
        }
    });

    // Create student
    svr.Post("/api/students", [](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body);
        string name = body["name"];
        
        int newId = DB_CALL(db->createStudent(name, 0));
        if (newId > 0) {
            res.set_content(successResponse({{"id", newId}}).dump(), "application/json");
        } else {
            res.set_content(errorResponse("Failed to create student").dump(), "application/json");
        }
    });

    // Delete student
    svr.Delete("/api/students/(\\d+)", [](const httplib::Request& req, httplib::Response& res) {
        int studentId = stoi(req.matches[1]);
        
        if (DB_CALL(db->deleteStudent(studentId))) {
            res.set_content(successResponse().dump(), "application/json");
        } else {
            res.set_content(errorResponse("Failed to delete student").dump(), "application/json");
        }
    });

    // Assign student to class
    svr.Post("/api/students/(\\d+)/assign-class", [](const httplib::Request& req, httplib::Response& res) {
        int studentId = stoi(req.matches[1]);
        auto body = json::parse(req.body);
        int classId = getIntField(body, "classId", 0);
        
        auto student = DB_CALL(db->getStudentById(studentId));
        if (!student.empty()) {
            DB_CALL(db->deleteStudent(studentId));
            if (DB_CALL(db->createStudent(student["name"], classId))) {
                res.set_content(successResponse().dump(), "application/json");
            } else {
                res.set_content(errorResponse("Failed to assign student").dump(), "application/json");
            }
        } else {
            res.set_content(errorResponse("Student not found").dump(), "application/json");
        }
    });

    // Update student profile
    svr.Put("/api/students/(\\d+)/profile", [](const httplib::Request& req, httplib::Response& res) {
        int studentId = stoi(req.matches[1]);
        auto body = json::parse(req.body);
        string newName = body["name"];
        
        if (DB_CALL(db->updateStudentName(studentId, newName))) {
            res.set_content(successResponse().dump(), "application/json");
        } else {
            res.set_content(errorResponse("Failed to update profile").dump(), "application/json");
        }
    });
}

// Attendance endpoints
void setupAttendanceEndpoints(httplib::Server& svr) {
    // Mark attendance
    svr.Post("/api/attendance", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = json::parse(req.body);
            if (!body.contains("studentId") || !body.contains("subjectId") || 
                !body.contains("date") || !body.contains("status")) {
                res.set_content(errorResponse("Missing required fields").dump(), "application/json");
                return;
            }
            int studentId = getIntField(body, "studentId", 0);
            int subjectId = getIntField(body, "subjectId", 0);
            string date = body["date"];
            string status = body["status"];
        
        auto student = DB_CALL(db->getStudentById(studentId));
        int classId = 0;
        if (!student.empty() && student.find("class_id") != student.end() && !student["class_id"].empty()) {
            classId = stoi(student["class_id"]);
        }

            if (classId == 0) {
                res.set_content(errorResponse("Student is not assigned to any class").dump(), "application/json");
                return;
            }

            // Verify subject is assigned to student's class
            auto classSubjects = DB_CALL(db->getClassSubjects(classId));
            bool subjectInClass = false;
            for (const auto& s : classSubjects) {
                auto it = s.find("subject_id");
                if (it != s.end()) {
                    try {
                        if (stoi(it->second) == subjectId) { subjectInClass = true; break; }
                    } catch (...) {}
                }
            }
            if (!subjectInClass) {
                res.set_content(errorResponse("Subject is not assigned to the student's class").dump(), "application/json");
                return;
            }

            // Check duplicate attendance for same student-subject-date
            if (DB_CALL(db->isAttendanceMarked(studentId, subjectId, date))) {
                res.set_content(errorResponse("Attendance already marked for this student, subject, and date").dump(), "application/json");
                return;
            }
        
            if (classId > 0 && DB_CALL(db->markAttendance(studentId, subjectId, classId, date, status))) {
                res.set_content(successResponse().dump(), "application/json");
            } else {
                res.set_content(errorResponse("Failed to mark attendance").dump(), "application/json");
            }
        } catch (const exception& e) {
            res.set_content(errorResponse(string("Error marking attendance: ") + e.what()).dump(), "application/json");
        }
    });

    // Get student attendance by subject
    svr.Get("/api/students/(\\d+)/attendance/subject/(\\d+)", [](const httplib::Request& req, httplib::Response& res) {
        int studentId = stoi(req.matches[1]);
        int subjectId = stoi(req.matches[2]);
        
        auto allRecords = DB_CALL(db->getStudentAttendance(studentId));
        json result = json::array();
        
        for (const auto& recordMap : allRecords) {
            if (recordMap.find("subject_id") != recordMap.end() && stoi(recordMap.at("subject_id")) == subjectId) {
                json record;
                for (const auto& pair : recordMap) {
                    record[pair.first] = pair.second;
                }
                result.push_back(record);
            }
        }
        
        res.set_content(successResponse(result).dump(), "application/json");
    });

    // Get overall attendance percentage
    svr.Get("/api/students/(\\d+)/attendance-percentage", [](const httplib::Request& req, httplib::Response& res) {
        int studentId = stoi(req.matches[1]);
        
        auto allRecords = DB_CALL(db->getStudentAttendance(studentId));
        int totalDays = allRecords.size();
        int presentDays = 0;
        
        for (const auto& record : allRecords) {
            if (record.count("status") && (record.at("status") == "Present" || record.at("status") == "present")) {
                presentDays++;
            }
        }
        
        double percentage = totalDays > 0 ? (presentDays * 100.0 / totalDays) : 0.0;
        res.set_content(successResponse({{"percentage", percentage}}).dump(), "application/json");
    });

    // Get subject attendance percentage
    svr.Get("/api/students/(\\d+)/attendance-percentage/subject/(\\d+)", [](const httplib::Request& req, httplib::Response& res) {
        int studentId = stoi(req.matches[1]);
        int subjectId = stoi(req.matches[2]);
        
        double percentage = DB_CALL(db->getAttendancePercentage(studentId, subjectId));
        res.set_content(successResponse({{"percentage", percentage}}).dump(), "application/json");
    });

    // Get class attendance for a date
    svr.Get("/api/classes/(\\d+)/attendance", [](const httplib::Request& req, httplib::Response& res) {
        int classId = stoi(req.matches[1]);
        string date = req.get_param_value("date");
        int subjectId = stoi(req.get_param_value("subjectId"));
        
        auto students = DB_CALL(db->getStudentsByClass(classId));
        json result = json::array();
        
        for (const auto& studentMap : students) {
            if (!studentMap.count("id") || !studentMap.count("name")) continue;
            
            int studentId = stoi(studentMap.at("id"));
            auto records = DB_CALL(db->getStudentAttendance(studentId));
            string status = "Not Marked";
            
            for (const auto& record : records) {
                if (record.count("date") && record.count("status") && 
                    record.at("date") == date && record.find("subject_id") != record.end() && 
                    stoi(record.at("subject_id")) == subjectId) {
                    status = record.at("status");
                    break;
                }
            }
            
            result.push_back({
                {"studentId", studentId},
                {"studentName", studentMap.at("name")},
                {"status", status}
            });
        }
        
        res.set_content(successResponse(result).dump(), "application/json");
    });

    // Check if attendance marked
    svr.Get("/api/attendance/check", [](const httplib::Request& req, httplib::Response& res) {
        int studentId = stoi(req.get_param_value("studentId"));
        int subjectId = stoi(req.get_param_value("subjectId"));
        string date = req.get_param_value("date");
        
        bool isMarked = DB_CALL(db->isAttendanceMarked(studentId, subjectId, date));
        res.set_content(successResponse({{"isMarked", isMarked}}).dump(), "application/json");
    });
}

int main() {
    cout << "Starting Attendance Management System API Server..." << endl;

    // Load configuration - try multiple locations
    auto config = Config::loadConfig("../config.txt");
    if (config.empty()) {
        config = Config::loadConfig("config.txt");
    }
    if (config.empty()) {
        config = Config::loadConfig("../../config.txt");
    }
    if (config.empty()) {
        cerr << "Failed to load configuration file!" << endl;
        cerr << "Tried: ../config.txt, config.txt, ../../config.txt" << endl;
        return 1;
    }

    // Initialize database
    db = new Database(config);

    if (!db->isConnected()) {
        cerr << "Failed to connect to database!" << endl;
        delete db;
        return 1;
    }

    cout << "Database connected successfully!" << endl;

    // Create HTTP server
    httplib::Server svr;

    // Enable CORS
    svr.set_default_headers({
        {"Access-Control-Allow-Origin", "*"},
        {"Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS"},
        {"Access-Control-Allow-Headers", "Content-Type"}
    });

    // Logger
    svr.set_logger([](const httplib::Request& req, const httplib::Response& res) {
        cout << "[" << req.method << "] " << req.path << " -> " << res.status << endl;
    });

    svr.Options(".*", [](const httplib::Request&, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.status = 200;
    });

    // Setup endpoints
    setupAuthEndpoints(svr);
    setupSubjectEndpoints(svr);
    setupClassEndpoints(svr);
    setupTeacherEndpoints(svr);
    setupStudentEndpoints(svr);
    setupAttendanceEndpoints(svr);

    // Add error handler
    svr.set_exception_handler([](const httplib::Request& req, httplib::Response& res, exception_ptr ep) {
        try {
            rethrow_exception(ep);
        } catch (const exception& e) {
            cerr << "Exception in handler: " << e.what() << endl;
            res.status = 500;
            res.set_content(errorResponse(string("Server error: ") + e.what()).dump(), "application/json");
        } catch (...) {
            cerr << "Unknown exception in handler" << endl;
            res.status = 500;
            res.set_content(errorResponse("Unknown server error").dump(), "application/json");
        }
    });

    // Serve static files (resolve path based on current working directory)
    namespace fs = std::filesystem;
    vector<fs::path> staticCandidates = {
        fs::current_path() / "web/public",        // when run from project root
        fs::current_path() / "public",            // when run from web/
        fs::current_path() / "../public",         // when run from web/bin/
        fs::current_path() / "../../web/public"   // fallback
    };
    string staticDir;
    for (const auto& p : staticCandidates) {
        std::error_code ec;
        if (fs::exists(p, ec) && fs::is_directory(p, ec)) {
            staticDir = p.string();
            break;
        }
    }
    if (staticDir.empty()) {
        cerr << "Could not locate static folder. Expected one of:" << endl;
        for (const auto& p : staticCandidates) {
            cerr << "  - " << p << endl;
        }
        cerr << "Start the server from the project root or adjust paths." << endl;
        return 1;
    }
    svr.set_mount_point("/", staticDir.c_str());
    
    // Set multi-threaded mode (8 threads)
    // Note: Database access is protected by mutex, so this is safe but serialized at DB level
    svr.new_task_queue = [] { return new httplib::ThreadPool(8); };

    cout << "API Server running on http://localhost:8080" << endl;
    cout << "Access web interface at http://localhost:8080" << endl;

    // Start server
    svr.listen("0.0.0.0", 8080);

    // Cleanup
    delete db;
    return 0;
}
