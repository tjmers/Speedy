#pragma once

#include <deque>
#include <string>
#include <vector>

#include "edit.h"
#include "graphics.h"
#include "selection.h"

class OpenedFile {
public:
    /// @brief Constructs a new OpenedFile object and attempts to open the file at the specified path.
    OpenedFile(const std::string& path);

    OpenedFile(const OpenedFile& other);
    OpenedFile& operator=(const OpenedFile& other);

    OpenedFile(OpenedFile&& other) noexcept;
    OpenedFile& operator=(OpenedFile&& other) noexcept;

    /// @brief Checks if the file is successfully opened.
    inline bool is_open() const { return open;}

    /// @brief Writes the current contents back to the file.
    bool write() const;

    /// @brief Adds a new line at the specified line number or at the current line if no line number is provided.
    void new_line(int line_number = -1, int character_number = -1, bool move_cursor = true);

    /// @brief Inserts a character at the specified line and character position.
    void insert_character(char character, int line_number = -1, int char_position = -1, bool move_cursor = true);

    /// @brief Deletes a character at the specified line and character position.
    void delete_character(int line_number = -1, int char_position = -1, bool move_cursor = true);

    /// @brief Deletes a range of text from start position to end position.
    void delete_range(int start_line, int start_char, int end_line, int end_char, bool move_cursor = true);

    /// @brief Undos the last edit action.
    bool undo();

    /// @brief Redos the last undone action.
    bool redo();

    // Selection methods
    /// @brief Starts a selection at the current cursor position
    void start_selection();
    
    /// @brief Updates selection to current cursor position
    void update_selection();
    
    /// @brief Clears the current selection
    void clear_selection();
    
    /// @brief Gets the selected text
    std::wstring get_selected_text() const;
    
    /// @brief Deletes the selected text
    void delete_selection();
    
    /// @brief Inserts text at current position, replacing selection if exists
    void insert_text(const std::wstring& text);

    // Getters and setters
    inline int get_current_line() const { return current_line; }
    inline int get_current_character_index() const { return current_character; }
    inline char get_current_character() const { return lines[current_line][current_character]; }
    inline const std::wstring& get_current_line_contents() const { return lines[current_line]; }
    inline int get_num_lines() const { return static_cast<int>(lines.size()); }
    inline int get_num_characters(int line_number = -1) const { 
        if (line_number == -1) line_number = current_line; 
        return static_cast<int>(lines[line_number].size()); 
    }
    inline void set_current_line(int line) { current_line = line; }
    inline void set_current_character(int character) { current_character = character; }
    inline const Selection& get_selection() const { return selection; }

    /// @brief Draws the contents of the opened file using the provided Graphics object.
    void draw(Graphics* g, int start_x, int start_y, int max_chars_per_line, int max_lines) const;

private:
    std::string file_path;
    std::vector<std::wstring> lines;
    int current_line;
    int current_character;
    bool open;
    std::deque<Edit> past_actions;
    std::deque<Edit> past_undos;
    Selection selection;
};