#include "utils.h"
#include <map> // Ensure map is included for dayAbbreviations

// --- Calendar Implementation (subset) ---
std::string getCurrentDateYYYYMMDD() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm ltm;
#ifdef _WIN32
    #if defined(_MSC_VER)
        localtime_s(&ltm, &now_time_t);
    #else
        std::tm* p_ltm = std::localtime(&now_time_t);
        if (p_ltm) ltm = *p_ltm; else return "ERR_TIME";
    #endif
#else
    #if defined(__unix__) || defined(__APPLE__)
        localtime_r(&now_time_t, &ltm);
    #else
        std::tm* p_ltm = std::localtime(&now_time_t);
        if (p_ltm) ltm = *p_ltm; else return "ERR_TIME";
    #endif
#endif
    std::ostringstream oss;
    oss << std::put_time(&ltm, "%Y-%m-%d");
    return oss.str();
}

std::string getCurrentDayOfWeek() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm ltm;
#if defined(_WIN32) && !defined(__GNUC__) // MSVC or MinGW-w64 specific
    localtime_s(&ltm, &now_time_t);
#elif defined(__unix__) || defined(__APPLE__) // POSIX
    localtime_r(&now_time_t, &ltm);
#else // Fallback for other compilers, less safe
    std::tm* p_ltm = std::localtime(&now_time_t);
    if (p_ltm) ltm = *p_ltm; else return "ERR_DAY";
#endif
    const char* days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    if (ltm.tm_wday >= 0 && ltm.tm_wday <= 6) {
        return days[ltm.tm_wday];
    }
    return "ERR";
}

// --- Helper functions for time/day validation (originally for addClass) ---
bool isValidDay(const std::string& day_param) {
    std::string day = day_param; // Modifiable copy
    std::transform(day.begin(), day.end(), day.begin(), ::toupper);

    const static std::set<std::string> validDaysFullAndAbbrev = {
        "MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN",
        "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY", "SUNDAY",
        "M", "T", "W", "F" // TH is distinct
    };
    if (day == "TH") return true; // Special case for TH vs T
    return validDaysFullAndAbbrev.count(day);
}

bool parseDaysOfWeek(const std::string& daysInput, std::vector<std::string>& daysOfWeek) {
    daysOfWeek.clear();
    std::stringstream ss(daysInput);
    std::string day_token;
    bool allValid = true;
    std::set<std::string> unique_days_set;

    const static std::map<std::string, std::string> dayStandardizationMap = {
        {"M", "Mon"}, {"MON", "Mon"}, {"MONDAY", "Mon"},
        {"T", "Tue"}, {"TUE", "Tue"}, {"TUESDAY", "Tue"},
        {"W", "Wed"}, {"WED", "Wed"}, {"WEDNESDAY", "Wed"},
        {"TH", "Thu"}, {"THU", "Thu"}, {"THURSDAY", "Thu"},
        {"F", "Fri"}, {"FRI", "Fri"}, {"FRIDAY", "Fri"},
        {"SAT", "Sat"}, {"SATURDAY", "Sat"},
        {"SUN", "Sun"}, {"SUNDAY", "Sun"}
    };

    while (std::getline(ss, day_token, ',')) {
        day_token.erase(0, day_token.find_first_not_of(" \t\n\r\f\v"));
        day_token.erase(day_token.find_last_not_of(" \t\n\r\f\v") + 1);
        std::string upper_day_token = day_token;
        std::transform(upper_day_token.begin(), upper_day_token.end(), upper_day_token.begin(), ::toupper);

        auto it = dayStandardizationMap.find(upper_day_token);
        if (it != dayStandardizationMap.end()) {
            unique_days_set.insert(it->second);
        } else {
            // Check if the original token (non-uppercased, but trimmed) is a standard short form
            // This handles cases like "Mon" directly if not caught by the uppercase map logic
            bool foundDirectShort = false;
            for(const auto& pair : dayStandardizationMap){
                if(pair.second == day_token) { // e.g. day_token is "Mon"
                    unique_days_set.insert(day_token);
                    foundDirectShort = true;
                    break;
                }
            }
            if (!foundDirectShort) {
                 allValid = false;
            }
        }
    }

    if (unique_days_set.empty() && !daysInput.empty() && !allValid) {
         daysOfWeek.clear(); // Ensure daysOfWeek is empty if errors occurred and nothing valid was parsed
         return false;
    }
    if (unique_days_set.empty() && daysInput.empty()){
        // This case means the input string was empty.
        // Depending on requirements, this could be valid (e.g., editing a class to have no scheduled days)
        // or invalid (e.g., adding a new class requires days).
        // For now, assume empty input is valid and results in an empty daysOfWeek vector.
        // std::cout << "<Days of week cannot be empty if input is provided but not parsed.>" << std::endl;
        // return false; // Or true, depending on desired behavior for empty input string.
    }

    if (!allValid) { // If any day was invalid, clear and return false
        daysOfWeek.clear();
        return false;
    }

    // If allValid is true (or no input was given leading to an empty set but no errors)
    daysOfWeek.assign(unique_days_set.begin(), unique_days_set.end());
    return true;
}

bool isValidTimeFormat(const std::string& timeStr) {
    const std::regex timeRegex(R"(^(0[1-9]|1[0-2]):([0-5][0-9])\s+(AM|PM|am|pm)$)");
    if (!std::regex_match(timeStr, timeRegex)) {
        // Message should be handled by the caller for UI consistency
        // std::cout << "<Invalid time format. Please use HH:MM AM/PM (e.g., 09:30 AM).>" << std::endl;
        return false;
    }
    return true;
}

int timeToMinutes(const std::string& timeStr) {
    if (!isValidTimeFormat(timeStr)) { // relies on isValidTimeFormat NOT printing errors
        return -1;
    }

    // std::tm t{}; // Removed unused variable
    std::istringstream ss(timeStr);
    // std::get_time can be tricky with AM/PM depending on locale.
    // Manual parsing after regex validation is often more robust.

    int hours = std::stoi(timeStr.substr(0, 2));
    int minutes = std::stoi(timeStr.substr(3, 2));
    std::string ampm_str = timeStr.substr(6, 2); // Adjusted index if space is at index 5
    // Robustly find AM/PM part:
    size_t first_space = timeStr.find(' ');
    if(first_space != std::string::npos){
        ampm_str = timeStr.substr(first_space + 1);
    } else {
        return -1; // Should be caught by regex, but good to be safe
    }

    std::transform(ampm_str.begin(), ampm_str.end(), ampm_str.begin(), ::toupper);

    if (ampm_str == "AM") {
        if (hours == 12) { // Midnight case: 12 AM is 00:xx hours.
            hours = 0;
        }
    } else if (ampm_str == "PM") {
        if (hours != 12) { // 12 PM is Noon, no change. Others add 12.
            hours += 12;
        }
    } else {
        return -1; // Invalid AM/PM string, though regex should catch.
    }
    return hours * 60 + minutes;
}

// --- General Input Helper Functions ---
void clear_input_buffer() {
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    std::tm ltm;
#ifdef _WIN32
    #if defined(_MSC_VER)
        localtime_s(&ltm, &in_time_t);
    #else
        std::tm* p_ltm = std::localtime(&in_time_t);
        if (p_ltm) ltm = *p_ltm; else return "ERR_TS";
    #endif
#else
    #if defined(__unix__) || defined(__APPLE__)
        localtime_r(&in_time_t, &ltm);
    #else
        std::tm* p_ltm = std::localtime(&in_time_t);
        if (p_ltm) ltm = *p_ltm; else return "ERR_TS";
    #endif
#endif
    ss << std::put_time(&ltm, "%X %Y-%m-%d"); // %X is locale's time representation
    return ss.str();
}

std::string get_string_input(const std::string& prompt) {
    std::string input;
    std::cout << prompt;
    std::getline(std::cin, input); // Assumes cin buffer is clean or handled before this call
    return input;
}
