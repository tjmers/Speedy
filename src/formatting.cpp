#include "formatting.h"
#include <algorithm>

void FormattingManager::add_formatting(int start_line, int start_char, int end_line, int end_char, FormatType type) {
    // Remove any existing formatting of the same type in this range
    remove_formatting(start_line, start_char, end_line, end_char, type);
    
    // Add the new formatting
    format_ranges.emplace_back(start_line, start_char, end_line, end_char, type);
}

void FormattingManager::remove_formatting(int start_line, int start_char, int end_line, int end_char, FormatType type) {
    format_ranges.erase(
        std::remove_if(format_ranges.begin(), format_ranges.end(),
            [start_line, start_char, end_line, end_char, type](const FormatRange& range) {
                return range.type == type &&
                       range.start_line == start_line &&
                       range.start_char == start_char &&
                       range.end_line == end_line &&
                       range.end_char == end_char;
            }),
        format_ranges.end()
    );
}

std::vector<FormatRange> FormattingManager::get_formatting_at(int line, int char_pos) const {
    std::vector<FormatRange> result;
    
    for (const auto& range : format_ranges) {
        // Check if the position is within this range
        bool in_range = false;
        
        if (range.start_line == range.end_line) {
            // Single line range
            if (line == range.start_line && char_pos >= range.start_char && char_pos < range.end_char) {
                in_range = true;
            }
        } else {
            // Multi-line range
            if (line == range.start_line && char_pos >= range.start_char) {
                in_range = true;
            } else if (line == range.end_line && char_pos < range.end_char) {
                in_range = true;
            } else if (line > range.start_line && line < range.end_line) {
                in_range = true;
            }
        }
        
        if (in_range) {
            result.push_back(range);
        }
    }
    
    return result;
}

void FormattingManager::clear_formatting() {
    format_ranges.clear();
}
