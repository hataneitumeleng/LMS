PRAGMA foreign_keys = ON;

CREATE TABLE department (
  id   INTEGER PRIMARY KEY,
  name TEXT NOT NULL UNIQUE
);

CREATE TABLE user (
  id              INTEGER PRIMARY KEY,
  username        TEXT NOT NULL UNIQUE,
  password_hash   TEXT NOT NULL,              -- Argon2id string (NFR-03)
  full_name       TEXT NOT NULL,
  role            TEXT NOT NULL CHECK (role IN ('Student','Lecturer','HoD','Administrator')),
  department_id   INTEGER REFERENCES department(id),
  active          INTEGER NOT NULL DEFAULT 1 CHECK (active IN (0,1)),
  failed_attempts INTEGER NOT NULL DEFAULT 0,
  locked_until    TEXT,                       -- NFR-04 lockout
  CHECK ((role = 'HoD') = (department_id IS NOT NULL))
);
-- at most one ACTIVE HoD per department; "at least one" is enforced by AdminService (Section 5.2)
CREATE UNIQUE INDEX one_active_hod_per_dept ON user(department_id) WHERE role = 'HoD' AND active = 1;

CREATE TABLE course (
  id            INTEGER PRIMARY KEY,
  code          TEXT NOT NULL UNIQUE,
  title         TEXT NOT NULL,
  department_id INTEGER NOT NULL REFERENCES department(id)
);

CREATE TABLE course_lecturer (
  course_id   INTEGER NOT NULL REFERENCES course(id),
  lecturer_id INTEGER NOT NULL REFERENCES user(id),
  PRIMARY KEY (course_id, lecturer_id)
);

CREATE TABLE enrolment (
  course_id  INTEGER NOT NULL REFERENCES course(id),
  student_id INTEGER NOT NULL REFERENCES user(id),
  PRIMARY KEY (course_id, student_id)
);

CREATE TABLE assignment (
  id          INTEGER PRIMARY KEY,
  course_id   INTEGER NOT NULL REFERENCES course(id),
  title       TEXT NOT NULL,
  description TEXT,
  deadline    TEXT NOT NULL,                  -- ISO 8601, local school time
  max_mark    INTEGER NOT NULL CHECK (max_mark BETWEEN 1 AND 100),
  created_by  INTEGER NOT NULL REFERENCES user(id)
);

CREATE TABLE extension (
  assignment_id INTEGER NOT NULL REFERENCES assignment(id),
  student_id    INTEGER NOT NULL REFERENCES user(id),
  new_deadline  TEXT NOT NULL,                -- later than deadline, <= 14 days (BR-03, checked in service)
  reason        TEXT NOT NULL CHECK (length(trim(reason)) > 0),
  granted_by    INTEGER NOT NULL REFERENCES user(id),
  granted_at    TEXT NOT NULL,
  PRIMARY KEY (assignment_id, student_id)
);

CREATE TABLE submission (
  id            INTEGER PRIMARY KEY,
  assignment_id INTEGER NOT NULL REFERENCES assignment(id),
  student_id    INTEGER NOT NULL REFERENCES user(id),
  status        TEXT NOT NULL CHECK (status IN ('Submitted','Late','Marked','Moderated','Released')),
  days_late     INTEGER NOT NULL DEFAULT 0 CHECK (days_late >= 0),
  raw_mark      REAL CHECK (raw_mark >= 0),
  final_mark    REAL CHECK (final_mark >= 0),
  feedback      TEXT,
  marked_by     INTEGER REFERENCES user(id),
  moderated_by  INTEGER REFERENCES user(id),
  moderated_at  TEXT,
  UNIQUE (assignment_id, student_id),
  CHECK (status <> 'Submitted' OR days_late = 0),
  CHECK (status <> 'Late' OR days_late >= 1),
  CHECK (status IN ('Submitted','Late') OR (raw_mark IS NOT NULL AND final_mark IS NOT NULL
         AND marked_by IS NOT NULL AND length(trim(coalesce(feedback, ''))) > 0)),
  CHECK (status NOT IN ('Moderated','Released') OR (moderated_by IS NOT NULL AND moderated_at IS NOT NULL))
);
CREATE INDEX idx_submission_status ON submission(assignment_id, status);   -- NFR-02, R12

CREATE TABLE submission_version (
  submission_id INTEGER NOT NULL REFERENCES submission(id),
  version_no    INTEGER NOT NULL CHECK (version_no >= 1),
  file_path     TEXT NOT NULL,
  submitted_at  TEXT NOT NULL,
  PRIMARY KEY (submission_id, version_no)
);

CREATE TABLE audit_entry (
  id            INTEGER PRIMARY KEY,
  submission_id INTEGER NOT NULL REFERENCES submission(id),
  user_id       INTEGER NOT NULL REFERENCES user(id),
  changed_at    TEXT NOT NULL,
  old_value     REAL,                         -- NULL for the first mark
  new_value     REAL NOT NULL,
  reason        TEXT NOT NULL CHECK (length(trim(reason)) > 0)
);
CREATE INDEX idx_audit_submission ON audit_entry(submission_id);
