# EduTrack v2 - Attendance Management System

## Project Overview
EduTrack v2 is a comprehensive Attendance Management System designed to streamline the process of tracking student attendance in educational institutions. It features a robust C++ backend exposing a RESTful API and a responsive web-based frontend for Admins, Teachers, and Students.

The system supports role-based access control, allowing administrators to manage the entire system, teachers to mark and view attendance, and students to track their own records.

## Technical Architecture
The project follows a modern client-server architecture:

- **Backend**: C++17 application acting as a REST API server.
- **Frontend**: Vanilla HTML5, CSS3, and JavaScript (ES6+) for a lightweight and fast user interface.
- **Database**: MySQL for persistent data storage.

## Libraries & Technologies Used

### Backend (C++)
| Library | Purpose | Usage in Project |
|---------|---------|------------------|
| **cpp-httplib** | HTTP Server | Provides the core web server functionality, handling HTTP requests (GET, POST, PUT, DELETE) and routing. Configured with a thread pool for concurrent request handling. |
| **nlohmann/json** | JSON Parsing | Used for parsing incoming JSON payloads from the frontend and formatting JSON responses sent back to the client. |
| **libmysqlclient** | Database Connectivity | The official MySQL C API client. Used in the `Database` class to execute SQL queries, manage connections, and handle result sets. |
| **pthread** | Multi-threading | POSIX Threads library. Used implicitly by `cpp-httplib` for its thread pool and explicitly for mutex locking (`std::mutex`) to ensure thread-safe database access. |
| **Standard Template Library (STL)** | Core Data Structures | Extensive use of `std::vector`, `std::map`, `std::string`, and `std::iostream` for data manipulation and logic. |

### Frontend (Web)
| Technology | Purpose |
|------------|---------|
| **HTML5** | Structure of the web pages (Admin, Teacher, Student dashboards). |
| **CSS3** | Styling and layout, including responsive design for different screen sizes. |
| **JavaScript (Vanilla)** | Client-side logic, DOM manipulation, and asynchronous API communication using `fetch`. |

### Database
| Technology | Purpose |
|------------|---------|
| **MySQL** | Relational database management system. Stores data for users, classes, subjects, and attendance records. |

## Key Features

### 1. Admin Module
- **Dashboard**: Centralized view of system statistics.
- **Teacher Management**: Add, update, and delete teacher profiles. Assign teachers to specific classes (Class Teacher) or subjects (Subject Teacher).
- **Student Management**: Add, update, and delete student profiles. Assign students to classes.
- **Class & Subject Management**: Create and manage classes and subjects. Assign subjects to classes.
- **Data Integrity**: Ensures valid relationships between entities (e.g., a student must belong to a class).

### 2. Teacher Module
- **Role-Based Views**:
    - **Class Teachers**: Can manage their assigned class, add students, and view comprehensive reports.
    - **Subject Teachers**: Can mark attendance for their assigned subjects.
- **Mark Attendance**: Intuitive interface to mark students as Present, Absent, or Late for a specific date and subject.
- **View Attendance**: View attendance history for individual students or the entire class.
- **Real-time Feedback**: Immediate confirmation of successful attendance marking.

### 3. Student Module
- **Simple Login**: ID-based login for easy access.
- **Profile View**: View personal details and assigned class.
- **Attendance Tracking**: View personal attendance records and percentage per subject.

### 4. System Features
- **Multi-threading**: The backend server uses a thread pool (8 threads) to handle multiple concurrent users efficiently.
- **Thread Safety**: All database operations are protected by mutexes to prevent race conditions and ensure data consistency.
- **RESTful API**: Clean and structured API endpoints allowing for potential future expansion (e.g., mobile app integration).
- **Secure Authentication**: Role-based login system to protect sensitive data.
