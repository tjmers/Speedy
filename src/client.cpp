#include "client.h"

std::unordered_set<char> Client::insertable_characters;

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

    // Add backspace and carriage return
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
    : opened_files{}, current_file(-1), autosave_timer(NULL) {}

Client::~Client() {}

bool Client::open_file(const std::string& file_path) {
    opened_files.push_back(OpenedFile(file_path));
    current_file = static_cast<int>(opened_files.size()) - 1;
    return opened_files[current_file].is_open();
}

void Client::process_character(const char character) {
    // Do not insert a character if it isn't allowed
    if (!insertable_characters.contains(character)) return;
    
    OpenedFile& working_file = opened_files[current_file];
    
    // If there's a selection and user types, delete the selection first
    if (working_file.get_selection().has_selection() && character != VK_BACK) {
        working_file.delete_selection();
    }
    
    switch (character) {
        case '\r': // Carriage return (enter key)
            working_file.new_line();
            break;
        case VK_BACK:
            // If there's a selection, delete it; otherwise delete character
            if (working_file.get_selection().has_selection()) {
                working_file.delete_selection();
            } else {
                working_file.delete_character();
            }
            break;
        default:
            working_file.insert_character(character);
            break;
    }
}

void Client::move_left(bool extend_selection) {
    OpenedFile& working_file = opened_files[current_file];
    
    if (extend_selection) {
        if (!working_file.get_selection().has_selection()) {
            working_file.start_selection();
        }
    } else {
        working_file.clear_selection();
    }
    
    // Move cursor left
    if (working_file.get_current_character_index() == 0 && working_file.get_current_line() > 0) {
        // Move to end of previous line
        int prev_line = working_file.get_current_line() - 1;
        working_file.set_current_line(prev_line);
        working_file.set_current_character(working_file.get_num_characters(prev_line));
    } else if (working_file.get_current_character_index() > 0) {
        working_file.set_current_character(working_file.get_current_character_index() - 1);
    }
    
    if (extend_selection) {
        working_file.update_selection();
    }
}

void Client::move_right(bool extend_selection) {
    OpenedFile& working_file = opened_files[current_file];
    
    if (extend_selection) {
        if (!working_file.get_selection().has_selection()) {
            working_file.start_selection();
        }
    } else {
        working_file.clear_selection();
    }
    
    // Move cursor right
    if (working_file.get_current_character_index() == working_file.get_num_characters(working_file.get_current_line()) && 
        working_file.get_current_line() < working_file.get_num_lines() - 1) {
        // Move to start of next line
        int next_line = working_file.get_current_line() + 1;
        working_file.set_current_line(next_line);
        working_file.set_current_character(0);
    } else if (working_file.get_current_character_index() < working_file.get_num_characters(working_file.get_current_line())) {
        working_file.set_current_character(working_file.get_current_character_index() + 1);
    }
    
    if (extend_selection) {
        working_file.update_selection();
    }
}

void Client::move_up(bool extend_selection) {
    OpenedFile& working_file = opened_files[current_file];
    
    if (extend_selection) {
        if (!working_file.get_selection().has_selection()) {
            working_file.start_selection();
        }
    } else {
        working_file.clear_selection();
    }
    
    // Move cursor up
    if (working_file.get_current_line() > 0) {
        working_file.set_current_line(working_file.get_current_line() - 1);
        working_file.set_current_character(std::min(working_file.get_current_character_index(), 
                                                   working_file.get_num_characters(working_file.get_current_line())));
    } else {
        // Already at top: Stay put or go to line start?
        working_file.set_current_character(0);
    }
    
    if (extend_selection) {
        working_file.update_selection();
    }
}

void Client::move_down(bool extend_selection) {
    OpenedFile& working_file = opened_files[current_file];
    
    if (extend_selection) {
        if (!working_file.get_selection().has_selection()) {
            working_file.start_selection();
        }
    } else {
        working_file.clear_selection();
    }
    
    // Move cursor down
    if (working_file.get_current_line() < working_file.get_num_lines() - 1) {
        working_file.set_current_line(working_file.get_current_line() + 1);
        working_file.set_current_character(std::min(working_file.get_current_character_index(), 
                                                   working_file.get_num_characters(working_file.get_current_line())));
    } else {
        // Already at bottom: Go to end of line
        working_file.set_current_character(working_file.get_num_characters(working_file.get_current_line()));
    }
    
    if (extend_selection) {
        working_file.update_selection();
    }
}

void Client::jump_left(bool extend_selection) {
    OpenedFile& working_file = opened_files[current_file];
    
    if (extend_selection && !working_file.get_selection().has_selection()) {
        working_file.start_selection();
    }
    
    const std::wstring& line_contents = working_file.get_current_line_contents();
    int char_index = working_file.get_current_character_index();
    
    // Skip trailing spaces
    while (char_index > 0 && line_contents[char_index - 1] == L' ') {
        --char_index;
    }
    
    // Skip to start of word
    while (char_index > 0 && line_contents[char_index - 1] != L' ') {
        --char_index;
    }
    
    working_file.set_current_character(char_index);
    
    if (extend_selection) {
        working_file.update_selection();
    } else {
        working_file.clear_selection();
    }
}

void Client::jump_right(bool extend_selection) {
    OpenedFile& working_file = opened_files[current_file];
    
    if (extend_selection && !working_file.get_selection().has_selection()) {
        working_file.start_selection();
    }
    
    const std::wstring& line_contents = working_file.get_current_line_contents();
    int char_index = working_file.get_current_character_index();
    
    // Skip leading spaces
    while (char_index < working_file.get_num_characters(working_file.get_current_line()) && line_contents[char_index] == L' ') {
        ++char_index;
    }
    
    // Skip to end of word
    while (char_index < working_file.get_num_characters(working_file.get_current_line()) && line_contents[char_index] != L' ') {
        ++char_index;
    }
    
    working_file.set_current_character(char_index);
    
    if (extend_selection) {
        working_file.update_selection();
    } else {
        working_file.clear_selection();
    }
}

void Client::delete_group() {
    OpenedFile& working_file = opened_files[current_file];
    
    // Can't delete if we're at the start of the file
    if (working_file.get_current_character_index() == 0 && working_file.get_current_line() == 0) {
        return;
    }
    
    // If selection active, delete the whole range instead
    if (working_file.get_selection().has_selection()) {
        working_file.delete_selection();
        return;
    }
    
    int current_char_index = working_file.get_current_character_index();
    
    // If at the start of a line, just delete one character (merge with previous line)
    if (current_char_index == 0) {
        working_file.delete_character();
        return;
    }
    
    const std::wstring& line_contents = working_file.get_current_line_contents();
    
    // Start from the position before cursor
    int delete_to = current_char_index;
    int delete_from = current_char_index - 1;
    
    // Step 1: Skip backwards over any spaces
    while (delete_from > 0 && line_contents[delete_from] == L' ') {
        --delete_from;
    }
    
    // Step 2: If we found spaces, check if there's a word before them
    if (delete_from < current_char_index - 1) {
        // We skipped some spaces
        if (delete_from >= 0 && line_contents[delete_from] != L' ') {
            // There's a word before the spaces, delete the word too
            while (delete_from > 0 && line_contents[delete_from - 1] != L' ') {
                --delete_from;
            }
        } else {
            // Only spaces before cursor, just delete the spaces
            delete_from = delete_from + 1; // Move back to first space
        }
    } else {
        // No spaces immediately before cursor, we're at a word
        // Delete backwards through the word
        while (delete_from > 0 && line_contents[delete_from - 1] != L' ') {
            --delete_from;
        }
    }
    
    // Now delete from delete_from to delete_to
    working_file.delete_range(
        working_file.get_current_line(), 
        delete_from, 
        working_file.get_current_line(), 
        delete_to
    );
}

void Client::copy(HWND hwnd) {
    OpenedFile& working_file = opened_files[current_file];
    if (!working_file.get_selection().has_selection()) return;
    
    std::wstring selected_text = working_file.get_selected_text();
    
    if (OpenClipboard(hwnd)) {
        EmptyClipboard();
        
        size_t size = (selected_text.length() + 1) * sizeof(wchar_t);
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
        if (hMem) {
            wchar_t* pMem = static_cast<wchar_t*>(GlobalLock(hMem));
            if (pMem) {
                wcscpy_s(pMem, selected_text.length() + 1, selected_text.c_str());
                GlobalUnlock(hMem);
                SetClipboardData(CF_UNICODETEXT, hMem);
            }
        }
        CloseClipboard();
    }
}

void Client::cut(HWND hwnd) {
    OpenedFile& working_file = opened_files[current_file];
    if (!working_file.get_selection().has_selection()) return;
    
    copy(hwnd);
    working_file.delete_selection();
}

void Client::paste(HWND hwnd) {
    OpenedFile& working_file = opened_files[current_file];
    
    // If there's a selection, delete it first
    if (working_file.get_selection().has_selection()) {
        working_file.delete_selection();
    }
    
    if (OpenClipboard(hwnd)) {
        HANDLE hData = GetClipboardData(CF_UNICODETEXT);
        if (hData) {
            wchar_t* pText = static_cast<wchar_t*>(GlobalLock(hData));
            if (pText) {
                std::wstring text(pText);
                GlobalUnlock(hData);
                
                // Insert the text character by character
                for (wchar_t wc : text) {
                    if (wc == L'\n' || wc == L'\r') {
                        if (wc == L'\r') {
                            working_file.new_line();
                        }
                        // Skip \n if it follows \r (Windows line endings)
                        continue;
                    } else {
                        char c = static_cast<char>(wc);
                        if (insertable_characters.contains(c)) {
                            working_file.insert_character(c);
                        }
                    }
                }
            }
        }
        CloseClipboard();
    }
}

void Client::select_all() {
    OpenedFile& working_file = opened_files[current_file];
    
    // Move to start of file
    working_file.set_current_line(0);
    working_file.set_current_character(0);
    working_file.start_selection();
    
    // Move to end of file
    int last_line = working_file.get_num_lines() - 1;
    working_file.set_current_line(last_line);
    working_file.set_current_character(working_file.get_num_characters(last_line));
    working_file.update_selection();
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
        500, // Autosave delay in MS
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
    save_file();
}