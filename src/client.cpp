#include "client.h"

#include <codecvt>
#include <locale>
#include <sstream>

std::unordered_set<char> Client::insertable_characters;


std::vector<std::wstring> splitStringToWStringVector(const std::string& input) {
    std::vector<std::wstring> result;
    std::istringstream ss(input);
    std::string line;

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

    while (std::getline(ss, line, '\n')) {
        result.push_back(converter.from_bytes(line));
    }

    return result;
}

void Client::init() {
    // Initialize the insertable_characters set

    // Add all uppercase characters
    for (char c = 'A'; c <= 'Z'; ++c) {
        insertable_characters.insert(c);
    }

    // Add all lowercase characters
    for (char c = 'a'; c <= 'z'; ++c) {
        insertable_characters.insert(c);
    }

    // Add all numbers
    for (char c = '0'; c <= '9'; ++c) {
        insertable_characters.insert(c);
    }

    // Add special characters
    std::string char_to_insert = " `~!@#$%^&*()-_=+[{]}\\|;:\'\",<.>/?";
    for (char c : char_to_insert) {
        insertable_characters.insert(c);
    }

    // Add backspace and carrige return
    insertable_characters.insert(VK_BACK);
    insertable_characters.insert('\r');

    // Add tab
    insertable_characters.insert('\t');

    instance = new Client();
}

void Client::cleanup() {
    delete instance;
}

Client* Client::get_instance() {
    return instance;
}

Client* Client::instance = nullptr;
    

Client::Client()
    : opened_files{}, current_file(-1), autosave_timer(NULL), diff(), syncer("3.95.174.32", 8080, ""), mut() {}

Client::~Client() {}

bool Client::open_file(const std::string& file_path) {
    opened_files.push_back(OpenedFile(file_path));
    current_file = static_cast<int>(opened_files.size()) - 1;
    
    // Add to the syncer
    std::string file_data = syncer.set_file(file_path);
    std::vector<std::wstring> lines = splitStringToWStringVector(file_data);
    opened_files[current_file].set_lines(std::move(lines));
    return opened_files[current_file].is_open();
}

void Client::process_character(const char character) {
    // Do not insert a character if it isn't allowed
    if (!insertable_characters.contains(character)) return;
    switch (character) {
        case '\r': // Carrige return (enter key)
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


void Client::begin_autosave() {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
    CreateTimerQueueTimer(
        &autosave_timer,
        NULL,
        [] (PVOID lp_param, BOOLEAN TimerOrWaitFired) {
            static_cast<Client*>(lp_param)->autosave();
        },
        this,
        0,
        1000, // Autosave delay in MS
        WT_EXECUTEDEFAULT
    );
#pragma GCC diagnostic pop
}

void Client::end_autosave() {
    if (autosave_timer) {
        DeleteTimerQueueTimer(NULL, &autosave_timer, NULL);
    }
}


void Client::autosave() {
    // std::cout << "STARTING AUTOSAVE\n";
    // // Get the differences between the two files
    // // Iterate through each line and add to a std::string
    std::string working;

    // std::cout << "Part 1\n";
    // // Convert the current working files lines into a std::string
    OpenedFile& of = opened_files[current_file];
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    for (int line_number = 0; line_number < of.get_num_lines(); ++line_number) {
        of.set_current_line(line_number);
        working.append(converter.to_bytes(of.get_current_line_contents()));
    }
    syncer.write_to_remote(working);
    std::string remote;
    if (syncer.update_from_remote(remote)) {
        of.set_lines(splitStringToWStringVector(remote));
    }
    // std::string remote;
    // if (syncer.update_from_remote(remote)) {
    //     std::cout << "Part 0\n";
    //     // Save the line that was on before
    //     int selected_line = of.get_current_line();
    
    
    
    //     std::vector<Diff> diffs = diff.diff_main(working, remote);
    //     remote = diff.diff_text2(diffs); // gives text2 with diffs applied
    //     if (remote[remote.size() - 1] != '\0') remote.push_back(0);
    
    
    //     // Remote is now the version that is to be used
    //     OpenedFile new_file(of);
    
    //     std::cout << "Part 2\n";
    //     of.set_lines(splitStringToWStringVector(remote));
    //     of.set_current_line(selected_line);
        
    //     // Update the remote with remote
    //     std::cout << "Attempting to write " << remote << " to remote\n";
    //     syncer.write_to_remote(remote);
    
    //     std::cout << "Part 3\n";
    // } else {
    //     std::cout << "Did not read. Writing: " << working << " to remote\n";
    //     syncer.write_to_remote(working);
    // }
    
    

}