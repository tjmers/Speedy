#pragma once

#include <functional>

class Edit {
    
public:
    Edit(const std::function<bool()>& edit_function, const std::function<bool()>& undo_function);

    inline bool edit() { return this->edit_function(); }
    inline bool undo() { return this->undo_function(); }

private:
    std::function<bool()> edit_function;
    std::function<bool()> undo_function;
    
};