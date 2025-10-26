#pragma once
#include <string>
#include <vector>
#include <utility>

struct Diff {
    enum class Operation { DEL, INSERT, EQUAL } operation;
    std::string text;

    Diff(Operation op, const std::string& t) : operation(op), text(t) {}
};

class diff_match_patch {
public:
    diff_match_patch();

    // Returns a vector of Diff objects between two strings
    std::vector<Diff> diff_main(const std::string& text1, const std::string& text2);

    // Convert a vector of Diff objects back into a single string
    std::string diff_text2(const std::vector<Diff>& diffs);
};
