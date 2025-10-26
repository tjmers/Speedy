#ifndef CLIENT_H
#define CLIENT_H

#include <winsock2.h>

#include "diff_match_patch.h"
#include <windows.h>
#include "sync_client.h"

#include <cwctype>  // For iswalnum in is_word_char
#include <vector> 
#include <string>
#include <mutex>
#include <unordered_set>

#include "config.h" // For Config
#include "graphics.h"     // For Graphics*
#include "opened_file.h"  // Assuming this includes Selection, Edit, etc.

class Client {
private:
    static Client* instance;
    std::vector<OpenedFile> opened_files;
    int current_file;
    HANDLE autosave_timer;

    static std::unordered_set<char> insertable_characters;

public:
    static void init();
    static void cleanup();
    static Client* get_instance();

    Client();
    ~Client();

    bool open_file(const std::string& file_path);
    void process_character(const char character);

    void move_left(bool extend_selection = false);
    void move_right(bool extend_selection = false);
    void move_up(bool extend_selection = false);
    void move_down(bool extend_selection = false);
    void jump_left(bool extend_selection = false);
    void jump_right(bool extend_selection = false);
    bool is_word_char(wchar_t ch);  // Declaration for word character check

    void delete_group();
    void copy(HWND hwnd);
    void cut(HWND hwnd);
    void paste(HWND hwnd);
    void select_all();
    
    // Text formatting methods
    void format_bold();
    void format_italic();
    void format_underline();
    void format_highlight();

    void draw(Graphics* g);
    void save_file(int file_id = -1) const;
    void close_file(int file_id = -1);

    void begin_autosave();
    void end_autosave();
    void autosave();

    OpenedFile& get_working_file();

    // Object to compare files
    diff_match_patch diff; 

    // Obejct to write to files
    Syncer syncer;

    std::mutex mut;
};

#endif // CLIENT_H