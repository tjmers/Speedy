#pragma once

#include <vector>
#include <map>

enum class FormatType {
    BOLD,
    ITALIC,
    UNDERLINE,
    HIGHLIGHT
};

struct FormatRange {
    int start_line;
    int start_char;
    int end_line;
    int end_char;
    FormatType type;
    
    FormatRange(int sl, int sc, int el, int ec, FormatType t)
        : start_line(sl), start_char(sc), end_line(el), end_char(ec), type(t) {}
};

class FormattingManager {
public:
    void add_formatting(int start_line, int start_char, int end_line, int end_char, FormatType type);
    void remove_formatting(int start_line, int start_char, int end_line, int end_char, FormatType type);
    std::vector<FormatRange> get_formatting_at(int line, int char_pos) const;
    void clear_formatting();
    
    // Get all formatting ranges (for efficient line-based access)
    const std::vector<FormatRange>& get_all_ranges() const { return format_ranges; }
    
private:
    std::vector<FormatRange> format_ranges;
};
