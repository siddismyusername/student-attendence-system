# **Software Requirements Specification (SRS)**

## **Attendance Management System – C++ CLI Application**

---

# **1. Introduction**

## **1.1 Purpose**

The purpose of this system is to provide a command-line–based Attendance Management System implemented in C++. The system enables administrators, teachers, and students to manage and interact with academic attendance data. It uses a **MySQL** backend for persistent data storage and follows a **three-controller architecture** (Admin, Teacher, Student) built on top of a unified **Database** module.

The SRS defines the functional requirements, system behavior, validation rules, and access permissions based on the UML design.

---

## **1.2 Scope**

The system supports:

* Admin-controlled management of subjects, classes, teachers, and students
* Attendance marking by teachers
* Restricted teacher operations based on assignment
* Student self-access to their own attendance
* Secure authentication for admins and teachers
* Data validation and controlled CRUD operations
* Generation of attendance reports

This is a **local CLI application**, not networked, and interacts directly with MySQL.

---

## **1.3 Definitions**

* **ClassTeacher** – A teacher assigned to a specific class.
* **SubjectTeacher** – A teacher assigned to a specific subject in a class.
* **Attendance Record** – A single entry for a student on a specific date.
* **CRUD** – Create, Read, Update, Delete operations.

---

# **2. Overall Description**

## **2.1 System Architecture**

The system consists of:

### **Database Layer**

* Handles all interaction with MySQL
* Provides CRUD functions for all tables
* Performs validation checks before write operations

### **Controller Layer**

* **AdminController** – High-level management and deletion control
* **TeacherController** – Attendance entry and report viewing
* **StudentController** – Self-attendance access and profile updates

### **CLI Interface**

* Menu-based
* All inputs read through controller methods
* Output includes formatted text and simple tables

---

## **2.2 User Classes**

| Role               | Capabilities                                                                        |
| ------------------ | ----------------------------------------------------------------------------------- |
| **Admin**          | Full CRUD, delete entities, manage assignments                                      |
| **ClassTeacher**   | Mark attendance for own class, add students to own class, view own class attendance |
| **SubjectTeacher** | Mark attendance only for assigned subject/class, view own subject/class attendance  |
| **Student**        | View personal attendance and profile                                                |

---

# **3. Functional Requirements**

# **3.1 Authentication Requirements**

### **FR-1 Admin Login**

System must authenticate admin via email and password.

### **FR-2 Teacher Login**

System must authenticate teacher and load:

* teacherId
* teacherType (ClassTeacher / SubjectTeacher)

### **FR-3 Password Validation**

Passwords must match exactly (plain text for scope; hashing optional).

---

# **3.2 Admin Functional Requirements**

### **FR-4 Create Subject**

Admin can create a subject with name and max marks.

### **FR-5 Create Class**

Admin can create a class with a unique name.

### **FR-6 Create Teacher**

Admin can create a teacher and assign type.

### **FR-7 Create Student**

Admin can create a student and assign them to any class.

### **FR-8 Delete Entities**

Admin can delete:

* Subject
* Teacher
* Student
* Class

Validation:

* Cannot delete if referenced by assignments unless overridden internally.

### **FR-9 Assign Class Teacher**

Admin can assign exactly one teacher to a class.

### **FR-10 Assign Subject Teacher**

Admin can assign teacher to subject & class pair.

### **FR-11 View All**

Admin can view:

* All subjects
* All classes
* All teachers
* All students

---

# **3.3 Teacher Functional Requirements**

### **FR-12 Mark Attendance**

Teacher can mark attendance only if:

For **ClassTeacher**:

```
teacher.classId == student's classId
```

For **SubjectTeacher**:

```
teacher.subjectId == attendance.subjectId
AND
teacher.classId == student's classId
```

### **FR-13 Add Student to Own Class**

Only ClassTeachers can add new students to the class assigned to them.

### **FR-14 View Student Attendance**

Teacher can view attendance of students in:

* Their class (ClassTeacher)
* Their subject-class combination (SubjectTeacher)

### **FR-15 View Class Attendance**

Teacher can view attendance overview for their assigned class/subject.

---

# **3.4 Student Functional Requirements**

### **FR-16 View Personal Attendance**

Students can view all attendance records belonging to them.

### **FR-17 View Attendance Percentage**

Students can view:

* subject-wise attendance %
* overall attendance %

### **FR-18 Update Profile (Limited)**

Students can update:

* name
* password

They cannot update:

* class assignment
* student ID

---

# **3.5 Validation Requirements (Very Important)**

### **FR-19 Prevent Duplicate Entries**

* No two classes with same name
* No two subjects with same name
* No duplicate teacher-class assignment
* No student added twice to the same class

### **FR-20 Parent Existence Validation**

Before creating records, system must verify:

* Class exists before adding student
* Teacher exists before assignment
* Subject exists before assignment

### **FR-21 Attendance Validation**

System must ensure that:

* Student exists
* Date is valid
* Attendance is not marked twice for the same date & subject

---

# **3.6 Attendance Reporting Requirements**

### **FR-22 Student Full Report**

All records sorted in date order.

### **FR-23 Class Attendance Summary**

Counts:

* Present
* Absent
* Percentage

### **FR-24 Subject-wise Attendance**

For teachers and students:

* Total classes held
* Total attended
* Percentage

### **FR-25 Monthly or Date-Range Report**

User can filter:

* by month
* by date range

---

# **5. Constraints**

* MySQL must be installed and accessible
* System uses C++17
* CLI-only, no GUI for now
* No concurrency

---