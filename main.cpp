#include "main.h"

void init() {
    std::fstream f("mydump", std::ios::binary | std::ios::out | std::ios::trunc);
    if (!f.is_open()) return;

    initfs init;
    init.offset = 100;
    init.count = 0;
    init.fileEndOffset = 1000;
    init.currentDirOffset = 0;

    f.write((char*)(&init), sizeof(init));
    f.close();
}

initfs getInitfs() {
    initfs result;
    std::fstream f("mydump", std::ios::binary | std::ios::in);
    f.read((char*)(&result), sizeof(result));
    f.close();
    return result;
}

void setInitfs(initfs fs) {
    std::fstream f("mydump", std::ios::binary | std::ios::in | std::ios::out);
    if (!f.is_open()) return;
    f.write((char*)(&fs), sizeof(fs));
    f.close();
}

void writeFile(const std::string filename) {
    mfile my_file;
    memcpy(my_file.meta.name, filename.data(), filename.size());
    memcpy(my_file.meta.name + filename.size(), "\0", 1);

    std::ifstream ifs(filename, std::ios::binary);
    my_file.meta.count = 0;
    char bufferChar;
    while (ifs.read(&bufferChar, 1)) {
        my_file.meta.count++;
        my_file.data += bufferChar;
    }
    ifs.close();

    initfs fs = getInitfs();
    std::fstream f("mydump", std::ios::binary | std::ios::in | std::ios::out);

    pair* arr = new pair[fs.count];
    f.seekg(fs.offset, std::ios::beg);
    f.read((char*)arr, sizeof(pair) * fs.count);
    for (int i = 0; i < fs.count; i++) {
        if (!arr[i].isDirectory && strcmp(arr[i].name, filename.c_str()) == 0 && 
            arr[i].OffsetP0 == fs.currentDirOffset) {
            std::cout << "File already exists\n";
            delete[] arr;
            f.close();
            return;
        }
    }
    delete[] arr;

    int localOffset = fs.offset + fs.count * sizeof(pair);
    int localFileEndOffset = fs.fileEndOffset;

    fs.count++;
    fs.fileEndOffset += sizeof(my_file.meta) + my_file.meta.count;
    setInitfs(fs);

    pair p;
    memcpy(p.name, my_file.meta.name, sizeof(my_file.meta.name));
    p.offset = localFileEndOffset;
    p.isDirectory = false;
    p.OffsetP0 = fs.currentDirOffset;
    
    f.seekg(localOffset, std::ios::beg);
    f.write((char*)(&p), sizeof(p));

    f.seekg(localFileEndOffset, std::ios::beg);
    f.write((char*)(&my_file.meta), sizeof(my_file.meta));
    f.write(my_file.data.data(), my_file.data.size());
    f.close();
}

void readFile(const std::string filename) {
    initfs fs = getInitfs();
    pair* p = nullptr;

    std::fstream f("mydump", std::ios::binary | std::ios::in);
    pair* arr = new pair[fs.count];
    f.seekg(fs.offset, std::ios::beg);
    f.read((char*)arr, sizeof(pair) * fs.count);
    
    for (int i = 0; i < fs.count; i++) {
        if (!arr[i].isDirectory && strcmp(arr[i].name, filename.c_str()) == 0 &&
            arr[i].OffsetP0 == fs.currentDirOffset) {
            p = arr + i;
            break;
        }
    }

    if (p == nullptr) {
        std::cout << "File not found\n";
        delete[] arr;
        return;
    }

    f.seekg(p->offset, std::ios::beg);
    delete[] arr;

    mfile my_file;
    f.read((char*)(&my_file.meta), sizeof(my_file.meta));
    my_file.data.resize(my_file.meta.count);
    f.read(my_file.data.data(), my_file.meta.count);

    std::string new_filename = "tmp_" + std::string(my_file.meta.name);
    std::ofstream outfile(new_filename);
    outfile << my_file.data;
    outfile.close();
    f.close();
}

void createDirectory(const std::string dirname) {
    initfs fs = getInitfs();
    
    if (dirname.size() >= 20) {
        std::cout << "Directory name too long\n";
        return;
    }

    std::fstream f("mydump", std::ios::binary | std::ios::in | std::ios::out);
    pair* arr = new pair[fs.count];
    f.seekg(fs.offset, std::ios::beg);
    f.read((char*)arr, sizeof(pair) * fs.count);
    
    for (int i = 0; i < fs.count; i++) {
        if (arr[i].isDirectory && strcmp(arr[i].name, dirname.c_str()) == 0 &&
            arr[i].OffsetP0 == fs.currentDirOffset) {
            std::cout << "Directory already exists\n";
            delete[] arr;
            f.close();
            return;
        }
    }
    delete[] arr;

    int localOffset = fs.offset + fs.count * sizeof(pair);
    fs.count++;
    setInitfs(fs);

    pair p;
    memcpy(p.name, dirname.c_str(), dirname.size() + 1);
    p.offset = localOffset;
    p.isDirectory = true;
    p.OffsetP0 = fs.currentDirOffset;
    
    f.seekg(localOffset, std::ios::beg);
    f.write((char*)(&p), sizeof(p));
    f.close();
}

void changeDirectory(const std::string dirname) {
    initfs fs = getInitfs();
    
    if (dirname == "..") {
        if (fs.currentDirOffset != 0) {
            pair* arr = new pair[fs.count];
            std::fstream f("mydump", std::ios::binary | std::ios::in);
            f.seekg(fs.offset, std::ios::beg);
            f.read((char*)arr, sizeof(pair) * fs.count);
            
            for (int i = 0; i < fs.count; i++) {
                if (arr[i].offset == fs.currentDirOffset) {
                    fs.currentDirOffset = arr[i].OffsetP0;
                    setInitfs(fs);
                    break;
                }
            }
            delete[] arr;
            f.close();
        }
        return;
    }

    pair* arr = new pair[fs.count];
    std::fstream f("mydump", std::ios::binary | std::ios::in);
    f.seekg(fs.offset, std::ios::beg);
    f.read((char*)arr, sizeof(pair) * fs.count);
    
    for (int i = 0; i < fs.count; i++) {
        if (arr[i].isDirectory && strcmp(arr[i].name, dirname.c_str()) == 0 &&
            arr[i].OffsetP0 == fs.currentDirOffset) {
            fs.currentDirOffset = arr[i].offset;
            setInitfs(fs);
            delete[] arr;
            f.close();
            return;
        }
    }
    
    std::cout << "Directory not found\n";
    delete[] arr;
    f.close();
}

void listDirectory() {
    initfs fs = getInitfs();
    pair* arr = new pair[fs.count];
    
    std::fstream f("mydump", std::ios::binary | std::ios::in);
    f.seekg(fs.offset, std::ios::beg);
    f.read((char*)arr, sizeof(pair) * fs.count);
    
    std::cout << "Contents:\n";
    for (int i = 0; i < fs.count; i++) {
        if (arr[i].OffsetP0 == fs.currentDirOffset) {
            if (arr[i].isDirectory) {
                std::cout << "[DIR] " << arr[i].name << "\n";
            } else {
                std::cout << "[FILE] " << arr[i].name << "\n";
            }
        }
    }
    
    delete[] arr;
    f.close();
}

std::string getCurrentPath() {
    initfs fs = getInitfs();
    if (fs.currentDirOffset == 0) return "/";
    
    std::vector<std::string> path;
    int current = fs.currentDirOffset;
    
    std::fstream f("mydump", std::ios::binary | std::ios::in);
    pair* arr = new pair[fs.count];
    f.seekg(fs.offset, std::ios::beg);
    f.read((char*)arr, sizeof(pair) * fs.count);
    
    while (current != 0) {
        for (int i = 0; i < fs.count; i++) {
            if (arr[i].offset == current && arr[i].isDirectory) {
                path.push_back(arr[i].name);
                current = arr[i].OffsetP0;
                break;
            }
        }
    }
    
    delete[] arr;
    f.close();
    
    std::reverse(path.begin(), path.end());
    std::string result = "/";
    for (const auto& dir : path) {
        result.append(dir).append("/");
    }
    return result;
}

int main(int argc, char** argv) {
    std::string command, filename;

    while (true) {
        if (argc == 3) {
            command = argv[1];
            filename = argv[2];
            argc = 1;
        } else {
            std::cout << getCurrentPath() << "> ";
            if (!std::getline(std::cin, command)) {
                if (std::cin.eof()) break;
                continue;
            }
            
            if (command.empty()) continue;

            size_t pos = command.find(' ');
            if (pos != std::string::npos) {
                filename = command.substr(pos + 1);
                command = command.substr(0, pos);
            } else {
                filename.clear();
            }
        }

        if (command == "i") {
            init();
        }
        else if (command == "r") {
            if (!filename.empty()) {
                readFile(filename);
            } else {
                std::cout << "Filename required\n";
            }
        }
        else if (command == "w") {
            if (!filename.empty()) {
                writeFile(filename);
            } else {
                std::cout << "Filename required\n";
            }
        }
        else if (command == "mkdir") {
            if (!filename.empty()) {
                createDirectory(filename);
            } else {
                std::cout << "Directory name required\n";
            }
        }
        else if (command == "cd") {
            if (!filename.empty()) {
                changeDirectory(filename);
            } else {
                std::cout << "Directory name required\n";
            }
        }
        else if (command == "ls") {
            listDirectory();
        }
        else {
            std::cout << "Available commands:\n"
                      << "i - initialize filesystem\n"
                      << "r <file> - read file\n"
                      << "w <file> - write file\n"
                      << "mkdir <dir> - create directory\n"
                      << "cd <dir> - change directory\n"
                      << "ls - list contents\n";
        }
    }
    return 0;
}