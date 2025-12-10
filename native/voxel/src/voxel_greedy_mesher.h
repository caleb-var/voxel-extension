// voxel_greedy_mesher.h
#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/packed_int64_array.hpp>

using namespace godot;

/// GDExtension wrapper around Erik Johansson's greedy mesher.
/// Thread-safe via thread_local scratch buffers.
/// Designed to minimise per-thread data: voxels in, packed quads out.
class VoxelGreedyMesher : public RefCounted {
    GDCLASS(VoxelGreedyMesher, RefCounted);

protected:
    static void _bind_methods();

public:
    VoxelGreedyMesher() = default;
    ~VoxelGreedyMesher() = default;

    /// Mesh a 64^3 XYZ chunk into packed quads (uint64).
    ///
    /// material64_xyz:
    ///   - PackedByteArray of length 64^3 (262144).
    ///   - Layout: idx = x + y*64 + z*64*64 (same as your C# New_RunMesher64).
    ///   - Includes 1-voxel padding -> logical 62^3 mesh.
    ///
    /// Returns:
    ///   - PackedInt64Array of packed quads.
    ///   - Bits: [63......32 | 31..24 | 23..18 | 17..12 | 11..6 | 5..0]
    ///            type       |    h   |    w   |    z   |   y   |  x
    ///   - You already have C# vertex-pulling code: reuse it on these quads.
    PackedInt64Array mesh_chunk_quads(const PackedByteArray &material64_xyz);
};
