#include "main.h"

void init() {
    std::fstream f("mydump");

    if (!f.is_open()) {
        return;
    }

    f.seekg(0, std::ios::beg);
    initfs init;
    init.offset = 100;
    init.count = 0;
    init.fileEndOffset = 1000;

    f.write((char*)(&(init)), sizeof(init));
    f.close();
}

initfs getInitfs() {
    initfs result;
    std::fstream f("mydump");

    f.seekg(0, std::ios::beg);
    f.read((char*)(&(result)), sizeof(result));
    f.close();
    return result;
}

void setInitfs(initfs fs) {
    std::fstream f("mydump");

    if (!f.is_open()) {
        return;
    }
    f.seekg(0, std::ios::beg);
    f.write((char*)(&(fs)), sizeof(fs));
    f.close();
}

void readFile(const std::string filename) {
    initfs fs = getInitfs();

    pair* p = nullptr;

    std::fstream f("mydump");
    f.seekg(fs.offset, std::ios::beg);
    pair* arr = new pair[fs.count];
    f.read((char*)arr, sizeof(pair) * fs.count);
    for (int i = 0; i < fs.count; ++i) {
        if (!arr[i].isDirectory && strcmp(arr[i].name, filename.c_str()) == 0) {
            p = arr + i;
            break;
        }
    }

    if (p == nullptr) {
        std::cout << "File not exist\n" << filename << "\n";
        delete[] arr;
        return;
    }

    f.seekg(p->offset, std::ios::beg);
    delete[] arr;

    mfile my_file;
    f.read((char*)(&(my_file.meta)), sizeof(my_file.meta));
    my_file.data.resize(my_file.meta.count);
    f.read(my_file.data.data(), my_file.meta.count);

    std::string new_filename = std::string("tmp_") + my_file.meta.name;
    std::ofstream outfile(new_filename);
    outfile << my_file.data;
    outfile.close();
    std::cout << "End write to " << filename << "\n";
    f.close();
}

void writeFile(const std::string filename) {
    mfile my_file;
    memcpy(my_file.meta.name, filename.data(), filename.size());
    memcpy(my_file.meta.name + filename.size(), "\0", 1);

    std::string line;
    std::ifstream ifs(filename, std::ios_base::binary);
    my_file.meta.count = 0;
    char bufferChar;
    while (ifs.read(&bufferChar, 1)) {
        my_file.meta.count++;
        my_file.data += bufferChar;
    }
    ifs.close();

    initfs fs = getInitfs();
    int localOffset = fs.offset + fs.count * sizeof(pair);
    int localFileEndOffset = fs.fileEndOffset;

    fs.count++;
    fs.fileEndOffset += sizeof(my_file.meta) + my_file.meta.count;
    setInitfs(fs);

    std::fstream f("mydump");
    if (!f.is_open()) {
        return;
    }

    pair p;
    memcpy(p.name, my_file.meta.name, sizeof(my_file.meta.name));
    p.offset = localFileEndOffset;
    p.isDirectory = false;
    f.seekg(localOffset, std::ios::beg);
    f.write((char*)(&(p)), sizeof(p));

    f.seekg(localFileEndOffset, std::ios::beg);
    f.write((char*)(&(my_file.meta)), sizeof(my_file.meta));
    f.write(my_file.data.data(), my_file.data.size());

    f.close();
}


void createDirectory(const std::string dirname) {
    initfs fs = getInitfs();
    
    if (dirname.size() >= 20) {
        std::cout << "Directory name max 19 char\n";
        return;
    }

    pair* arr = new pair[fs.count];
    std::fstream f("mydump");
    f.seekg(fs.offset, std::ios::beg);
    f.read((char*)arr, sizeof(pair) * fs.count);
    
    for (int i = 0; i < fs.count; ++i) {
        if (arr[i].isDirectory && strcmp(arr[i].name, dirname.c_str()) == 0) {
            std::cout << "Directory already exists\n";
            delete[] arr;
            return;
        }
    }
    delete[] arr;

    int localOffset = fs.offset + fs.count * sizeof(pair);
    fs.count++;
    setInitfs(fs);

    pair p;
    memcpy(p.name, dirname.c_str(), dirname.size() + 1);
    p.offset = 0; 
    p.isDirectory = true;
    
    f.seekg(localOffset, std::ios::beg);
    f.write((char*)(&p), sizeof(p));
    f.close();
    
    std::cout << "Directory created: " << dirname << "\n";
}

void changeDirectory(const std::string dirname) {
    std::cout << "Currently only \n";
}

void listDirectory() {
    initfs fs = getInitfs();
    pair* arr = new pair[fs.count];
    
    std::fstream f("mydump");
    f.seekg(fs.offset, std::ios::beg);
    f.read((char*)arr, sizeof(pair) * fs.count);
    
    std::cout << "Directory contents:\n";
    for (int i = 0; i < fs.count; ++i) {
        if (arr[i].isDirectory) {
            std::cout << "[DIR] " << arr[i].name << "\n";
        } else {
            std::cout << "[FILE] " << arr[i].name << "\n";
        }
    }
    
    delete[] arr;
    f.close();
}

int main(int argc, char** argv) {
    std::string command, filename;

    while (true) {
        if (argc == 3) {
            command = argv[1];
            filename = argv[2];
            argc = 1;
        } else {
            std::cout << "\nInput command (i|r|w|mkdir|cd|ls): ";
            std::getline(std::cin, command);
            
            if (command.empty()) {
                std::cout << "Press Enter again to exit...";
                if (std::cin.get() == '\n') break;
                continue;
            }

            size_t pos = command.find(' ');
            if (pos != std::string::npos) {
                filename = command.substr(pos + 1);
                command = command.substr(0, pos);
            } else filename.clear();
        }

        switch (command[0]) {
            case 'i':
                init();
                break;
                
            case 'r':
                if (!filename.empty()) {
                    readFile(filename);
                } else {
                    std::cout << "Filename required for read operation\n";
                }
                break;
                
            case 'w':
                if (!filename.empty()) {
                    writeFile(filename);
                } else {
                    std::cout << "Filename required for write operation\n";
                }
                break;
                
            case 'm': // mkdir
                if (!filename.empty()) {
                    createDirectory(filename);
                } else {
                    std::cout << "Directory name required\n";
                }
                break;
                
            case 'c': // cd
                if (!filename.empty()) {
                    changeDirectory(filename);
                } else {
                    std::cout << "Directory name required\n";
                }
                break;
                
            case 'l': // ls
                listDirectory();
                break;
                
            default:
                std::cout << "Unknown command. Available commands:\n"
                          << "i - initialize filesystem\n"
                          << "r <file> - read file\n"
                          << "w <file> - write file\n"
                          << "mkdir <dir> - create directory\n"
                          << "cd <dir> - change directory\n"
                          << "ls - list contents\n";
                break;
        }
    }
    return 0;
}