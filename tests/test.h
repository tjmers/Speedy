#pragma once

#include <iostream>

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

template <>
void assert_equals(const std::wstring& w_expected, const std::wstring& w_tested) {
    if (w_expected != w_tested) {
        std::wcerr << L"Test case " << test_no << L" failed.\nExpected: " << w_expected << L"\nResult: " << w_tested << '\n';
        std::exit(1);
    }
    ++test_no;
}