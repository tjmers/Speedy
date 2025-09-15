#pragma once

#include <functional>
#include <unordered_map>
#include <vector>

#include "client.h"
#include "command.h"


class CommandController {

public:

    static inline CommandController* get_instance() {
        return instance;
    }

    static void init(Client* c);

    bool run_commands() const;

    void save_commands() const;

private:

    void get_default_commands();

    CommandController(Client* c);

    static CommandController* instance;

    bool load_commands();

    std::vector<Command> commands;

    Client* client;


};