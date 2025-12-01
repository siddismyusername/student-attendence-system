# Attendance Management System

A command-line based Attendance Management System implemented in C++ with MySQL backend. The system enables administrators, teachers, and students to manage and interact with academic attendance data.

## Features

### Admin Features
- Create and manage subjects, classes, teachers, and students
- Assign class teachers to classes
- Assign subject teachers to specific subject-class combinations
- Add subjects to classes
- View all entities in the system

### Teacher Features
- **Class Teachers**: Mark attendance for their assigned class, add students to their class
- **Subject Teachers**: Mark attendance for their assigned subject-class combinations
- View student attendance records
- View class attendance overview

### Student Features
- View personal attendance records
- View attendance percentage (overall and subject-wise)

## Prerequisites

- C++ compiler with C++17 support (g++, clang++)
- MySQL Server (5.7 or higher)
- MySQL development libraries (`libmysqlclient-dev`)
- CMake 3.10+ (optional, for CMake build)
- Make (for Makefile build)

## Installation

### 1. Install MySQL Development Libraries

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install mysql-server libmysqlclient-dev
```

**Fedora/RHEL:**
```bash
sudo dnf install mysql-server mysql-devel
```

**macOS:**
```bash
brew install mysql
```

### 2. Setup MySQL Database

Start MySQL service:
```bash
sudo systemctl start mysql
# or on macOS:
brew services start mysql
```

Login to MySQL:
```bash
mysql -u root -p
```

Create the database and tables:
```bash
mysql -u root -p < database.sql
```

Or manually run the SQL file:
```sql
source /path/to/database.sql
```

### 3. Configure Database Connection

Edit `config.txt` with your MySQL credentials:
```
host=localhost
port=3306
user=root
password=yourpassword
database=attendance_system
```

### 4. Build the Application

**Option 1: Using Make (Recommended)**
```bash
make
```

**Option 2: Using CMake**
```bash
mkdir build
cd build
cmake ..
make
```

**Option 3: Direct Compilation**
```bash
g++ -std=c++17 -I./include -I/usr/include/mysql \
    src/*.cpp \
    -o attendance_system \
    -lmysqlclient
```

## Running the Application

```bash
./attendance_system
```

## Default Credentials

**Admin:**
- Email: `admin@school.com`
- Password: `admin123`

**Students:**
- Login with Student ID (no password required)

**Teachers:**
- Login credentials set during teacher creation by admin

## Usage Guide

### 1. Admin Workflow

1. Login as admin
2. Create subjects (e.g., Mathematics, Physics)
3. Create classes (e.g., Class 10A, Class 10B)
4. Add subjects to classes
5. Create teachers (ClassTeacher or SubjectTeacher)
6. Assign class teachers to classes
7. Assign subject teachers to subject-class combinations
8. Create students and assign them to classes

### 2. Teacher Workflow

**Class Teacher:**
1. Login with teacher credentials
2. Mark attendance for students in your class
3. Add new students to your class
4. View class attendance reports

**Subject Teacher:**
1. Login with teacher credentials
2. Mark attendance for your assigned subject-class
3. View attendance reports for your classes

### 3. Student Workflow

1. Login with your Student ID
2. View your attendance records
3. Check your attendance percentage

## Project Structure

```
Edutrackv2/
├── include/
│   ├── AdminController.h
│   ├── BaseController.h
│   ├── Config.h
│   ├── Database.h
│   ├── StudentController.h
│   ├── TeacherController.h
│   └── UIHelper.h
├── src/
│   ├── AdminController.cpp
│   ├── BaseController.cpp
│   ├── Config.cpp
│   ├── Database.cpp
│   ├── main.cpp
│   ├── StudentController.cpp
│   ├── TeacherController.cpp
│   └── UIHelper.cpp
├── database.sql
├── config.txt
├── CMakeLists.txt
├── Makefile
├── srs.md
└── README.md
```

## Database Schema

The system uses the following tables:
- `admins` - Administrator credentials
- `teachers` - Teacher information and credentials
- `students` - Student information
- `classes` - Class/section information
- `subjects` - Subject information
- `teacher_class_assignments` - Class teacher assignments
- `teacher_subject_assignments` - Subject teacher assignments
- `class_subjects` - Subject-class mappings
- `attendance_records` - Daily attendance records

## Troubleshooting

### MySQL Connection Error
- Verify MySQL service is running: `sudo systemctl status mysql`
- Check credentials in `config.txt`
- Ensure database exists: `mysql -u root -p -e "SHOW DATABASES;"`

### Compilation Errors
- Ensure MySQL development headers are installed
- Check include paths in Makefile/CMakeLists.txt
- Verify C++17 support: `g++ --version`

### Missing libmysqlclient
```bash
# Ubuntu/Debian
sudo apt-get install libmysqlclient-dev

# Fedora/RHEL
sudo dnf install mysql-devel

# macOS
brew install mysql
```

## Clean Build

```bash
make clean
make
```

Or with CMake:
```bash
cd build
make clean
cmake ..
make
```

## System Requirements

- **OS**: Linux, macOS, or Windows (with WSL)
- **RAM**: 512 MB minimum
- **Disk Space**: 50 MB for application and database
- **MySQL**: Version 5.7 or higher

## Notes

- This is a CLI-only application (no GUI)
- Passwords are stored in plain text (for educational purposes)
- No concurrent access control implemented
- Local database only (not networked)

## License

This project is for educational purposes.

## Author

Developed as per the Software Requirements Specification (SRS) for an Attendance Management System.
