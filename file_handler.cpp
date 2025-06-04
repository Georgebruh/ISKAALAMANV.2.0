#include "file_handler.h"
#include "scheduler_planner.h" // For ClassDetails, TaskDetails definitions
#include "study_hub.h"         // For Deck, Card, Note, Notebook definitions
#include <limits>              // Required for std::numeric_limits by load functions

// Define file constants
const std::string FLASHCARDS_FILE = "flashcards.dat";
const std::string NOTEBOOKS_FILE = "notebooks.dat";
const std::string NOTE_CONTENT_START_DELIMITER = "---CONTENT_START---";
const std::string NOTE_CONTENT_END_DELIMITER = "---CONTENT_END---";
const std::string CLASS_SCHEDULE_FILE = "schedule.dat";
const std::string TASKS_FILE = "tasks.dat";

// Note: The actual global data vectors (classSchedule, tasks, etc.)
// must be defined (not extern) in one .cpp file. For now, file_handler.cpp
// will assume they are defined elsewhere (e.g., in their respective feature .cpp files
// or in iskaalaman.cpp if they remain truly global).
// For this refactoring step, we are only moving the functions.
// The definition of these global vectors will be handled when we refactor
// scheduler_planner.cpp and study_hub.cpp.


// --- File Handling Implementations for Scheduler and Tasks ---

void saveClassScheduleToFile() {
    std::ofstream outfile(CLASS_SCHEDULE_FILE);
    if (!outfile) {
        std::cerr << "Error: Could not open " << CLASS_SCHEDULE_FILE << " for writing." << std::endl;
        return;
    }

    outfile << classSchedule.size() << std::endl;
    for (const auto& cls : classSchedule) {
        outfile << cls.subject << std::endl;
        outfile << cls.startTime << std::endl;
        outfile << cls.endTime << std::endl;
        outfile << cls.venue << std::endl;
        outfile << cls.daysOfWeek.size() << std::endl;
        for (const auto& day : cls.daysOfWeek) {
            outfile << day << std::endl;
        }
    }
    outfile.close();
}

void loadClassScheduleFromFile() {
    std::ifstream infile(CLASS_SCHEDULE_FILE);
    if (!infile) {
        // std::cerr << "Info: " << CLASS_SCHEDULE_FILE << " not found. Starting with an empty schedule." << std::endl;
        return;
    }

    classSchedule.clear();
    size_t numClasses;
    infile >> numClasses;
    if (infile.fail()) {
        classSchedule.clear();
        infile.close();
        return;
    }
    infile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    for (size_t i = 0; i < numClasses; ++i) {
        ClassDetails currentClass;
        if (!std::getline(infile, currentClass.subject) ||
            !std::getline(infile, currentClass.startTime) ||
            !std::getline(infile, currentClass.endTime) ||
            !std::getline(infile, currentClass.venue)) {
            classSchedule.clear();
            infile.close();
            return;
        }

        size_t numDays;
        infile >> numDays;
        if (infile.fail()) {
            classSchedule.clear();
            infile.close();
            return;
        }
        infile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        currentClass.daysOfWeek.clear();
        for (size_t j = 0; j < numDays; ++j) {
            std::string day;
            if (!std::getline(infile, day)) {
                classSchedule.clear();
                infile.close();
                return;
            }
            currentClass.daysOfWeek.push_back(day);
        }
        classSchedule.push_back(currentClass);
    }
    infile.close();
}

void saveTasksToFile() {
    std::ofstream outfile(TASKS_FILE);
    if (!outfile) {
        std::cerr << "Error: Could not open " << TASKS_FILE << " for writing." << std::endl;
        return;
    }

    outfile << tasks.size() << std::endl;
    for (const auto& task : tasks) {
        outfile << task.name << std::endl;
        outfile << task.subject << std::endl;
        std::string tempInfos = task.infos;
        std::replace(tempInfos.begin(), tempInfos.end(), '\n', ' ');
        outfile << tempInfos << std::endl;
        outfile << task.deadlineDate << std::endl;
        outfile << task.urgency << std::endl;
        outfile << task.completed << std::endl;
    }
    outfile.close();
}

void loadTasksFromFile() {
    std::ifstream infile(TASKS_FILE);
    if (!infile) {
        return;
    }

    tasks.clear();
    size_t numTasks;
    infile >> numTasks;
    if (infile.fail()) {
        tasks.clear();
        infile.close();
        return;
    }
    infile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    for (size_t i = 0; i < numTasks; ++i) {
        TaskDetails currentTask;
        if (!std::getline(infile, currentTask.name) ||
            !std::getline(infile, currentTask.subject) ||
            !std::getline(infile, currentTask.infos) ||
            !std::getline(infile, currentTask.deadlineDate) ||
            !(infile >> currentTask.urgency) ||
            !(infile >> currentTask.completed)) {
            tasks.clear();
            infile.close();
            return;
        }
        infile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        tasks.push_back(currentTask);
    }
    infile.close();
}

// --- File Handling Implementations for Study Hub ---

void save_flashcards_to_file() {
    std::ofstream outfile(FLASHCARDS_FILE);
    if (!outfile) {
        std::cerr << "Error: Could not open " << FLASHCARDS_FILE << " for writing." << std::endl;
        return;
    }

    outfile << flashcard_decks.size() << std::endl;
    for (const auto& deck : flashcard_decks) {
        outfile << deck.subject << std::endl;
        outfile << deck.title << std::endl;
        outfile << deck.timestamp << std::endl;
        outfile << deck.cards.size() << std::endl;

        for (const auto& card : deck.cards) {
            outfile << card.type << std::endl;
            outfile << card.question << std::endl;
            outfile << card.answer << std::endl;
            if (card.type == "multiple_choice") {
                outfile << card.options.size() << std::endl;
                for (const auto& option : card.options) {
                    outfile << option << std::endl;
                }
            }
        }
    }
    outfile.close();
}

void load_flashcards_from_file() {
    std::ifstream infile(FLASHCARDS_FILE);
    if (!infile) {
        return;
    }

    int num_decks; // Matching original type, though size_t might be more consistent
    infile >> num_decks;
    infile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (infile.fail() || num_decks < 0) {
        flashcard_decks.clear(); // Ensure clear on corruption
        infile.close();
        return;
    }

    flashcard_decks.clear();
    for (int i = 0; i < num_decks; ++i) {
        Deck current_deck;
        if (!std::getline(infile, current_deck.subject) ||
            !std::getline(infile, current_deck.title) ||
            !std::getline(infile, current_deck.timestamp)) {
            flashcard_decks.clear(); infile.close(); return; // Corruption
        }

        int num_cards;
        infile >> num_cards;
        infile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (infile.fail() || num_cards < 0) { flashcard_decks.clear(); infile.close(); return; }


        for (int j = 0; j < num_cards; ++j) {
            Card current_card;
            if (!std::getline(infile, current_card.type) ||
                !std::getline(infile, current_card.question) ||
                !std::getline(infile, current_card.answer)) {
                 flashcard_decks.clear(); infile.close(); return;
            }

            if (current_card.type == "multiple_choice") {
                int num_options;
                infile >> num_options;
                infile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                if (infile.fail() || num_options < 0) { flashcard_decks.clear(); infile.close(); return; }

                for (int k = 0; k < num_options; ++k) {
                    std::string option;
                    if (!std::getline(infile, option)) { flashcard_decks.clear(); infile.close(); return;}
                    current_card.options.push_back(option);
                }
            }
            current_deck.cards.push_back(current_card);
        }
        flashcard_decks.push_back(current_deck);
    }
    infile.close();
}

void save_notebooks_to_file() {
    std::ofstream outfile(NOTEBOOKS_FILE);
    if (!outfile) {
        std::cerr << "Error: Could not open " << NOTEBOOKS_FILE << " for writing." << std::endl;
        return;
    }

    outfile << notebooks.size() << std::endl;
    for (const auto& notebook : notebooks) {
        outfile << notebook.subject << std::endl;
        outfile << notebook.notes.size() << std::endl;

        for (const auto& note : notebook.notes) {
            outfile << note.topic_title << std::endl;
            outfile << note.timestamp << std::endl;
            outfile << NOTE_CONTENT_START_DELIMITER << std::endl;
            outfile << note.content << std::endl; // Assuming content does not contain NOTE_CONTENT_END_DELIMITER internally
            outfile << NOTE_CONTENT_END_DELIMITER << std::endl;
        }
    }
    outfile.close();
}

void load_notebooks_from_file() {
    std::ifstream infile(NOTEBOOKS_FILE);
    if (!infile) {
        return;
    }

    int num_notebooks; // Matching original type
    infile >> num_notebooks;
    infile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (infile.fail() || num_notebooks < 0) {
        notebooks.clear(); // Ensure clear on corruption
        infile.close();
        return;
    }

    notebooks.clear();
    for (int i = 0; i < num_notebooks; ++i) {
        Notebook current_notebook;
        if (!std::getline(infile, current_notebook.subject)) {
            notebooks.clear(); infile.close(); return;
        }

        int num_notes;
        infile >> num_notes;
        infile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (infile.fail() || num_notes < 0) { notebooks.clear(); infile.close(); return; }

        for (int j = 0; j < num_notes; ++j) {
            Note current_note;
            if (!std::getline(infile, current_note.topic_title) ||
                !std::getline(infile, current_note.timestamp)) {
                 notebooks.clear(); infile.close(); return;
            }

            std::string delimiter_check;
            if (!std::getline(infile, delimiter_check) || delimiter_check != NOTE_CONTENT_START_DELIMITER) {
                 notebooks.clear(); infile.close(); return;
            }

            std::string line;
            current_note.content = "";
            while (std::getline(infile, line)) {
                if (line == NOTE_CONTENT_END_DELIMITER) {
                    break;
                }
                current_note.content += line + "\n";
            }
            // Remove trailing newline if content is not empty and loop broke on delimiter
            if (!current_note.content.empty() && line == NOTE_CONTENT_END_DELIMITER) {
                 // current_note.content.pop_back(); // Remove last '\n'
                 // Corrected logic:
                 if (current_note.content.length() > 0 && current_note.content.back() == '\n') {
                    current_note.content.pop_back();
                 }
            } else if (line != NOTE_CONTENT_END_DELIMITER) { // File ended unexpectedly
                 notebooks.clear(); infile.close(); return;
            }

            current_notebook.notes.push_back(current_note);
        }
        notebooks.push_back(current_notebook);
    }
    infile.close();
}
