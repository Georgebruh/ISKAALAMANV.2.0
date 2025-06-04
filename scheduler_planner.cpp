#include "scheduler_planner.h"
#include "utils.h"        // For various utility functions
#include "file_handler.h" // For saving/loading schedule and tasks
#include <algorithm>      // For std::sort, std::transform
#include <limits>         // For std::numeric_limits
#include <sstream>        // For std::stringstream in addClass (day parsing, though primary parsing is in utils)
#include <iomanip>        // For std::put_time (though this is now in utils.cpp) - remove if not directly used

// Definition of global data vectors for scheduler and planner
std::vector<ClassDetails> classSchedule;
std::vector<TaskDetails> tasks;

// --- Calendar Implementation ---
void displayCalendar() {
    std::string today_s_date = getCurrentDateYYYYMMDD();
    std::string current_day_of_week = getCurrentDayOfWeek();

    std::cout << "\n--- Calendar ---" << std::endl;
    std::cout << "Today's Date: " << today_s_date << " (" << current_day_of_week << ")" << std::endl;

    std::cout << "\n--- Today's Classes (" << current_day_of_week << ") ---" << std::endl;
    bool found_class_today = false;
    int class_display_count = 1;
    if (!classSchedule.empty()) {
        for (const auto& cls : classSchedule) {
            bool scheduled_for_today = false;
            for (const auto& day : cls.daysOfWeek) {
                if (day == current_day_of_week) {
                    scheduled_for_today = true;
                    break;
                }
            }
            if (scheduled_for_today) {
                if (!found_class_today) found_class_today = true;
                std::cout << class_display_count++ << ". Subject: " << cls.subject
                          << ", Start: " << cls.startTime
                          << ", End: " << cls.endTime
                          << ", Venue: " << cls.venue << std::endl;
            }
        }
    }
    if (!found_class_today) {
        std::cout << "<No classes scheduled for today>" << std::endl;
    }

    std::cout << "\n--- Today's Tasks (Due Today or Overdue and Not Completed) ---" << std::endl;
    bool found_task_for_today = false;
    int task_display_count = 1;
    // Ensure tasks is accessible, might need to be passed or made available if not global
    for (size_t i = 0; i < tasks.size(); ++i) {
        if (!tasks[i].completed && tasks[i].deadlineDate <= today_s_date) {
            if (!found_task_for_today) found_task_for_today = true;
            std::cout << task_display_count++ << ". Name: " << tasks[i].name
                      << " | Subject: " << tasks[i].subject
                      << " | Deadline: " << tasks[i].deadlineDate
                      << " | Urgency: " << urgencyToString(tasks[i].urgency)
                      << " | Infos: " << tasks[i].infos
                      << std::endl;
        }
    }
    if (!found_task_for_today) {
        std::cout << "<No tasks due today or overdue>" << std::endl;
    }

    std::cout << "\nPress Enter to return to the menu...";
    clear_input_buffer(); // From utils.h
    // Wait for a new line
    std::string dummy;
    std::getline(std::cin, dummy);
}

// --- Menu Display Functions for Scheduler/Planner ---
void displaySchedulerPlannerMenu() {
    std::cout << "\nISKAALAMAN Scheduler and Planner Menu:" << std::endl;
    std::cout << "1. Calendar" << std::endl;
    std::cout << "2. Class Scheduler" << std::endl;
    std::cout << "3. Task Manager" << std::endl;
    std::cout << "4. Back to Main Menu" << std::endl;
    std::cout << "Enter your choice (1-4): ";
}

void displayClassScheduleMenu() {
    std::cout << "\n--- Class Schedule ---" << std::endl;
    displayClassSchedule(); // Assumes this is now part of scheduler_planner.cpp
    std::cout << "\nClass Scheduler Options:" << std::endl;
    std::cout << "1. Add Class" << std::endl;
    std::cout << "2. Edit Class" << std::endl;
    std::cout << "3. Back to Scheduler/Planner Menu" << std::endl;
    std::cout << "Enter your choice (1-3): ";
}

void displayTaskManagerMenu() {
    std::cout << "\n--- Task Manager ---" << std::endl;
    std::cout << "1. Show Tasks" << std::endl;
    std::cout << "2. Add Task" << std::endl;
    std::cout << "3. Delete Task" << std::endl;
    std::cout << "4. Back to Scheduler/Planner Menu" << std::endl;
    std::cout << "Enter your choice (1-4): ";
}

// --- Class Scheduler Implementation ---
void displayClassSchedule() {
    if (classSchedule.empty()) {
        std::cout << "<no class schedule is available>" << std::endl;
    } else {
        std::cout << "Current Class Schedule:" << std::endl;
        for (size_t i = 0; i < classSchedule.size(); ++i) {
            std::cout << i + 1 << ". Subject: " << classSchedule[i].subject << ", Days: ";
            if (classSchedule[i].daysOfWeek.empty()) {
                std::cout << "N/A";
            } else {
                for (size_t j = 0; j < classSchedule[i].daysOfWeek.size(); ++j) {
                    std::cout << classSchedule[i].daysOfWeek[j] << (j < classSchedule[i].daysOfWeek.size() - 1 ? "," : "");
                }
            }
            std::cout << ", Start: " << classSchedule[i].startTime
                      << ", End: " << classSchedule[i].endTime
                      << ", Venue: " << classSchedule[i].venue << std::endl;
        }
    }
}

void addClass() {
    ClassDetails newClass;
    std::cout << "--- Add New Class ---" << std::endl;
    clear_input_buffer(); // From utils.h

    std::cout << "Enter Subject: ";
    std::getline(std::cin, newClass.subject);

    while(true) {
        std::cout << "Enter Start Time (e.g., 09:00 AM): ";
        std::getline(std::cin, newClass.startTime);
        if (isValidTimeFormat(newClass.startTime)) break; // from utils.h
        std::cout << "<Invalid time format. Please use HH:MM AM/PM (e.g., 09:30 AM).>" << std::endl;
    }

    while(true) {
        std::cout << "Enter End Time (e.g., 10:00 AM): ";
        std::getline(std::cin, newClass.endTime);
        if (isValidTimeFormat(newClass.endTime)) break; // from utils.h
        std::cout << "<Invalid time format. Please use HH:MM AM/PM (e.g., 09:30 AM).>" << std::endl;
    }

    bool daysParsedSuccessfully = false;
    std::string daysInput;
    while(!daysParsedSuccessfully){
        std::cout << "Enter Days of Week (e.g., Mon,Wed,Fri or M,T,W,TH,F,Sat,Sun): ";
        std::getline(std::cin, daysInput);
        if (daysInput.empty()){
            std::cout << "<Days of week cannot be empty when adding a new class.>" << std::endl;
            continue;
        }
        if (parseDaysOfWeek(daysInput, newClass.daysOfWeek)) { // parseDaysOfWeek from utils.h
            if (newClass.daysOfWeek.empty() && !daysInput.empty()) {
                std::cout << "<No valid days were recognized. Please check format (e.g., Mon,Tue,Wed).>" << std::endl;
            } else if (newClass.daysOfWeek.empty() && daysInput.empty()) {
                // This case should ideally not be hit if daysInput.empty() is checked above.
                // However, if parseDaysOfWeek can somehow result in empty daysOfWeek for an empty daysInput (e.g. only whitespace)
                // and still return true, this could be a fallback.
                // For addClass, empty days are an error.
                std::cout << "<No days entered. Please enter days for the class.>" << std::endl;
            }
            else { // Valid days parsed and daysOfWeek is not empty
                daysParsedSuccessfully = true;
            }
        } else {
            std::cout << "<Invalid day format or unrecognized day(s) entered. Please use formats like Mon,Tue,Wed or M,T,W,TH,F,Sat,Sun.>" << std::endl;
        }
    }

    std::cout << "Enter Venue: ";
    std::getline(std::cin, newClass.venue);

    if (checkClassConflict(newClass, -1)) { // checkClassConflict is now part of scheduler_planner.cpp
        // Error message printed by checkClassConflict
    } else {
        classSchedule.push_back(newClass);
        std::cout << "Class '" << newClass.subject << "' added successfully." << std::endl;
        saveClassScheduleToFile(); // from file_handler.h
    }
}

bool checkClassConflict(const ClassDetails& classToValidate, int editingClassIndex) {
    int newStartTimeMinutes = timeToMinutes(classToValidate.startTime); // from utils.h
    int newEndTimeMinutes = timeToMinutes(classToValidate.endTime);   // from utils.h

    if (newStartTimeMinutes == -1 || newEndTimeMinutes == -1) {
        // This implies an invalid format that wasn't caught by initial input validation,
        // or data was corrupted. isValidTimeFormat (called by timeToMinutes) should prevent this.
        std::cout << "<Internal Error: Invalid time format in class being validated. Conflict check aborted.>" << std::endl;
        return true; // Treat as conflict to be safe
    }

    if (newStartTimeMinutes >= newEndTimeMinutes) {
        std::cout << "<Start time must be before end time. Class not added/updated.>" << std::endl;
        return true;
    }

    for (size_t i = 0; i < classSchedule.size(); ++i) {
        if (static_cast<int>(i) == editingClassIndex) {
            continue;
        }
        const auto& existingClass = classSchedule[i];
        int existingStartTimeMinutes = timeToMinutes(existingClass.startTime);
        int existingEndTimeMinutes = timeToMinutes(existingClass.endTime);

        if (existingStartTimeMinutes == -1 || existingEndTimeMinutes == -1) {
            std::cout << "<Warning: Existing class '" << existingClass.subject << "' has invalid time format. Skipping for conflict check.>" << std::endl;
            continue;
        }

        bool commonDayFound = false;
        for (const auto& newDay : classToValidate.daysOfWeek) {
            for (const auto& existingDay : existingClass.daysOfWeek) {
                if (newDay == existingDay) {
                    commonDayFound = true;
                    break;
                }
            }
            if (commonDayFound) break;
        }

        if (commonDayFound) {
            if (newStartTimeMinutes < existingEndTimeMinutes && newEndTimeMinutes > existingStartTimeMinutes) {
                std::cout << "<Conflict detected with class: " << existingClass.subject
                          << " on common day(s). Time overlap: "
                          << existingClass.startTime << "-" << existingClass.endTime << " vs "
                          << classToValidate.startTime << "-" << classToValidate.endTime << ".>" << std::endl;
                return true;
            }
        }
    }
    return false;
}

void editClass() {
    if (classSchedule.empty()) {
        std::cout << "<No classes to edit.>" << std::endl;
        clear_input_buffer(); // Ensure buffer is clear for potential getline later
        std::cout << "Press Enter to return to the menu...";
        std::string dummy;
        std::getline(std::cin, dummy);
        return;
    }

    std::cout << "--- Edit Class ---" << std::endl;
    displayClassSchedule();

    int choice_num;
    while(true){
        std::cout << "Enter the number of the class to edit (or 0 to cancel): ";
        std::cin >> choice_num;
        if (std::cin.good()) {
            clear_input_buffer(); // Consume newline after number
            break;
        } else {
            std::cout << "<Invalid input. Please enter a number.>" << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }

    if (choice_num == 0) {
        std::cout << "Edit cancelled." << std::endl;
        return;
    }
    if (choice_num < 1 || static_cast<size_t>(choice_num) > classSchedule.size()) {
        std::cout << "<Invalid class number.>" << std::endl;
        return;
    }

    size_t classIndex = static_cast<size_t>(choice_num - 1);
    ClassDetails tempClass = classSchedule[classIndex];
    ClassDetails originalClass = classSchedule[classIndex];
    bool changed = false;
    std::string input;

    std::cout << "Current Subject: " << originalClass.subject << ". New (blank to keep): ";
    std::getline(std::cin, input);
    if (!input.empty()) { tempClass.subject = input; changed = true; }

    std::cout << "Current Days: ";
    for (size_t j = 0; j < originalClass.daysOfWeek.size(); ++j) {
        std::cout << originalClass.daysOfWeek[j] << (j < originalClass.daysOfWeek.size() - 1 ? "," : "");
    }
    std::cout << ".\nNew Days (blank to keep): ";
    std::getline(std::cin, input);
    if (!input.empty()) {
        std::vector<std::string> newDays;
        if (parseDaysOfWeek(input, newDays)) { // parseDaysOfWeek from utils.h
            if (newDays.empty() && !input.empty()){ // Successfully parsed an empty list from non-empty input.
                 std::cout << "<No valid days recognized from your input. Days not changed.>" << std::endl;
            } else { // newDays might be empty if input was just whitespace (which parseDaysOfWeek should handle and return true), or it might contain valid days
                tempClass.daysOfWeek = newDays; // Allow setting to empty days list if parse was successful
                changed = true;
            }
        } else {
            // parseDaysOfWeek returned false, indicating a parsing error (e.g. invalid characters, malformed token)
            std::cout << "<Invalid day format or unrecognized day(s) entered. Days not changed.>" << std::endl;
        }
    }

    std::cout << "Current Start Time: " << originalClass.startTime << ". New (blank to keep): ";
    std::getline(std::cin, input);
    if (!input.empty()) {
        if (isValidTimeFormat(input)) { // isValidTimeFormat from utils.h
            tempClass.startTime = input; changed = true;
        } else { std::cout << "<Start Time not changed due to invalid format.>" << std::endl; }
    }

    std::cout << "Current End Time: " << originalClass.endTime << ". New (blank to keep): ";
    std::getline(std::cin, input);
    if (!input.empty()) {
        if (isValidTimeFormat(input)) { // isValidTimeFormat from utils.h
            tempClass.endTime = input; changed = true;
        } else { std::cout << "<End Time not changed due to invalid format.>" << std::endl; }
    }

    std::cout << "Current Venue: " << originalClass.venue << ". New (blank to keep): ";
    std::getline(std::cin, input);
    if (!input.empty()) { tempClass.venue = input; changed = true; }

    if (!changed) {
        std::cout << "<No changes were made.>" << std::endl;
        return;
    }

    if (checkClassConflict(tempClass, static_cast<int>(classIndex))) {
        // Error message printed by checkClassConflict
        std::cout << "<Edit not saved due to conflict or invalid time range.>" << std::endl;
    } else {
        classSchedule[classIndex] = tempClass;
        std::cout << "Class '" << tempClass.subject << "' updated successfully." << std::endl;
        saveClassScheduleToFile(); // from file_handler.h
    }
}

// Function to get unique subject names from the class schedule
std::vector<std::string> get_scheduler_subjects() {
    std::set<std::string> unique_subjects;
    for (const auto& cls : classSchedule) { // classSchedule is global in this file
        if (!cls.subject.empty()) {
            unique_subjects.insert(cls.subject);
        }
    }
    // Convert set to vector
    std::vector<std::string> subjects_vector(unique_subjects.begin(), unique_subjects.end());
    return subjects_vector;
}

void classSchedulerMenu() {
    int choice;
    bool running = true;
    while (running) {
        displayClassScheduleMenu(); // Part of scheduler_planner.cpp
        std::cin >> choice;
        if (std::cin.good()) {
            clear_input_buffer(); // From utils.h, after reading number
            switch (choice) {
                case 1: addClass(); break;    // Part of scheduler_planner.cpp
                case 2: editClass(); break;   // Part of scheduler_planner.cpp
                case 3: running = false; std::cout << "Returning to Scheduler/Planner Menu..." << std::endl; break;
                default: std::cout << "Invalid choice. Please enter a number between 1 and 3." << std::endl; break;
            }
        } else {
            std::cout << "Invalid input. Please enter a number." << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}

// --- Task Manager Implementation ---
std::string urgencyToString(int urgency) {
    switch (urgency) {
        case 1: return "High";
        case 2: return "Moderate";
        case 3: return "Low";
        default: return "Unknown";
    }
}

void addTask() {
    TaskDetails newTask;
    std::cout << "--- Add New Task ---" << std::endl;
    clear_input_buffer(); // From utils.h

    std::cout << "Enter Task Name: ";
    std::getline(std::cin, newTask.name);

    if (!classSchedule.empty()) {
        std::set<std::string> uniqueSubjects;
        for (const auto& cls : classSchedule) { // classSchedule is global in this file
            if(!cls.subject.empty()) uniqueSubjects.insert(cls.subject);
        }

        if (!uniqueSubjects.empty()) {
            std::vector<std::string> subjectList(uniqueSubjects.begin(), uniqueSubjects.end());
            std::cout << "Available Subjects from Schedule:" << std::endl;
            for (size_t i = 0; i < subjectList.size(); ++i) {
                std::cout << i + 1 << ". " << subjectList[i] << std::endl;
            }
            int otherOptionNumber = subjectList.size() + 1;
            std::cout << otherOptionNumber << ". Other (Enter manually)" << std::endl;

            int subjectChoiceNum;
            bool subjectChosen = false;
            while (!subjectChosen) {
                std::cout << "Choose Subject by number (or " << otherOptionNumber << " to enter manually): ";
                std::cin >> subjectChoiceNum;
                if (std::cin.good()) {
                    clear_input_buffer(); // Consume newline
                    if (subjectChoiceNum > 0 && static_cast<size_t>(subjectChoiceNum) <= subjectList.size()) {
                        newTask.subject = subjectList[subjectChoiceNum - 1];
                        subjectChosen = true;
                    } else if (subjectChoiceNum == otherOptionNumber) {
                        std::cout << "Enter Subject: ";
                        std::getline(std::cin, newTask.subject);
                        subjectChosen = true;
                    } else {
                        std::cout << "<Invalid choice. Please try again.>" << std::endl;
                    }
                } else {
                    std::cout << "<Invalid input. Please enter a number.>" << std::endl;
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                }
            }
        } else {
             std::cout << "No subjects available from schedule. Enter Subject manually: ";
             std::getline(std::cin, newTask.subject);
        }
    } else {
        std::cout << "Enter Subject: ";
        std::getline(std::cin, newTask.subject);
    }

    std::cout << "Enter Infos (or type 'none'): ";
    std::getline(std::cin, newTask.infos);
    if (newTask.infos.empty() || newTask.infos == "none") {
        newTask.infos = "No info available";
    }
    std::cout << "Enter Deadline Date (YYYY-MM-DD): "; // Add date validation if desired (from utils?)
    std::getline(std::cin, newTask.deadlineDate);

    int urgencyInput;
    while (true) {
        std::cout << "Enter Urgency (1:High, 2:Moderate, 3:Low): ";
        std::cin >> urgencyInput;
        if (std::cin.good() && (urgencyInput >= 1 && urgencyInput <= 3)) {
            newTask.urgency = urgencyInput;
            clear_input_buffer(); // Consume newline
            break;
        } else {
            std::cout << "Invalid urgency. Please enter 1, 2, or 3." << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
    newTask.completed = false;
    tasks.push_back(newTask); // tasks is global in this file
    std::cout << "Task '" << newTask.name << "' added successfully." << std::endl;
    saveTasksToFile(); // from file_handler.h
}

void showTasks() {
    std::cout << "--- Show Tasks ---" << std::endl;
    if (tasks.empty()) { // tasks is global in this file
        std::cout << "<No tasks available>" << std::endl;
        return;
    }

    std::vector<size_t> uncompletedTaskIndices;
    for (size_t i = 0; i < tasks.size(); ++i) {
        if (!tasks[i].completed) {
            uncompletedTaskIndices.push_back(i);
        }
    }

    if (uncompletedTaskIndices.empty()) {
        std::cout << "<No pending tasks available>" << std::endl;
        return;
    }

    std::sort(uncompletedTaskIndices.begin(), uncompletedTaskIndices.end(),
        [&](size_t a, size_t b) { // Lambda captures `tasks` by reference (implicitly)
        if (tasks[a].urgency != tasks[b].urgency) {
            return tasks[a].urgency < tasks[b].urgency;
        }
        return tasks[a].deadlineDate < tasks[b].deadlineDate;
    });

    std::cout << "Pending Tasks (Sorted by Urgency, then Deadline):" << std::endl;
    for (size_t i = 0; i < uncompletedTaskIndices.size(); ++i) {
        const auto& task = tasks[uncompletedTaskIndices[i]];
        std::cout << i + 1 << ". Name: " << task.name
                  << " | Subject: " << task.subject
                  << " | Deadline: " << task.deadlineDate
                  << " | Urgency: " << urgencyToString(task.urgency) // urgencyToString is in this file
                  << " | Infos: " << task.infos
                  << std::endl;
    }

    int taskNumberToMark;
    std::cout << "\nMark a task as completed? (Enter task number or 0 to skip): ";
    std::cin >> taskNumberToMark;

    if (std::cin.good()) { // Input was numerically valid (though maybe out of range)
        if (taskNumberToMark > 0 && static_cast<size_t>(taskNumberToMark) <= uncompletedTaskIndices.size()) {
            size_t actualIndexInTasksVector = uncompletedTaskIndices[taskNumberToMark - 1];
            tasks[actualIndexInTasksVector].completed = true;
            std::cout << "Task '" << tasks[actualIndexInTasksVector].name << "' marked as completed." << std::endl;
            saveTasksToFile();
        } else if (taskNumberToMark == 0) {
            // Skipped
        } else { // Good numeric input, but number out of logical range
            std::cout << "<Invalid task number.>" << std::endl;
        }
    } else { // std::cin.good() is false - e.g., non-numeric input like "abc"
        std::cout << "<Invalid input. Please enter a number.>" << std::endl;
        std::cin.clear(); // Clear error flags (like failbit)
        // The clear_input_buffer() below will now correctly consume the rest of the bad input line
    }
    clear_input_buffer(); // Crucial: Clears newline from good input, or the rest of the bad input line if cin was bad and is now cleared.
}

void deleteTask() {
    if (tasks.empty()) { // tasks is global in this file
        std::cout << "<No tasks to delete.>" << std::endl;
        clear_input_buffer();
        std::cout << "Press Enter to return to the menu...";
        std::string dummy;
        std::getline(std::cin, dummy);
        return;
    }

    std::cout << "--- Delete Task ---" << std::endl;
    std::cout << "Available Tasks:" << std::endl;
    for (size_t i = 0; i < tasks.size(); ++i) {
        const auto& task = tasks[i];
        std::cout << i + 1 << ". Name: " << task.name
                  << " | Subject: " << task.subject
                  << " | Deadline: " << task.deadlineDate
                  << " | Urgency: " << urgencyToString(task.urgency) // urgencyToString is in this file
                  << " | Status: " << (task.completed ? "Completed" : "Pending")
                  << std::endl;
    }

    int choice_num;
    while (true) {
        std::cout << "Enter the number of the task to delete (or 0 to cancel): ";
        std::cin >> choice_num;
        if (std::cin.good()) {
            clear_input_buffer(); // Consume newline
            if (choice_num >= 0 && static_cast<size_t>(choice_num) <= tasks.size()) {
                break;
            } else {
                std::cout << "<Invalid task number. Please try again.>" << std::endl;
            }
        } else {
            std::cout << "<Invalid input. Please enter a number.>" << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }

    if (choice_num == 0) {
        std::cout << "Deletion cancelled." << std::endl;
        return;
    }

    size_t taskIndex = static_cast<size_t>(choice_num - 1);
    TaskDetails taskToDelete = tasks[taskIndex];

    std::string confirmStr;
    while(true) {
        std::cout << "Are you sure you want to delete task '" << taskToDelete.name << "'? (yes/no): ";
        std::getline(std::cin, confirmStr); // std::cin was cleared by clear_input_buffer
        std::transform(confirmStr.begin(), confirmStr.end(), confirmStr.begin(), ::tolower);
        if (confirmStr == "yes" || confirmStr == "y" || confirmStr == "no" || confirmStr == "n") {
            break;
        }
        std::cout << "<Invalid input. Please type 'yes' or 'no'.>" << std::endl;
    }

    if (confirmStr == "yes" || confirmStr == "y") {
        tasks.erase(tasks.begin() + taskIndex);
        std::cout << "Task '" << taskToDelete.name << "' deleted successfully." << std::endl;
        saveTasksToFile(); // from file_handler.h
    } else {
        std::cout << "Deletion cancelled." << std::endl;
    }
}

void taskManagerMenu() {
    int choice;
    bool running = true;
    while (running) {
        displayTaskManagerMenu(); // Part of scheduler_planner.cpp
        std::cin >> choice;
        if (std::cin.good()) {
            clear_input_buffer(); // From utils.h
            switch (choice) {
                case 1: showTasks(); break;   // Part of scheduler_planner.cpp
                case 2: addTask(); break;     // Part of scheduler_planner.cpp
                case 3: deleteTask(); break;  // Part of scheduler_planner.cpp
                case 4: running = false; std::cout << "Returning to Scheduler/Planner Menu..." << std::endl; break;
                default: std::cout << "Invalid choice. Please enter a number between 1 and 4." << std::endl; break;
            }
        } else {
            std::cout << "Invalid input. Please enter a number." << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}

// This is the top-level menu for this module
void schedulerPlannerMenu() {
    int choice;
    bool running = true;
    while (running) {
        displaySchedulerPlannerMenu(); // Part of scheduler_planner.cpp
        std::cin >> choice;
        if (std::cin.good()) {
            clear_input_buffer(); // From utils.h
            switch (choice) {
                case 1: displayCalendar(); break;    // Part of scheduler_planner.cpp
                case 2: classSchedulerMenu(); break; // Part of scheduler_planner.cpp
                case 3: taskManagerMenu(); break;    // Part of scheduler_planner.cpp
                case 4: running = false; std::cout << "Returning to Main Menu..." << std::endl; break;
                default: std::cout << "Invalid choice. Please enter a number between 1 and 4." << std::endl; break;
            }
        } else {
            std::cout << "Invalid input. Please enter a number." << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}
