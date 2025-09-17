#include "command.h"

Command::Command(const std::string& name, const std::string& description, const std::string& action_string, const std::vector<char>& key_reqs, const std::function<void()>& action)
    : name(name), description(description), action_string(action_string), action(action), key_requirements{} {
    for (char key : key_reqs) {
        key_requirements[static_cast<size_t>(key)] = true;
    }
}

Command::Command(const std::string& name, const std::string& description, const std::string& action_string, const std::array<bool, 256>& key_reqs, const std::function<void()>& action)
    : name(name), description(description), action_string(action_string), action(action), key_requirements(key_reqs) {}