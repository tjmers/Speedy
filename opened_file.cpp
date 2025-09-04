#include "opened_file.h"

#include <fstream>
#include <iostream>

OpenedFile::OpenedFile(const std::string& path)
    : file_path(path), current_line(0), current_character(0), open(false) {
    std::wifstream file(file_path);
    if (file.is_open()) {
        open = true;
        std::wstring line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        file.close();
    }
}

bool OpenedFile::write() const {
    if (!open) {
        return false;
    }
    std::wofstream file(file_path);
    if (!file.is_open()) {
        return false;
    }
    for (const auto& line : lines) {
        file << line << '\n';
    }
    file.close();
    return true;
}

void OpenedFile::new_line(int line_number, int character_position, bool move_cursor) {
    if (line_number == -1) {
        line_number = current_line;
    }
    if (character_position == -1) {
        character_position = current_character;
    }
    std::wstring new_line;
    if (character_position < static_cast<int>(lines[line_number].size())) {
        new_line = lines[line_number].substr(character_position);
        lines[line_number] = lines[line_number].substr(0, character_position);
    }
    lines.insert(lines.begin() + line_number + 1, new_line);
    if (move_cursor) {
        current_line = line_number + 1;
        current_character = 0;
    }
}

void OpenedFile::insert_character(char character, int line_number, int char_position) {
    if (line_number == -1) {
        line_number = current_line;
    }
    if (char_position == -1) {
        char_position = current_character;
    }
    lines[line_number].insert(lines[line_number].begin() + char_position, character);
    current_character = char_position + 1;
}

void OpenedFile::delete_character(int line_number, int char_position) {
    if (line_number == -1) {
        line_number = current_line;
    }
    if (char_position == -1) {
        char_position = current_character;
    }
    if (char_position == 0) {
        if (line_number == 0) {
            return; // Can't delete before the start of the file
        }
        // Merge with previous line
        int prev_line_length = static_cast<int>(lines[line_number - 1].length());
        lines[line_number - 1] += lines[line_number];
        lines.erase(lines.begin() + line_number);
        current_line = line_number - 1;
        current_character = prev_line_length;
        return;
    }
    lines[line_number].erase(lines[line_number].begin() + char_position);
    current_character = char_position - 1;
}

void OpenedFile::draw(Graphics* g, int start_x, int start_y, int max_chars_per_line, int max_lines) const {

    g->SetColor(D2D1::ColorF(0.18f, 0.18f, 0.18f));
    constexpr int line_height = 20; 
    for (int i = 0; i < max_lines && i < static_cast<int>(lines.size()); ++i) {
        std::wstring line = lines[i];
        if (start_x > 0 || start_x + max_chars_per_line < static_cast<int>(line.length())) {
            std::wstring substringed_line = line.substr(start_x, max_chars_per_line);
            g->DrawString(substringed_line.c_str(), static_cast<int>(line.length()), static_cast<float>(start_x), static_cast<float>(start_y + i * line_height), 800.0f, static_cast<float>(line_height));
        } else {
            g->DrawString(line.c_str(), static_cast<int>(line.length()), static_cast<float>(start_x), static_cast<float>(start_y + i * line_height), 800.0f, static_cast<float>(line_height));
        }
    }
}