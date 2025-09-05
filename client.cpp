#include "client.h"

Client::Client()
    : current_file(-1) {}

Client::~Client() {}

bool Client::open_file(const std::string& file_path) {
    opened_files.push_back(OpenedFile(file_path));
    current_file = static_cast<int>(opened_files.size()) - 1;
    return opened_files[current_file].is_open();
}

void Client::process_character(const char character) {
    // Do not process character input in command mode here.
    if (in_command) return;
    switch (character) {
        case '\r': // Enter key
            opened_files[current_file].new_line();
            break;
        case VK_BACK:
            opened_files[current_file].delete_character();
            break;
        default:
            opened_files[current_file].insert_character(character, -1, -1);
            break;
    }
}


void Client::process_special_key(const char key) {
    OpenedFile& working_file = opened_files[current_file];

    switch (key) {
        case VK_LEFT:
            // Move cursor left
            if (working_file.get_current_character() == 0 && working_file.get_current_line() > 0) {
                // Move to end of previous line
                int prev_line = working_file.get_current_line() - 1;
                working_file.set_current_line(prev_line);
                working_file.set_current_character(working_file.get_num_characters(prev_line));
            } else if (working_file.get_current_character() > 0) {
                working_file.set_current_character(working_file.get_current_character() - 1);
            }
            break;
        case VK_RIGHT:
            // Move cursor right
            if (working_file.get_current_character() == working_file.get_num_characters() && working_file.get_current_line() < working_file.get_num_lines() - 1) {
                // Move to start of next line
                int next_line = working_file.get_current_line() + 1;
                working_file.set_current_line(next_line);
                working_file.set_current_character(0);
            } else if (working_file.get_current_character() < working_file.get_num_characters()) {
                working_file.set_current_character(working_file.get_current_character() + 1);
            }
            break;
        case VK_UP:
            // Move cursor up
            if (working_file.get_current_line() > 0) {
                working_file.set_current_line(working_file.get_current_line() - 1);
                working_file.set_current_character(std::min(working_file.get_current_character(), working_file.get_num_characters()));
            }
            break;
        case VK_DOWN:
            // Move cursor down
            if (working_file.get_current_line() < working_file.get_num_lines() - 1) {
                working_file.set_current_line(working_file.get_current_line() + 1);
                working_file.set_current_character(std::min(working_file.get_current_character(), working_file.get_num_characters()));
            }
            break;
        default:
            break;
    }

    if (!in_command) return;

    // Process all command keys here.
    switch (key) {
        case 'S': // Save file
            save_file();
            break;
        default:
            break;
    }
}


void Client::draw(Graphics* g) {
    if (current_file == -1) {
        return;
    }
    const OpenedFile& file = opened_files[current_file];

    file.draw(g, 0, 0, 80, 40);

}

void Client::save_file() const {
    if (current_file == -1) {
        return;
    }
    opened_files[current_file].write();
}