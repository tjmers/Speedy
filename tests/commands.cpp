#include <iostream>
#include <stdlib.h>

#include "../src/client.h"
#include "../src/command_controller.h"
#include "../src/config.h"
#include "../src/opened_file.h"

int test_no = 0;

template <typename T>
void assert_equals(const T& expected_value, const T& tested_value) {
    if (expected_value != tested_value) {
        std::cerr << "Test case " << test_no << " failed.\nExpected: " << expected_value << "\nResult: " << tested_value << '\n';
        // Terminate program
        std::exit(1);
    }
    ++test_no;
}

// // Add support to print wstring
// std::ostream& operator<<(std::ostream& os, const std::wstring& str) {
//     std::string s(str.begin(), str.end());
//     return os << s;
// }

template <>
void assert_equals(const std::wstring& w_expected, const std::wstring& w_tested) {
    if (w_expected != w_tested) {
        std::wcerr << L"Test case " << test_no << L" failed.\nExpected: " << w_expected << L"\nResult: " << w_tested << '\n';
        std::exit(1);
    }
    ++test_no;
}

void test_delete(Client& c);
void test_undo(Client& c);

int main() {
    Client::init();
    Config::create();
    Client c;
    CommandController::init(&c);

    // Open an empty file
    // TODO Delete the file if it exists
    c.open_file("./test/commands.txt");

    test_undo(c);



    Config::destroy();

    std::cout << "All " << test_no << " test cases passed\n";
    return 0;
}

void test_delete(Client& c) {
    // OpenedFile& of = c.get_working_file();
}

void test_undo(Client& c) {
    OpenedFile& of = c.get_working_file();

    of.insert_character('A');
    of.insert_character('B');
    of.insert_character('C');
    
    assert_equals(3, of.get_current_character_index());
    assert_equals(std::wstring(L"ABC"), of.get_current_line_contents());
    of.undo();
    assert_equals(2, of.get_current_character_index());
    assert_equals(std::wstring(L"AB"), of.get_current_line_contents());
    of.undo();
    assert_equals(1, of.get_current_character_index());
    assert_equals(std::wstring(L"A"), of.get_current_line_contents());
    of.undo();
    assert_equals(0, of.get_current_character_index());
    assert_equals(std::wstring(), of.get_current_line_contents());
    of.undo();
    assert_equals(0, of.get_current_character_index());
    assert_equals(std::wstring(), of.get_current_line_contents());
}