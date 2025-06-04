#ifndef SCHEDULER_PLANNER_H
#define SCHEDULER_PLANNER_H

#include <string>
#include <vector>
#include <iostream> // For std::cout, std::cin in menu/display functions
#include <set>      // For std::set in addTask subject handling (can be forward declared if only in .cpp)
// Forward declare if only pointers/references are used in header. Otherwise, include full header.
// For now, assume full definitions might be indirectly needed by users of this header.
// #include "utils.h" // For utility function declarations if they were not moved to utils.h yet
// #include "file_handler.h" // For file operations if needed by functions declared here

// --- Data structures ---
struct ClassDetails {
    std::string subject;
    std::vector<std::string> daysOfWeek;
    std::string startTime;
    std::string endTime;
    std::string venue;
};

struct TaskDetails {
    std::string name;
    std::string subject;
    std::string infos;
    std::string deadlineDate; // Format "YYYY-MM-DD"
    int urgency;             // 1:High, 2:Moderate, 3:Low
    bool completed;

    TaskDetails() : urgency(3), completed(false) {}
};

// --- Function Declarations ---

// Calendar
void displayCalendar(); // Uses ClassDetails, TaskDetails, utils::getCurrentDateYYYYMMDD, utils::getCurrentDayOfWeek, utils::urgencyToString

// Class Scheduler
void classSchedulerMenu();    // Calls displayClassScheduleMenu, addClass, editClass
void displayClassSchedule();  // Uses ClassDetails
void addClass();              // Uses ClassDetails, utils::parseDaysOfWeek, utils::isValidTimeFormat, checkClassConflict, file_handler::saveClassScheduleToFile
void editClass();             // Uses ClassDetails, utils::parseDaysOfWeek, utils::isValidTimeFormat, checkClassConflict, file_handler::saveClassScheduleToFile
bool checkClassConflict(const ClassDetails& classToValidate, int editingClassIndex = -1); // Uses ClassDetails, utils::timeToMinutes

// Task Manager
void taskManagerMenu();   // Calls displayTaskManagerMenu, showTasks, addTask, deleteTask
void addTask();           // Uses TaskDetails, ClassDetails (for subject list), file_handler::saveTasksToFile
void showTasks();         // Uses TaskDetails, utils::urgencyToString, file_handler::saveTasksToFile
void deleteTask();        // Uses TaskDetails, utils::urgencyToString, file_handler::saveTasksToFile
std::string urgencyToString(int urgency); // Simple utility, but closely tied to TaskDetails display

// Menu Display functions specific to Scheduler/Planner
void displaySchedulerPlannerMenu(); // Specific menu display
void displayClassScheduleMenu(); // Calls displayClassSchedule
void displayTaskManagerMenu();
void schedulerPlannerMenu(); // Top-level menu for this module

// Subject retrieval
std::vector<std::string> get_scheduler_subjects();

#endif // SCHEDULER_PLANNER_H
