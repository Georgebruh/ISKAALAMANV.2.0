#ifndef STUDY_HUB_H
#define STUDY_HUB_H

#include <string>
#include <vector>
#include <iostream> // For std::cout, std::cin in menu/display functions

// Enum for Study Modes
enum StudyMode {
    NORMAL,
    CRAM
};

// --- Data structures for Study Hub (Flashcards & Notebooks) ---
struct Card {
    std::string type; // "true_false", "identification", "multiple_choice"
    std::string question;
    std::string answer;
    std::vector<std::string> options; // For multiple_choice type
};

struct Deck {
    std::string subject;
    std::string title;
    std::string timestamp;
    std::vector<Card> cards;
};

struct Note {
    std::string topic_title;
    std::string content;
    std::string timestamp;
};

struct Notebook {
    std::string subject;
    std::vector<Note> notes;
};

// --- Function Declarations for Study Hub ---
void studyHubMenu(); // Main menu for Study Hub

// Flashcard related functions
void show_flashcard_menu();
void create_deck();
void add_card_to_deck();
void delete_deck();
void view_specific_deck_content(const Deck& deck); // Helper to display deck
void study_deck_menu(Deck& deck); // Menu for studying a specific deck
void start_study_session(Deck& deck, StudyMode mode); // Starts a study session
void add_card_to_specific_deck(Deck& current_deck); // Adds card to an already selected deck
bool delete_specific_deck(size_t deck_index); // Deletes a deck by its index

// Notebook related functions
void show_notebook_menu();
void create_new_note(const std::string& subject);
// void view_note_content(const Note& note); // If a separate view function for single note is needed

#endif // STUDY_HUB_H
