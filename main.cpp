#include "lib.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <algorithm> // For std::transform
#include <set>       // For unique line numbers and unique URLs
#include <locale>    // For std::locale, std::tolower (locale-aware)
#include <regex>     // For std::regex, std::sregex_iterator, std::smatch
#include <utility>   // For std::pair (used with vector of pairs)
#include <cctype>    // For std::ispunct (though we'll extend its usage)
#include <cwctype>   // For wide character checks if needed, but not directly used here for char

// For _getch() (Windows specific)
#ifdef _WIN32
#include <conio.h>
#endif


// --- Helper Functions for Word Processing (NEW/MODIFIED) ---

// This function checks if a character is considered part of a valid word.
// It's a pragmatic approach for UTF-8 std::string where full Unicode char-by-char
// processing with std::iswalpha is not trivial.
// It assumes the current locale might influence isalpha/isdigit, but prioritizes ASCII and
// common letter ranges if it can't rely on full locale support for complex UTF-8.
// The task emphasizes "letters".
bool is_valid_word_char(char c, const std::locale& loc) {
    // Check for ASCII letters and digits directly using std::isalnum
    if (std::isalnum(c, loc)) {
        return true;
    }
    // Handle specific common internal characters like hyphen or apostrophe if they
    // should be part of a word (e.g., "co-operate", "don't").
    // Based on the task, it seems like these should also be stripped if at ends.
    // So, we primarily want letters and digits inside a word.
    // For now, let's keep it simple: only alphanumeric characters are "word" chars.
    return false;
}

// Checks if a string contains at least one alphabetic character.
// This helps filter out pure numbers or strings of only punctuation.
bool contains_alphabetic_char(const std::string& s, const std::locale& loc) {
    for (char c : s) {
        if (std::isalpha(c, loc)) { // Use std::isalpha from <cctype> with locale
            return true;
        }
    }
    return false;
}

// --- Main Program ---
int main()
{
    SetConsoleOutputCP(CP_UTF8);

    std::unordered_map<std::string, std::vector<int>> tekstas;
    std::set<std::string> found_urls;

    std::cout << "Iveskite failo pavadinima: ";
    std::string failas;
    std::cin >> failas;
    std::ifstream fd(failas);
    while (!fd.is_open())
    {
        std::cout << "Failas nerastas, iveskite dar karta: ";
        std::cin >> failas;
        fd.open(failas);
    }

    std::locale current_locale(""); // Use default locale

    // Regex for URL detection (unchanged, robust)
    std::regex url_regex(
        R"((?:https?:\/\/)?(?:www\.)?[-a-zA-Z0-9@:%._+~#=]{1,256}\.[a-zA-Z0-9()]{1,6}\b(?:[-a-zA-Z0-9()@:%_+.~#?&/=]*))",
        std::regex::icase
    );

    // --- Processing Loop ---
    std::string line;
    int line_number = 0;
    while (std::getline(fd, line))
    {
        ++line_number;

        std::string line_for_words = line; // Copy of original line for word processing

        // --- URL Extraction & Removal ---
        std::sregex_iterator currentMatch(line.begin(), line.end(), url_regex);
        std::sregex_iterator lastMatch;
        std::vector<std::pair<size_t, size_t>> replacement_ranges;

        while (currentMatch != lastMatch) {
            std::smatch match = *currentMatch;
            std::string url_str = match.str();
            if (!url_str.empty()) {
                found_urls.insert(url_str);
                replacement_ranges.push_back({ match.position(), match.length() });
            }
            currentMatch++;
        }

        // Replace URLs with spaces in the copy to prevent them from being tokenized as words
        for (auto it = replacement_ranges.rbegin(); it != replacement_ranges.rend(); ++it) {
            line_for_words.replace(it->first, it->second, " ");
        }

        // --- Word Processing (MODIFIED) ---
        std::istringstream iss(line_for_words);
        std::string x;
        while (iss >> x)
        {
            // --- Punctuation Removal (More robust) ---
            // Remove leading punctuation until a valid word character is found
            while (!x.empty() && !is_valid_word_char(x.front(), current_locale)) {
                x.erase(x.begin());
            }
            // Remove trailing punctuation until a valid word character is found
            while (!x.empty() && !is_valid_word_char(x.back(), current_locale)) {
                x.pop_back();
            }

            // --- Lowercasing ---
            std::transform(x.begin(), x.end(), x.begin(), [&](char c) {
                return std::tolower(c, current_locale);
                });

            // --- Final Word Validation (NEW) ---
            // Only add to map if the string is not empty AND contains at least one alphabetic character.
            // This filters out pure numbers, or strings of only punctuation that might have survived.
            if (!x.empty() && contains_alphabetic_char(x, current_locale))
            {
                tekstas[x].push_back(line_number);
            }
        }
    }
    fd.close();

    // --- Output Word Count and Cross-reference (Unchanged) ---
    std::ofstream fr("rezultatai.txt");
    if (!fr.is_open()) {
        std::cerr << "Nepavyko atidaryti rezultatai.txt failo rasymui!\n";
        return 1;
    }
    for (const auto& p : tekstas)
    {
        if (p.second.size() > 1)
        {
            fr << p.first << " (" << p.second.size() << " kartus) eilutese: ";
            std::set<int> unique_lines(p.second.begin(), p.second.end());
            size_t count = 0;
            for (int line_num : unique_lines)
            {
                fr << line_num;
                if (++count < unique_lines.size())
                    fr << ", ";
            }
            fr << "\n";
        }
    }
    fr.close();

    // --- Output URLs (Unchanged) ---
    std::ofstream fr2("url.txt");
    if (!fr2.is_open()) {
        std::cerr << "Nepavyko atidaryti url.txt failo rasymui!\n";
        return 1;
    }

    if (found_urls.empty())
    {
        fr2 << "Nerasta nei viena nuoroda\n";
    }
    else
    {
        for (const auto& url : found_urls)
        {
            fr2 << url << "\n";
        }
    }
    fr2.close();

    std::cout << "Rezultatai issaugoti 'rezultatai.txt' ir 'url.txt' failuose.\n";
    std::cout << "Iseiti is programos paspauskite bet kuri klavisa...\n";
    _getch();

    return 0;
}