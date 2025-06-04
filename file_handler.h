#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream> // For std::cerr (though consider minimizing iostream in headers)
#include <algorithm> // For std::replace in saveTasksToFile, if definition is here

// Forward declarations for data structures used by file handlers
// These structures will be fully defined in their respective feature headers (e.g., scheduler_planner.h, study_hub.h)
struct ClassDetails;
struct TaskDetails;
struct Deck;
// struct Card; // Card is part of Deck, so Deck's definition will include it.
// struct Note; // Note is part of Notebook.
struct Notebook;

// Extern declarations for global data vectors that file handlers will operate on.
// These will be defined in the .cpp file where they logically belong (e.g., scheduler_planner.cpp, study_hub.cpp or a central data.cpp)
extern std::vector<ClassDetails> classSchedule;
extern std::vector<TaskDetails> tasks;
extern std::vector<Deck> flashcard_decks;
extern std::vector<Notebook> notebooks;

// Extern declarations for file constants
extern const std::string FLASHCARDS_FILE;
extern const std::string NOTEBOOKS_FILE;
extern const std::string NOTE_CONTENT_START_DELIMITER;
extern const std::string NOTE_CONTENT_END_DELIMITER;
extern const std::string CLASS_SCHEDULE_FILE;
extern const std::string TASKS_FILE;

// Function Declarations
void saveClassScheduleToFile();
void loadClassScheduleFromFile();
void saveTasksToFile();
void loadTasksFromFile();
void save_flashcards_to_file();
void load_flashcards_from_file();
void save_notebooks_to_file();
void load_notebooks_from_file();

#endif // FILE_HANDLER_H
