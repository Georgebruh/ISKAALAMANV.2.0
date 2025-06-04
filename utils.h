#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <limits>
#include <algorithm>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>
#include <regex>
#include <set>
#include <iostream> // For std::cout, std::cin, std::cerr
#include <map>     // For isValidDay

// Function Declarations
std::string getCurrentDateYYYYMMDD();
std::string getCurrentDayOfWeek();
// std::string urgencyToString(int urgency); // Declaration will be in scheduler_planner.h
bool isValidDay(const std::string& day);
bool parseDaysOfWeek(const std::string& daysInput, std::vector<std::string>& daysOfWeek); // Uses isValidDay, std::cout
bool isValidTimeFormat(const std::string& timeStr); // Uses std::cout
int timeToMinutes(const std::string& timeStr);      // Uses isValidTimeFormat
void clear_input_buffer();                          // Uses std::cin
std::string getCurrentTimestamp();                  // For Flashcards and Notes
std::string get_string_input(const std::string& prompt); // Uses std::cout, std::cin

#endif // UTILS_H
