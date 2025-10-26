#include "../src/sync_client.h"

#include <iostream>
#include <fstream>
#include <string>

void appendNullTerminator(const std::string& filename) {
    std::ofstream file(filename, std::ios::binary | std::ios::app);
    if (!file) {
        std::cerr << "Failed to open file: " << filename << '\n';
        return;
    }
    char nullChar = '1';
    file.write(&nullChar, 1);
    file.close();
}

int main() {
    std::string path = "test.txt";
    appendNullTerminator(path);

    Syncer s("3.95.174.32", 8080, path);
    s.write_to_remote(path);

    return 0;
    
}