#include <iostream>
#include <string>       // For std::string if used directly in main or displayMainMenu
#include <vector>       // For std::vector if used directly
#include <limits>       // For std::numeric_limits

// Include all the new module headers
#include "main.h" // For displayMainMenu
#include "utils.h" // For utility functions like clear_input_buffer (if main uses it)
#include "file_handler.h" // For loadClassScheduleFromFile, loadTasksFromFile
#include "scheduler_planner.h" // For schedulerPlannerMenu
#include "study_hub.h"         // For studyHubMenu

// --- Main Menu Display Function ---
void displayMainMenu() {
    std::cout << "\nISKAALAMAN Main Menu:" << std::endl;
    std::cout << "1. ISKAALAMAN scheduler and Planner" << std::endl;
    std::cout << "2. ISKAALAMAN study hub" << std::endl;
    std::cout << "3. Exit" << std::endl;
    std::cout << "Enter your choice (1-3): ";
}

// --- Main Application Logic ---
int main() {
    // Load initial data
    loadClassScheduleFromFile(); // From file_handler.h
    loadTasksFromFile();         // From file_handler.h
    // Note: Study Hub data (flashcards.dat, notebooks.dat) is loaded at the start of studyHubMenu()

    int choice;
    bool running = true;
    while (running) {
        displayMainMenu(); // From main.h (defined in this file)
        std::cin >> choice;

        if (std::cin.good()) {
            clear_input_buffer(); // Essential after std::cin >> choice and before other menus might use std::getline
                                  // Assuming clear_input_buffer is now in utils.h and properly included.

            switch (choice) {
                case 1:
                    schedulerPlannerMenu(); // From scheduler_planner.h
                    break;
                case 2:
                    studyHubMenu();         // From study_hub.h
                    break;
                case 3:
                    running = false;
                    std::cout << "Exiting ISKAALAMAN. Goodbye!" << std::endl;
                    break;
                default:
                    std::cout << "Invalid choice. Please enter a number between 1 and 3." << std::endl;
                    // No need for clear_input_buffer here as next iter will re-prompt after error.
                    break;
            }
        } else {
            std::cout << "Invalid input. Please enter a number." << std::endl;
            std::cin.clear(); // Clear error flags
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard bad input
        }
    }
    return 0;
}
