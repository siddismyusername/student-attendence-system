// API Base URL
const API_BASE_URL = '/api';

// API Helper Class
class API {
    // Authentication
    static async login(role, identifier, password) {
        return await this.post(`/login/${role}`, { email: identifier, password });
    }

    // Subjects
    static async getSubjects() {
        return await this.get('/subjects');
    }

    static async createSubject(name) {
        return await this.post('/subjects', { name });
    }

    static async deleteSubject(id) {
        return await this.delete(`/subjects/${id}`);
    }

    // Classes
    static async getClasses() {
        return await this.get('/classes');
    }

    static async createClass(name) {
        return await this.post('/classes', { name });
    }

    static async deleteClass(id) {
        return await this.delete(`/classes/${id}`);
    }

    static async getClassSubjects(classId) {
        return await this.get(`/classes/${classId}/subjects`);
    }

    static async addSubjectToClass(classId, subjectId) {
        return await this.post(`/classes/${classId}/subjects`, { subjectId });
    }

    static async getClassStudents(classId) {
        return await this.get(`/classes/${classId}/students`);
    }

    // Teachers
    static async getTeachers() {
        return await this.get('/teachers');
    }

    static async createTeacher(name, email, password, type) {
        return await this.post('/teachers', { name, email, password, type });
    }

    static async deleteTeacher(id) {
        return await this.delete(`/teachers/${id}`);
    }

    static async assignClassTeacher(teacherId, classId) {
        return await this.post(`/teachers/${teacherId}/assign-class`, { classId });
    }

    static async assignSubjectTeacher(teacherId, subjectId, classId) {
        return await this.post(`/teachers/${teacherId}/assign-subject`, { subjectId, classId });
    }

    static async getTeacherSubjects(teacherId) {
        return await this.get(`/teachers/${teacherId}/subjects`);
    }

    // Students
    static async getStudents() {
        return await this.get('/students');
    }

    static async createStudent(name) {
        return await this.post('/students', { name });
    }

    static async deleteStudent(id) {
        return await this.delete(`/students/${id}`);
    }

    static async assignStudentToClass(studentId, classId) {
        return await this.post(`/students/${studentId}/assign-class`, { classId });
    }

    static async updateStudentProfile(studentId, name) {
        return await this.put(`/students/${studentId}/profile`, { name });
    }

    // Attendance
    static async markAttendance(studentId, subjectId, date, status) {
        return await this.post('/attendance', { studentId, subjectId, date, status });
    }

    static async getStudentAttendance(studentId, subjectId) {
        return await this.get(`/students/${studentId}/attendance/subject/${subjectId}`);
    }

    static async getAttendancePercentage(studentId) {
        return await this.get(`/students/${studentId}/attendance-percentage`);
    }

    static async getSubjectAttendancePercentage(studentId, subjectId) {
        return await this.get(`/students/${studentId}/attendance-percentage/subject/${subjectId}`);
    }

    static async getClassAttendance(classId, date, subjectId) {
        return await this.get(`/classes/${classId}/attendance?date=${date}&subjectId=${subjectId}`);
    }

    static async checkAttendanceMarked(studentId, subjectId, date) {
        return await this.get(`/attendance/check?studentId=${studentId}&subjectId=${subjectId}&date=${date}`);
    }

    // HTTP Methods
    static async get(endpoint) {
        try {
            const response = await fetch(API_BASE_URL + endpoint, {
                method: 'GET',
                headers: {
                    'Content-Type': 'application/json'
                }
            });
            const text = await response.text();
            if (!text) return { success: false, error: 'Empty response from server' };
            return JSON.parse(text);
        } catch (error) {
            console.error('GET request failed:', error);
            return { success: false, error: error.message };
        }
    }

    static async post(endpoint, data) {
        try {
            const response = await fetch(API_BASE_URL + endpoint, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(data)
            });
            const text = await response.text();
            if (!text) return { success: false, error: 'Empty response from server' };
            return JSON.parse(text);
        } catch (error) {
            console.error('POST request failed:', error);
            return { success: false, error: error.message };
        }
    }

    static async put(endpoint, data) {
        try {
            const response = await fetch(API_BASE_URL + endpoint, {
                method: 'PUT',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify(data)
            });
            const text = await response.text();
            if (!text) return { success: false, error: 'Empty response from server' };
            return JSON.parse(text);
        } catch (error) {
            console.error('PUT request failed:', error);
            return { success: false, error: error.message };
        }
    }

    static async delete(endpoint) {
        try {
            const response = await fetch(API_BASE_URL + endpoint, {
                method: 'DELETE',
                headers: {
                    'Content-Type': 'application/json'
                }
            });
            const text = await response.text();
            if (!text) return { success: false, error: 'Empty response from server' };
            return JSON.parse(text);
        } catch (error) {
            console.error('DELETE request failed:', error);
            return { success: false, error: error.message };
        }
    }
}

// Utility Functions
function getCurrentDate() {
    const now = new Date();
    const year = now.getFullYear();
    const month = String(now.getMonth() + 1).padStart(2, '0');
    const day = String(now.getDate()).padStart(2, '0');
    return `${year}-${month}-${day}`;
}

function formatDate(dateString) {
    const date = new Date(dateString);
    return date.toLocaleDateString('en-US', {
        year: 'numeric',
        month: 'long',
        day: 'numeric'
    });
}

function showLoading(elementId) {
    const element = document.getElementById(elementId);
    if (element) {
        element.innerHTML = '<div class="loading"><div class="spinner"></div><p>Loading...</p></div>';
    }
}

function showError(elementId, message) {
    const element = document.getElementById(elementId);
    if (element) {
        element.innerHTML = `<div class="error-message">${message}</div>`;
    }
}

function showSuccess(elementId, message) {
    const element = document.getElementById(elementId);
    if (element) {
        element.innerHTML = `<div class="success-message">${message}</div>`;
    }
}

function logout() {
    sessionStorage.removeItem('user');
    window.location.href = 'index.html';
}

function checkAuth() {
    const user = sessionStorage.getItem('user');
    if (!user) {
        window.location.href = 'index.html';
        return null;
    }
    return JSON.parse(user);
}

function getAttendanceStatusClass(status) {
    switch (status.toLowerCase()) {
        case 'present':
            return 'status-present';
        case 'absent':
            return 'status-absent';
        case 'late':
            return 'status-late';
        default:
            return '';
    }
}
