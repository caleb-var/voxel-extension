#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>

namespace godot {

class Voxel : public RefCounted {
    GDCLASS(Voxel, RefCounted);

protected:
    static void _bind_methods();

public:
    Voxel() = default;
    ~Voxel() = default;

    int add_ints(int a, int b);
};

} // namespace godot
