#include "selection.h"
#include <algorithm>

Selection::Selection()
    : is_active(false),
      start_line(0),
      start_char(0),
      end_line(0),
      end_char(0) {}

void Selection::start_selection(int line, int character) {
    is_active = true;
    start_line = line;
    start_char = character;
    end_line = line;
    end_char = character;
}

void Selection::update_selection(int line, int character) {
    if (!is_active) {
        start_selection(line, character);
        return;
    }
    end_line = line;
    end_char = character;
}

void Selection::clear_selection() {
    is_active = false;
}

void Selection::get_normalized_range(int& out_start_line, int& out_start_char, 
                                     int& out_end_line, int& out_end_char) const {
    // Normalize so that start is always before end
    if (start_line < end_line || (start_line == end_line && start_char <= end_char)) {
        out_start_line = start_line;
        out_start_char = start_char;
        out_end_line = end_line;
        out_end_char = end_char;
    } else {
        out_start_line = end_line;
        out_start_char = end_char;
        out_end_line = start_line;
        out_end_char = start_char;
    }
}