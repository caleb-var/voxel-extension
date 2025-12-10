using System;
using Godot;

/// <summary>
/// Static helpers to fill VoxelBrick with procedural terrain.
/// No Godot Nodes here: just pure functions.
/// </summary>
public static class TerrainGenerator
{
	/// <summary>
	/// Create a configured FastNoiseLite instance for terrain.
	/// You can tweak this later without touching chunk code.
	/// </summary>
	public static FastNoiseLite CreateDefaultNoise(int seed = 0628)
	{
		var noise = new FastNoiseLite();
		noise.Seed = seed;
		noise.NoiseType = 0;
		noise.FractalType = FastNoiseLite.FractalTypeEnum.Fbm;
		noise.FractalOctaves = 4;
		noise.Frequency = 0.001f;
		return noise;
	}

	/// <summary>
	/// Fill a chunk with a simple layered terrain:
	/// stone base, dirt middle, grass top, optional "sea level".
	/// chunkCoord is in chunk-space (XZ grid).
	/// </summary>
	public static void FillChunk(
		VoxelMesh chunk,
		VoxelPalette palette,
		Vector2I chunkCoord,
		FastNoiseLite noise,
		int seaLevel = 0,
		int baseHeight = 1,
		int heightAmplitude = 24
	)
	{
		if (chunk == null) throw new ArgumentNullException(nameof(chunk));
		if (palette == null) throw new ArgumentNullException(nameof(palette));
		if (noise == null) throw new ArgumentNullException(nameof(noise));

		// Material IDs from palette (cached once).
		byte grassId = palette.GetId("grass");
		byte dirtId  = palette.GetId("dirt");
		byte stoneId = palette.GetId("stone");
		byte sandId  = palette.GetId("sand");

		int N = VoxelMesh.Size;

		for (int lx = 0; lx < N; lx++)
		{
			for (int lz = 0; lz < N; lz++)
			{
				// Convert local voxel coords to world coords
				int wx = chunkCoord.X * N + lx;
				int wz = chunkCoord.Y * N + lz;

				// Noise in [-1,1] -> [0,1]
				float n = noise.GetNoise2D(wx, wz) * 0.5f + 0.5f;

				int columnHeight = baseHeight + (int)(n * heightAmplitude);
				columnHeight = Mathf.Clamp(columnHeight, 1, N - 1);

				for (int y = 0; y < N; y++)
				{
					byte mat = 0;

					if (y > columnHeight)
					{
						// air above ground
						mat = 0;
					}
					else
					{
						// below surface
						if (y == columnHeight)
						{
							// surface block
							if (y <= seaLevel + 1)
								mat = sandId;
							else
								mat = grassId;
						}
						else if (y >= columnHeight - 3)
						{
							// top soil layer
							mat = dirtId;
						}
						else
						{
							// deep stone
							mat = stoneId;
						}
					}

					chunk.SetVoxel(lx, y, lz, mat);
				}
			}
		}
	}
}
