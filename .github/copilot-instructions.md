# EduTrack v2 - AI Agent Instructions

## Project Overview
Dual-interface attendance management system with C++ backend (CLI + REST API) and MySQL database. Three-tier architecture: Database layer, Controller layer (Admin/Teacher/Student), and dual interfaces (CLI terminal + web frontend).

## Architecture Fundamentals

### Two Separate Applications
1. **CLI Application** (`attendance_system`): Terminal-based interface using controller classes
2. **Web Application** (`web/bin/api_server`): REST API server + static frontend serving

**Critical**: These share `Database.cpp` and `Config.cpp` but are built separately with different Makefiles.

### Database Layer Pattern
- **Single source of truth**: `Database` class handles ALL MySQL interactions
- **No raw SQL in controllers**: Controllers call `Database` methods exclusively
- **Field normalization**: Database returns `map<string, string>` with raw MySQL field names (`subject_id`, `teacher_id`, etc.)
- **Web API normalization**: `api_server.cpp` normalizes `*_id` → `id` in JSON responses using `mapVectorToJson()`

Example:
```cpp
// Database.cpp returns: {"subject_id": "1", "name": "Math"}
// api_server.cpp normalizes to: {"id": "1", "name": "Math"}
```

### Controller Inheritance Hierarchy
```
BaseController (abstract)
├── AdminController (full CRUD, assignments, deletions)
├── TeacherController (attendance marking, restricted by assignment)
└── StudentController (view-only, personal data)
```

## Critical Database Schema Knowledge

### Student Table Structure
```sql
students(student_id, name, class_id)  -- NO email, NO password fields
```
Students authenticate by ID only. Do NOT add email/password to student operations.

### Subject Table Structure
```sql
subjects(subject_id, name, max_marks)  -- NO code field
```
Subjects use auto-increment ID as identifier. Do NOT reference `subject.code`.

### Teacher Types & Constraints
- **ClassTeacher**: Assigned to ONE class, can mark attendance for all subjects in that class
- **SubjectTeacher**: Assigned to subject-class pairs, can only mark attendance for assigned combinations
- Constraint enforced in `teacher_class_assignments`: `class_id` is UNIQUE (one class teacher per class)

### Attendance Uniqueness
```sql
UNIQUE KEY unique_attendance (student_id, subject_id, attendance_date)
```
Cannot mark same student-subject-date twice. Check with `isAttendanceMarked()` before creating.

## Build System

### CLI Application
```bash
# From project root
make                  # Builds to ./attendance_system
make clean && make    # Clean rebuild
```
- Uses root `Makefile`
- Compiles: main.cpp + all controllers + Database + Config + UIHelper
- Links: `-lmysqlclient`

### Web Application
```bash
# From web/ directory
cd web
make                  # Builds to ./bin/api_server
make run              # Build and start server
```
- Uses `web/Makefile`
- Compiles: api_server.cpp + Database.cpp + Config.cpp (from parent `src/`)
- Includes: `httplib.h`, `json.hpp` (in `web/include/`)
- Links: `-lmysqlclient -lpthread`
- Serves static files from `web/public/`

**Important**: Web app must run from project root: `cd .. && ./web/bin/api_server` (config.txt location)

## Configuration & Database

### Database Connection
1. Reads `config.txt` in project root (key=value format)
2. Database constructor requires: `host`, `port`, `user`, `password`, `database`
3. Connection auto-reconnects on failure (see `Database::ensureConnection()`)

### Database Setup
```bash
mysql -u root -p < database.sql  # Creates DB + tables + default admin
```
Default admin: `admin@school.com` / `admin123`

## Web API Patterns

### Endpoint Structure
- Base: `http://localhost:8080/api`
- Auth: `/api/login/{admin|teacher|student}`
- Resources: `/api/{subjects|classes|teachers|students}`
- Relations: `/api/classes/:id/subjects`, `/api/teachers/:id/assign-class`

### Request/Response Pattern
All endpoints return:
```json
{"success": true, "data": {...}}  // on success
{"success": false, "error": "message"}  // on failure
```

### CORS Configuration
```cpp
res.set_header("Access-Control-Allow-Origin", "*");  // Development only
```
For production, restrict to specific domain.

## Common Development Tasks

### Adding New Entity Type
1. Add table to `database.sql`
2. Add CRUD methods to `Database.h` and `Database.cpp`
3. Add controller methods (AdminController for create/delete)
4. For web: Add endpoints in `api_server.cpp`, normalize field names
5. For web: Add UI forms in `public/*.html`, API calls in `public/js/api.js`

### Adding Validation
- Add validation method to `Database` class (e.g., `isSubjectNameUnique()`)
- Call before create/update in controller or API handler
- Return error message to user/client

### Modifying Attendance Logic
- Core logic in `Database::markAttendance()`
- Permission checks in `TeacherController::markAttendance()` (CLI) or API handler (web)
- Verify: teacher assignment, student-class match, subject-class relationship
- Check `isAttendanceMarked()` before insertion

## Frontend-Backend Sync Rules

1. **Match database schema exactly**: Frontend forms must only request fields that exist in tables
2. **Use normalized IDs**: Frontend expects `id` field, backend must normalize `*_id` fields
3. **Check REFACTORING_FIXES.md**: Documents all known sync issues and resolutions
4. **Test API before frontend**: `curl http://localhost:8080/api/subjects` to verify response structure

## Known Pitfalls

❌ **Don't** add email/password to student creation (not in schema)
❌ **Don't** reference `subject.code` (field doesn't exist)
❌ **Don't** run web server from `web/` directory (config.txt not found)
❌ **Don't** modify attendance without checking `isAttendanceMarked()`
❌ **Don't** assign multiple class teachers to same class (violates UNIQUE constraint)

✅ **Do** normalize field names in API responses (`*_id` → `id`)
✅ **Do** validate parent existence before creating relations
✅ **Do** check teacher type and assignments before allowing operations
✅ **Do** use `Database::escapeString()` for user input in queries

## Testing Workflow

1. **Start MySQL**: `sudo systemctl start mysql`
2. **Setup DB**: `mysql -u root -p < database.sql`
3. **Test CLI**: `make && ./attendance_system`
4. **Test Web**: `cd web && make && cd .. && ./web/bin/api_server`
5. **Access**: http://localhost:8080 (admin@school.com / admin123)

## Reference Files

- **Schema**: `database.sql` - Single source of truth for data model
- **SRS**: `srs.md` - Functional requirements and validation rules
- **API Docs**: `web/README.md` - Complete endpoint reference
- **Fixes Log**: `web/REFACTORING_FIXES.md` - Frontend-backend sync issues
- **Quick Start**: `QUICKSTART.md` - Web app workflow guide
