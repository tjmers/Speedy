#include <iostream>
#include <stdlib.h>

#include "test.h"

#include "../src/client.h"
#include "../src/command_controller.h"
#include "../src/config.h"
#include "../src/opened_file.h"


void test_delete(Client& c);
void test_undo(Client& c);

int main() {
    Client::init();
    Config::create();
    CommandController::init(Client::get_instance());

    // Open an empty file
    // TODO Delete the file if it exists
    Client::get_instance()->open_file("./test/commands.txt");

    test_undo(*Client::get_instance());


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

