#pragma once

#include <deque>
#include <string>
#include <vector>

#include "edit.h"
#include "graphics.h"

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
    /// @param line_number The line number where the new line should be added. If -1, adds at the current line.
    void new_line(int line_number = -1, int character_number = -1, bool move_cursor = true); // if -1, adds to the position specified by current_line

    /// @brief Inserts a character at the specified line and character position or and the current position if no parameters are provided.
    /// @param character The character to insert.
    /// @param char_position The character position within the line where the character should be inserted.
    /// @param line_number The line number where the character should be inserted.
    void insert_character(char character, int line_number = -1, int char_position = -1, bool move_cursor = true);

    /// @brief Deletes a character at the specified line and character position. If no parameters are provided, deletes at the current position. Updates current position accordingly.
    /// @param line_number The line number from which the character should be deleted.
    /// @param char_position The character position within the line to delete.
    void delete_character(int line_number = -1, int char_position = -1, bool move_cursor = true);

    /// @brief Deletes a range of text from start position to end position.
    /// @param start_line The starting line number of the range to delete.
    /// @param start_char The starting character position within the starting line (included).
    /// @param end_line The ending line number of the range to delete.
    /// @param end_char The ending character position within the ending line (excluded).
    /// @param move_cursor If true, moves the cursor to the start position after deletion.
    void delete_range(int start_line, int start_char, int end_line, int end_char, bool move_cursor = true);

    /// @brief Undos the last edit action.
    /// @return True if the undo was successful, false if there was no undo.
    bool undo();

    /// @brief Redos the last undone action.
    /// @return True if the redo was successful, false if there was no redo.
    bool redo();


    // Getters and setters
    inline int get_current_line() const { return current_line; }
    inline int get_current_character_index() const { return current_character; }
    inline char get_current_character() const { return lines[current_line][current_character]; }
    inline const std::wstring& get_current_line_contents() const { return lines[current_line]; }
    inline int get_num_lines() const { return static_cast<int>(lines.size()); }
    inline int get_num_characters(int line_number = -1) const { if (line_number == -1) line_number = current_line; return static_cast<int>(lines[line_number].size()); }
    inline void set_current_line(int line) { current_line = line; }
    inline void set_current_character(int character) { current_character = character; }
    inline void set_line(const std::wstring& str) { lines[current_line] = str; }
    inline void set_line(std::wstring&& str) { lines[current_line] = std::move(str); }
    inline void set_lines(const std::vector<std::wstring>& new_lines) { lines = new_lines; }
    inline void set_lines(std::vector<std::wstring>&& new_lines) { lines = std::move(new_lines); }


    /// @brief Draws the contents of the opened file using the provided Graphics object.
    /// @param g Graphics object used for drawing.
    /// @param start_x Position of the first character to draw.
    /// @param start_y Position of the first line to draw.
    /// @param max_chars_per_line Maximum number of characters to draw per line.
    /// @param max_lines Maximum number of lines to draw.
    void draw(Graphics* g, int start_x, int start_y, int max_chars_per_line, int max_lines) const;

private:

    /// @brief The file path of the opened file.
    std::string file_path;

    /// @brief The lines of text in the opened file.
    std::vector<std::wstring> lines;
    
    /// @brief The current line number being edited or viewed.
    int current_line;

    /// @brief The current character position within the current line.
    int current_character;

    /// @brief Indicates whether the file is successfully opened.
    bool open;

    /// @brief A deque storing the history of edit actions for undo functionality.
    std::deque<Edit> past_actions;

    /// @brief A deque storing the history of undone actions for redo functionality.
    std::deque<Edit> past_undos;
};