#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/dictionary.hpp>

using namespace godot;

class BinaryGreedyMesher64 : public RefCounted {
    GDCLASS(BinaryGreedyMesher64, RefCounted);

    static constexpr int SIZE       = 64; // padded size
    static constexpr int INNER_MIN  = 1;  // logical region starts at 1
    static constexpr int INNER_MAX  = 63; // exclusive (1..62)
    static constexpr int INNER_SIZE = INNER_MAX - INNER_MIN; // 62
    static constexpr float VOXEL_SIZE = 0.5f; // match your C# mesher

protected:
    static void _bind_methods() {
        // C# usage: BinaryGreedyMesher64.MeshChunk(materialId)
        ClassDB::bind_static_method(
            "BinaryGreedyMesher64",
            D_METHOD("mesh_chunk", "voxels"),
            &BinaryGreedyMesher64::mesh_chunk
        );
    }

    // Helper: global 1D index from x,y,z (XYZ layout)
    static _ALWAYS_INLINE_ int idx(int x, int y, int z) {
        return x + y * SIZE + z * SIZE * SIZE;
    }

    // Emit one quad; face_dir: 0=+X,1=-X,2=+Y,3=-Y,4=+Z,5=-Z
    static void emit_quad_x_pos(
        int x_plane, int y0, int y1, int z0, int z1,
        uint8_t mat, int face_dir,
        PackedVector3Array &verts,
        PackedInt32Array &inds,
        PackedVector2Array &uvs
    ) {
        const int start = verts.size();

        float fx  = x_plane * VOXEL_SIZE;
        float fy0 = y0 * VOXEL_SIZE;
        float fy1 = y1 * VOXEL_SIZE;
        float fz0 = z0 * VOXEL_SIZE;
        float fz1 = z1 * VOXEL_SIZE;

        // +X faces; winding such that normal points +X
        verts.push_back(Vector3(fx, fy0, fz0)); // 0
        verts.push_back(Vector3(fx, fy1, fz0)); // 1
        verts.push_back(Vector3(fx, fy1, fz1)); // 2
        verts.push_back(Vector3(fx, fy0, fz1)); // 3

        inds.push_back(start + 0); inds.push_back(start + 2); inds.push_back(start + 1);
        inds.push_back(start + 2); inds.push_back(start + 0); inds.push_back(start + 3);

        Vector2 uv(mat, face_dir);
        uvs.push_back(uv);
        uvs.push_back(uv);
        uvs.push_back(uv);
        uvs.push_back(uv);
    }

    static void emit_quad_x_neg(
        int x_plane, int y0, int y1, int z0, int z1,
        uint8_t mat, int face_dir,
        PackedVector3Array &verts,
        PackedInt32Array &inds,
        PackedVector2Array &uvs
    ) {
        const int start = verts.size();

        float fx  = x_plane * VOXEL_SIZE;
        float fy0 = y0 * VOXEL_SIZE;
        float fy1 = y1 * VOXEL_SIZE;
        float fz0 = z0 * VOXEL_SIZE;
        float fz1 = z1 * VOXEL_SIZE;

        // -X faces; mirrored winding to point -X
        verts.push_back(Vector3(fx, fy0, fz1)); // 0
        verts.push_back(Vector3(fx, fy1, fz1)); // 1
        verts.push_back(Vector3(fx, fy1, fz0)); // 2
        verts.push_back(Vector3(fx, fy0, fz0)); // 3

        inds.push_back(start + 0); inds.push_back(start + 2); inds.push_back(start + 1);
        inds.push_back(start + 2); inds.push_back(start + 0); inds.push_back(start + 3);

        Vector2 uv(mat, face_dir);
        uvs.push_back(uv);
        uvs.push_back(uv);
        uvs.push_back(uv);
        uvs.push_back(uv);
    }

    static void emit_quad_y_pos(
        int y_plane, int x0, int x1, int z0, int z1,
        uint8_t mat, int face_dir,
        PackedVector3Array &verts,
        PackedInt32Array &inds,
        PackedVector2Array &uvs
    ) {
        const int start = verts.size();

        float fy  = y_plane * VOXEL_SIZE;
        float fx0 = x0 * VOXEL_SIZE;
        float fx1 = x1 * VOXEL_SIZE;
        float fz0 = z0 * VOXEL_SIZE;
        float fz1 = z1 * VOXEL_SIZE;

        // +Y faces
        verts.push_back(Vector3(fx0, fy, fz0)); // 0
        verts.push_back(Vector3(fx1, fy, fz0)); // 1
        verts.push_back(Vector3(fx1, fy, fz1)); // 2
        verts.push_back(Vector3(fx0, fy, fz1)); // 3

        inds.push_back(start + 0); inds.push_back(start + 2); inds.push_back(start + 1);
        inds.push_back(start + 2); inds.push_back(start + 0); inds.push_back(start + 3);

        Vector2 uv(mat, face_dir);
        uvs.push_back(uv);
        uvs.push_back(uv);
        uvs.push_back(uv);
        uvs.push_back(uv);
    }

    static void emit_quad_y_neg(
        int y_plane, int x0, int x1, int z0, int z1,
        uint8_t mat, int face_dir,
        PackedVector3Array &verts,
        PackedInt32Array &inds,
        PackedVector2Array &uvs
    ) {
        const int start = verts.size();

        float fy  = y_plane * VOXEL_SIZE;
        float fx0 = x0 * VOXEL_SIZE;
        float fx1 = x1 * VOXEL_SIZE;
        float fz0 = z0 * VOXEL_SIZE;
        float fz1 = z1 * VOXEL_SIZE;

        // -Y faces
        verts.push_back(Vector3(fx0, fy, fz1)); // 0
        verts.push_back(Vector3(fx1, fy, fz1)); // 1
        verts.push_back(Vector3(fx1, fy, fz0)); // 2
        verts.push_back(Vector3(fx0, fy, fz0)); // 3

        inds.push_back(start + 0); inds.push_back(start + 2); inds.push_back(start + 1);
        inds.push_back(start + 2); inds.push_back(start + 0); inds.push_back(start + 3);

        Vector2 uv(mat, face_dir);
        uvs.push_back(uv);
        uvs.push_back(uv);
        uvs.push_back(uv);
        uvs.push_back(uv);
    }

    static void emit_quad_z_pos(
        int z_plane, int x0, int x1, int y0, int y1,
        uint8_t mat, int face_dir,
        PackedVector3Array &verts,
        PackedInt32Array &inds,
        PackedVector2Array &uvs
    ) {
        const int start = verts.size();

        float fz  = z_plane * VOXEL_SIZE;
        float fx0 = x0 * VOXEL_SIZE;
        float fx1 = x1 * VOXEL_SIZE;
        float fy0 = y0 * VOXEL_SIZE;
        float fy1 = y1 * VOXEL_SIZE;

        // +Z faces
        verts.push_back(Vector3(fx0, fy0, fz)); // 0
        verts.push_back(Vector3(fx1, fy0, fz)); // 1
        verts.push_back(Vector3(fx1, fy1, fz)); // 2
        verts.push_back(Vector3(fx0, fy1, fz)); // 3

        inds.push_back(start + 0); inds.push_back(start + 2); inds.push_back(start + 1);
        inds.push_back(start + 2); inds.push_back(start + 0); inds.push_back(start + 3);

        Vector2 uv(mat, face_dir);
        uvs.push_back(uv);
        uvs.push_back(uv);
        uvs.push_back(uv);
        uvs.push_back(uv);
    }

    static void emit_quad_z_neg(
        int z_plane, int x0, int x1, int y0, int y1,
        uint8_t mat, int face_dir,
        PackedVector3Array &verts,
        PackedInt32Array &inds,
        PackedVector2Array &uvs
    ) {
        const int start = verts.size();

        float fz  = z_plane * VOXEL_SIZE;
        float fx0 = x0 * VOXEL_SIZE;
        float fx1 = x1 * VOXEL_SIZE;
        float fy0 = y0 * VOXEL_SIZE;
        float fy1 = y1 * VOXEL_SIZE;

        // -Z faces
        verts.push_back(Vector3(fx0, fy0, fz)); // 0
        verts.push_back(Vector3(fx0, fy1, fz)); // 1
        verts.push_back(Vector3(fx1, fy1, fz)); // 2
        verts.push_back(Vector3(fx1, fy0, fz)); // 3

        inds.push_back(start + 0); inds.push_back(start + 2); inds.push_back(start + 1);
        inds.push_back(start + 2); inds.push_back(start + 0); inds.push_back(start + 3);

        Vector2 uv(mat, face_dir);
        uvs.push_back(uv);
        uvs.push_back(uv);
        uvs.push_back(uv);
        uvs.push_back(uv);
    }

    // 2D greedy merge on mask[INNER_SIZE * INNER_SIZE].
    // mask[u + v*INNER_SIZE] holds 0 (no face) or matId.
    static void greedy_merge_2d(
        uint8_t *mask,
        int face_dir,
        int fixed_plane,
        bool axis_x,  // true if quad spans (y,z) or (x,z); false if spans (x,y) – see callers
        bool first_axis_is_row, // y or x -> row index
        PackedVector3Array &verts,
        PackedInt32Array &inds,
        PackedVector2Array &uvs,
        const PackedByteArray &voxels // not used here but kept for signature symmetry if you extend
    ) {
        // We will not use this generic version; we call separate helpers per axis
        // to keep mapping simpler. Leaving here in case you want to refactor.
    }

public:
    // Static entry point you call from C#.
    static Dictionary mesh_chunk(const PackedByteArray &voxels) {
        Dictionary result;

        const int expected_size = SIZE * SIZE * SIZE;
        if (voxels.size() != expected_size) {
            // Return empty; caller should assert size.
            result["vertices"] = PackedVector3Array();
            result["indices"]  = PackedInt32Array();
            result["uvs"]      = PackedVector2Array();
            return result;
        }

        const uint8_t *data = voxels.ptr(); // thread-safe: local pointer

        PackedVector3Array vertices;
        PackedInt32Array   indices;
        PackedVector2Array uvs;


        // Temporary 2D mask for greedy merging on each slice.
        uint8_t mask[INNER_SIZE * INNER_SIZE];

        // --------------------------------------------------
        // X+ faces (face_dir = 0)
        // --------------------------------------------------
        for (int x = INNER_MIN; x < INNER_MAX; ++x) {
            // Build mask over (y,z)
            for (int i = 0; i < INNER_SIZE * INNER_SIZE; ++i)
                mask[i] = 0;

            for (int y = INNER_MIN; y < INNER_MAX; ++y) {
                int row = (y - INNER_MIN) * INNER_SIZE;
                for (int z = INNER_MIN; z < INNER_MAX; ++z) {
                    uint8_t mat = data[idx(x, y, z)];
                    if (!mat)
                        continue;
                    if (data[idx(x + 1, y, z)] != 0)
                        continue; // neighbor not air

                    int ci = row + (z - INNER_MIN);
                    mask[ci] = mat;
                }
            }

            // Greedy merge mask into quads
            for (int y_off = 0; y_off < INNER_SIZE; ++y_off) {
                int row_base = y_off * INNER_SIZE;
                for (int z_off = 0; z_off < INNER_SIZE; ) {
                    uint8_t mat = mask[row_base + z_off];
                    if (!mat) {
                        ++z_off;
                        continue;
                    }

                    // Grow width along +Z in this row
                    int w = 1;
                    while (z_off + w < INNER_SIZE &&
                           mask[row_base + z_off + w] == mat) {
                        ++w;
                    }

                    // Grow height along +Y while rows match
                    int h = 1;
                    bool done = false;
                    while (y_off + h < INNER_SIZE && !done) {
                        int next_row_base = (y_off + h) * INNER_SIZE;
                        for (int k = 0; k < w; ++k) {
                            if (mask[next_row_base + z_off + k] != mat) {
                                done = true;
                                break;
                            }
                        }
                        if (!done)
                            ++h;
                    }

                    // Clear used cells
                    for (int yy = 0; yy < h; ++yy) {
                        int r = (y_off + yy) * INNER_SIZE;
                        for (int k = 0; k < w; ++k) {
                            mask[r + z_off + k] = 0;
                        }
                    }

                    int y0 = INNER_MIN + y_off;
                    int y1 = y0 + h;
                    int z0 = INNER_MIN + z_off;
                    int z1 = z0 + w;

                    emit_quad_x_pos(
                        x + 1, // plane at x+1
                        y0, y1,
                        z0, z1,
                        mat, 0,
                        vertices, indices, uvs
                    );

                    z_off += w;
                }
            }
        }

        // --------------------------------------------------
        // X- faces (face_dir = 1)
        // --------------------------------------------------
        for (int x = INNER_MIN; x < INNER_MAX; ++x) {
            for (int i = 0; i < INNER_SIZE * INNER_SIZE; ++i)
                mask[i] = 0;

            for (int y = INNER_MIN; y < INNER_MAX; ++y) {
                int row = (y - INNER_MIN) * INNER_SIZE;
                for (int z = INNER_MIN; z < INNER_MAX; ++z) {
                    uint8_t mat = data[idx(x, y, z)];
                    if (!mat)
                        continue;
                    if (data[idx(x - 1, y, z)] != 0)
                        continue;

                    int ci = row + (z - INNER_MIN);
                    mask[ci] = mat;
                }
            }

            for (int y_off = 0; y_off < INNER_SIZE; ++y_off) {
                int row_base = y_off * INNER_SIZE;
                for (int z_off = 0; z_off < INNER_SIZE; ) {
                    uint8_t mat = mask[row_base + z_off];
                    if (!mat) {
                        ++z_off;
                        continue;
                    }

                    int w = 1;
                    while (z_off + w < INNER_SIZE &&
                           mask[row_base + z_off + w] == mat) {
                        ++w;
                    }

                    int h = 1;
                    bool done = false;
                    while (y_off + h < INNER_SIZE && !done) {
                        int next_row_base = (y_off + h) * INNER_SIZE;
                        for (int k = 0; k < w; ++k) {
                            if (mask[next_row_base + z_off + k] != mat) {
                                done = true;
                                break;
                            }
                        }
                        if (!done)
                            ++h;
                    }

                    for (int yy = 0; yy < h; ++yy) {
                        int r = (y_off + yy) * INNER_SIZE;
                        for (int k = 0; k < w; ++k) {
                            mask[r + z_off + k] = 0;
                        }
                    }

                    int y0 = INNER_MIN + y_off;
                    int y1 = y0 + h;
                    int z0 = INNER_MIN + z_off;
                    int z1 = z0 + w;

                    emit_quad_x_neg(
                        x, // plane at x
                        y0, y1,
                        z0, z1,
                        mat, 1,
                        vertices, indices, uvs
                    );

                    z_off += w;
                }
            }
        }

        // --------------------------------------------------
        // Y+ faces (face_dir = 2)
        // --------------------------------------------------
        for (int y = INNER_MIN; y < INNER_MAX; ++y) {
            // mask over (x,z)
            for (int i = 0; i < INNER_SIZE * INNER_SIZE; ++i)
                mask[i] = 0;

            for (int x = INNER_MIN; x < INNER_MAX; ++x) {
                int row = (x - INNER_MIN) * INNER_SIZE;
                for (int z = INNER_MIN; z < INNER_MAX; ++z) {
                    uint8_t mat = data[idx(x, y, z)];
                    if (!mat)
                        continue;
                    if (data[idx(x, y + 1, z)] != 0)
                        continue;

                    int ci = row + (z - INNER_MIN);
                    mask[ci] = mat;
                }
            }

            for (int x_off = 0; x_off < INNER_SIZE; ++x_off) {
                int row_base = x_off * INNER_SIZE;
                for (int z_off = 0; z_off < INNER_SIZE; ) {
                    uint8_t mat = mask[row_base + z_off];
                    if (!mat) {
                        ++z_off;
                        continue;
                    }

                    int w = 1;
                    while (z_off + w < INNER_SIZE &&
                           mask[row_base + z_off + w] == mat) {
                        ++w;
                    }

                    int h = 1;
                    bool done = false;
                    while (x_off + h < INNER_SIZE && !done) {
                        int next_row_base = (x_off + h) * INNER_SIZE;
                        for (int k = 0; k < w; ++k) {
                            if (mask[next_row_base + z_off + k] != mat) {
                                done = true;
                                break;
                            }
                        }
                        if (!done)
                            ++h;
                    }

                    for (int xx = 0; xx < h; ++xx) {
                        int r = (x_off + xx) * INNER_SIZE;
                        for (int k = 0; k < w; ++k) {
                            mask[r + z_off + k] = 0;
                        }
                    }

                    int x0 = INNER_MIN + x_off;
                    int x1 = x0 + h;
                    int z0 = INNER_MIN + z_off;
                    int z1 = z0 + w;

                    emit_quad_y_pos(
                        y + 1,
                        x0, x1,
                        z0, z1,
                        mat, 2,
                        vertices, indices, uvs
                    );

                    z_off += w;
                }
            }
        }

        // --------------------------------------------------
        // Y- faces (face_dir = 3)
        // --------------------------------------------------
        for (int y = INNER_MIN; y < INNER_MAX; ++y) {
            for (int i = 0; i < INNER_SIZE * INNER_SIZE; ++i)
                mask[i] = 0;

            for (int x = INNER_MIN; x < INNER_MAX; ++x) {
                int row = (x - INNER_MIN) * INNER_SIZE;
                for (int z = INNER_MIN; z < INNER_MAX; ++z) {
                    uint8_t mat = data[idx(x, y, z)];
                    if (!mat)
                        continue;
                    if (data[idx(x, y - 1, z)] != 0)
                        continue;

                    int ci = row + (z - INNER_MIN);
                    mask[ci] = mat;
                }
            }

            for (int x_off = 0; x_off < INNER_SIZE; ++x_off) {
                int row_base = x_off * INNER_SIZE;
                for (int z_off = 0; z_off < INNER_SIZE; ) {
                    uint8_t mat = mask[row_base + z_off];
                    if (!mat) {
                        ++z_off;
                        continue;
                    }

                    int w = 1;
                    while (z_off + w < INNER_SIZE &&
                           mask[row_base + z_off + w] == mat) {
                        ++w;
                    }

                    int h = 1;
                    bool done = false;
                    while (x_off + h < INNER_SIZE && !done) {
                        int next_row_base = (x_off + h) * INNER_SIZE;
                        for (int k = 0; k < w; ++k) {
                            if (mask[next_row_base + z_off + k] != mat) {
                                done = true;
                                break;
                            }
                        }
                        if (!done)
                            ++h;
                    }

                    for (int xx = 0; xx < h; ++xx) {
                        int r = (x_off + xx) * INNER_SIZE;
                        for (int k = 0; k < w; ++k) {
                            mask[r + z_off + k] = 0;
                        }
                    }

                    int x0 = INNER_MIN + x_off;
                    int x1 = x0 + h;
                    int z0 = INNER_MIN + z_off;
                    int z1 = z0 + w;

                    emit_quad_y_neg(
                        y,
                        x0, x1,
                        z0, z1,
                        mat, 3,
                        vertices, indices, uvs
                    );

                    z_off += w;
                }
            }
        }

        // --------------------------------------------------
        // Z+ faces (face_dir = 4)
        // --------------------------------------------------
        for (int z = INNER_MIN; z < INNER_MAX; ++z) {
            for (int i = 0; i < INNER_SIZE * INNER_SIZE; ++i)
                mask[i] = 0;

            for (int x = INNER_MIN; x < INNER_MAX; ++x) {
                int row = (x - INNER_MIN) * INNER_SIZE;
                for (int y = INNER_MIN; y < INNER_MAX; ++y) {
                    uint8_t mat = data[idx(x, y, z)];
                    if (!mat)
                        continue;
                    if (data[idx(x, y, z + 1)] != 0)
                        continue;

                    int ci = row + (y - INNER_MIN);
                    mask[ci] = mat;
                }
            }

            for (int x_off = 0; x_off < INNER_SIZE; ++x_off) {
                int row_base = x_off * INNER_SIZE;
                for (int y_off = 0; y_off < INNER_SIZE; ) {
                    uint8_t mat = mask[row_base + y_off];
                    if (!mat) {
                        ++y_off;
                        continue;
                    }

                    int w = 1;
                    while (y_off + w < INNER_SIZE &&
                           mask[row_base + y_off + w] == mat) {
                        ++w;
                    }

                    int h = 1;
                    bool done = false;
                    while (x_off + h < INNER_SIZE && !done) {
                        int next_row_base = (x_off + h) * INNER_SIZE;
                        for (int k = 0; k < w; ++k) {
                            if (mask[next_row_base + y_off + k] != mat) {
                                done = true;
                                break;
                            }
                        }
                        if (!done)
                            ++h;
                    }

                    for (int xx = 0; xx < h; ++xx) {
                        int r = (x_off + xx) * INNER_SIZE;
                        for (int k = 0; k < w; ++k) {
                            mask[r + y_off + k] = 0;
                        }
                    }

                    int x0 = INNER_MIN + x_off;
                    int x1 = x0 + h;
                    int y0 = INNER_MIN + y_off;
                    int y1 = y0 + w;

                    emit_quad_z_pos(
                        z + 1,
                        x0, x1,
                        y0, y1,
                        mat, 4,
                        vertices, indices, uvs
                    );

                    y_off += w;
                }
            }
        }

        // --------------------------------------------------
        // Z- faces (face_dir = 5)
        // --------------------------------------------------
        for (int z = INNER_MIN; z < INNER_MAX; ++z) {
            for (int i = 0; i < INNER_SIZE * INNER_SIZE; ++i)
                mask[i] = 0;

            for (int x = INNER_MIN; x < INNER_MAX; ++x) {
                int row = (x - INNER_MIN) * INNER_SIZE;
                for (int y = INNER_MIN; y < INNER_MAX; ++y) {
                    uint8_t mat = data[idx(x, y, z)];
                    if (!mat)
                        continue;
                    if (data[idx(x, y, z - 1)] != 0)
                        continue;

                    int ci = row + (y - INNER_MIN);
                    mask[ci] = mat;
                }
            }

            for (int x_off = 0; x_off < INNER_SIZE; ++x_off) {
                int row_base = x_off * INNER_SIZE;
                for (int y_off = 0; y_off < INNER_SIZE; ) {
                    uint8_t mat = mask[row_base + y_off];
                    if (!mat) {
                        ++y_off;
                        continue;
                    }

                    int w = 1;
                    while (y_off + w < INNER_SIZE &&
                           mask[row_base + y_off + w] == mat) {
                        ++w;
                    }

                    int h = 1;
                    bool done = false;
                    while (x_off + h < INNER_SIZE && !done) {
                        int next_row_base = (x_off + h) * INNER_SIZE;
                        for (int k = 0; k < w; ++k) {
                            if (mask[next_row_base + y_off + k] != mat) {
                                done = true;
                                break;
                            }
                        }
                        if (!done)
                            ++h;
                    }

                    for (int xx = 0; xx < h; ++xx) {
                        int r = (x_off + xx) * INNER_SIZE;
                        for (int k = 0; k < w; ++k) {
                            mask[r + y_off + k] = 0;
                        }
                    }

                    int x0 = INNER_MIN + x_off;
                    int x1 = x0 + h;
                    int y0 = INNER_MIN + y_off;
                    int y1 = y0 + w;

                    emit_quad_z_neg(
                        z,
                        x0, x1,
                        y0, y1,
                        mat, 5,
                        vertices, indices, uvs
                    );

                    y_off += w;
                }
            }
        }

        result["vertices"] = vertices;
        result["indices"]  = indices;
        result["uvs"]      = uvs;
        return result;
    }
};

// Registration
extern "C" void initialize_binary_greedy_mesher64(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
    ClassDB::register_class<BinaryGreedyMesher64>();
}
