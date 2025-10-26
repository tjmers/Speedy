#pragma once

#include <windows.h>

/// @brief Manages text selection state within a document
class Selection {
public:
    Selection();
    
    /// @brief Starts a new selection at the given position
    void start_selection(int line, int character);
    
    /// @brief Updates the selection end position
    void update_selection(int line, int character);
    
    /// @brief Clears the current selection
    void clear_selection();
    
    /// @brief Checks if there is an active selection
    inline bool has_selection() const { return is_active; }
    
    /// @brief Gets the start line of the selection
    inline int get_start_line() const { return start_line; }
    
    /// @brief Gets the start character of the selection
    inline int get_start_char() const { return start_char; }
    
    /// @brief Gets the end line of the selection
    inline int get_end_line() const { return end_line; }
    
    /// @brief Gets the end character of the selection
    inline int get_end_char() const { return end_char; }
    
    /// @brief Gets the normalized start position (ensures start comes before end)
    void get_normalized_range(int& norm_start_line, int& norm_start_char, 
                             int& norm_end_line, int& norm_end_char) const;
    
    /// @brief Checks if a position is within the selection range
    bool is_position_selected(int line, int character) const;
    
private:
    bool is_active;
    int start_line;
    int start_char;
    int end_line;
    int end_char;
};