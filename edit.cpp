#include "edit.h"

Edit::Edit(const std::function<bool()>& edit_function, const std::function<bool()>& undo_function)
    : edit_function(edit_function), undo_function(undo_function) {}