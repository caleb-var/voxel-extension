extends Node3D

@export var iterations: int = 100
@export var run_on_ready: bool = true

const SIZE := 64

func _ready() -> void:
	if run_on_ready:
		run_benchmark()


func _set_voxel(vox: PackedByteArray, x: int, y: int, z: int, v: int) -> void:
	var idx := x + y * SIZE + z * SIZE * SIZE
	vox[idx] = v


func run_benchmark() -> void:
	print("=== GDScript Native Mesher Benchmark ===")

	# 1) Check if the Voxel class is actually registered
	var exists := ClassDB.class_exists("Voxel")
	print("Class 'Voxel' exists? ", exists)
	if not exists:
		push_error("Voxel GDExtension class not found. Check register_class and .gdextension setup.")
		return

	# 2) Build a simple test chunk: 64^3 padded, inner 62^3 logical.
	var vox := PackedByteArray()
	vox.resize(SIZE * SIZE * SIZE)

	# Simple terrain: solid column up to some height
	for z in SIZE:
		for x in SIZE:
			var height := 20
			for y in SIZE:
				if y <= height:
					_set_voxel(vox, x, y, z, 1)
				else:
					_set_voxel(vox, x, y, z, 0)

	# 3) Warmup + benchmark
	var last_vertices: PackedVector3Array = PackedVector3Array()
	var last_indices: PackedInt32Array = PackedInt32Array()
	var last_uvs: PackedVector2Array = PackedVector2Array()

	print("Calling Voxel.mesh_chunk once (warmup)...")

	var t0 := Time.get_ticks_msec()
	var result := Voxel.mesh_chunk(vox)    # if bound as STATIC
	var t1 := Time.get_ticks_msec()
	print("Warmup took: %s ms" % (t1 - t0))

	last_vertices = result["vertices"]
	last_indices  = result["indices"]
	last_uvs      = result["uvs"]

	var total_ms := 0.0
	var min_ms := INF
	var max_ms := 0.0

	for i in iterations:
		var start := Time.get_ticks_msec()
		result = Voxel.mesh_chunk(vox)
		var end := Time.get_ticks_msec()

		var ms := float(end - start)
		total_ms += ms
		if ms < min_ms:
			min_ms = ms
		if ms > max_ms:
			max_ms = ms

	var avg_ms := total_ms / float(iterations)

	last_vertices = result["vertices"]
	last_indices  = result["indices"]
	last_uvs      = result["uvs"]

	print("=== Native mesher results ===")
	print("Iterations: ", iterations)
	print("avg mesh time: ", "%.4f ms" % avg_ms)
	print("min mesh time: ", "%.4f ms" % min_ms)
	print("max mesh time: ", "%.4f ms" % max_ms)
	print("last vertex count: ", last_vertices.size())
	print("last index count:  ", last_indices.size())
	print("last uv count:     ", last_uvs.size())

	# 4) Build and display a mesh from the last result
	if last_vertices.size() > 0:
		var arrays := []
		arrays.resize(Mesh.ARRAY_MAX)

		arrays[Mesh.ARRAY_VERTEX] = last_vertices
		arrays[Mesh.ARRAY_INDEX]  = last_indices
		arrays[Mesh.ARRAY_TEX_UV] = last_uvs

		var mesh := ArrayMesh.new()
		mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, arrays)

		var mi := MeshInstance3D.new()
		mi.mesh = mesh
		mi.transform = Transform3D(Basis(), Vector3.ZERO)

		var shader := Shader.new()
		shader.code = """
			shader_type spatial;
			render_mode wireframe;
		"""
		var mat := ShaderMaterial.new()
		mat.shader = shader
		mi.material_override = mat

		add_child(mi)
