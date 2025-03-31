#ifndef MAIN_H
#define MAIN_H

#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <sstream>

struct metafile {
    int count;
    char name[20];
};

struct mfile {
    metafile meta;
    std::string data;
};

struct initfs {
    int offset;
    int count;
    int fileEndOffset;
    int currentDirOffset;
};

struct pair {
    char name[20];
    int offset;
    bool isDirectory;
    int OffsetP0;
};

class FileSystem {
private:
    std::string fsFileName;
    initfs fs;
    std::string errorLogFile = "fs_errors.log";

    void logError(const std::string& error);
    void readInitfs();
    void writeInitfs();
    bool fileExists(const std::string& filename);
    bool dirExists(const std::string& dirname);
    std::string getCurrentTime();

public:
    FileSystem() : fsFileName("mydump") {
        std::ifstream test(fsFileName);
        if (!test.good()) {
            init();
        }
        test.close();
        readInitfs();
    }

    void init();
    void writeFile(const std::string& filename);
    void readFile(const std::string& filename);
    void createDirectory(const std::string& dirname);
    void changeDirectory(const std::string& dirname);
    void listDirectory();
    std::string getCurrentPath();
    void showErrorLog();
};

#endif