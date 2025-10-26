#pragma once

#include "graphics.h"
#include "opened_file.h"

#include <iostream>
#include <unordered_set>
#include <vector>

class Client {
    
public:
    static void init();
    static void cleanup();
    static Client* get_instance();
    
private:
    Client();
public:
    ~Client();
    
    /// @brief Opens a file and adds it to the list of opened files
    bool open_file(const std::string& file_path);
    
    /// @brief Processes a character input from the user
    void process_character(const char character);

    /// @brief Saves the current file
    void CALLBACK save_file(int file_id = -1) const;

    /// @brief Closes the specified file
    void close_file(int file_id = -1);

    /// @brief Draws the current state of the client
    void draw(Graphics* g);

    inline OpenedFile& get_working_file() { return opened_files[current_file]; }

    // Navigation methods
    void move_left(bool extend_selection = false);
    void move_right(bool extend_selection = false);
    void move_up(bool extend_selection = false);
    void move_down(bool extend_selection = false);
    void jump_left(bool extend_selection = false);
    void jump_right(bool extend_selection = false);

    void delete_group();

    // Selection methods
    void start_selection();
    void clear_selection();
    
    // Clipboard methods
    void copy(HWND hwnd);
    void cut(HWND hwnd);
    void paste(HWND hwnd);
    
    // Select all
    void select_all();

    void begin_autosave();
    void end_autosave();

private:
    static Client* instance;
    std::vector<OpenedFile> opened_files;
    int current_file;
    static std::unordered_set<char> insertable_characters;
    void autosave();
    HANDLE autosave_timer;
    bool is_selecting;
};