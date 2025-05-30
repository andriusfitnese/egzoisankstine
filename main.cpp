#include <iostream>
#include <fstream>
#include <unordered_map>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <regex>
#include <algorithm>
#include <windows.h>

using namespace std;

const string allowedLetters =
"abcdefghijklmnopqrstuvwxyz"
"ąčęėįšųūž";

string cleanWord(const string& word) {
    string result;
    bool hasLetter = false;

    for (size_t i = 0; i < word.size(); ++i) {
        unsigned char c = word[i];

        if ((c & 0x80) == 0) {
            char lower = tolower(c);
            if (allowedLetters.find(lower) != string::npos) {
                result += lower;
                hasLetter = true;
            }
        }
        else {
            size_t len = 1;
            if ((c & 0xE0) == 0xC0) len = 2;
            else if ((c & 0xF0) == 0xE0) len = 3;
            else if ((c & 0xF8) == 0xF0) len = 4;

            if (i + len <= word.size()) {
                string utfChar = word.substr(i, len);
                if (allowedLetters.find(utfChar) != string::npos) {
                    result += utfChar;
                    hasLetter = true;
                }
                i += len - 1;
            }
        }
    }

    return hasLetter ? result : "";
}

int main() {
    SetConsoleOutputCP(CP_UTF8);

    ifstream input("tekstas.txt");
    ifstream tldFile("links.txt");
    ofstream output_count("output_count.txt");
    ofstream output_cross("cross-reference.txt");
    ofstream output_url("output_urls.txt");

    if (!input || !tldFile) {
        cerr << "Nepavyko atidaryti tekstas.txt arba links.txt" << endl;
        return 1;
    }

    // skaitom teisingus TLDs (aprasytus links.txt)
    set<string> validTLDs;
    string tld;
    while (getline(tldFile, tld)) {
        transform(tld.begin(), tld.end(), tld.begin(), ::tolower);
        validTLDs.insert(tld);
    }

    unordered_map<string, int> wordCount;
    unordered_map<string, set<int>> wordLines;
    set<string> foundUrls;

    regex urlRegex(R"((https?:\/\/)?(www\.)?[\w\-]+\.[\w\.\-]+)", regex::icase);

    string line;
    int line_number = 0;

    while (getline(input, line)) {
        line_number++;
        vector<pair<size_t, size_t>> urlRanges;

        // random ir laikom url + traukiam domenus
        for (sregex_iterator it(line.begin(), line.end(), urlRegex), end; it != end; ++it) {
            size_t start = it->position();
            size_t len = it->length();
            urlRanges.emplace_back(start, start + len);

            string url = it->str();
            size_t protocolEnd = url.find("://");
            size_t domainStart = (protocolEnd != string::npos) ? protocolEnd + 3 : 0;
            size_t slashPos = url.find('/', domainStart);
            string domain = (slashPos != string::npos) ? url.substr(domainStart, slashPos - domainStart) : url.substr(domainStart);

            size_t lastDot = domain.rfind('.');
            if (lastDot != string::npos) {
                string tldPart = domain.substr(lastDot + 1);
                transform(tldPart.begin(), tldPart.end(), tldPart.begin(), ::tolower);
                if (validTLDs.count(tldPart)) {
                    foundUrls.insert(url);
                }
            }
        }

        // istraukiam zodzius ir skaiciuojam
        stringstream ss(line);
        string word;
        size_t currentPos = 0;

        while (ss >> word) {
            size_t pos = line.find(word, currentPos);
            currentPos = pos + word.length();

            bool inURL = false;
            for (const auto& [start, end] : urlRanges) {
                if (pos < end && (pos + word.length()) > start) {
                    inURL = true;
                    break;
                }
            }
            if (inURL) continue;

            //aptvarkom ir skaiciuojam
            word.erase(remove_if(word.begin(), word.end(), ::ispunct), word.end());
            string cleaned = cleanWord(word);
            if (!cleaned.empty() && cleaned.length() > 1) {
                wordCount[cleaned]++;
                wordLines[cleaned].insert(line_number);
            }
        }
    }

    // spausdinam zodziu kieki (jei count > 1)
    for (const auto& [word, count] : wordCount) {
        if (count > 1) {
            output_count << word << " : " << count << '\n';
        }
    }

    // spausdinam cross-reference
    for (const auto& [word, lines] : wordLines) {
        if (wordCount[word] > 1) {
            output_cross << word << " : ";
            for (const int line : lines) {
                output_cross << line << " ";
            }
            output_cross << '\n';
        }
    }

    // spausdinam rastus URL
    output_url << "Rasti URL adresai\n" << string(50, '-') << '\n';
    for (const string& url : foundUrls) {
        output_url << url << '\n';
    }

    cout << "Baigta!" << endl;
    cout << "Iseiti is programos paspauskite bet kuri klavisa...\n";
	cin.get();
    return 0;
}
