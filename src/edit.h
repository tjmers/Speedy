#pragma once

#include <functional>

class Edit {
    
public:
    Edit(const std::function<bool()>& edit_function, const std::function<bool()>& undo_function, const std::function<void()>& cleanup = [](){});
    Edit(std::function<bool()>&& edit_function, std::function<bool()>&& undo_function, std::function<void()>&& cleanup = [](){});

    Edit(const Edit&);
    Edit(Edit&& other) noexcept;
    Edit& operator=(const Edit&);
    Edit&& operator=(Edit&& other) noexcept;

    inline bool edit() { return this->edit_function(); }
    inline bool undo() { return this->undo_function(); }

    ~Edit();

private:
    std::function<bool()> edit_function;
    std::function<bool()> undo_function;
    std::function<void()> cleanup;
    
};