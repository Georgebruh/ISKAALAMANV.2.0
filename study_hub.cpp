#include "study_hub.h"
#include "scheduler_planner.h" // For get_scheduler_subjects()
#include "utils.h"        // For getCurrentTimestamp, clear_input_buffer, get_string_input
#include "file_handler.h" // For save/load operations for flashcards and notebooks
#include <algorithm>      // For std::transform
#include <limits>         // For std::numeric_limits
#include <sstream>        // For std::stringstream (if any, e.g. in create_deck for options)
#include <stdexcept>      // For std::stoi exception handling
#include <random>         // For std::random_device, std::mt19937, std::shuffle

// Definition of global data vectors for Study Hub
std::vector<Deck> flashcard_decks;
std::vector<Notebook> notebooks;

// --- Study Hub Helper Functions (previously in iskaalaman.cpp) ---
// Note: getCurrentTimestamp and clear_input_buffer are now in utils.cpp

// --- Flashcard Functions ---
void view_specific_deck_content(const Deck& deck) {
    std::cout << "\n--- Deck Details ---" << std::endl;
    std::cout << "Subject: " << deck.subject << std::endl;
    std::cout << "Title: " << deck.title << std::endl;
    std::cout << "Created: " << deck.timestamp << std::endl;

    if (deck.cards.empty()) {
        std::cout << "  No cards in this deck." << std::endl;
    } else {
        std::cout << "  Cards:" << std::endl;
        for (size_t j = 0; j < deck.cards.size(); ++j) {
            const Card& card = deck.cards[j];
            std::cout << "    Card " << j + 1 << ":" << std::endl;
            std::cout << "      Type: " << card.type << std::endl;
            std::cout << "      Question: " << card.question << std::endl;
            if (card.type == "multiple_choice") {
                std::cout << "      Options: ";
                for (size_t k = 0; k < card.options.size(); ++k) {
                    std::cout << card.options[k] << (k == card.options.size() - 1 ? "" : ", ");
                }
                std::cout << std::endl;
            }
            std::cout << "      Answer: " << card.answer << std::endl;
        }
    }
    std::cout << "----------------------\n" << std::endl;
}

void study_deck_menu(Deck& deck) {
    std::string choice_str;
    int choice = 0;
    // clear_input_buffer(); // Potentially needed if previous input was not getline

    while (true) {
        std::cout << "\n--- Study Deck: " << deck.title << " ---" << std::endl;
        std::cout << "1. Normal Study Mode" << std::endl;
        std::cout << "2. Cram Mode" << std::endl;
        std::cout << "3. Back to Flashcard Menu" << std::endl;
        choice_str = get_string_input("Enter your choice (1-3): ");

        try {
            choice = std::stoi(choice_str);
        } catch (const std::invalid_argument&) {
            std::cout << "Invalid input. Please enter a number." << std::endl;
            continue;
        } catch (const std::out_of_range&) {
            std::cout << "Input out of range. Please enter a valid number." << std::endl;
            continue;
        }

        switch (choice) {
            case 1:
                start_study_session(deck, StudyMode::NORMAL);
                break;
            case 2:
                start_study_session(deck, StudyMode::CRAM);
                break;
            case 3:
                std::cout << "Returning to Flashcard Menu..." << std::endl;
                return;
            default:
                std::cout << "Invalid choice. Please enter a number between 1 and 3." << std::endl;
        }
    }
}

// --- Study Session Helper Functions ---
static void display_card_interface(const Card& card) {
    std::cout << "\n-------------------- CARD --------------------" << std::endl;
    std::cout << "Front: " << card.question << std::endl;
    if (card.type == "multiple_choice") {
        std::cout << "Options:" << std::endl;
        for (size_t i = 0; i < card.options.size(); ++i) {
            std::cout << "  " << i + 1 << ". " << card.options[i] << std::endl;
        }
    }
    get_string_input("Press Enter to flip...");
    std::cout << "Back: " << card.answer << std::endl;
    std::cout << "------------------------------------------" << std::endl;
}

// --- Study Session Core Logic ---

// Implementation for Normal Mode
static void _run_normal_mode(Deck& deck) {
    if (deck.cards.empty()) { // Defensive check, though start_study_session also checks
        std::cout << "This deck is empty. Nothing to study in Normal Mode." << std::endl;
        get_string_input("Press Enter to return...");
        return;
    }

    std::vector<Card*> active_cards;
    for (size_t i = 0; i < deck.cards.size(); ++i) {
        active_cards.push_back(&deck.cards[i]);
    }

    // Optional: Shuffle active_cards here if random order is desired
    // std::random_device rd;
    // std::mt19937 g(rd());
    // std::shuffle(active_cards.begin(), active_cards.end(), g);

    int known_count = 0;

    std::cout << "Normal Mode: Reviewing " << active_cards.size() << " cards. Type 'quit' at any prompt to end the session." << std::endl;
    get_string_input("Press Enter to start...");

    while (!active_cards.empty()) {
        Card* current_card_ptr = active_cards.front();

        display_card_interface(*current_card_ptr);

        std::string user_response_str;
        while(true) {
            user_response_str = get_string_input("Did you get it right? (y/n/quit): ");
            std::transform(user_response_str.begin(), user_response_str.end(), user_response_str.begin(), ::tolower);

            if (user_response_str == "quit" || user_response_str == "q") {
                std::cout << "Session ended." << std::endl;
                return;
            } else if (user_response_str == "y" || user_response_str == "yes") {
                active_cards.erase(active_cards.begin()); // Remove from front
                known_count++;
                std::cout << "Correct! " << active_cards.size() << " cards remaining in this round." << std::endl;
                break;
            } else if (user_response_str == "n" || user_response_str == "no") {
                active_cards.erase(active_cards.begin()); // Remove from front
                active_cards.push_back(current_card_ptr);   // Add to the end
                std::cout << "Incorrect. This card will be shown again. " << active_cards.size() << " cards in the current review pile." << std::endl;
                break;
            } else {
                std::cout << "Invalid input. Please type 'y', 'n', or 'quit'." << std::endl;
            }
        }
        if (!active_cards.empty()) {
             get_string_input("Press Enter for next card...");
        }
    }

    std::cout << "\nCongratulations! You've correctly answered all " << known_count << " cards in this session!" << std::endl;
    get_string_input("Press Enter to return to the study menu...");
}

// Implementation for Cram Mode
static void _run_cram_mode(Deck& deck) {
    if (deck.cards.empty()) { // Defensive check
        std::cout << "This deck is empty. Nothing to study in Cram Mode." << std::endl;
        get_string_input("Press Enter to return...");
        return;
    }

    std::vector<Card*> current_round_cards;
    for (size_t i = 0; i < deck.cards.size(); ++i) {
        current_round_cards.push_back(&deck.cards[i]);
    }

    std::random_device rd;
    std::mt19937 g(rd());

    std::cout << "Cram Mode: Go through all cards. Incorrect cards will be repeated until correct. Type 'quit' to end." << std::endl;
    get_string_input("Press Enter to start...");

    while (!current_round_cards.empty()) {
        std::cout << "\n--- Starting new round with " << current_round_cards.size() << " card(s) ---" << std::endl;
        std::vector<Card*> next_round_cards;
        std::shuffle(current_round_cards.begin(), current_round_cards.end(), g);

        for (size_t i = 0; i < current_round_cards.size(); ++i) {
            Card* current_card_ptr = current_round_cards[i];
            display_card_interface(*current_card_ptr);

            std::string user_response_str;
            while(true) {
                user_response_str = get_string_input("Correct? (y/n/quit): ");
                std::transform(user_response_str.begin(), user_response_str.end(), user_response_str.begin(), ::tolower);

                if (user_response_str == "quit" || user_response_str == "q") {
                    std::cout << "Session ended." << std::endl;
                    return;
                } else if (user_response_str == "y" || user_response_str == "yes") {
                    std::cout << "Correct!" << std::endl;
                    break;
                } else if (user_response_str == "n" || user_response_str == "no") {
                    next_round_cards.push_back(current_card_ptr);
                    std::cout << "Incorrect. This card will appear in the next round if any." << std::endl;
                    break;
                } else {
                    std::cout << "Invalid input. Please type 'y', 'n', or 'quit'." << std::endl;
                }
            }
            if (i < current_round_cards.size() - 1 && !next_round_cards.empty() && next_round_cards.back() == current_card_ptr) {
                 // If card was incorrect and it's not the last one in the loop, pause.
                 // Or simply pause always if not the last card.
                 get_string_input("Press Enter for next card...");
            } else if (i < current_round_cards.size() - 1) {
                 get_string_input("Press Enter for next card...");
            }
        }

        current_round_cards = next_round_cards;

        if (!current_round_cards.empty()) {
            std::cout << "\n--- Round Complete ---" << std::endl;
            std::cout << current_round_cards.size() << " card(s) to review again." << std::endl;
            std::string continue_choice = get_string_input("Press Enter to continue to next round, or type 'quit' to end: ");
            std::transform(continue_choice.begin(), continue_choice.end(), continue_choice.begin(), ::tolower);
            if (continue_choice == "quit" || continue_choice == "q") {
                std::cout << "Session ended." << std::endl;
                return;
            }
        }
    }

    std::cout << "\nCongratulations! You've correctly answered all cards in Cram Mode!" << std::endl;
    get_string_input("Press Enter to return to the study menu...");
}

void start_study_session(Deck& deck, StudyMode mode) {
    if (deck.cards.empty()) {
        std::cout << "This deck has no cards to study. Please add some cards first." << std::endl;
        get_string_input("Press Enter to return...");
        return;
    }

    switch (mode) {
        case StudyMode::NORMAL:
            std::cout << "\nStarting Normal Mode for deck '" << deck.title << "'..." << std::endl;
            _run_normal_mode(deck);
            break;
        case StudyMode::CRAM:
            std::cout << "\nStarting Cram Mode for deck '" << deck.title << "'..." << std::endl;
            _run_cram_mode(deck);
            break;
        default:
            std::cout << "Unknown study mode selected." << std::endl; // Should not happen
            break;
    }
    std::cout << "Study session for '" << deck.title << "' ended." << std::endl;
}

void create_deck() {
    Deck new_deck;
    std::cout << "\n--- Create New Deck ---" << std::endl;
    // clear_input_buffer(); // Already called before studyHubMenu -> show_flashcard_menu -> create_deck

    std::vector<std::string> subjects = get_scheduler_subjects();
    if (!subjects.empty()) {
        std::cout << "Available subjects from scheduler:" << std::endl;
        for (size_t i = 0; i < subjects.size(); ++i) {
            std::cout << i + 1 << ". " << subjects[i] << std::endl;
        }
        int manual_option = subjects.size() + 1;
        std::cout << manual_option << ". Manually add new subject" << std::endl;

        std::string choice_str;
        int choice = 0;
        while (true) {
            choice_str = get_string_input("Choose a subject by number or select manual option: ");
            try {
                choice = std::stoi(choice_str);
                if (choice > 0 && choice <= manual_option) {
                    break;
                }
                std::cout << "Invalid choice. Please enter a number between 1 and " << manual_option << "." << std::endl;
            } catch (const std::invalid_argument&) {
                std::cout << "Invalid input. Please enter a number." << std::endl;
            } catch (const std::out_of_range&) {
                std::cout << "Input out of range." << std::endl;
            }
        }

        if (choice > 0 && static_cast<size_t>(choice) <= subjects.size()) {
            new_deck.subject = subjects[choice - 1];
        } else { // Manual option selected
            new_deck.subject = get_string_input("Enter the subject of the new deck: ");
            if (new_deck.subject.empty()) {
                std::cout << "Subject cannot be empty. Defaulting to 'General'." << std::endl;
                new_deck.subject = "General";
            }
        }
    } else {
        std::cout << "No subjects available from scheduler." << std::endl;
        std::string manual_subject_choice_str;
        while (true) {
            manual_subject_choice_str = get_string_input("Do you want to manually add a subject? (yes/no): ");
            std::transform(manual_subject_choice_str.begin(), manual_subject_choice_str.end(), manual_subject_choice_str.begin(), ::tolower);
            if (manual_subject_choice_str == "yes" || manual_subject_choice_str == "y" || manual_subject_choice_str == "no" || manual_subject_choice_str == "n") {
                break;
            }
            std::cout << "Invalid input. Please type 'yes' or 'no'." << std::endl;
        }
        if (manual_subject_choice_str == "yes" || manual_subject_choice_str == "y") {
            new_deck.subject = get_string_input("Enter the subject of the new deck: ");
            if (new_deck.subject.empty()) {
                std::cout << "Subject cannot be empty. Defaulting to 'General'." << std::endl;
                new_deck.subject = "General";
            }
        } else {
            std::cout << "No subject selected. Defaulting to 'General'." << std::endl;
            new_deck.subject = "General";
        }
    }

    new_deck.title = get_string_input("Enter the title of the new deck (default: Untitled Deck): ");
    if (new_deck.title.empty()) new_deck.title = "Untitled Deck";

    new_deck.timestamp = getCurrentTimestamp(); // From utils.h

    std::cout << "Deck '" << new_deck.title << "' (" << new_deck.subject << ") created on " << new_deck.timestamp << "." << std::endl;
    std::string add_cards_now_str;
    while (true) {
        add_cards_now_str = get_string_input("Do you want to add cards to this deck now? (yes/no): ");
        std::transform(add_cards_now_str.begin(), add_cards_now_str.end(), add_cards_now_str.begin(), ::tolower);
        if (add_cards_now_str == "yes" || add_cards_now_str == "y" || add_cards_now_str == "no" || add_cards_now_str == "n") {
            break;
        }
        std::cout << "Invalid input. Please type 'yes' or 'no'." << std::endl;
    }

    if (add_cards_now_str == "yes" || add_cards_now_str == "y") {
        // Temporarily add cards to new_deck.cards directly.
        // This avoids issues with selected_deck pointer if create_deck is called, then add_card_to_deck.
        Deck* temp_deck_ptr_for_card_adding = &new_deck;

        while (true) {
            std::string add_another_card_str = get_string_input("Add a card to this deck? (yes/no): ");
            std::transform(add_another_card_str.begin(), add_another_card_str.end(), add_another_card_str.begin(), ::tolower);
            if (add_another_card_str != "yes" && add_another_card_str != "y") {
                break;
            }

            Card new_card;
            std::cout << "\nCard Types:\n1. True/False\n2. Identification\n3. Multiple Choice" << std::endl;
            std::string card_type_choice_str;
            int card_type_choice = 0;

            while (true) {
                card_type_choice_str = get_string_input("Choose a card type (1-3): ");
                try {
                    card_type_choice = std::stoi(card_type_choice_str);
                    if (card_type_choice >= 1 && card_type_choice <= 3) break;
                    std::cout << "Invalid choice. Please enter a number between 1 and 3." << std::endl;
                } catch (const std::invalid_argument&) { std::cout << "Invalid input. Please enter a number." << std::endl; }
                  catch (const std::out_of_range&) { std::cout << "Input out of range." << std::endl; }
            }

            new_card.question = get_string_input("Enter the question: ");

            if (card_type_choice == 1) { // True/False
                new_card.type = "true_false";
                std::string answer_str;
                while (true) {
                    answer_str = get_string_input("Enter the answer (true/false): ");
                    std::transform(answer_str.begin(), answer_str.end(), answer_str.begin(), ::tolower);
                    if (answer_str == "true" || answer_str == "t") { new_card.answer = "true"; break; }
                    if (answer_str == "false" || answer_str == "f") { new_card.answer = "false"; break; }
                    std::cout << "Invalid input. Please enter 'true' or 'false'." << std::endl;
                }
            } else if (card_type_choice == 2) { // Identification
                new_card.type = "identification";
                new_card.answer = get_string_input("Enter the answer: ");
            } else { // Multiple Choice
                new_card.type = "multiple_choice";
                std::string options_line = get_string_input("Enter the options, separated by commas: ");
                std::stringstream ss(options_line);
                std::string option_token;
                while(std::getline(ss, option_token, ',')) {
                    option_token.erase(0, option_token.find_first_not_of(" \t")); // Trim leading
                    option_token.erase(option_token.find_last_not_of(" \t") + 1); // Trim trailing
                    if (!option_token.empty()) new_card.options.push_back(option_token);
                }
                if (new_card.options.empty()) {
                    std::cout << "No options entered. Please add options." << std::endl;
                    // Optionally loop here to force option entry
                }

                while (true) {
                    std::cout << "Enter the correct answer from options (";
                    for (size_t i = 0; i < new_card.options.size(); ++i) {
                        std::cout << new_card.options[i] << (i == new_card.options.size() - 1 ? "" : ", ");
                    }
                    std::cout << "): ";
                    std::getline(std::cin, new_card.answer); // Use getline for answer that might have spaces
                    bool valid_answer = false;
                    for (const auto& opt : new_card.options) {
                        if (opt == new_card.answer) { valid_answer = true; break; }
                    }
                    if (valid_answer) break;
                    std::cout << "Answer not in options. Please try again." << std::endl;
                }
            }
            temp_deck_ptr_for_card_adding->cards.push_back(new_card);
            std::cout << "Card added successfully to this deck!\n" << std::endl;
        }
    }
    flashcard_decks.push_back(new_deck); // flashcard_decks is global in this file
    std::cout << "\nDeck '" << new_deck.title << "' under subject '" << new_deck.subject << "' is now set up." << std::endl;
    if (new_deck.cards.empty() && (add_cards_now_str == "no" || add_cards_now_str == "n")) {
        std::cout << "You can add cards later using the 'Add Card to Deck' option." << std::endl;
    }
    std::cout << std::endl;
    save_flashcards_to_file(); // From file_handler.h
}

void add_card_to_deck() {
    if (flashcard_decks.empty()) {
        std::cout << "No decks available. Please create a deck first.\n" << std::endl;
        return;
    }
    // clear_input_buffer(); // Should be handled by show_flashcard_menu before calling this
    std::cout << "\n--- Add Card to Existing Deck ---" << std::endl;
    std::cout << "Available Decks:" << std::endl;
    for (size_t i = 0; i < flashcard_decks.size(); ++i) {
        std::cout << i + 1 << ". " << flashcard_decks[i].title << " (" << flashcard_decks[i].subject << ")" << std::endl;
    }

    int deck_choice_num = 0;
    Deck* selected_deck = nullptr; // Pointer to the chosen deck in flashcard_decks

    while (true) {
        std::string deck_choice_str = get_string_input("Choose a deck number to add a card to: ");
        try {
            deck_choice_num = std::stoi(deck_choice_str);
            if (deck_choice_num >= 1 && static_cast<size_t>(deck_choice_num) <= flashcard_decks.size()) {
                selected_deck = &flashcard_decks[deck_choice_num - 1];
                break;
            }
            std::cout << "Invalid deck number. Please try again." << std::endl;
        } catch (const std::invalid_argument&) { std::cout << "Invalid input. Please enter a number." << std::endl; }
          catch (const std::out_of_range&) { std::cout << "Input out of range." << std::endl; }
    }

    Card new_card;
    std::cout << "\nCard Types:\n1. True/False\n2. Identification\n3. Multiple Choice" << std::endl;
    std::string card_type_choice_str;
    int card_type_choice = 0;

    while (true) {
        card_type_choice_str = get_string_input("Choose a card type (1-3): ");
        try {
            card_type_choice = std::stoi(card_type_choice_str);
            if (card_type_choice >= 1 && card_type_choice <= 3) break;
            std::cout << "Invalid choice. Please enter a number between 1 and 3." << std::endl;
        } catch (const std::invalid_argument&) { std::cout << "Invalid input. Please enter a number." << std::endl; }
          catch (const std::out_of_range&) { std::cout << "Input out of range." << std::endl; }
    }

    new_card.question = get_string_input("Enter the question: ");

    if (card_type_choice == 1) { // True/False
        new_card.type = "true_false";
        std::string answer_str;
        while (true) {
            answer_str = get_string_input("Enter the answer (true/false): ");
            std::transform(answer_str.begin(), answer_str.end(), answer_str.begin(), ::tolower);
            if (answer_str == "true" || answer_str == "t") { new_card.answer = "true"; break; }
            if (answer_str == "false" || answer_str == "f") { new_card.answer = "false"; break; }
            std::cout << "Invalid input. Please enter 'true' or 'false'." << std::endl;
        }
    } else if (card_type_choice == 2) { // Identification
        new_card.type = "identification";
        new_card.answer = get_string_input("Enter the answer: ");
    } else { // Multiple Choice
        new_card.type = "multiple_choice";
        std::string options_line = get_string_input("Enter the options, separated by commas: ");
        std::stringstream ss(options_line);
        std::string option_token;
        while(std::getline(ss, option_token, ',')) {
            option_token.erase(0, option_token.find_first_not_of(" \t"));
            option_token.erase(option_token.find_last_not_of(" \t") + 1);
            if (!option_token.empty()) new_card.options.push_back(option_token);
        }
         if (new_card.options.empty()) {
            std::cout << "No options entered. Please add options for multiple choice." << std::endl;
            // Optionally loop here or handle as error
        }
        while (true) {
            std::cout << "Enter the correct answer from options (";
            for (size_t i = 0; i < new_card.options.size(); ++i) {
                std::cout << new_card.options[i] << (i == new_card.options.size() - 1 ? "" : ", ");
            }
            std::cout << "): ";
            std::getline(std::cin, new_card.answer);
            bool valid_answer = false;
            for (const auto& opt : new_card.options) { if (opt == new_card.answer) { valid_answer = true; break; } }
            if (valid_answer) break;
            std::cout << "Answer not in options. Please try again." << std::endl;
        }
    }
    selected_deck->cards.push_back(new_card);
    std::cout << "Card added successfully to deck '" << selected_deck->title << "'!\n" << std::endl;
    save_flashcards_to_file(); // From file_handler.h
}

void add_card_to_specific_deck(Deck& current_deck) {
    std::cout << "\n--- Adding New Card to Deck: " << current_deck.title << " ---" << std::endl;
    // clear_input_buffer(); // Should be handled by calling menu if necessary

    while (true) {
        std::string add_another_card_str = get_string_input("Add a card to this deck? (yes/no, or type 'quit' to finish): ");
        std::transform(add_another_card_str.begin(), add_another_card_str.end(), add_another_card_str.begin(), ::tolower);

        if (add_another_card_str == "quit" || add_another_card_str == "q") {
            break;
        }
        if (add_another_card_str != "yes" && add_another_card_str != "y") {
            if (add_another_card_str == "no" || add_another_card_str == "n") {
                break; // Stop if user says no
            }
            std::cout << "Invalid input. Please type 'yes', 'no', or 'quit'." << std::endl;
            continue; // Re-ask if input is neither yes/no nor quit
        }

        Card new_card;
        std::cout << "\nCard Types:\n1. True/False\n2. Identification\n3. Multiple Choice" << std::endl;
        std::string card_type_choice_str;
        int card_type_choice = 0;

        while (true) {
            card_type_choice_str = get_string_input("Choose a card type (1-3): ");
            try {
                card_type_choice = std::stoi(card_type_choice_str);
                if (card_type_choice >= 1 && card_type_choice <= 3) break;
                std::cout << "Invalid choice. Please enter a number between 1 and 3." << std::endl;
            } catch (const std::invalid_argument&) { std::cout << "Invalid input. Please enter a number." << std::endl; }
              catch (const std::out_of_range&) { std::cout << "Input out of range." << std::endl; }
        }

        new_card.question = get_string_input("Enter the question: ");
        if (new_card.question.empty()) {
            std::cout << "Question cannot be empty. Card not added." << std::endl;
            continue;
        }

        if (card_type_choice == 1) { // True/False
            new_card.type = "true_false";
            std::string answer_str;
            while (true) {
                answer_str = get_string_input("Enter the answer (true/false): ");
                std::transform(answer_str.begin(), answer_str.end(), answer_str.begin(), ::tolower);
                if (answer_str == "true" || answer_str == "t") { new_card.answer = "true"; break; }
                if (answer_str == "false" || answer_str == "f") { new_card.answer = "false"; break; }
                std::cout << "Invalid input. Please enter 'true' or 'false'." << std::endl;
            }
        } else if (card_type_choice == 2) { // Identification
            new_card.type = "identification";
            new_card.answer = get_string_input("Enter the answer: ");
            if (new_card.answer.empty()) {
                 std::cout << "Answer cannot be empty for Identification. Card not added." << std::endl;
                 continue;
            }
        } else { // Multiple Choice
            new_card.type = "multiple_choice";
            std::string options_line = get_string_input("Enter the options, separated by commas: ");
            std::stringstream ss(options_line);
            std::string option_token;
            while(std::getline(ss, option_token, ',')) {
                option_token.erase(0, option_token.find_first_not_of(" \t"));
                option_token.erase(option_token.find_last_not_of(" \t") + 1);
                if (!option_token.empty()) new_card.options.push_back(option_token);
            }
            if (new_card.options.empty()) {
                std::cout << "No options entered for Multiple Choice. Card not added." << std::endl;
                continue;
            }
            while (true) {
                std::cout << "Enter the correct answer from options (";
                for (size_t i = 0; i < new_card.options.size(); ++i) {
                    std::cout << new_card.options[i] << (i == new_card.options.size() - 1 ? "" : ", ");
                }
                std::cout << "): ";
                // std::getline(std::cin, new_card.answer); // Original problematic line if get_string_input was used before
                new_card.answer = get_string_input(""); // Use get_string_input to be safe with buffer
                bool valid_answer = false;
                for (const auto& opt : new_card.options) { if (opt == new_card.answer) { valid_answer = true; break; } }
                if (valid_answer) break;
                std::cout << "Answer not in options. Please try again." << std::endl;
            }
        }
        current_deck.cards.push_back(new_card);
        std::cout << "Card added successfully to deck '" << current_deck.title << "'!\n" << std::endl;
    }
    save_flashcards_to_file(); // Save after finishing adding cards
    std::cout << "Finished adding cards to '" << current_deck.title << "'.\n" << std::endl;
}

void delete_deck() {
    if (flashcard_decks.empty()) {
        std::cout << "No decks available to delete.\n" << std::endl;
        return;
    }
    // clear_input_buffer(); // Handled by show_flashcard_menu
    std::cout << "\n--- Delete Flashcard Deck ---" << std::endl;
    std::cout << "Available Decks to Delete:" << std::endl;
    for (size_t i = 0; i < flashcard_decks.size(); ++i) {
        std::cout << i + 1 << ". " << flashcard_decks[i].title << " (" << flashcard_decks[i].subject << ") - Created: " << flashcard_decks[i].timestamp << std::endl;
    }

    int deck_choice_num = 0;
    size_t deck_to_delete_idx = 0; // Use size_t for index

    while (true) {
        std::string deck_choice_str = get_string_input("Enter the number of the deck to delete: ");
        try {
            deck_choice_num = std::stoi(deck_choice_str);
            if (deck_choice_num >= 1 && static_cast<size_t>(deck_choice_num) <= flashcard_decks.size()) {
                deck_to_delete_idx = static_cast<size_t>(deck_choice_num - 1);
                break;
            }
            std::cout << "Invalid deck number. Please try again." << std::endl;
        } catch (const std::invalid_argument&) { std::cout << "Invalid input. Please enter a number." << std::endl; }
          catch (const std::out_of_range&) { std::cout << "Input out of range." << std::endl; }
    }

    std::string confirm_str = get_string_input("Are you sure you want to delete the deck '" + flashcard_decks[deck_to_delete_idx].title + "'? (yes/no): ");
    std::transform(confirm_str.begin(), confirm_str.end(), confirm_str.begin(), ::tolower);

    if (confirm_str == "yes" || confirm_str == "y") {
        std::string deleted_deck_title = flashcard_decks[deck_to_delete_idx].title;
        flashcard_decks.erase(flashcard_decks.begin() + deck_to_delete_idx);
        std::cout << "Deck '" << deleted_deck_title << "' deleted successfully.\n" << std::endl;
        save_flashcards_to_file(); // From file_handler.h
    } else {
        std::cout << "Deletion cancelled.\n" << std::endl;
    }
}

bool delete_specific_deck(size_t deck_index) {
    if (deck_index >= flashcard_decks.size()) {
        std::cout << "Invalid deck index. Cannot delete." << std::endl;
        return false; // Should not happen if called correctly
    }

    // Confirmation
    std::string confirm_str = get_string_input("Are you sure you want to delete the deck '" + flashcard_decks[deck_index].title + "'? (yes/no): ");
    std::transform(confirm_str.begin(), confirm_str.end(), confirm_str.begin(), ::tolower);

    if (confirm_str == "yes" || confirm_str == "y") {
        std::string deleted_deck_title = flashcard_decks[deck_index].title;
        flashcard_decks.erase(flashcard_decks.begin() + deck_index);
        save_flashcards_to_file(); // Save changes
        std::cout << "Deck '" << deleted_deck_title << "' deleted successfully.\n" << std::endl;
        return true;
    } else {
        std::cout << "Deletion cancelled.\n" << std::endl;
        return false;
    }
}

void show_flashcard_menu() {
    std::string choice_str;
    int choice = 0;
    clear_input_buffer(); // From utils.h - clear after main menu's choice input if study hub chosen

    while (true) {
        std::cout << "\n--- Flashcard Decks ---" << std::endl;
        size_t num_decks = flashcard_decks.size();

        if (num_decks == 0) {
            std::cout << "No flashcard decks available." << std::endl;
        } else {
            for (size_t i = 0; i < num_decks; ++i) {
                const auto& deck = flashcard_decks[i];
                std::cout << i + 1 << ". " << deck.title << " | " << deck.subject
                          << " | " << deck.timestamp << " (" << deck.cards.size() << " cards)" << std::endl;
            }
        }

        std::cout << "\nFlashcard Menu Options:" << std::endl;
        int view_deck_option_start = 1;
        int view_deck_option_end = num_decks;
        int make_new_option = num_decks + 1;
        int add_card_option = num_decks + 2;
        int delete_deck_option = num_decks + 3;
        int back_to_hub_option = num_decks + 4;

        if (num_decks > 0) {
             std::cout << view_deck_option_start << "-" << view_deck_option_end << ". View/Manage Deck Content" << std::endl;
        }
        std::cout << make_new_option << ". Make New Flashcard Deck" << std::endl;
        std::cout << add_card_option << ". Add Card to Existing Deck" << std::endl;
        std::cout << delete_deck_option << ". Delete Flashcard Deck" << std::endl;
        std::cout << back_to_hub_option << ". Back to Study Hub Menu" << std::endl;
        std::cout << "Enter your choice: ";
        std::getline(std::cin, choice_str);

        try {
            choice = std::stoi(choice_str);
        } catch (const std::invalid_argument&) { std::cout << "Invalid input. Please enter a number." << std::endl; continue; }
          catch (const std::out_of_range&) { std::cout << "Input out of range." << std::endl; continue; }

        if (num_decks > 0 && choice >= view_deck_option_start && choice <= view_deck_option_end) {
            Deck& selected_deck = flashcard_decks[choice - 1]; // Use reference
            bool back_to_all_decks = false;
            while (!back_to_all_decks) {
                std::cout << "\n--- Managing Deck: " << selected_deck.title << " ---" << std::endl;
                std::cout << "1. View Cards" << std::endl;
                std::cout << "2. Study This Deck" << std::endl;
                std::cout << "3. Add New Card to This Deck" << std::endl;
                std::cout << "4. Delete This Deck" << std::endl;
                std::cout << "5. Back to All Decks" << std::endl;

                std::string sub_choice_str = get_string_input("Enter your choice (1-5): ");
                int sub_choice = 0;

                try {
                    sub_choice = std::stoi(sub_choice_str);
                } catch (const std::invalid_argument&) { std::cout << "Invalid input. Please enter a number." << std::endl; continue; }
                  catch (const std::out_of_range&) { std::cout << "Input out of range." << std::endl; continue; }

                switch (sub_choice) {
                    case 1:
                        view_specific_deck_content(selected_deck);
                        get_string_input("Press Enter to continue...");
                        break;
                    case 2:
                        study_deck_menu(selected_deck); // Call the new study menu
                        break;
                    case 3:
                        add_card_to_specific_deck(selected_deck);
                        // No get_string_input here as add_card_to_specific_deck handles its own flow.
                        break;
                    case 4:
                        { // Scope for deleted_deck_idx
                            size_t deleted_deck_idx = static_cast<size_t>(choice - 1); // choice is from the outer scope
                            if (delete_specific_deck(deleted_deck_idx)) {
                                back_to_all_decks = true; // Exit sub-menu as deck is gone
                                // num_decks, and option numbers (make_new_option etc.) will be naturally updated
                                // at the start of the next iteration of the outer while(true) loop of show_flashcard_menu().
                            }
                            // If not deleted, just loop back in sub-menu
                        }
                        break;
                    case 5:
                        back_to_all_decks = true;
                        break;
                    default:
                        std::cout << "Invalid choice. Please enter a number between 1 and 5." << std::endl;
                }
            }
        } else if (choice == make_new_option) {
            create_deck(); // Part of study_hub.cpp
            // After creating a deck, num_decks might change, so update option numbers for next iteration
            num_decks = flashcard_decks.size();
            // view_deck_option_start = 1; // Stays 1
            view_deck_option_end = num_decks;
            make_new_option = num_decks + 1;
            add_card_option = num_decks + 2;
            delete_deck_option = num_decks + 3;
            back_to_hub_option = num_decks + 4;
        } else if (choice == add_card_option) {
            add_card_to_deck(); // Part of study_hub.cpp (general version)
        } else if (choice == delete_deck_option) {
            delete_deck(); // Part of study_hub.cpp (general version)
            // After deleting a deck, num_decks might change, so update option numbers
            num_decks = flashcard_decks.size();
            // view_deck_option_start = 1; // Stays 1
            view_deck_option_end = num_decks;
            make_new_option = num_decks + 1;
            add_card_option = num_decks + 2;
            delete_deck_option = num_decks + 3;
            back_to_hub_option = num_decks + 4;
        } else if (choice == back_to_hub_option) {
            return; // Back to studyHubMenu
        } else {
            // This case handles choices that are not a deck selection AND not one of the global options
            // (e.g. if num_decks is 0, and user enters 1 when make_new_option is 1)
            // Or if user enters a number outside the valid range of any option.
            std::cout << "Invalid choice. Please select an option from the menu." << std::endl;
        }
    }
}

// --- Notebook Functions ---
void create_new_note(const std::string& subject) {
    std::cout << "\n--- Create New Note for " << subject << " ---" << std::endl;
    Note new_note;
    new_note.timestamp = getCurrentTimestamp(); // From utils.h
    // clear_input_buffer(); // Handled by show_notebook_menu before calling

    new_note.topic_title = get_string_input("Enter topic title: ");
    if (new_note.topic_title.empty()){
        std::cout << "Topic title cannot be empty. Using default title 'Untitled Note'." << std::endl;
        new_note.topic_title = "Untitled Note";
    }

    std::cout << "Enter your notes (type SAVE_AND_EXIT on a new line to finish):" << std::endl;
    std::string line;
    new_note.content = "";
    while (std::getline(std::cin, line)) {
        if (line == "SAVE_AND_EXIT") break;
        new_note.content += line + "\n";
    }
    if (!new_note.content.empty() && new_note.content.back() == '\n') {
        new_note.content.pop_back();
    }


    Notebook* subject_notebook = nullptr;
    for (auto& nb : notebooks) { // notebooks is global in this file
        if (nb.subject == subject) {
            subject_notebook = &nb;
            break;
        }
    }

    if (!subject_notebook) { // Subject notebook doesn't exist, create it
        notebooks.emplace_back();
        subject_notebook = &notebooks.back();
        subject_notebook->subject = subject;
    }

    subject_notebook->notes.push_back(new_note);
    std::cout << "Note '" << new_note.topic_title << "' saved successfully!\n" << std::endl;
    save_notebooks_to_file(); // From file_handler.h
}

void show_notebook_menu() {
    std::string choice_str;
    int choice = 0;
    clear_input_buffer(); // From utils.h - clear after studyHubMenu choice

    while (true) {
        std::cout << "\nNotebook Subjects:" << std::endl;
        std::vector<std::string> subjects = get_scheduler_subjects();

        if (subjects.empty()) {
            std::cout << "No subjects found from scheduler." << std::endl;
        } else {
            for(size_t i = 0; i < subjects.size(); ++i) {
                std::cout << i + 1 << ". " << subjects[i] << std::endl;
            }
        }

        int add_new_subject_option = subjects.size() + 1;
        std::cout << add_new_subject_option << ". Create Notebook for New Subject" << std::endl;
        int back_option = subjects.size() + 2;
        std::cout << back_option << ". Back to Study Hub Menu" << std::endl;
        std::cout << "Enter your choice: ";
        std::getline(std::cin, choice_str);

        try {
            choice = std::stoi(choice_str);
        } catch (const std::invalid_argument&) { std::cout << "Invalid input. Please enter a number." << std::endl; continue; }
          catch (const std::out_of_range&) { std::cout << "Input out of range." << std::endl; continue; }

        std::string selected_subject_for_notes;
        bool proceed_to_note_management = false;

        if (choice > 0 && static_cast<size_t>(choice) <= subjects.size()) {
            selected_subject_for_notes = subjects[choice - 1];
            proceed_to_note_management = true;
        } else if (choice == add_new_subject_option) {
            std::string new_subject_name = get_string_input("Enter the name for the new subject: ");
            if (new_subject_name.empty()) {
                std::cout << "Subject name cannot be empty. Using 'General'." << std::endl;
                selected_subject_for_notes = "General";
            } else {
                selected_subject_for_notes = new_subject_name;
            }
            proceed_to_note_management = true;
        } else if (choice == back_option) {
            return; // Back to studyHubMenu
        } else {
            std::cout << "Invalid choice. Please select an option from the menu." << std::endl;
            // continue; // This continue is implicitly handled by proceed_to_note_management being false
        }

        if (proceed_to_note_management) {
            bool back_to_subject_menu = false;
            while (!back_to_subject_menu) {
                Notebook* current_notebook = nullptr;
                for (auto& nb : notebooks) {
                    if (nb.subject == selected_subject_for_notes) {
                        current_notebook = &nb;
                        break;
                    }
                }

                std::cout << "\n--- Notes for " << selected_subject_for_notes << " ---" << std::endl;
                if (current_notebook && !current_notebook->notes.empty()) {
                    for (size_t i = 0; i < current_notebook->notes.size(); ++i) {
                        const auto& note = current_notebook->notes[i];
                        std::cout << i + 1 << ". " << note.topic_title << " : [" << note.timestamp << "]" << std::endl;
                    }
                } else {
                    std::cout << "No notes found for " << selected_subject_for_notes << "." << std::endl;
                }

                std::cout << "\nOptions:" << std::endl;
                std::cout << "1. Create New Note" << std::endl;
                std::cout << "2. View Note Content" << std::endl;
                std::cout << "3. Back to Notebook Subjects" << std::endl;

                std::string note_action_str = get_string_input("Enter your choice (1-3): ");
                int note_action = 0;
                try {
                    note_action = std::stoi(note_action_str);
                } catch (const std::invalid_argument&) { std::cout << "Invalid input." << std::endl; continue; }
                  catch (const std::out_of_range&) { std::cout << "Input out of range." << std::endl; continue; }

                switch (note_action) {
                    case 1:
                        create_new_note(selected_subject_for_notes);
                        break;
                    case 2: {
                        if (current_notebook && !current_notebook->notes.empty()) {
                            std::string note_num_str = get_string_input("Enter the number of the note to view: ");
                            int note_num_choice = 0;
                            try {
                                note_num_choice = std::stoi(note_num_str);
                                if (note_num_choice >= 1 && static_cast<size_t>(note_num_choice) <= current_notebook->notes.size()) {
                                    const auto& selected_note_to_view = current_notebook->notes[note_num_choice - 1];
                                    std::cout << "\n--- Note: " << selected_note_to_view.topic_title << " ---" << std::endl;
                                    std::cout << "Timestamp: " << selected_note_to_view.timestamp << std::endl;
                                    std::cout << "Content:\n" << selected_note_to_view.content << std::endl;
                                    std::cout << "---------------------------------" << std::endl;
                                } else { std::cout << "Invalid note number." << std::endl; }
                            } catch (const std::invalid_argument&) { std::cout << "Invalid input for note number." << std::endl; }
                              catch (const std::out_of_range&) { std::cout << "Note number out of range." << std::endl; }
                        } else { std::cout << "No notes to view." << std::endl; }
                        get_string_input("Press Enter to continue...");
                        break;
                    }
                    case 3:
                        back_to_subject_menu = true;
                        break;
                    default:
                        std::cout << "Invalid choice. Please enter 1, 2, or 3." << std::endl;
                }
            }
        } else if (choice == back_option) {
            return; // Back to studyHubMenu
        } else {
            std::cout << "Invalid choice. Please select an option from the menu." << std::endl;
        }
    }
}

// --- Main Study Hub Menu ---
void studyHubMenu() {
    // Load data at the start of Study Hub. These functions are from file_handler.h
    load_flashcards_from_file();
    load_notebooks_from_file();

    std::cout << "Welcome to the ISKAALAMAN Study Hub!" << std::endl;
    std::string choice_str;
    int choice = 0;
    clear_input_buffer(); // From utils.h - after main menu choice

    while (true) {
        std::cout << "\nISKAALAMAN Study Hub Menu:" << std::endl;
        std::cout << "1. Notebook" << std::endl;
        std::cout << "2. Flashcards" << std::endl;
        std::cout << "3. Back to Main Menu" << std::endl;
        std::cout << "Enter your choice (1-3): ";
        std::getline(std::cin, choice_str); // Use getline for menu choice to be consistent

        try {
            choice = std::stoi(choice_str);
        } catch (const std::invalid_argument&) { std::cout << "Invalid input. Please enter a number." << std::endl; continue; }
          catch (const std::out_of_range&) { std::cout << "Input out of range." << std::endl; continue; }

        switch (choice) {
            case 1:
                show_notebook_menu(); // Part of study_hub.cpp
                break;
            case 2:
                show_flashcard_menu(); // Part of study_hub.cpp
                break;
            case 3:
                std::cout << "Returning to ISKAALAMAN Main Menu..." << std::endl;
                // Data is saved within specific functions (create_deck, add_card, create_note etc.)
                return; // Return to iskaalaman.cpp's main menu loop
            default:
                std::cout << "Invalid choice. Please enter a number between 1 and 3." << std::endl;
        }
    }
}
