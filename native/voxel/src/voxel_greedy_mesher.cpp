// voxel_greedy_mesher.cpp

#include "voxel_greedy_mesher.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>

// Erik Johansson's mesher.
// This is your 'mesher.h' / 'cgerikj_mesher.h' with CS=62 etc.
#define BM_IMPLEMENTATION
#include <cgerikj_mesher.h>

#ifndef BM_MEMSET
#define BM_MEMSET memset
#include <string.h>
#endif

using namespace godot;

// -----------------------------------------------------------------------------
// Internal per-thread buffers & helpers
// -----------------------------------------------------------------------------

// Expect 64^3 voxels from caller (X,Y,Z order).
static constexpr int VOX_SIZE  = CS_P;                          // 64
static constexpr int VOX_COUNT = VOX_SIZE * VOX_SIZE * VOX_SIZE;

// Per-thread scratch: safe to call from many worker threads simultaneously.
static thread_local MeshData           g_mesh_data;
static thread_local uint8_t            g_voxels_zxy[CS_P3];      // padded ZXY
static thread_local uint64_t           g_face_masks[CS_2 * 6];
static thread_local uint64_t           g_opaque_mask[CS_P2];
static thread_local uint8_t            g_forward_merged[CS_2];
static thread_local uint8_t            g_right_merged[CS];
static thread_local BM_VECTOR<uint64_t> g_vertices;

// Caller layout XYZ: idx = x + y*64 + z*64*64
static inline int src_index_xyz(int x, int y, int z) {
    return x + y * VOX_SIZE + z * VOX_SIZE * VOX_SIZE;
}

// Mesher layout ZXY: z is fastest axis.
static inline int dst_index_zxy(int z, int x, int y) {
    return z + x * CS_P + y * CS_P2;
}

static inline void build_opaque_mask_from_voxels(const uint8_t *voxels_zxy, uint64_t *opaque_mask) {
    for (int a = 0; a < CS_P; ++a) {
        for (int b = 0; b < CS_P; ++b) {
            const int column_index = a * CS_P + b;
            const int base_index   = a * CS_P + b * CS_P2; // z runs fastest

            uint64_t bits = 0;

            for (int c = 0; c < CS_P; ++c) {
                const uint8_t type = voxels_zxy[base_index + c];
                if (type != 0) {
                    bits |= (1ull << c);
                }
            }

            opaque_mask[column_index] = bits;
        }
    }
}

static inline void ensure_mesh_data_initialized() {
    g_mesh_data.faceMasks     = g_face_masks;
    g_mesh_data.opaqueMask    = g_opaque_mask;
    g_mesh_data.forwardMerged = g_forward_merged;
    g_mesh_data.rightMerged   = g_right_merged;
    g_mesh_data.vertices      = &g_vertices;

    if (g_mesh_data.maxVertices == 0) {
        // Conservative starting capacity; grows automatically in insertQuad().
        g_mesh_data.maxVertices = CS * CS * CS * 6;
        g_vertices.resize(g_mesh_data.maxVertices, 0);
    }
}

// -----------------------------------------------------------------------------
// Godot class implementation
// -----------------------------------------------------------------------------

void VoxelGreedyMesher::_bind_methods() {
    ClassDB::bind_method(
        D_METHOD("mesh_chunk_quads", "material64_xyz"),
        &VoxelGreedyMesher::mesh_chunk_quads
    );
}

PackedInt64Array VoxelGreedyMesher::mesh_chunk_quads(const PackedByteArray &material64_xyz) {
    PackedInt64Array out;

    // Expect exactly 64^3 bytes (your logical 62^3 lives inside this, with padding).
    if (material64_xyz.size() != 64 * 64 * 64) {
        return out;
    }

    // 1) Copy XYZ -> ZXY (whatever you already had)
    const uint8_t *src = material64_xyz.ptr();

    for (int z = 0; z < VOX_SIZE; ++z) {
        for (int x = 0; x < VOX_SIZE; ++x) {
            for (int y = 0; y < VOX_SIZE; ++y) {
                int src_idx = src_index_xyz(x, y, z);
                int dst_idx = dst_index_zxy(z, x, y);
                g_voxels_zxy[dst_idx] = src[src_idx];
            }
        }
    }

    // 2) Build opaque mask
    build_opaque_mask_from_voxels(g_voxels_zxy, g_opaque_mask);

    // 3) Prepare MeshData and call Erik's mesher
    ensure_mesh_data_initialized();
    BM_MEMSET(g_face_masks,     0, sizeof(g_face_masks));
    BM_MEMSET(g_forward_merged, 0, sizeof(g_forward_merged));
    BM_MEMSET(g_right_merged,   0, sizeof(g_right_merged));
    g_mesh_data.vertexCount = 0;

    mesh(g_voxels_zxy, g_mesh_data);

    // 4) Count quads per face
    int total_quads = 0;
    for (int face = 0; face < 6; ++face) {
        total_quads += g_mesh_data.faceVertexLength[face];
    }

    if (total_quads <= 0) {
        return out;
    }

    out.resize(total_quads);

    int dst = 0;
    BM_VECTOR<uint64_t> &verts = *g_mesh_data.vertices;

    for (int face = 0; face < 6; ++face) {
        const int begin = g_mesh_data.faceVertexBegin[face];
        const int len   = g_mesh_data.faceVertexLength[face];

        for (int i = 0; i < len; ++i) {
            uint64_t base_quad = verts[begin + i];
            // *** THIS IS THE IMPORTANT LINE ***
            // Put face (0..5) into the top 3 bits.
            uint64_t packed = base_quad | (uint64_t(face) << 61);
            out.set(dst++, (int64_t)packed);
        }
    }

    return out;
}
