#include "opened_file.h"

#include "config.h"

#include <fstream>
#include <iostream>

OpenedFile::OpenedFile(const std::string& path)
    : file_path(path), current_line(0), current_character(0), open(false), past_actions() {
    std::wifstream file(file_path);

    // Open the file and read its contents into the lines vector.
    if (file.is_open()) {
        open = true;
        std::wstring line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        file.close();
    }

    if (lines.empty()) {
        lines.push_back(L"");
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

void OpenedFile::insert_character(char character, int line_number, int char_position, bool move_cursor) {
    if (line_number == -1) {
        line_number = current_line;
    }
    if (char_position == -1) {
        char_position = current_character;
    }

    if (character == '\t') {
        int tab_size = Config::get_instance()->get_tab_size();
        past_actions.push_back(Edit(
            [this, line_number, char_position, tab_size, move_cursor]() {
                this->lines[line_number].insert(this->lines[line_number].begin() + char_position, tab_size, ' ');
                if (move_cursor) {
                    this->current_line = line_number;
                    this->current_character = char_position + tab_size;
                }
                return true;
            },
            [this, line_number, char_position, tab_size, move_cursor]() {
                this->lines[line_number].erase(this->lines[line_number].begin() + char_position, this->lines[line_number].begin() + char_position + tab_size);
                if (move_cursor) {
                    this->current_line = line_number;
                    this->current_character = char_position;
                }
                return true;
            }
        ));
    } else {
        past_actions.push_back(Edit(
            [this, line_number, char_position, character, move_cursor]() {
                this->lines[line_number].insert(this->lines[line_number].begin() + char_position, character);
                if (move_cursor) {
                    this->current_line = line_number;
                    this->current_character = char_position + 1;
                }
                return true;
            },
            [this, line_number, char_position, move_cursor]() {
                this->lines[line_number].erase(this->lines[line_number].begin() + char_position);
                if (move_cursor) {
                    this->current_line = line_number;
                    this->current_character = char_position;
                }
                return true;
            }
        ));
    }
    

    past_actions.back().edit();
    if (static_cast<int>(past_actions.size()) > Config::get_instance()->get_undo_history_size()) {
        past_actions.pop_front();
    }
}

void OpenedFile::delete_character(int line_number, int char_position, bool move_cursor) {
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
        past_actions.push_back(Edit(
            [this, line_number, move_cursor]() {
                int prev_line_length = static_cast<int>(lines[line_number - 1].length());
                lines[line_number - 1] += lines[line_number];
                lines.erase(lines.begin() + line_number);
                if (move_cursor) {
                    current_line = line_number - 1;
                    current_character = prev_line_length;
                }
                return true;
            },
            [this, line_number, move_cursor]() {
                std::wstring merged_part = lines[line_number - 1].substr(lines[line_number - 1].length() - lines[line_number].length());
                lines.insert(lines.begin() + line_number, merged_part);
                lines[line_number - 1] = lines[line_number - 1].substr(0, lines[line_number - 1].length() - merged_part.length());
                if (move_cursor) {
                    current_line = line_number;
                    current_character = 0;
                }
                return true;
            }
        ));
        past_actions.back().edit();
        if (static_cast<int>(past_actions.size()) > Config::get_instance()->get_undo_history_size()) {
            past_actions.pop_front();
        }
        return;
    } else {
        wchar_t deleted_char = lines[line_number][char_position - 1];
        past_actions.push_back(Edit(
            [this, line_number, char_position, move_cursor]() {
                this->lines[line_number].erase(this->lines[line_number].begin() + char_position - 1);
                if (move_cursor) {
                    this->current_line = line_number;
                    this->current_character = char_position - 1;
                }
                return true;
            },
            [this, line_number, char_position, move_cursor, deleted_char]() {
                this->lines[line_number].insert(this->lines[line_number].begin() + char_position - 1, deleted_char);
                if (move_cursor) {
                    this->current_line = line_number;
                    this->current_character = char_position;
                }
                return true;
            }
        ));
    }
    lines[line_number].erase(lines[line_number].begin() + char_position - 1);
    current_character = char_position - 1;
}

bool OpenedFile::undo() {
    if (past_actions.empty()) {
        return false;
    }
    bool result = past_actions.back().undo();
    past_actions.pop_back();
    return result;
}

void OpenedFile::draw(Graphics* g, int start_x, int start_y, int max_chars_per_line, int max_lines) const {
    const float& font_size = Config::get_instance()->get_font_size();

    // Draw line numbers if enabled
    if (Config::get_instance()->get_show_line_numbers()) {
        g->SetColor(Config::get_instance()->get_line_number_color());
        int line_height = static_cast<int>(Config::get_instance()->get_font_size() * 1.25f);
        for (int i = 0; i < max_lines && i < static_cast<int>(lines.size()); ++i) {
            std::wstring line_number = std::to_wstring(i + 1);
            float numbers_x = Config::get_instance()->get_explorer_width() + Config::get_instance()->get_left_margin() - font_size * 0.6f * (2 + (i + 1 >= 10) + (i + 1 >= 100) + (i + 1 >= 1000));
            g->DrawString(line_number.c_str(), static_cast<int>(line_number.length()), numbers_x, static_cast<float>(start_y + i * line_height), 50.0f, static_cast<float>(line_height));
        }
    }

    float x = Config::get_instance()->get_left_margin() + Config::get_instance()->get_explorer_width();
    int y = 0;

    g->SetColor(Config::get_instance()->get_text_color());
    int line_height = static_cast<int>(Config::get_instance()->get_font_size() * 1.25f);

    for (int i = 0; i < max_lines && i < static_cast<int>(lines.size()); ++i) {
        std::wstring line = lines[i];
        if (start_x > 0 || start_x + max_chars_per_line < static_cast<int>(line.length())) {
            std::wstring substringed_line = line.substr(start_x, max_chars_per_line);
            g->DrawString(substringed_line.c_str(), static_cast<int>(line.length()), static_cast<float>(start_x), static_cast<float>(start_y + i * line_height), 800.0f, static_cast<float>(line_height));
        } else {
            g->DrawString(line.c_str(), static_cast<int>(line.length()), x, static_cast<float>(y + i * line_height), 800.0f, static_cast<float>(line_height));
        }
    }

    // Draw the indicator
    g->SetColor(Config::get_instance()->get_indicator_color());
    g->DrawLine(
        current_character * font_size * 0.6f + x,
        current_line * font_size * 1.25f,
        current_character * font_size * 0.6f + x,
        (current_line + 1) * font_size * 1.25f,
        2.0f
    );
}