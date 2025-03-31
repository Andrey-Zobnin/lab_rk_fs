
#include <cstring>
#include <fstream>
#include <iostream>

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
    //    int fileOffset;
    int fileEndOffset;
};

struct pair {
    char name[20];
    int offset;
};

void init() {
    std::fstream f("mydump");

    if (!f.is_open()) {
        return;
    }

    f.seekg(0, std::ios::beg);
    initfs init;
    init.offset = 100;
    init.count = 0;
    //    init.fileOffset = 1000;
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
    // for (int i = 0; i < fs.count; ++i) {
    //     f.read((char*)&(p), sizeof(p));
    //     if (p.name == filename) {
    //         break;
    //     }
    // }
    pair* arr = new pair[fs.count];
    f.read((char*)arr, sizeof(pair) * fs.count);
    for (int i = 0; i < fs.count; ++i) {
        if (arr[i].name == filename) {
            p = arr + i;
            break;
        }
    }

    if (p == nullptr) {
        std::cout << "File not exist\n" << filename << "\n";
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

// Чтение указатель на таблицу
// Если файлов нет -> seek(tabSize * count) -> ( filename, ptr=fileEndOffset)
// (meta, data)
// Если файл есть -> seek(tabSize * count) -> (filenema, ptr=fileEndOffset)
// (meta, data)
//

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
    int localOffset = fs.offset + fs.count * sizeof(pair);  // начало пары
    int localFileEndOffset = fs.fileEndOffset;              // начало для файла

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
    f.seekg(localOffset, std::ios::beg);
    f.write((char*)(&(p)), sizeof(p));

    f.seekg(localFileEndOffset, std::ios::beg);
    f.write((char*)(&(my_file.meta)), sizeof(my_file.meta));
    f.write(my_file.data.data(), my_file.data.size());

    f.close();
}

int main(int argc, char** argv) {
    std::string choice;
    std::string filename;
    if (argc == 3) {
        choice = argv[1];
        filename = argv[2];
    } else {
        std::cout << "Input r|w|i: ";
        std::cin >> choice;
        if (choice[0] != 'i') {
            std::cin >> filename;
        }
    }

    switch (choice[0]) {
        case 'i':
            init();
            break;
        case 'r':
            readFile(filename);
            break;
        case 'w':
            writeFile(filename);
            break;
        default:
            std::cout << "not op\n";
            break;
    }
    return 0;
}
