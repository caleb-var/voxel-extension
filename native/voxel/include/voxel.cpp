#include "voxel.h"

void Voxel::_bind_methods() {
    // Expose methods to Godot here.
    ClassDB::bind_method(D_METHOD("add_ints", "a", "b"), &Voxel::add_ints);

    // TODO: bind your real voxel algorithms here later.
}

int Voxel::add_ints(int a, int b) {
    // Placeholder logic
    return a + b;
}
