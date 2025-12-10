using System;
using System.Collections.Generic;
using Godot;

/// <summary>
/// Single voxel material definition.
/// ID 0 is reserved for "air" / empty.
/// </summary>
public class VoxelMaterial
{
	public byte Id;           // 0..255
	public string Name;       // e.g. "grass", "stone"

	public Color Albedo;      // base color
	public float Roughness;   // 0 = smooth, 1 = rough
	public float Metallic;    // 0 = dielectric, 1 = metal
	public float Emissive;    // simple emissive strength

	public bool IsSolid;      // for collision/meshing logic later
}

/// <summary>
/// Palette of up to 256 voxel materials.
/// Index 0 is reserved as "air" and should be transparent / unused.
///
/// - Lets you register materials and get their IDs to store in materialId[].
/// </summary>
public class VoxelPalette
{
	public const int MaxMaterials = 255;

		// Full material data if/when you need it.
		private readonly VoxelMaterial[] _materials = new VoxelMaterial[MaxMaterials];

	// Simple name lookup (optional, but handy).
	private readonly Dictionary<string, byte> _nameToId =
		new Dictionary<string, byte>(StringComparer.OrdinalIgnoreCase);

	// Next free ID (0 reserved for air).
	private byte _nextId = 1;

		public VoxelMaterial[] Materials => _materials;

	public VoxelPalette()
	{
		// ID 0 = air: leave default black/transparent.
				var air = new VoxelMaterial
				{
						Id = 0,
						Name = "air",
						Albedo = new Color(0, 0, 0, 0),
						Roughness = 1.0f,
						Metallic = 0.0f,
						Emissive = 0.0f,
						IsSolid = false
				};
				_materials[0] = air;
				_nameToId["air"] = 0;
		}

	/// <summary>
	/// Register a new material and return its ID (1..255).
	/// Throws if we run out of IDs.
	/// </summary>
	public byte RegisterMaterial(
		string name,
		Color albedo,
		float roughness = 0.8f,
		float metallic = 0.0f,
		float emissive = 0.0f,
		bool isSolid = true
	)
	{
		if (string.IsNullOrWhiteSpace(name))
			throw new ArgumentException("Material name cannot be null or empty.", nameof(name));

		if (_nextId >= MaxMaterials)
			throw new InvalidOperationException("VoxelPalette is full (max 256 materials).");

		// If name exists, return existing ID
		if (_nameToId.TryGetValue(name, out byte existing))
			return existing;

		byte id = _nextId++;

				var mat = new VoxelMaterial
				{
						Id = id,
			Name = name,
			Albedo = albedo,
			Roughness = roughness,
			Metallic = metallic,
			Emissive = emissive,
			IsSolid = isSolid
				};

				_materials[id] = mat;
				_nameToId[name] = id;

				return id;
		}

	/// <summary>
	/// Get material ID by name. Throws if not found.
	/// </summary>
	public byte GetId(string name)
	{
		if (!_nameToId.TryGetValue(name, out var id))
			throw new KeyNotFoundException($"Voxel material '{name}' is not registered.");
		return id;
	}

	/// <summary>
	/// Try get material ID by name.
	/// </summary>
	public bool TryGetId(string name, out byte id)
	{
		return _nameToId.TryGetValue(name, out id);
	}

		public VoxelMaterial GetMaterial(byte id)
		{
				if (id >= MaxMaterials) throw new ArgumentOutOfRangeException(nameof(id));

				var material = _materials[id];
				if (material == null)
						throw new KeyNotFoundException($"Voxel material id '{id}' is not registered.");

				return material;
		}

	/// <summary>
	/// Example default palette you can tweak.
	/// </summary>
	public static VoxelPalette CreateDefault()
	{
		var p = new VoxelPalette();

		p.RegisterMaterial("dirt",  new Color(0.35f, 0.22f, 0.12f));
		p.RegisterMaterial("grass", new Color(0.15f, 0.55f, 0.15f));
		p.RegisterMaterial("stone", new Color(0.5f,  0.5f,  0.5f));
		p.RegisterMaterial("sand",  new Color(0.85f, 0.78f, 0.55f));

		return p;
	}
}
