#pragma once

#include <d2d1.h>

#include <string>
#include <vector>

/// @brief Singleton class to manage configuration settings for the application.
class Config {

public:

    // Delete copy constructor and assignment operator to prevent copies of the singleton.
    Config(Config const&) = delete;
    void operator=(Config const&) = delete;

    static void create();

    static void destroy();

    static bool exists();

    void save();

    bool load(const std::string& file_path);

    static Config* get_instance();

    // Getters and setters for configuration properties
    inline D2D1::ColorF get_background_color() const { return background_color; }
    inline void set_background_color(const D2D1::ColorF& color) { background_color = color; }
    inline float get_line_spacing() const { return line_spacing; }
    inline void set_line_spacing(const float spacing) { line_spacing = spacing; }
    inline float get_character_spacing() const { return character_spacing; }
    inline void set_character_spacing(const float spacing) { character_spacing = spacing; }
    inline int get_tab_size() const { return tab_size; }
    inline void set_tab_size(const int size) { tab_size = size; }
    inline bool get_show_line_numbers() const { return show_line_numbers; }
    inline void set_show_line_numbers(const bool show) { show_line_numbers = show; }
    inline D2D1::ColorF get_line_number_color() const { return line_number_color; }
    inline void set_line_number_color(const D2D1::ColorF& color) { line_number_color = color; }
    inline D2D1::ColorF get_indicator_color() const { return indicator_color; }
    inline void set_indicator_color(const D2D1::ColorF& color) { indicator_color = color; }
    inline int get_left_margin() const { return left_margin; }
    inline void set_left_margin(const int margin) { left_margin = margin; }
    inline int get_explorer_width() const { return explorer_width; }
    inline void set_explorer_width(const int width) { explorer_width = width; }
    inline float get_font_size() const { return font_size; }
    inline void set_font_size(const float size) { font_size = size; }
    inline std::string get_font_family() const { return font_family; }
    inline void set_font_family(const std::string& family) { font_family = family; }
    inline D2D1::ColorF get_text_color() const { return text_color; }
    inline void set_text_color(const D2D1::ColorF& color) { text_color = color; }
    inline std::vector<std::string> get_recent_files() const { return recent_files; }
    inline void set_recent_files(const std::vector<std::string>& files) { recent_files = files; }
    inline std::string get_last_opened_file() const { return last_opened_file; }
    inline void set_last_opened_file(const std::string& file) { last_opened_file = file; }
    inline std::string get_working_directory() const { return working_directory; }
    inline void set_working_directory(const std::string& directory) { working_directory = directory; }

private:
    Config(); // Private constructor to prevent instantiation.

    static Config* instance; // Static instance pointer.

    // Properties
    D2D1::ColorF background_color;
    float line_spacing;
    float character_spacing;
    int tab_size;
    bool show_line_numbers;
    D2D1::ColorF line_number_color;
    D2D1::ColorF indicator_color;
    int left_margin;
    int explorer_width;

    float font_size;
    std::string font_family;
    D2D1::ColorF text_color;

    std::vector<std::string> recent_files;
    std::string last_opened_file;
    std::string working_directory;
};