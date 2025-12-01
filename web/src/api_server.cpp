#include "../../include/Database.h"
#include "../../include/Config.h"
#include "../include/httplib.h"
#include "../include/json.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

using json = nlohmann::json;
using namespace std;

// Global database instance
Database* db = nullptr;

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
            // Normalize ID field names to 'id' and skip the original
            if (pair.first == "subject_id" || pair.first == "class_id" || 
                pair.first == "teacher_id" || pair.first == "student_id") {
                obj["id"] = pair.second;
            } else if (pair.first != "class_name" || obj.find("name") == obj.end()) {
                // For classes, prefer class_name as name
                if (pair.first == "class_name") {
                    obj["name"] = pair.second;
                } else {
                    obj[pair.first] = pair.second;
                }
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
            
            if (db->authenticateAdmin(email, password)) {
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
            
            auto teacherData = db->authenticateTeacher(email, password);
            if (!teacherData.empty() && teacherData.count("id") && teacherData["id"] != "0") {
                json data = {
                    {"role", "teacher"},
                    {"teacherId", stoi(teacherData["id"])},
                    {"name", teacherData["name"]},
                    {"email", teacherData["email"]},
                    {"type", teacherData["type"]}
                };
                
                // Get class assignment if exists
                auto classAssignment = db->getTeacherClassAssignment(stoi(teacherData["id"]));
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

    // Student login - using name as identifier (simplified for demo)
    svr.Post("/api/login/student", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = json::parse(req.body);
            if (!body.contains("email")) {
                res.set_content(errorResponse("Missing email").dump(), "application/json");
                return;
            }
            string email = body["email"];
            
            auto students = db->getAllStudents();
            bool found = false;
            json data;
            
            for (const auto& studentMap : students) {
                if (studentMap.count("name") && studentMap.at("name") == email) {
                    if (!studentMap.count("id")) continue;
                    
                    data = {
                        {"role", "student"},
                        {"studentId", stoi(studentMap.at("id"))},
                        {"name", studentMap.at("name")},
                        {"email", email}
                    };
                    
                    if (studentMap.find("class_id") != studentMap.end() && !studentMap.at("class_id").empty()) {
                        int classId = stoi(studentMap.at("class_id"));
                        data["classId"] = classId;
                        auto classInfo = db->getClassById(classId);
                        data["className"] = (classInfo.count("name") && !classInfo["name"].empty()) ? classInfo["name"] : "Unknown";
                    } else {
                        data["classId"] = 0;
                        data["className"] = "Not Assigned";
                    }
                    found = true;
                    break;
                }
            }
            
            if (found) {
                res.set_content(successResponse(data).dump(), "application/json");
            } else {
                res.set_content(errorResponse("Invalid credentials").dump(), "application/json");
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
        auto subjects = db->getAllSubjects();
        res.set_content(successResponse(mapVectorToJson(subjects)).dump(), "application/json");
    });

    // Create subject
    svr.Post("/api/subjects", [](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body);
        string name = body["name"];
        
        if (db->createSubject(name, 100)) {
            res.set_content(successResponse().dump(), "application/json");
        } else {
            res.set_content(errorResponse("Failed to create subject").dump(), "application/json");
        }
    });

    // Delete subject
    svr.Delete("/api/subjects/(\\d+)", [](const httplib::Request& req, httplib::Response& res) {
        int subjectId = stoi(req.matches[1]);
        
        if (db->deleteSubject(subjectId)) {
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
        auto classes = db->getAllClasses();
        res.set_content(successResponse(mapVectorToJson(classes)).dump(), "application/json");
    });

    // Create class
    svr.Post("/api/classes", [](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body);
        string name = body["name"];
        
        if (db->createClass(name)) {
            res.set_content(successResponse().dump(), "application/json");
        } else {
            res.set_content(errorResponse("Failed to create class").dump(), "application/json");
        }
    });

    // Delete class
    svr.Delete("/api/classes/(\\d+)", [](const httplib::Request& req, httplib::Response& res) {
        int classId = stoi(req.matches[1]);
        
        if (db->deleteClass(classId)) {
            res.set_content(successResponse().dump(), "application/json");
        } else {
            res.set_content(errorResponse("Failed to delete class").dump(), "application/json");
        }
    });

    // Get subjects for a class
    svr.Get("/api/classes/(\\d+)/subjects", [](const httplib::Request& req, httplib::Response& res) {
        int classId = stoi(req.matches[1]);
        auto subjects = db->getClassSubjects(classId);
        res.set_content(successResponse(mapVectorToJson(subjects)).dump(), "application/json");
    });

    // Add subject to class
    svr.Post("/api/classes/(\\d+)/subjects", [](const httplib::Request& req, httplib::Response& res) {
        int classId = stoi(req.matches[1]);
        auto body = json::parse(req.body);
        int subjectId = body["subjectId"];
        
        if (db->addSubjectToClass(classId, subjectId)) {
            res.set_content(successResponse().dump(), "application/json");
        } else {
            res.set_content(errorResponse("Failed to add subject to class").dump(), "application/json");
        }
    });

    // Get students in a class
    svr.Get("/api/classes/(\\d+)/students", [](const httplib::Request& req, httplib::Response& res) {
        int classId = stoi(req.matches[1]);
        auto students = db->getStudentsByClass(classId);
        res.set_content(successResponse(mapVectorToJson(students)).dump(), "application/json");
    });
}

// Teacher endpoints
void setupTeacherEndpoints(httplib::Server& svr) {
    // Get all teachers
    svr.Get("/api/teachers", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto teachers = db->getAllTeachers();
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
                
                int teacherId = stoi(teacherMap.at("teacher_id"));
                auto classAssignment = db->getTeacherClassAssignment(teacherId);
                if (!classAssignment.empty() && classAssignment.count("class_id")) {
                    teacher["classId"] = stoi(classAssignment["class_id"]);
                    auto classInfo = db->getClassById(stoi(classAssignment["class_id"]));
                    if (!classInfo.empty() && classInfo.find("class_name") != classInfo.end()) {
                        teacher["className"] = classInfo["class_name"];
                    } else {
                        teacher["className"] = "Unknown";
                    }
                } else {
                    teacher["classId"] = 0;
                    teacher["className"] = "Not Assigned";
                }
                
                result.push_back(teacher);
            }
            
            res.set_content(successResponse(result).dump(), "application/json");
        } catch (const exception& e) {
            res.set_content(errorResponse(string("Error loading teachers: ") + e.what()).dump(), "application/json");
        }
    });

    // Create teacher
    svr.Post("/api/teachers", [](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body);
        string name = body["name"];
        string email = body["email"];
        string password = body["password"];
        
        if (db->createTeacher(name, email, password, 50000.0, "2025-12-01", "Teacher")) {
            res.set_content(successResponse().dump(), "application/json");
        } else {
            res.set_content(errorResponse("Failed to create teacher").dump(), "application/json");
        }
    });

    // Delete teacher
    svr.Delete("/api/teachers/(\\d+)", [](const httplib::Request& req, httplib::Response& res) {
        int teacherId = stoi(req.matches[1]);
        
        if (db->deleteTeacher(teacherId)) {
            res.set_content(successResponse().dump(), "application/json");
        } else {
            res.set_content(errorResponse("Failed to delete teacher").dump(), "application/json");
        }
    });

    // Assign class teacher
    svr.Post("/api/teachers/(\\d+)/assign-class", [](const httplib::Request& req, httplib::Response& res) {
        int teacherId = stoi(req.matches[1]);
        auto body = json::parse(req.body);
        int classId = body["classId"];
        
        if (db->assignClassTeacher(classId, teacherId)) {
            res.set_content(successResponse().dump(), "application/json");
        } else {
            res.set_content(errorResponse("Failed to assign class teacher").dump(), "application/json");
        }
    });

    // Assign subject teacher
    svr.Post("/api/teachers/(\\d+)/assign-subject", [](const httplib::Request& req, httplib::Response& res) {
        int teacherId = stoi(req.matches[1]);
        auto body = json::parse(req.body);
        int subjectId = body["subjectId"];
        
        auto classAssignment = db->getTeacherClassAssignment(teacherId);
        int classId = 0;
        if (!classAssignment.empty()) {
            classId = stoi(classAssignment["class_id"]);
        }
        
        if (classId > 0 && db->assignSubjectTeacher(teacherId, subjectId, classId)) {
            res.set_content(successResponse().dump(), "application/json");
        } else {
            res.set_content(errorResponse("Teacher must be assigned to a class first").dump(), "application/json");
        }
    });

    // Get teacher's assigned subjects
    svr.Get("/api/teachers/(\\d+)/subjects", [](const httplib::Request& req, httplib::Response& res) {
        int teacherId = stoi(req.matches[1]);
        auto subjects = db->getTeacherSubjectAssignments(teacherId);
        res.set_content(successResponse(mapVectorToJson(subjects)).dump(), "application/json");
    });
}

// Student endpoints
void setupStudentEndpoints(httplib::Server& svr) {
    // Get all students
    svr.Get("/api/students", [](const httplib::Request& req, httplib::Response& res) {
        try {
            auto students = db->getAllStudents();
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
                    auto classInfo = db->getClassById(classId);
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
        
        if (db->createStudent(name, 0)) {
            res.set_content(successResponse().dump(), "application/json");
        } else {
            res.set_content(errorResponse("Failed to create student").dump(), "application/json");
        }
    });

    // Delete student
    svr.Delete("/api/students/(\\d+)", [](const httplib::Request& req, httplib::Response& res) {
        int studentId = stoi(req.matches[1]);
        
        if (db->deleteStudent(studentId)) {
            res.set_content(successResponse().dump(), "application/json");
        } else {
            res.set_content(errorResponse("Failed to delete student").dump(), "application/json");
        }
    });

    // Assign student to class
    svr.Post("/api/students/(\\d+)/assign-class", [](const httplib::Request& req, httplib::Response& res) {
        int studentId = stoi(req.matches[1]);
        auto body = json::parse(req.body);
        int classId = body["classId"];
        
        auto student = db->getStudentById(studentId);
        if (!student.empty()) {
            db->deleteStudent(studentId);
            if (db->createStudent(student["name"], classId)) {
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
        
        if (db->updateStudentName(studentId, newName)) {
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
            int studentId = body["studentId"];
            int subjectId = body["subjectId"];
            string date = body["date"];
            string status = body["status"];
        
        auto student = db->getStudentById(studentId);
        int classId = 0;
        if (!student.empty() && student.find("class_id") != student.end() && !student["class_id"].empty()) {
            classId = stoi(student["class_id"]);
        }
        
            if (classId > 0 && db->markAttendance(studentId, subjectId, classId, date, status)) {
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
        
        auto allRecords = db->getStudentAttendance(studentId);
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
        
        auto allRecords = db->getStudentAttendance(studentId);
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
        
        double percentage = db->getAttendancePercentage(studentId, subjectId);
        res.set_content(successResponse({{"percentage", percentage}}).dump(), "application/json");
    });

    // Get class attendance for a date
    svr.Get("/api/classes/(\\d+)/attendance", [](const httplib::Request& req, httplib::Response& res) {
        int classId = stoi(req.matches[1]);
        string date = req.get_param_value("date");
        int subjectId = stoi(req.get_param_value("subjectId"));
        
        auto students = db->getStudentsByClass(classId);
        json result = json::array();
        
        for (const auto& studentMap : students) {
            if (!studentMap.count("id") || !studentMap.count("name")) continue;
            
            int studentId = stoi(studentMap.at("id"));
            auto records = db->getStudentAttendance(studentId);
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
        
        bool isMarked = db->isAttendanceMarked(studentId, subjectId, date);
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

    // Serve static files
    svr.set_mount_point("/", "./web/public");

    cout << "API Server running on http://localhost:8080" << endl;
    cout << "Access web interface at http://localhost:8080" << endl;

    // Start server
    svr.listen("0.0.0.0", 8080);

    // Cleanup
    delete db;
    return 0;
}
