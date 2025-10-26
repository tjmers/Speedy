#pragma once

#include <winsock2.h>

#include "diff_match_patch.h"
#include "graphics.h"
#include "opened_file.h"
#include "sync_client.h"

#include <iostream>
#include <mutex>
#include <unordered_set>
#include <vector>


class Client {
    
public:

    static void init();
    static void cleanup();
    static Client* get_instance();
    
    /// @brief Constructs a new Client object.
private:
    Client(); // Prevent instantiation
public:
    
    /// @brief Destroys the Client object and releases any allocated resources.
    ~Client();
    
    /// @brief adds it to the list of opened files and selects it as the current file.
    bool open_file(const std::string& file_path);
    
    /// @brief Processes a character input from the user.
    /// @param character the character to process.
    void process_character(const char character);


    /// @brief Saves the current file.
    void CALLBACK save_file(int file_id = -1) const;

    /// @brief Closes the specified file and removes it from the list of opened files.
    /// @param file_id The unique identifier of the file to close.
    void close_file(int file_id = -1);

    /// @brief Draws the current state of the client using the provided Graphics object.
    /// @param g the Graphics object used for drawing.
    void draw(Graphics* g); // Implementation for drawing the client state can be added here.

    inline OpenedFile& get_working_file() { return opened_files[current_file]; }

    void move_left();
    void move_right();
    void move_up();
    void move_down();
    void jump_left();
    void jump_right();

    void delete_group();

    void begin_autosave();
    void end_autosave();

private:


    static Client* instance;


    /// @brief The list of opened files.
    std::vector<OpenedFile> opened_files;

    /// @brief The index of the currently active file in the opened_files vector.
    int current_file;

    static std::unordered_set<char> insertable_characters;
    void autosave();

    HANDLE autosave_timer;

    // Object to compare files
    diff_match_patch diff; 

    // Obejct to write to files
    Syncer syncer;

    std::mutex mut;
};