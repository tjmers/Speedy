#include "config.h"
#include <fstream>

Config* Config::instance = nullptr;

// Default constructor initializing default values (order matches header decls)
Config::Config() : 
    background_color(D2D1::ColorF(0.2f, 0.2f, 0.2f, 1.0f)),
    line_spacing(1.0f),
    character_spacing(1.0f),
    tab_size(4),
    show_line_numbers(true),
    line_number_color(D2D1::ColorF(D2D1::ColorF::Gray)),
    indicator_color(D2D1::ColorF(D2D1::ColorF::Red)),
    left_margin(10),
    explorer_width(0),
    font_size(16.0f),
    font_family("Courier New"),
    text_color(D2D1::ColorF(0.9f, 0.9f, 0.9f, 1.0f)),
    recent_files(),
    last_opened_file(""),
    working_directory(""),
    undo_history_size(50),
    selection_color(D2D1::ColorF(0.2f, 0.5f, 1.0f, 0.3f))  // Semi-transparent blue for selections
{}

void Config::create() {
    instance = new Config();

    // Load all variables from the config file here.

    instance->load("./config/config.cfg");
}

void Config::destroy() {
    delete instance;
    instance = nullptr;
}

bool Config::exists() {
    return instance != nullptr;
}

void Config::save() {
    // Save all variables to the config file here.
    std::ofstream config_file("./config/config.cfg");
    if (config_file.is_open()) {
        config_file << "background_color " << background_color.r << " " << background_color.g << " " << background_color.b << " " << background_color.a << "\n";
        config_file << "line_spacing " << line_spacing << "\n";
        config_file << "character_spacing " << character_spacing << "\n";
        config_file << "tab_size " << tab_size << "\n";
        config_file << "show_line_numbers " << show_line_numbers << "\n";
        config_file << "line_number_color " << line_number_color.r << " " << line_number_color.g << " " << line_number_color.b << " " << line_number_color.a << "\n";
        config_file << "indicator_color " << indicator_color.r << " " << indicator_color.g << " " << indicator_color.b << " " << indicator_color.a << "\n";
        config_file << "left_margin " << left_margin << "\n";
        config_file << "explorer_width " << explorer_width << "\n";
        config_file << "font_size " << font_size << "\n";
        config_file << "font_family " << font_family << "\n";
        config_file << "text_color " << text_color.r << " " << text_color.g << " " << text_color.b << " " << text_color.a << "\n";
        config_file << "recent_files";
        for (const auto& file : recent_files) {
            config_file << " " << file;
        }
        config_file << "\n";
        config_file << "last_opened_file " << last_opened_file << "\n";
        config_file << "working_directory " << working_directory << "\n";
        config_file << "undo_history_size " << undo_history_size << "\n";
        config_file << "selection_color " << selection_color.r << " " << selection_color.g << " " << selection_color.b << " " << selection_color.a << "\n";
        config_file.close();
    }
}

bool Config::load(const std::string& file_name) {
    // Load all variables from the config file here.
    std::ifstream config_file(file_name);
    if (!config_file.is_open()) {
        return false;
    }
    std::string key;
    while (config_file >> key) {
        if (key == "background_color") {
            float r, g, b, a;
            config_file >> r >> g >> b >> a;
            background_color = D2D1::ColorF(r, g, b, a);
        } else if (key == "line_spacing") {
            config_file >> line_spacing;
        } else if (key == "character_spacing") {
            config_file >> character_spacing;
        } else if (key == "tab_size") {
            config_file >> tab_size;
        } else if (key == "show_line_numbers") {
            config_file >> show_line_numbers;
        } else if (key == "line_number_color") {
            float r, g, b, a;
            config_file >> r >> g >> b >> a;
            line_number_color = D2D1::ColorF(r, g, b, a);
        } else if (key == "indicator_color") {
            float r, g, b, a;
            config_file >> r >> g >> b >> a;
            indicator_color = D2D1::ColorF(r, g, b, a);
        } else if (key == "left_margin") {
            config_file >> left_margin;
        } else if (key == "explorer_width") {
            config_file >> explorer_width;
        } else if (key == "font_size") {
            config_file >> font_size;
        } else if (key == "font_family") {
            // Font family can be multiple words
            while (config_file.peek() == ' ') config_file.get(); // Skip spaces
            std::getline(config_file, font_family);
        } else if (key == "text_color") {
            float r, g, b, a;
            config_file >> r >> g >> b >> a;
            text_color = D2D1::ColorF(r, g, b, a);
        } else if (key == "recent_files") {
            recent_files.clear();
            std::string file;
            while (config_file.peek() != '\n' && config_file >> file) {
                recent_files.push_back(file);
            }
        } else if (key == "last_opened_file") {
            config_file >> last_opened_file;
        } else if (key == "working_directory") {
            config_file >> working_directory;
        } else if (key == "undo_history_size") {
            config_file >> undo_history_size;
        } else if (key == "selection_color") {
            float r, g, b, a;
            config_file >> r >> g >> b >> a;
            selection_color = D2D1::ColorF(r, g, b, a);
        } else {
            // Unknown key, skip the rest of the line
            std::string rest_of_line;
            std::getline(config_file, rest_of_line);
        }
    }
    return true;
}



Config* Config::get_instance() {
    return instance;
}