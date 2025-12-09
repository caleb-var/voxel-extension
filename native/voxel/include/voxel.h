#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

// Shell class: later we can add mesher/generator entrypoints here.
// For now just a trivial method to prove the wiring works.
class Voxel : public RefCounted {
    GDCLASS(Voxel, RefCounted);

protected:
    static void _bind_methods();

public:
    Voxel() = default;
    ~Voxel() = default;

    int add_ints(int a, int b);
};
