#include "command_controller.h"

#include <array>
#include <fstream>
#include <sstream>

#include <windows.h>

#include "client.h"

CommandController* CommandController::instance = nullptr;
void CommandController::init(Client* c) {
    instance = new CommandController(c);
}
CommandController::CommandController(Client* c) 
    : client(c) {
    if (!load_commands()) {
        commands.clear();
        get_default_commands();
    }
}

bool CommandController::run_commands() const {
    BYTE key_state[256];
    if (!GetKeyboardState(key_state)) return false;;

    std::array<bool, 256> down;

    for (int i = 0; i < 256; ++i) {
        down[i] = key_state[i] & 0x80;
    }
    
    // Map VK_LSHIFT and VK_RSHIFT to VK_SHIFT for easier matching
    if (down[VK_LSHIFT] || down[VK_RSHIFT]) {
        down[VK_SHIFT] = true;
        down[VK_LSHIFT] = false;  // Clear the left/right shift keys so they don't interfere
        down[VK_RSHIFT] = false;
    }
    // Map VK_LCONTROL and VK_RCONTROL to VK_CONTROL for easier matching
    if (down[VK_LCONTROL] || down[VK_RCONTROL]) {
        down[VK_CONTROL] = true;
        down[VK_LCONTROL] = false;  // Clear the left/right control keys
        down[VK_RCONTROL] = false;
    }
    // Map VK_LMENU and VK_RMENU to VK_MENU for easier matching
    if (down[VK_LMENU] || down[VK_RMENU]) {
        down[VK_MENU] = true;
        down[VK_LMENU] = false;  // Clear the left/right alt 
        // keys
        down[VK_RMENU] = false;
    }
    
    for (const Command& c : commands) {
        if (c.get_key_requirements() == down) {
            c.execute();
            return true; // Only allow one command per call
        }
    }
    return false;
}

std::array<bool, 256> get_bools_from_string(const std::string& str) {
    std::array<bool, 256> ret_val;

    for (int i = 0; i < 256; ++i) {
        ret_val[i] = str[i] - '0';
    }

    return ret_val;
}

bool CommandController::load_commands() {
    // Attempt to load commands from commands.cfg
    // Use default command list if not
    std::ifstream commands_file("./config/commands.cfg");

    if (!commands_file.is_open()) {
        return false;
    }

    std::string command_name;
    while (std::getline(commands_file, command_name)) {
        std::string description;
        std::string key_reqs;
        std::string actions;
        if (!std::getline(commands_file, description) ||
            !std::getline(commands_file, key_reqs) ||
            !std::getline(commands_file, actions)) return false;

        // Get the function from the action string and then create the command object and add it to the vector
        std::istringstream iss(actions);
        std::string action_name;
        std::string next_token;
        std::vector<std::function<void()>> action_functions;
        while (iss >> action_name) {
            if (action_name == "CHAR_LEFT") {
                action_functions.push_back([this](){this->client->move_left();});
            } else if (action_name == "CHAR_RIGHT") {
                action_functions.push_back([this](){this->client->move_right();});
            } else if (action_name == "CHAR_UP") {
                action_functions.push_back([this](){this->client->move_up();});
            } else if (action_name == "CHAR_DOWN") {
                action_functions.push_back([this](){this->client->move_down();});
            } else if (action_name == "JUMP_LEFT") {
                action_functions.push_back([this](){this->client->jump_left();});
            } else if (action_name == "JUMP_RIGHT") {
                action_functions.push_back([this](){this->client->jump_right();});
            } else if (action_name == "SAVE") {
                iss >> next_token;
                int file_id = std::stoi(next_token);
                action_functions.push_back([this, file_id](){this->client->save_file(file_id); });
            } else if (action_name == "DEL") {
                iss >> next_token;
                int line_number = std::stoi(next_token);
                iss >> next_token;
                int char_number = std::stoi(next_token);
                iss >> next_token;
                bool should_jump = next_token == "TRUE";
                action_functions.push_back([this, line_number, char_number, should_jump](){this->client->get_working_file().delete_character(line_number, char_number, should_jump);});
            } else if (action_name == "DEL_WORD") {
                action_functions.push_back([this](){this->client->delete_group();});
            } else if (action_name == "UNDO") {
                action_functions.push_back([this](){this->client->get_working_file().undo();});
            } else if (action_name == "REDO") {
                action_functions.push_back([this](){this->client->get_working_file().redo();});
            } else if (action_name == "CLOSE_FILE") {
                iss >> next_token;
                int file_id = std::stoi(next_token);
                action_functions.push_back([this, file_id](){this->client->close_file(file_id);});
            } else if (action_name == "SELECT_LEFT") {
                action_functions.push_back([this](){this->client->move_left(true);});
            } else if (action_name == "SELECT_RIGHT") {
                action_functions.push_back([this](){this->client->move_right(true);});
            } else if (action_name == "SELECT_UP") {
                action_functions.push_back([this](){this->client->move_up(true);});
            } else if (action_name == "SELECT_DOWN") {
                action_functions.push_back([this](){this->client->move_down(true);});
            } else if (action_name == "SELECT_WORD_LEFT") {
                action_functions.push_back([this](){this->client->jump_left(true);});
            } else if (action_name == "SELECT_WORD_RIGHT") {
                action_functions.push_back([this](){this->client->jump_right(true);});
            } else if (action_name == "COPY") {
                action_functions.push_back([this](){
                    HWND hwnd = GetActiveWindow();
                    this->client->copy(hwnd);
                });
            } else if (action_name == "CUT") {
                action_functions.push_back([this](){
                    HWND hwnd = GetActiveWindow();
                    this->client->cut(hwnd);
                });
            } else if (action_name == "PASTE") {
                action_functions.push_back([this](){
                    HWND hwnd = GetActiveWindow();
                    this->client->paste(hwnd);
                });
            } else if (action_name == "SELECT_ALL") {
                action_functions.push_back([this](){this->client->select_all();});
            }
        }

        // Combine actions into single lambda
        std::function<void()> command_action = [action_functions = std::move(action_functions)]() {
            for (const auto& f : action_functions) {
                f();
            }
        };

        std::array<bool, 256> key_req = get_bools_from_string(key_reqs);
        commands.emplace_back(command_name, description, actions, key_req, command_action);
    }

    commands_file.close();
    return true;
}

void CommandController::get_default_commands() {
    // Movement commands
    commands.emplace_back(
        "Move Left",
        "Moves the cursor left one character",
        "CHAR_LEFT",
        std::vector<char>({VK_LEFT}),
        [this] () {this->client->move_left();}
    );
    commands.emplace_back(
        "Move Right",
        "Moves the cursor right one character",
        "CHAR_RIGHT",
        std::vector<char>({VK_RIGHT}),
        [this] () {this->client->move_right();}
    );
    commands.emplace_back(
        "Move Up",
        "Moves the cursor up one line",
        "CHAR_UP",
        std::vector<char>({VK_UP}),
        [this] () {this->client->move_up();}
    );
    commands.emplace_back(
        "Move Down",
        "Moves the cursor down one line",
        "CHAR_DOWN",
        std::vector<char>({VK_DOWN}),
        [this] () {this->client->move_down();}
    );
    commands.emplace_back(
        "Jump Left",
        "Jumps left to the beginning of the previous word",
        "JUMP_LEFT",
        std::vector<char>({VK_LEFT, VK_CONTROL}),
        [this] () {this->client->jump_left();}
    );
    commands.emplace_back(
        "Jump Right",
        "Jumps right to the end of the next word",
        "JUMP_RIGHT",
        std::vector<char>({VK_RIGHT, VK_CONTROL}),
        [this] () {this->client->jump_right();}
    );
    // Save command
    commands.emplace_back(
        "Save File",
        "Saves the working file",
        "SAVE -1",
        std::vector<char>{'S', 17},
        [this] () {this->client->save_file(-1);}
    );
    // Delete Word
    commands.emplace_back(
        "Delete Word",
        "Delete the current word",
        "DEL_WORD",
        std::vector<char>{VK_BACK, VK_CONTROL},
        [this] () {this->client->delete_group();}
    );

    // Undo and Redo
    commands.emplace_back(
        "Undo",
        "Undos the last action in the working file",
        "UNDO",
        std::vector<char>{VK_CONTROL, 'Z'},
        [this] () {this->client->get_working_file().undo();}
    );
    commands.emplace_back(
        "Redo",
        "Redoes the past undo in the working file if no edits have been made",
        "REDO",
        std::vector<char>{VK_CONTROL, 'Y'},
        [this] () {this->client->get_working_file().redo();}
    );
    commands.emplace_back(
        "Close File",
        "Closes the current working file",
        "CLOSE_FILE -1",
        std::vector<char>{VK_CONTROL, 'W'},
        [this] () {this->client->close_file();}
    );
    // Arrow Keys with Shift for selection
    commands.emplace_back(
        "Select Left",
        "Extends selection to the left",
        "SELECT_LEFT",
        std::vector<char>({VK_LEFT, VK_SHIFT}),
        [this] () {this->client->move_left(true);}
    );
    commands.emplace_back(
        "Select Right",
        "Extends selection to the right",
        "SELECT_RIGHT",
        std::vector<char>({VK_RIGHT, VK_SHIFT}),
        [this] () {this->client->move_right(true);}
    );
    commands.emplace_back(
        "Select Up",
        "Extends selection upward",
        "SELECT_UP",
        std::vector<char>({VK_UP, VK_SHIFT}),
        [this] () {this->client->move_up(true);}
    );
    commands.emplace_back(
        "Select Down",
        "Extends selection downward",
        "SELECT_DOWN",
        std::vector<char>({VK_DOWN, VK_SHIFT}),
        [this] () {this->client->move_down(true);}
    );
    
    // Ctrl+Shift+Arrow for word selection
    commands.emplace_back(
        "Select Word Left",
        "Extends selection to the previous word",
        "SELECT_WORD_LEFT",
        std::vector<char>({VK_LEFT, VK_CONTROL, VK_SHIFT}),
        [this] () {this->client->jump_left(true);}
    );
    commands.emplace_back(
        "Select Word Right",
        "Extends selection to the next word",
        "SELECT_WORD_RIGHT",
        std::vector<char>({VK_RIGHT, VK_CONTROL, VK_SHIFT}),
        [this] () {this->client->jump_right(true);}
    );
    
    // Copy, Cut, Paste
    commands.emplace_back(
        "Copy",
        "Copies selected text to clipboard",
        "COPY",
        std::vector<char>({VK_CONTROL, 'C'}),
        [this] () {
            HWND hwnd = GetActiveWindow();
            this->client->copy(hwnd);
        }
    );
    commands.emplace_back(
        "Cut",
        "Cuts selected text to clipboard",
        "CUT",
        std::vector<char>({VK_CONTROL, 'X'}),
        [this] () {
            HWND hwnd = GetActiveWindow();
            this->client->cut(hwnd);
        }
    );
    commands.emplace_back(
        "Paste",
        "Pastes text from clipboard",
        "PASTE",
        std::vector<char>({VK_CONTROL, 'V'}),
        [this] () {
            HWND hwnd = GetActiveWindow();
            this->client->paste(hwnd);
        }
    );
    
    // Select All
    commands.emplace_back(
        "Select All",
        "Selects all text in the document",
        "SELECT_ALL",
        std::vector<char>({VK_CONTROL, 'A'}),
        [this] () {this->client->select_all();}
    );
    
    // Delete key
    commands.emplace_back(
        "Delete",
        "Deletes the character at cursor or selected text",
        "DELETE",
        std::vector<char>({VK_DELETE}),
        [this] () {
            OpenedFile& file = this->client->get_working_file();
            if (file.get_selection().has_selection()) {
                file.delete_selection();
            } else {
                int line = file.get_current_line();
                int pos = file.get_current_character_index();
                if (pos < file.get_num_characters(line)) {
                    file.delete_character(line, pos + 1, false);
                } else if (line < file.get_num_lines() - 1) {
                    // Delete newline - merge with next line
                    file.delete_character(line + 1, 0, false);
                }
            }
        }
    );
}

std::string key_string(const std::array<bool, 256>& keys) {
    std::string ret_val(256, '0');
    for (int i = 0; i < 256; ++i) {
        if (keys[i]) {
            ret_val[i] = '1';
        }
    }

    return ret_val;
}

void CommandController::save_commands() const {
    std::ofstream commands_file("./config/commands.cfg");
    // Each command takes four lines
    // 1: Name
    // 2: Description
    // 3: Keybind
    // 4: Action

    for (const Command& c : commands) {
        commands_file << c.get_name() << '\n' << c.get_description() << '\n' << key_string(c.get_key_requirements()) << '\n' << c.get_action_string() << '\n';
    }
}