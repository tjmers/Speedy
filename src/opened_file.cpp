#include "opened_file.h"

#include "config.h"

#include <fstream>
#include <iostream>
#include <memory>

OpenedFile::OpenedFile(const std::string& path)
    : file_path(path), current_line(0), current_character(0), open(false), past_actions(), past_undos() {
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

OpenedFile::OpenedFile(const OpenedFile& other)
    : file_path(other.file_path),
      lines(other.lines),
      current_line(other.current_line),
      current_character(other.current_character),
      open(other.open),
      past_actions(other.past_actions),
      past_undos(other.past_undos) {}

OpenedFile& OpenedFile::operator=(const OpenedFile& other) {
    if (this != &other) {
        file_path = other.file_path;
        lines = other.lines;
        current_line = other.current_line;
        current_character = other.current_character;
        open = other.open;
        past_actions = other.past_actions;
        past_undos = other.past_undos;
    }
    return *this;
}

OpenedFile::OpenedFile(OpenedFile&& other) noexcept
    : file_path(std::move(other.file_path)),
      lines(std::move(other.lines)),
      current_line(other.current_line),
      current_character(other.current_character),
      open(other.open),
      past_actions(std::move(other.past_actions)),
      past_undos(std::move(other.past_undos)) {
    other.current_line = 0;
    other.current_character = 0;
    other.open = false;
}

OpenedFile& OpenedFile::operator=(OpenedFile&& other) noexcept {
    if (this != &other) {
        file_path = std::move(other.file_path);
        lines = std::move(other.lines);
        current_line = other.current_line;
        current_character = other.current_character;
        open = other.open;
        past_actions = std::move(other.past_actions);
        past_undos = std::move(other.past_undos);

        other.current_line = 0;
        other.current_character = 0;
        other.open = false;
    }
    return *this;
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

    // Calculate number of spaces before the first character on the line
    int n_spaces = 0;
    while (n_spaces < static_cast<int>(lines[line_number].size()) && lines[line_number][n_spaces] == ' ') ++n_spaces;
    
    const wchar_t* deleted_characters = lines[line_number].substr(character_position).c_str();
    past_actions.push_back(Edit(
        [this, line_number, character_position, move_cursor, n_spaces]() {
            std::wstring new_line(n_spaces, L' ');
            if (character_position < static_cast<int>(lines[line_number].size())) {
                new_line += lines[line_number].substr(character_position);
                lines[line_number] = lines[line_number].substr(0, character_position);
            }
            lines.insert(lines.begin() + line_number + 1, std::move(new_line));
            if (move_cursor) {
                current_line = line_number + 1;
                current_character = n_spaces;
            }
            return true;
        },
        [this, line_number, character_position, move_cursor, &deleted_characters]() {
            lines.erase(lines.begin() + line_number + 1);
            lines[line_number] += deleted_characters;
            if (move_cursor) {
                current_line = line_number;
                current_character = character_position;
            }
            return true;
        }
    ));
    
    past_actions.back().edit();
    if (static_cast<int>(past_actions.size()) > Config::get_instance()->get_undo_history_size()) {
        past_actions.pop_front();
    }
    past_undos.clear();
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
    past_undos.clear();
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

        
    past_actions.back().edit();
    if (static_cast<int>(past_actions.size()) > Config::get_instance()->get_undo_history_size()) {
        past_actions.pop_front();
    }
    past_undos.clear();
}

void OpenedFile::delete_range(int start_line, int start_char, int end_line, int end_char, bool move_cursor) {

    std::shared_ptr<std::vector<std::wstring>> deleted_lines = std::make_shared<std::vector<std::wstring>>();
    if (start_line == end_line) {
        deleted_lines->push_back(lines[start_line].substr(start_char, end_char - start_char));
    } else {
        deleted_lines->push_back(lines[start_line].substr(start_char));
        for (int i = start_line + 1; i < end_line; ++i) {
            deleted_lines->push_back(lines[i]);
        }
        deleted_lines->push_back(lines[end_line].substr(0, end_char));
    }

    // Declare here to capture in lambda
    int cc = current_character;
    int cl = current_line;

    past_actions.push_back(Edit(
        [this, start_line, start_char, move_cursor, deleted_lines, end_line, end_char]() {
            if (start_line == end_line) {
                this->lines[start_line].erase(start_char, (*deleted_lines)[0].size());
            } else {
                this->lines[start_line].erase(start_char);
                this->lines[start_line] += this->lines[end_line].substr(end_char);
                this->lines.erase(this->lines.begin() + start_line + 1, this->lines.begin() + end_line);
                this->lines[start_line + 1].erase(end_char);
            }
            if (move_cursor) {
                this->current_line = start_line;
                this->current_character = start_char;
            }
            return true;
        },
        [this, start_line, start_char, deleted_lines, move_cursor, cc, cl]() {
            if (deleted_lines->size() == 1) {
                this->lines[start_line].insert(start_char, (*deleted_lines)[0]);
            } else {
                std::wstring first_part = this->lines[start_line].substr(0, start_char);
                std::wstring last_part = this->lines[start_line].substr(start_char);
                this->lines[start_line] = first_part + (*deleted_lines)[0];
                for (size_t i = 1; i < deleted_lines->size(); ++i) {
                    this->lines.insert(this->lines.begin() + start_line + i, (*deleted_lines)[i]);
                }
            }
            if (move_cursor) {
                this->current_line = cl;
                this->current_character = cc;
            }
            return true;
        }));
    past_actions.back().edit();
    past_undos.clear();
    if (static_cast<int>(past_actions.size()) > Config::get_instance()->get_undo_history_size()) {
        past_actions.pop_front();
    }
}

bool OpenedFile::undo() {
    if (past_actions.empty()) {
        return false;
    }
    bool result = past_actions.back().undo();
    past_undos.push_back(std::move(past_actions.back()));
    past_actions.pop_back();
    return result;
}

bool OpenedFile::redo() {
    if (!past_undos.empty()) {
        bool result = past_undos.back().edit();
        past_actions.push_back(past_undos.back());
        past_undos.pop_back();
        return result;
    }
    return false;
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