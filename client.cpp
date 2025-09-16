#include "client.h"

Client::Client()
    : current_file(-1), opened_files{} {}

Client::~Client() {}

bool Client::open_file(const std::string& file_path) {
    opened_files.push_back(OpenedFile(file_path));
    current_file = static_cast<int>(opened_files.size()) - 1;
    return opened_files[current_file].is_open();
}

void Client::process_character(const char character) {
    // Do not process character input in command mode here.
    switch (character) {
        case '\r': // Enter key
            opened_files[current_file].new_line();
            break;
        case VK_BACK:
            opened_files[current_file].delete_character();
            break;
        default:
            opened_files[current_file].insert_character(character);
            break;
    }
}

void Client::move_left() {

    OpenedFile& working_file = opened_files[current_file];
    // Move cursor left
    if (working_file.get_current_character_index() == 0 && working_file.get_current_line() > 0) {
        // Move to end of previous line
        int prev_line = working_file.get_current_line() - 1;
        working_file.set_current_line(prev_line);
        working_file.set_current_character(working_file.get_num_characters(prev_line));
    } else if (working_file.get_current_character_index() > 0) {
        working_file.set_current_character(working_file.get_current_character_index() - 1);
    }
}

void Client::move_right() {

    OpenedFile& working_file = opened_files[current_file];
    // Move cursor right
    if (working_file.get_current_character_index() == working_file.get_num_characters() && working_file.get_current_line() < working_file.get_num_lines() - 1) {
        // Move to start of next line
        int next_line = working_file.get_current_line() + 1;
        working_file.set_current_line(next_line);
        working_file.set_current_character(0);
    } else if (working_file.get_current_character_index() < working_file.get_num_characters()) {
        working_file.set_current_character(working_file.get_current_character_index() + 1);
    }
}

void Client::move_up() {

    OpenedFile& working_file = opened_files[current_file];
    // Move cursor up
    if (working_file.get_current_line() > 0) {
        working_file.set_current_line(working_file.get_current_line() - 1);
        working_file.set_current_character(std::min(working_file.get_current_character_index(), working_file.get_num_characters()));
    } else {
        // Set to first character
        working_file.set_current_character(0);
    }
}

void Client::move_down() {

    OpenedFile& working_file = opened_files[current_file];
    // Move cursor down
    if (working_file.get_current_line() < working_file.get_num_lines() - 1) {
        working_file.set_current_line(working_file.get_current_line() + 1);
        working_file.set_current_character(std::min(working_file.get_current_character_index(), working_file.get_num_characters()));
    } else {
        // Set to last character
        working_file.set_current_character(working_file.get_num_characters());
    }
}

void Client::jump_left() {

    OpenedFile& working_file = opened_files[current_file];
    const std::wstring& line_contents = working_file.get_current_line_contents();
    int char_index = working_file.get_current_character_index();
    while (char_index > 0 && line_contents[char_index - 1] == L' ') {
        --char_index;
    }
    while (char_index > 0 && line_contents[char_index - 1] != L' ') {
        --char_index;
    }
    working_file.set_current_character(char_index);
}

void Client::jump_right() {

    OpenedFile& working_file = opened_files[current_file];
    const std::wstring& line_contents = working_file.get_current_line_contents();
    int char_index = working_file.get_current_character_index();
    while (char_index < working_file.get_num_characters() && line_contents[char_index] == L' ') {
        ++char_index;
    }
    while (char_index < working_file.get_num_characters() && line_contents[char_index] != L' ') {
        ++char_index;
    }
    working_file.set_current_character(char_index);
}

void Client::delete_group() {

    OpenedFile& working_file = opened_files[current_file];
    // Delete characters to the left of the cursor until a space or the start of the line is reached
    int first_char = working_file.get_current_character_index() - 2;
    if (first_char < 0) {
        working_file.delete_character();
        return;
    }
    if (working_file.get_current_character() == L' ') {
        // Keep going until non-whitespace or start of line
        while (first_char >= 0 && working_file.get_current_line_contents()[first_char] == L' ') {
            --first_char;
        }
    } else {
        // Keep going until whitespace or start of line
        while (first_char >= 0 && working_file.get_current_line_contents()[first_char] != L' ') {
            --first_char;
        }
    }
    working_file.delete_range(working_file.get_current_line(), first_char + 1, working_file.get_current_line(), working_file.get_current_character_index());
}

void Client::draw(Graphics* g) {
    if (current_file == -1) {
        return;
    }
    const OpenedFile& file = opened_files[current_file];

    file.draw(g, 0, 0, 80, 40);

}

void Client::save_file(int file_id) const {
    if (file_id == -1) file_id = current_file;
    opened_files[file_id].write();
}

void Client::close_file(int file_id) {
    if (file_id == -1) file_id = current_file;
    opened_files.erase(opened_files.begin() + file_id);
    if (current_file >= static_cast<int>(opened_files.size())) current_file = static_cast<int>(opened_files.size()) - 1;
}