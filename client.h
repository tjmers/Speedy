#pragma once

#include "graphics.h"
#include "opened_file.h"

#include <vector>


class Client {
    
public:
    
    /// @brief Constructs a new Client object.
    Client();
    
    /// @brief Destroys the Client object and releases any allocated resources.
    ~Client();
    
    /// @brief adds it to the list of opened files and selects it as the current file.
    bool open_file(const std::string& file_path);
    
    /// @brief Processes a character input from the user.
    /// @param character the character to process.
    void process_character(const char character);

    /// @brief Processes a special key input from the user.
    /// @param key the special key to process.
    void process_special_key(const char key);

    inline void enter_command_mode() { in_command = true; }
    inline void exit_command_mode() { in_command = false; }

    /// @brief Draws the current state of the client using the provided Graphics object.
    /// @param g the Graphics object used for drawing.
    void draw(Graphics* g); // Implementation for drawing the client state can be added here.

private:
    /// @brief Saves the current file.
    void save_file() const;

    /// @brief Closes the specified file and removes it from the list of opened files.
    /// @param file_id The unique identifier of the file to close.
    void close_file(int file_id);

    /// @brief The list of opened files.
    std::vector<OpenedFile> opened_files;

    /// @brief The index of the currently active file in the opened_files vector.
    int current_file;

    /// @brief Indicates whether the client is currently in command mode. Indicated by holding down the ALT key.
    bool in_command;

};