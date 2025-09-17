#pragma once

#include <array>
#include <functional>
#include <string>
#include <vector>

class Command {

public:
    Command(const std::string& name, const std::string& description, const std::string& action_string, const std::vector<char>& key_reqs, const std::function<void()>& action);
    Command(const std::string& name, const std::string& description, const std::string& action_string, const std::array<bool, 256>& key_reqs, const std::function<void()>& action);

    inline const std::string& get_name() const { return name; }
    inline const std::string& get_description() const { return description; }
    inline const std::string& get_action_string() const { return action_string; }

    inline void execute() const { action(); }

    inline const std::array<bool, 256> get_key_requirements() const { return key_requirements; }


private:
    std::string name;
    std::string description;
    std::string action_string;
    std::function<void()> action;
    std::array<bool, 256> key_requirements; // Bitfield representing all keys that need to be pressed


};