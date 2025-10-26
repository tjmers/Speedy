#include "diff_match_patch.h"
#include <algorithm>

diff_match_patch::diff_match_patch() {}

std::vector<Diff> diff_match_patch::diff_main(const std::string& text1, const std::string& text2) {
    std::vector<Diff> diffs;

    size_t i = 0;
    size_t min_len = std::min(text1.size(), text2.size());

    // Find common prefix
    while (i < min_len && text1[i] == text2[i]) {
        ++i;
    }
    if (i > 0) {
        diffs.push_back(Diff(Diff::Operation::EQUAL, text1.substr(0, i)));
    }

    // Find common suffix
    size_t j = 0;
    while (j < min_len - i && text1[text1.size() - 1 - j] == text2[text2.size() - 1 - j]) {
        ++j;
    }

    // Middle parts are different
    std::string mid1 = text1.substr(i, text1.size() - i - j);
    std::string mid2 = text2.substr(i, text2.size() - i - j);

    if (!mid1.empty()) diffs.push_back(Diff(Diff::Operation::DEL, mid1));
    if (!mid2.empty()) diffs.push_back(Diff(Diff::Operation::INSERT, mid2));

    // Add common suffix
    if (j > 0) {
        diffs.push_back(Diff(Diff::Operation::EQUAL, text1.substr(text1.size() - j)));
    }

    return diffs;
}

std::string diff_match_patch::diff_text2(const std::vector<Diff>& diffs) {
    std::string result;
    for (const auto& d : diffs) {
        if (d.operation != Diff::Operation::DEL) {
            result += d.text;
        }
    }
    return result;
}
