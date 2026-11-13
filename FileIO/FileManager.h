

#pragma once


#include <string>
#include <vector>


class FileManager {
private:
    // Singleton
    FileManager();
    FileManager(const FileManager&) = delete; //ko cho phep sao chep doi tuong
    FileManager& operator=(const FileManager&) = delete;


public:
    static FileManager& getInstance();


    // Doc noi dung file
    std::vector<std::string> readFile(const std::string& fileName) const;


    // Ghi de noi dung file
    void writeFile(
        const std::string& fileName,
        const std::vector<std::string>& lines
    ) const;


    // Them mot dong vao cuoi file
    void appendLine(
        const std::string& fileName,
        const std::string& line
    ) const;


    // Kiem tra file co ton tai hay khong
    bool fileExists(const std::string& fileName) const;
};



