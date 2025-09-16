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
    std::ifstream commands_file("commands.cfg");

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
            } else {
                // Log info here
            }
        }
        commands.emplace_back(command_name, description, actions, get_bools_from_string(key_reqs), [action_functions](){for (const auto& f : action_functions)f();});
    }

    return true;

}

void CommandController::get_default_commands() {

    // Arrow Keys
    commands.emplace_back(
        "Move Left",
        "Moves the cursor to the left by one character, or up a line if at the beginning",
        "CHAR_LEFT",
        std::vector<char>({VK_LEFT}),
        [this] () {this->client->move_left();}
    );
    commands.emplace_back(
        "Move Right",
        "Moves the cursor to the right by one character, or down a line if at the end",
        "CHAR_RIGHT",
        std::vector<char>({VK_RIGHT}),
        [this] () {this->client->move_right();}
    );
    commands.emplace_back(
        "Move Up",
        "Moves the cursor up a line",
        "CHAR_UP",
        std::vector<char>({VK_UP}),
        [this] () {this->client->move_up();}
    );
    commands.emplace_back(
        "Move Down",
        "Moves the cursor down a line",
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
    std::ofstream commands_file("commands.cfg");
    // Each command takes four lines
    // 1: Name
    // 2: Description
    // 3: Keybind
    // 4: Action

    for (const Command& c : commands) {
        commands_file << c.get_name() << '\n' << c.get_description() << '\n' << key_string(c.get_key_requirements()) << '\n' << c.get_action_string() << '\n';
    }
}