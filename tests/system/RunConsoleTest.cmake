# Runs one system test on the real console program (Phase 5, step 5).
#
# ctest calls this script with:
#   -DLMS=<path of lms or lms.exe>  -DNAME=<test name>
#   -DSYSTEM_DIR=<tests/system>     -DWORK=<empty folder to run in>
#
# 1. Makes a fresh folder with a few files a user might choose (essay.txt, brief.txt,
#    empty.txt), so every test starts with no lms.db.
# 2. Types setup.txt and then <NAME>.txt into the program, line by line, exactly as a
#    person would at the keyboard.
# 3. Saves everything the program printed in <WORK>/output.txt (the "actual result").
# 4. Checks <NAME>.expect (the "expected result"):
#      a line            must appear in the output, in this order;
#      a line with ! ... must NOT appear anywhere;
#      a line with # ... is a comment.

foreach(var LMS NAME SYSTEM_DIR WORK)
    if(NOT DEFINED ${var})
        message(FATAL_ERROR "RunConsoleTest.cmake needs -D${var}=...")
    endif()
endforeach()

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")
file(WRITE "${WORK}/essay.txt" "My essay, version 1\n")
file(WRITE "${WORK}/essay2.txt" "My essay, version 2\n")
file(WRITE "${WORK}/brief.txt" "Project 1 brief: write a 1000-word essay.\n")
file(WRITE "${WORK}/empty.txt" "")

# A test whose name ends in _startup also gets a file left behind by a "crash",
# which the program must delete when it starts (Week 3, Section 4.3).
if(NAME MATCHES "_startup$")
    file(MAKE_DIRECTORY "${WORK}/submissions")
    file(WRITE "${WORK}/submissions/sub99_v1.txt" "left over after a crash\n")
endif()

# Line endings: git on Windows may turn \n into \r\n, so remove every \r first.
file(READ "${SYSTEM_DIR}/setup.txt" setup)
file(READ "${SYSTEM_DIR}/${NAME}.txt" scenario)
string(REPLACE "\r" "" setup "${setup}")
string(REPLACE "\r" "" scenario "${scenario}")
file(WRITE "${WORK}/input.txt" "${setup}${scenario}")

execute_process(
    COMMAND "${LMS}"
    WORKING_DIRECTORY "${WORK}"
    INPUT_FILE "${WORK}/input.txt"
    OUTPUT_VARIABLE output
    ERROR_VARIABLE errors
    RESULT_VARIABLE exitCode
    TIMEOUT 60)
string(REPLACE "\r" "" output "${output}")
file(WRITE "${WORK}/output.txt" "${output}")

set(failures "")
if(NOT exitCode EQUAL 0)
    string(APPEND failures "\n  the program ended with code ${exitCode}: ${errors}")
endif()

file(STRINGS "${SYSTEM_DIR}/${NAME}.expect" expectations)
set(rest "${output}")
foreach(line IN LISTS expectations)
    string(REPLACE "\r" "" line "${line}")
    if(line STREQUAL "" OR line MATCHES "^#")
        continue()
    endif()
    if(line MATCHES "^! (.*)$")
        string(FIND "${output}" "${CMAKE_MATCH_1}" at)
        if(NOT at EQUAL -1)
            string(APPEND failures "\n  must not appear, but did:  ${CMAKE_MATCH_1}")
        endif()
    else()
        string(FIND "${rest}" "${line}" at)
        if(at EQUAL -1)
            string(APPEND failures "\n  expected (in this order), not found:  ${line}")
        else()
            string(LENGTH "${line}" length)
            math(EXPR after "${at} + ${length}")
            string(SUBSTRING "${rest}" ${after} -1 rest)
        endif()
    endif()
endforeach()

if(failures)
    message(FATAL_ERROR "System test ${NAME} FAILED:${failures}\n"
                        "The program's full output is in ${WORK}/output.txt")
endif()
message(STATUS "System test ${NAME} passed. Output: ${WORK}/output.txt")
