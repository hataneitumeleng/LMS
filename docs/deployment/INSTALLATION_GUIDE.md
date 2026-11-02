# Installation and Deployment Guide

**Project:** Moderated Learning Management System (LMS)  
**Course:** CS4433 Software Engineering  
**Phase:** 7 — Deployment

## 1. Introduction

This guide describes how to install, build and run the Moderated Learning Management System (LMS). The application is developed in C++17 and supports both a console interface and a Qt 6 desktop graphical interface.

The LMS operates offline and uses SQLite for storing application data.

## 2. System Requirements

**Supported operating systems:**
- Windows 10/11
- Ubuntu 22.04

**Software requirements:**
- C++17-compatible compiler
- CMake
- Ninja build system
- SQLite3 development libraries
- libsodium development libraries
- Qt 6 Widgets development libraries for the GUI

GoogleTest is also required when building and executing the automated tests.

## 3. Obtaining the Source Code

The source code is available at:

https://github.com/hataneitumeleng/LMS

Clone the repository using:

```bash
git clone https://github.com/hataneitumeleng/LMS.git
cd LMS
```

Alternatively, download the repository as a ZIP file and extract it.

## 4. Building the Application

Open a terminal in the LMS project directory.

Configure the project:

```bash
cmake -S . -B build -G Ninja
```

Build the application:

```bash
cmake --build build
```

The build produces the console application and, when Qt 6 is available, the desktop GUI application.

## 5. Running the LMS

### 5.1 Console Application

On Windows:

```powershell
.\build\lms.exe
```

On Ubuntu:

```bash
./build/lms
```

### 5.2 Desktop GUI

On Windows:

```powershell
.\build\lms_gui.exe
```

On Ubuntu:

```bash
./build/lms_gui
```

The GUI executable is available when Qt 6 has been detected during CMake configuration.

## 6. Database and File Storage

The LMS uses an SQLite database named `lms.db` and a directory named `submissions` for uploaded student files.

Both are located relative to the application's working directory.

The application initializes the required storage when it starts.

For consistent operation, launch the application from the intended working directory.

## 7. Backup and Restore

The LMS provides backup functionality for protecting application data.

A complete backup includes:
- The SQLite database (`lms.db`)
- The `submissions` directory

Backups should be stored separately from the active application data.

Before restoring a backup, ensure that the application is not actively modifying the database or submission files.

## 8. Running Automated Tests

From the project directory, execute:

```bash
ctest --test-dir build --output-on-failure
```

The automated test suite includes unit, service, integration and system tests.

GUI acceptance checks require a build configured with the appropriate GUI testing option.

## 9. Troubleshooting

**CMake cannot find a required library:** Install the missing development dependency and configure the project again.

**GUI executable is missing:** Confirm that Qt 6 Widgets is installed and that CMake detected Qt during configuration.

**Database cannot be opened:** Check that the application has permission to create and modify files in its working directory.

**Uploaded files are missing:** Verify that the `submissions` directory is present and that the application is running from the correct working directory.

**Application does not start:** Check the terminal output for error messages and confirm that required runtime libraries are available.

## 10. Conclusion

The LMS can be deployed on supported Windows and Ubuntu computers using CMake. Its offline design, SQLite database and local submission storage allow the system to operate without a network connection after installation.