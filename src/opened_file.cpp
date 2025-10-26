#include "opened_file.h"
#include "config.h"
#include "graphics.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <algorithm>

OpenedFile::OpenedFile(const std::string& path)
    : file_path(path),
      lines(),
      current_line(0),
      current_character(0),
      open(false),
      past_actions(),
      past_undos(),
      selection() {
    std::wifstream file(file_path);
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
      past_undos(other.past_undos),
      selection(other.selection) {}

OpenedFile& OpenedFile::operator=(const OpenedFile& other) {
    if (this != &other) {
        file_path = other.file_path;
        lines = other.lines;
        current_line = other.current_line;
        current_character = other.current_character;
        open = other.open;
        past_actions = other.past_actions;
        past_undos = other.past_undos;
        selection = other.selection;
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
      past_undos(std::move(other.past_undos)),
      selection(std::move(other.selection)) {
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
        selection = std::move(other.selection);

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

// Selection methods
void OpenedFile::start_selection() {
    selection.start_selection(current_line, current_character);
}

void OpenedFile::update_selection() {
    selection.update_selection(current_line, current_character);
}

void OpenedFile::clear_selection() {
    selection.clear_selection();
}

std::wstring OpenedFile::get_selected_text() const {
    if (!selection.has_selection()) return L"";
    
    int start_line, start_char, end_line, end_char;
    selection.get_normalized_range(start_line, start_char, end_line, end_char);
    
    std::wstring result;
    
    if (start_line == end_line) {
        // Selection within single line
        result = lines[start_line].substr(start_char, end_char - start_char);
    } else {
        // Multi-line selection
        // First line
        result = lines[start_line].substr(start_char);
        result += L'\n';
        
        // Middle lines
        for (int i = start_line + 1; i < end_line; ++i) {
            result += lines[i];
            result += L'\n';
        }
        
        // Last line
        result += lines[end_line].substr(0, end_char);
    }
    
    return result;
}

void OpenedFile::delete_selection() {
    if (!selection.has_selection()) return;
    
    int start_line, start_char, end_line, end_char;
    selection.get_normalized_range(start_line, start_char, end_line, end_char);
    
    delete_range(start_line, start_char, end_line, end_char);
    clear_selection();
}

void OpenedFile::apply_formatting(FormatType type) {
    if (!selection.has_selection()) return;
    
    int start_line, start_char, end_line, end_char;
    selection.get_normalized_range(start_line, start_char, end_line, end_char);
    
    formatting_manager.add_formatting(start_line, start_char, end_line, end_char, type);
}

std::vector<FormatRange> OpenedFile::get_line_formatting(int line) const {
    std::vector<FormatRange> result;
    
    // Get all formatting ranges that intersect with this line
    for (const auto& range : formatting_manager.get_all_ranges()) {
        if (range.start_line <= line && range.end_line >= line) {
            result.push_back(range);
        }
    }
    
    return result;
}

// Edit methods
void OpenedFile::new_line(int line_number, int character_position, bool move_cursor) {
    if (line_number == -1) {
        line_number = current_line;
    }
    if (character_position == -1) {
        character_position = current_character;
    }

    if (selection.has_selection()) {
        delete_selection();
        line_number = current_line;
        character_position = current_character;
    }

    int n_spaces = 0;
    while (n_spaces < static_cast<int>(lines[line_number].size()) && lines[line_number][n_spaces] == ' ') ++n_spaces;
    
    std::wstring deleted_characters = lines[line_number].substr(character_position);
    past_actions.push_back(Edit(
        [this, line_number, character_position, move_cursor, n_spaces, deleted_characters = std::move(deleted_characters)]() mutable {
            std::wstring new_line(n_spaces, L' ');
            if (character_position < static_cast<int>(lines[line_number].size())) {
                new_line += lines[line_number].substr(character_position);
                lines[line_number] = lines[line_number].substr(0, character_position);
            }
            lines.insert(lines.begin() + line_number + 1, std::move(new_line));
            if (move_cursor) {
                set_current_line(line_number + 1);
                set_current_character(n_spaces);
            }
            return true;
        },
        [this, line_number, character_position, move_cursor, deleted_characters]() {
            lines.erase(lines.begin() + line_number + 1);
            lines[line_number] += deleted_characters;
            if (move_cursor) {
                set_current_line(line_number);
                set_current_character(character_position);
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

    if (selection.has_selection()) {
        delete_selection();
        line_number = current_line;
        char_position = current_character;
    }

    if (character == '\t') {
        int tab_size = Config::get_instance()->get_tab_size();
        past_actions.push_back(Edit(
            [this, line_number, char_position, tab_size, move_cursor]() {
                this->lines[line_number].insert(this->lines[line_number].begin() + char_position, tab_size, ' ');
                if (move_cursor) {
                    this->set_current_line(line_number);
                    this->set_current_character(char_position + tab_size);
                }
                return true;
            },
            [this, line_number, char_position, tab_size, move_cursor]() {
                this->lines[line_number].erase(this->lines[line_number].begin() + char_position, this->lines[line_number].begin() + char_position + tab_size);
                if (move_cursor) {
                    this->set_current_line(line_number);
                    this->set_current_character(char_position);
                }
                return true;
            }
        ));
    } else {
        wchar_t wch = static_cast<wchar_t>(character);
        past_actions.push_back(Edit(
            [this, line_number, char_position, wch, move_cursor]() {
                this->lines[line_number].insert(this->lines[line_number].begin() + char_position, 1, wch);
                if (move_cursor) {
                    this->set_current_line(line_number);
                    this->set_current_character(char_position + 1);
                }
                return true;
            },
            [this, line_number, char_position, move_cursor]() {
                this->lines[line_number].erase(this->lines[line_number].begin() + char_position, this->lines[line_number].begin() + char_position + 1);
                if (move_cursor) {
                    this->set_current_line(line_number);
                    this->set_current_character(char_position);
                }
                return true;
            }
        ));
    }
    
    past_actions.back().edit();
    past_undos.clear();
    
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

    if (selection.has_selection()) {
        delete_selection();
        return;
    }

    if (char_position > 0) {
        wchar_t deleted = lines[line_number][char_position - 1];
        past_actions.push_back(Edit(
            [this, line_number, char_position, move_cursor, deleted]() {
                this->lines[line_number].erase(this->lines[line_number].begin() + char_position - 1, this->lines[line_number].begin() + char_position);
                if (move_cursor) {
                    this->set_current_line(line_number);
                    this->set_current_character(char_position - 1);
                }
                return true;
            },
            [this, line_number, char_position, move_cursor, deleted]() {
                this->lines[line_number].insert(this->lines[line_number].begin() + char_position - 1, 1, deleted);
                if (move_cursor) {
                    this->set_current_line(line_number);
                    this->set_current_character(char_position);
                }
                return true;
            }
        ));
        
        past_actions.back().edit();
        past_undos.clear();
        
        if (static_cast<int>(past_actions.size()) > Config::get_instance()->get_undo_history_size()) {
            past_actions.pop_front();
        }
    }
}

void OpenedFile::delete_range(int start_line, int start_char, int end_line, int end_char, bool move_cursor) {
    if (start_line == -1) start_line = current_line;
    if (start_char == -1) start_char = current_character;
    if (end_line == -1) end_line = current_line;
    if (end_char == -1) end_char = current_character;

    if (start_line > end_line || (start_line == end_line && start_char > end_char)) {
        std::swap(start_line, end_line);
        std::swap(start_char, end_char);
    }

    std::wstring deleted_content;
    int num_lines_deleted = 0;

    if (start_line == end_line) {
        deleted_content = lines[start_line].substr(start_char, end_char - start_char);
        num_lines_deleted = 0;
    } else {
        deleted_content = lines[start_line].substr(start_char) + L"\n";
        for (int l = start_line + 1; l < end_line; ++l) {
            deleted_content += lines[l] + L"\n";
        }
        deleted_content += lines[end_line].substr(0, end_char);
        num_lines_deleted = end_line - start_line;
    }

    past_actions.push_back(Edit(
        [this, start_line, start_char, end_line, end_char, num_lines_deleted, move_cursor]() {
            if (num_lines_deleted == 0) {
                this->lines[start_line].erase(start_char, end_char - start_char);
            } else {
                this->lines[start_line].erase(start_char);
                this->lines[start_line] += this->lines[end_line].substr(end_char);
                this->lines.erase(this->lines.begin() + start_line + 1, this->lines.begin() + end_line + 1);
            }
            if (move_cursor) {
                set_current_line(start_line);
                set_current_character(start_char);
            }
            return true;
        },
        [this, start_line, start_char, deleted_content, num_lines_deleted, move_cursor]() {
            if (num_lines_deleted == 0) {
                this->lines[start_line].insert(start_char, deleted_content);
            } else {
                std::vector<std::wstring> restored_lines;
                std::wstring current_line_text;
                for (wchar_t ch : deleted_content) {
                    if (ch == L'\n') {
                        restored_lines.push_back(current_line_text);
                        current_line_text.clear();
                    } else {
                        current_line_text += ch;
                    }
                }
                restored_lines.push_back(current_line_text);
                std::wstring after_cursor = this->lines[start_line].substr(start_char);
                this->lines[start_line] = this->lines[start_line].substr(0, start_char) + restored_lines[0];
                for (size_t i = 1; i < restored_lines.size() - 1; ++i) {
                    this->lines.insert(this->lines.begin() + start_line + i, restored_lines[i]);
                }
                this->lines.insert(this->lines.begin() + start_line + restored_lines.size() - 1,
                                 restored_lines[restored_lines.size() - 1] + after_cursor);
            }
            return true;
        }
    ));
    
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
    int line_height = static_cast<int>(Config::get_instance()->get_font_size() * 1.25f);

    // Calculate line number area width
    float line_number_width = 0;
    if (Config::get_instance()->get_show_line_numbers()) {
        // Calculate width needed for line numbers (assuming up to 4 digits)
        line_number_width = font_size * 0.6f * 4 + 10; // 4 digits + 10px padding
        
        // Draw line numbers
        g->SetColor(Config::get_instance()->get_line_number_color());
        for (int i = 0; i < max_lines && i < static_cast<int>(lines.size()); ++i) {
            std::wstring line_number = std::to_wstring(i + 1);
            float numbers_x = Config::get_instance()->get_explorer_width() + 5; // 5 pixels from left edge
            g->DrawString(line_number.c_str(), static_cast<int>(line_number.length()), numbers_x, static_cast<float>(start_y + i * line_height), line_number_width, static_cast<float>(line_height));
        }   
    }

    float x = Config::get_instance()->get_left_margin() + Config::get_instance()->get_explorer_width() + line_number_width;
    
    // Get normalized selection range if active
    int sel_start_line = -1, sel_start_char = -1, sel_end_line = -1, sel_end_char = -1;
    bool has_sel = selection.has_selection();
    if (has_sel) {
        selection.get_normalized_range(sel_start_line, sel_start_char, sel_end_line, sel_end_char);
    }

    // Draw text and selection
    for (int i = 0; i < max_lines && i < static_cast<int>(lines.size()); ++i) {
        float line_y = static_cast<float>(start_y + i * line_height);
        std::wstring line = lines[i];
        
        // Draw selection highlighting for this line
        if (has_sel && i >= sel_start_line && i <= sel_end_line) {
            int highlight_start = 0;
            int highlight_end = static_cast<int>(line.length());
            
            if (i == sel_start_line) {
                highlight_start = sel_start_char;
            }
            if (i == sel_end_line) {
                highlight_end = sel_end_char;
            }
            
            if (highlight_start < highlight_end) {
                float highlight_x = x + highlight_start * font_size * 0.6f;
                float highlight_width = (highlight_end - highlight_start) * font_size * 0.6f;
                
                // Draw selection background
                g->SetColor(D2D1::ColorF(D2D1::ColorF::LightBlue, 0.4f));
                g->FillRect(highlight_x, line_y, highlight_x + highlight_width, line_y + line_height);
            }
        }
        
        // Draw text
        g->SetColor(Config::get_instance()->get_text_color());
        
        // Get formatting for this line
        auto line_formatting = get_line_formatting(i);
        
        if (start_x > 0 || start_x + max_chars_per_line < static_cast<int>(line.length())) {
            std::wstring substringed_line = line.substr(start_x, max_chars_per_line);
            
            // Adjust formatting ranges for substring
            std::vector<FormatRange> adjusted_formatting;
            for (const auto& range : line_formatting) {
                if (range.start_line == i && range.end_line == i) {
                    // Single line range - adjust for substring
                    int adj_start = std::max(0, range.start_char - start_x);
                    int adj_end = std::min(max_chars_per_line, range.end_char - start_x);
                    if (adj_start < adj_end) {
                        adjusted_formatting.emplace_back(i, adj_start, i, adj_end, range.type);
                    }
                }
            }
            
            g->DrawFormattedString(substringed_line.c_str(), static_cast<int>(substringed_line.length()), 
                                 static_cast<float>(start_x), line_y, 800.0f, static_cast<float>(line_height), adjusted_formatting);
        } else {
            g->DrawFormattedString(line.c_str(), static_cast<int>(line.length()), x, line_y, 
                                 800.0f, static_cast<float>(line_height), line_formatting);
        }
    }

    // Draw the cursor indicator
    g->SetColor(Config::get_instance()->get_indicator_color());
    g->DrawLine(
        current_character * font_size * 0.6f + x,
        current_line * font_size * 1.25f,
        current_character * font_size * 0.6f + x,
        (current_line + 1) * font_size * 1.25f,
        2.0f
    );
}