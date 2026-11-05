#include "FileManager.h"

#include <fstream>
#include <stdexcept>

using namespace std;

// ==================== CONSTRUCTOR ====================

FileManager::FileManager() {
}

// ==================== SINGLETON ====================

FileManager& FileManager::getInstance() {
    static FileManager instance;
    return instance;
}
// ==================== READ FILE ====================

vector<string> FileManager::readFile(const string& fileName) const {
    ifstream file(fileName);

    if (!file.is_open()) {
        throw runtime_error("Cannot open file for reading: " + fileName);
    }

    vector<string> lines;
    string line;

    while (getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        lines.push_back(line);
    }

    if (file.bad()) {
        throw runtime_error("Error while reading file: " + fileName);
    }

    return lines;
}
// ==================== WRITE FILE ====================

void FileManager::writeFile(
    const string& fileName,
    const vector<string>& lines
) const {
    ofstream file(fileName, ios::out | ios::trunc);

    if (!file.is_open()) {
        throw runtime_error("Cannot open file for writing: " + fileName);
    }

    for (const string& line : lines) {
        file << line << '\n';

        if (!file.good()) {
            throw runtime_error("Error while writing file: " + fileName);
        }
    }

    file.close();

    if (file.fail()) {
        throw runtime_error("Error while closing file: " + fileName);
    }
}
// ==================== APPEND LINE ====================

void FileManager::appendLine(
    const string& fileName,
    const string& line
) const {
    ofstream file(fileName, ios::out | ios::app);

    if (!file.is_open()) {
        throw runtime_error("Cannot open file for appending: " + fileName);
    }

    file << line << '\n';

    if (!file.good()) {
        throw runtime_error("Error while appending to file: " + fileName);
    }

    file.close();

    if (file.fail()) {
        throw runtime_error("Error while closing file: " + fileName);
    }
}
// ==================== CHECK FILE ====================

bool FileManager::fileExists(const string& fileName) const {
    ifstream file(fileName);
    return file.good();
}
