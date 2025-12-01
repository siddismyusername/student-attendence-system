# Quick Start Guide - Web Interface

## 🎉 Your Web-Based Attendance Management System is Ready!

### ✅ What's Been Completed

All components have been successfully implemented and tested:

1. **REST API Server** (C++ with cpp-httplib)
2. **Modern Web Frontend** (HTML/CSS/JavaScript)
3. **Complete Documentation** (web/README.md)

### 🚀 How to Run

#### 1. Start the API Server

```bash
cd /home/siddharth/siddharth/project/Edutrackv2
./web/bin/api_server
```

You should see:
```
Starting Attendance Management System API Server...
Database connected successfully!
API Server running on http://localhost:8080
Access web interface at http://localhost:8080
```

#### 2. Access the Web Interface

Open your web browser and navigate to:
```
http://localhost:8080
```

### 🔐 Default Login Credentials

#### Admin
- **Email:** `admin@school.com`
- **Password:** `admin123`

#### Teacher
Create teachers through the admin dashboard, then use:
- **Email:** (teacher's email)
- **Password:** (teacher's password)

#### Student
Students use their name as login (simplified for demo):
- **Email:** (student name)
- **Password:** (any value - not validated in current version)

### 📋 Quick Workflow

1. **Login as Admin** (`admin@school.com` / `admin123`)
2. **Create Subjects** (e.g., Mathematics, Physics, Chemistry)
3. **Create Classes** (e.g., Grade 10A, Grade 11B)
4. **Create Teachers** with email and password
5. **Create Students** 
6. **Assign**:
   - Class Teachers to classes
   - Subject Teachers to subjects
   - Subjects to classes
   - Students to classes

7. **Login as Teacher** to mark attendance
8. **Login as Student** to view attendance

### 🛠️ Rebuilding the Server

If you make changes to the API server:

```bash
cd /home/siddharth/siddharth/project/Edutrackv2/web
make clean
make
```

### 📚 Full Documentation

See `web/README.md` for:
- Complete API documentation
- All available endpoints
- Troubleshooting guide
- Security notes

### 🎨 Features

#### Admin Dashboard
- ✅ Create/Delete Subjects
- ✅ Create/Delete Classes
- ✅ Create/Delete Teachers
- ✅ Create/Delete Students
- ✅ Assign Class Teachers
- ✅ Assign Subject Teachers
- ✅ Add Subjects to Classes
- ✅ Assign Students to Classes

#### Teacher Dashboard
- ✅ Mark Attendance (bulk for entire class)
- ✅ View Student Attendance Records
- ✅ View Attendance Percentages
- ✅ Add Students to Class (Class Teachers only)

#### Student Dashboard
- ✅ View Overall Attendance Percentage
- ✅ View Subject-wise Attendance
- ✅ View Detailed Attendance Records
- ✅ Update Profile Name

### 🔧 Technology Stack

**Backend:**
- C++17
- cpp-httplib (HTTP server)
- nlohmann/json (JSON parsing)
- MySQL (database)

**Frontend:**
- Vanilla HTML5
- CSS3 (with gradients and animations)
- JavaScript (Fetch API)
- No frameworks - pure JS!

### 📞 Support

For issues or questions, refer to:
- `web/README.md` - Comprehensive documentation
- `srs.md` - System requirements specification
- Database schema in `database.sql`

---

**Server Status:** ✅ Running on http://localhost:8080

**Next Steps:** Open your browser and start managing attendance!
