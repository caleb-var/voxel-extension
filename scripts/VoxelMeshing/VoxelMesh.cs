using System;
using Godot;

public class VoxelMesh
{
	public const int Size = 64;
	public const int VoxCount = Size * Size * Size;

	public readonly byte[] MaterialId; // 0 = empty
	public readonly byte[] Flags; //
	public readonly Vector3[] VoxelNormals; // per-voxel normals (unit or zero)

	public static int Index(int x, int y, int z)
	{
		return x + y * Size + z * Size * Size;
	}
	public void SetVoxel(int x, int y, int z, byte material, byte flags = 0)
	{
		if ((uint)x >= Size || (uint)y >= Size || (uint)z >= Size)
			throw new ArgumentOutOfRangeException($"Voxel ({x},{y},{z}) out of range 0..{Size - 1}");

		int idx = Index(x, y, z);
		MaterialId[idx] = material;
		Flags[idx]      = flags;
	}
		public (byte material, byte Flags) GetVoxel(int x, int y, int z)
	{
		if ((uint)x >= Size || (uint)y >= Size || (uint)z >= Size)
			throw new ArgumentOutOfRangeException($"Voxel ({x},{y},{z}) out of range 0..{Size - 1}");

		int idx = Index(x, y, z);
		return (MaterialId[idx], Flags[idx]);
	}
	public VoxelMesh()
	{
		MaterialId = new byte[VoxCount];
		Flags      = new byte[VoxCount];
	}
	public VoxelMesh(byte[] materialId, byte[] flags = null)
	{
		MaterialId = materialId;
		Flags = flags;
		VoxelNormals = ComputeNormals(materialId);
	}
	
	private static Vector3[] ComputeNormals(byte[] materialId)
	{
		var normals = new Vector3[VoxCount];
		int N = Size;

		float Occ(int x, int y, int z)
		{
			if (x < 0 || x >= N || y < 0 || y >= N || z < 0 || z >= N)
				return 0.0f;
			int idx = x + y * N + z * N * N;
			return materialId[idx] != 0 ? 1.0f : 0.0f;
		}

		for (int x = 0; x < N; x++)
		{
			for (int y = 0; y < N; y++)
			{
				for (int z = 0; z < N; z++)
				{
					int idx = x + y * N + z * N * N;
					if (materialId[idx] == 0)
					{
						normals[idx] = Vector3.Zero;
						continue;
					}

					float gx = Occ(x + 1, y, z) - Occ(x - 1, y, z);
					float gy = Occ(x, y + 1, z) - Occ(x, y - 1, z);
					float gz = Occ(x, y, z + 1) - Occ(x, y, z - 1);

					Vector3 grad = new Vector3(gx, gy, gz);
					grad = -grad; // solid -> empty

					if (grad.LengthSquared() > 0.0001f)
						normals[idx] = grad.Normalized();
					else
						normals[idx] = Vector3.Zero;
				}
			}
		}

		return normals;
	}

	/// <summary>
	/// For testing: simple ground
	/// </summary>
	public static VoxelMesh CreateTest()
	{
		var voxels = new byte[VoxCount];

		int N = Size;

		for (int x = 0; x < N; x++)
		{
			for (int y = 0; y < N; y++)
			{
				for (int z = 0; z < N; z++)
				{
					int idx = x + y * N + z * N * N;

					// basic geometry like before
					if (y < 16)
						voxels[idx] = 1;

					int dx = x - 32;
					int dz = z - 32;
					int dist2 = dx * dx + dz * dz;

					if (dist2 < 20 * 20 && y < 16 + 8 - dist2 / 50)
						voxels[idx] = 2;

					if (dist2 < 8 * 8 && y >= 18 && y <= 22)
						voxels[idx] = 0;
				}
			}
		}

		return new VoxelMesh(voxels);
	}
}
