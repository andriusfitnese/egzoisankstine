#include "lib.h"

// Function to clean and normalize a word
std::string cleanWord(const std::string& word) {
    std::string cleaned;
    for (unsigned char ch : word) {
        if (std::isalnum(ch)) {
            cleaned += std::tolower(ch);
        }
    }
    return cleaned;
}

// Function to extract URLs using regex
std::unordered_set<std::string> extractURLs(const std::string& text) {
    std::unordered_set<std::string> urls;
    std::regex urlRegex(R"((https?:\/\/[^\s]+))");
    auto begin = std::sregex_iterator(text.begin(), text.end(), urlRegex);
    auto end = std::sregex_iterator();

    for (auto it = begin; it != end; ++it) {
        urls.insert(it->str());
    }
    return urls;
}

int main() {
    SetConsoleOutputCP(CP_UTF8); // Make console print UTF-8 characters correctly

    std::ifstream inFile("input.txt");
    inFile.seekg(0, std::ios::end);
    size_t size = inFile.tellg();
    inFile.seekg(0);
    std::string text(size, ' ');
    inFile.read(&text[0], size);

    std::unordered_map<std::string, int> wordCount;
    std::istringstream iss(text);
    std::string word;

    while (iss >> word) {
        std::string cleaned = cleanWord(word);
        if (!cleaned.empty()) {
            wordCount[cleaned]++;
        }
    }

    // Output word counts
    std::ofstream outWords("output_word_count.txt");
    for (const auto& [word, count] : wordCount) {
        outWords << word << ": " << count << "\n";
    }

    // Output URLs
    std::ofstream outUrls("output_urls.txt");
    std::unordered_set<std::string> urls = extractURLs(text);
    for (const std::string& url : urls) {
        outUrls << url << "\n";
    }

    std::cout << "?? Done! Check 'output_word_count.txt' and 'output_urls.txt'\n";
    return 0;
}
