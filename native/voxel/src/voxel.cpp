#include "voxel.h"

using namespace godot;

void Voxel::_bind_methods() {
    // Expose methods to Godot here.
    ClassDB::bind_method(D_METHOD("add_ints", "a", "b"), &Voxel::add_ints);

    // Later you’ll bind real voxel methods here.
}

int Voxel::add_ints(int a, int b) {
    return a + b;
}
