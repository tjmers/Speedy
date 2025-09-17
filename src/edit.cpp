#include "edit.h"

Edit::Edit(const std::function<bool()>& edit_function, const std::function<bool()>& undo_function, const std::function<void()>& cleanup)
    : edit_function(edit_function), undo_function(undo_function), cleanup(cleanup) {}

Edit::Edit(std::function<bool()>&& edit_function, std::function<bool()>&& undo_function, std::function<void()>&& cleanup)
    : edit_function(std::move(edit_function)), undo_function(std::move(undo_function)), cleanup(std::move(cleanup)) {

    cleanup = [] () { return; };
}

Edit::~Edit() {
    this->cleanup();
}

Edit::Edit(Edit&& other) noexcept
    : edit_function(std::move(other.edit_function)), undo_function(std::move(other.undo_function)), cleanup(std::move(other.cleanup)) {
    other.cleanup = [] () { return; };
}

Edit&& Edit::operator=(Edit&& other) noexcept {
    if (this != &other) {
        this->edit_function = std::move(other.edit_function);
        this->undo_function = std::move(other.undo_function);
        this->cleanup = std::move(other.cleanup);
        other.cleanup = [] () { return; };
    }
    return std::move(*this);
}

Edit::Edit(const Edit& other)
    : edit_function(other.edit_function), undo_function(other.undo_function), cleanup(other.cleanup) {}

Edit& Edit::operator=(const Edit& other) {
    if (this != &other) {
        this->edit_function = other.edit_function;
        this->undo_function = other.undo_function;
        this->cleanup = other.cleanup;
    }
    return *this;
}