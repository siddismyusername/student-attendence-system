# Web-Based Attendance Management System

A modern web interface for the Attendance Management System with a C++ REST API backend and HTML/CSS/JavaScript frontend.

## Features

- **Admin Dashboard**: Manage subjects, classes, teachers, students, and assignments
- **Teacher Dashboard**: Mark attendance, view student records, add students to class (class teachers)
- **Student Dashboard**: View attendance records, check attendance percentage, update profile
- **REST API**: Complete RESTful API with JSON endpoints
- **Responsive Design**: Modern, mobile-friendly interface with gradient backgrounds

## Technology Stack

### Backend
- **C++17**: Core backend logic
- **cpp-httplib**: Lightweight HTTP server library
- **nlohmann/json**: JSON parsing and serialization
- **MySQL**: Database management
- **Existing Database Layer**: Reuses Database, Config, and controller classes from CLI version

### Frontend
- **HTML5**: Structure and layout
- **CSS3**: Modern styling with gradients and animations
- **Vanilla JavaScript**: No frameworks, pure JavaScript with Fetch API
- **Session Storage**: Client-side authentication state management

## Directory Structure

```
web/
├── bin/                    # Compiled executables
├── obj/                    # Object files
├── include/                # Third-party headers
│   ├── httplib.h          # cpp-httplib header
│   └── json.hpp           # nlohmann/json header
├── src/
│   └── api_server.cpp     # REST API server implementation
├── public/                 # Frontend files (served by API server)
│   ├── index.html         # Login page
│   ├── admin.html         # Admin dashboard
│   ├── teacher.html       # Teacher dashboard
│   ├── student.html       # Student dashboard
│   ├── css/
│   │   └── style.css      # Stylesheet
│   └── js/
│       └── api.js         # API client library
├── Makefile               # Build configuration
└── README.md              # This file
```

## Prerequisites

- g++ with C++17 support
- MySQL development libraries (`libmysqlclient-dev`)
- wget (for downloading dependencies)
- Modern web browser (Chrome, Firefox, Edge, Safari)

## Installation

### 1. Dependencies are already downloaded

The required header files are already downloaded:
- `web/include/httplib.h` (cpp-httplib)
- `web/include/json.hpp` (nlohmann/json)

### 2. Build the API Server

```bash
cd web
make
```

This will:
- Create necessary directories (`bin/`, `obj/`)
- Compile the API server with Database and Config modules
- Create executable at `bin/api_server`

### 3. Database Setup

Make sure your MySQL database is set up (use `database.sql` from the parent directory):

```bash
cd ..
mysql -u root -p < database.sql
```

### 4. Configuration

Ensure `config.txt` exists in the root directory with database credentials:

```
db_host=localhost
db_user=root
db_password=your_password
db_name=attendance_system
```

## Running the Server

### Start the API Server

```bash
cd web
make run
```

Or run directly:

```bash
./bin/api_server
```

The server will start on `http://localhost:8080`

### Access the Web Interface

Open your web browser and navigate to:

```
http://localhost:8080
```

## Default Credentials

### Admin
- **Email**: admin@school.com
- **Password**: admin123

### Creating Test Accounts

Use the admin dashboard to create:
- **Subjects**: Mathematics, Physics, Chemistry, etc.
- **Classes**: Grade 10A, Grade 11B, etc.
- **Teachers**: Assign as ClassTeacher or SubjectTeacher
- **Students**: Add and assign to classes

## API Documentation

### Base URL
```
http://localhost:8080/api
```

### Authentication Endpoints

#### POST `/api/login/admin`
Login as administrator
```json
Request: { "email": "admin@school.com", "password": "admin123" }
Response: { "success": true, "data": { "role": "admin", "email": "..." } }
```

#### POST `/api/login/teacher`
Login as teacher
```json
Request: { "email": "teacher@school.com", "password": "password" }
Response: { 
  "success": true, 
  "data": { 
    "role": "teacher", 
    "teacherId": 1, 
    "name": "...", 
    "type": "ClassTeacher|SubjectTeacher",
    "classId": 1 
  } 
}
```

#### POST `/api/login/student`
Login as student
```json
Request: { "email": "student@school.com", "password": "password" }
Response: { 
  "success": true, 
  "data": { 
    "role": "student", 
    "studentId": 1, 
    "name": "...", 
    "classId": 1,
    "className": "Grade 10A" 
  } 
}
```

### Subject Endpoints

- `GET /api/subjects` - Get all subjects
- `POST /api/subjects` - Create subject `{ "code": "MATH101", "name": "Mathematics" }`
- `DELETE /api/subjects/:id` - Delete subject

### Class Endpoints

- `GET /api/classes` - Get all classes
- `POST /api/classes` - Create class `{ "name": "Grade 10A" }`
- `DELETE /api/classes/:id` - Delete class
- `GET /api/classes/:id/subjects` - Get class subjects
- `POST /api/classes/:id/subjects` - Add subject to class `{ "subjectId": 1 }`
- `GET /api/classes/:id/students` - Get class students

### Teacher Endpoints

- `GET /api/teachers` - Get all teachers
- `POST /api/teachers` - Create teacher `{ "name": "...", "email": "...", "password": "..." }`
- `DELETE /api/teachers/:id` - Delete teacher
- `POST /api/teachers/:id/assign-class` - Assign class teacher `{ "classId": 1 }`
- `POST /api/teachers/:id/assign-subject` - Assign subject teacher `{ "subjectId": 1 }`
- `GET /api/teachers/:id/subjects` - Get teacher's subjects

### Student Endpoints

- `GET /api/students` - Get all students
- `POST /api/students` - Create student `{ "name": "...", "email": "...", "password": "..." }`
- `DELETE /api/students/:id` - Delete student
- `POST /api/students/:id/assign-class` - Assign student to class `{ "classId": 1 }`
- `PUT /api/students/:id/profile` - Update student name `{ "name": "New Name" }`

### Attendance Endpoints

- `POST /api/attendance` - Mark attendance
  ```json
  { "studentId": 1, "subjectId": 1, "date": "2025-12-01", "status": "Present|Absent|Late" }
  ```
- `GET /api/students/:studentId/attendance/subject/:subjectId` - Get student attendance
- `GET /api/students/:studentId/attendance-percentage` - Overall attendance %
- `GET /api/students/:studentId/attendance-percentage/subject/:subjectId` - Subject attendance %
- `GET /api/classes/:classId/attendance?date=YYYY-MM-DD&subjectId=1` - Class attendance for date
- `GET /api/attendance/check?studentId=1&subjectId=1&date=YYYY-MM-DD` - Check if marked

## Frontend Features

### Admin Dashboard
1. **Subjects Management**: Create and delete subjects
2. **Classes Management**: Create and delete classes
3. **Teachers Management**: Create and delete teachers
4. **Students Management**: Create and delete students
5. **Assignments**: Assign class teachers, subject teachers, subjects to classes, students to classes

### Teacher Dashboard
1. **Mark Attendance**: Select subject, date, and mark attendance for all students
2. **View Attendance**: View individual student attendance records and percentages
3. **Add Students** (Class Teachers only): Add unassigned students to their class

### Student Dashboard
1. **Overview**: Overall attendance percentage and subject-wise breakdown
2. **My Attendance**: Detailed attendance records by subject
3. **Update Profile**: Change name (email and class are read-only)

## Building and Cleaning

### Build
```bash
make
```

### Clean
```bash
make clean
```

### Rebuild
```bash
make rebuild
```

## CORS Configuration

The API server is configured to allow cross-origin requests for development:
- `Access-Control-Allow-Origin: *`
- Supports GET, POST, PUT, DELETE methods
- Allows Content-Type header

For production, restrict CORS to your domain.

## Troubleshooting

### Server won't start
- Check if port 8080 is already in use
- Verify database connection in `config.txt`
- Ensure MySQL server is running

### Database connection failed
- Verify MySQL credentials in `config.txt`
- Check if database exists: `mysql -u root -p -e "SHOW DATABASES;"`
- Run database setup: `mysql -u root -p < ../database.sql`

### Compilation errors
- Ensure g++ supports C++17: `g++ --version`
- Install MySQL dev libraries: `sudo apt-get install libmysqlclient-dev`
- Check include paths in Makefile

### Frontend not loading
- Verify API server is running on port 8080
- Check browser console for JavaScript errors
- Ensure `public/` directory contains all HTML/CSS/JS files

### API returns errors
- Check server logs in terminal where API server is running
- Verify request format matches API documentation
- Check MySQL logs for database errors

## Development Notes

### Adding New Endpoints

1. Add handler function in `api_server.cpp`
2. Register endpoint in appropriate setup function
3. Add API method in `public/js/api.js`
4. Update frontend to use new endpoint

### Modifying Database Schema

1. Update `database.sql` in parent directory
2. Modify `Database.h` and `Database.cpp` as needed
3. Rebuild both CLI and web versions
4. Update API endpoints if necessary

## Performance Considerations

- API server uses single-threaded event loop (cpp-httplib default)
- Database connections are reused (one connection per server instance)
- For production, consider connection pooling
- Frontend uses asynchronous fetch for non-blocking requests

## Security Notes

⚠️ **This is a development version. For production:**

1. **Authentication**: Implement JWT or session tokens
2. **HTTPS**: Use TLS/SSL for encrypted communication
3. **Password Hashing**: Already implemented with MD5 (consider SHA-256)
4. **Input Validation**: Add server-side validation for all inputs
5. **CORS**: Restrict to specific domains
6. **SQL Injection**: Using prepared statements (already implemented)
7. **Rate Limiting**: Implement to prevent abuse

## License

This project is part of the Attendance Management System.

## Contact

For issues or questions, please refer to the main project documentation.
