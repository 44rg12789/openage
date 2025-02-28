// Copyright 2023-2023 the openage authors. See copying.md for legal info.

#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "coord/scene.h"
#include "time/time.h"
#include "util/vector.h"


namespace openage::renderer {

namespace resources {
class AssetManager;
}

namespace terrain {
class TerrainRenderMesh;
class TerrainRenderEntity;

class TerrainChunk {
public:
	/**
     * Create a new terrain chunk.
     *
     * @param asset_manager Asset manager for loading textures.
     */
	TerrainChunk(const std::shared_ptr<renderer::resources::AssetManager> &asset_manager,
	             const util::Vector2s size,
	             const coord::scene2_delta offset);

	~TerrainChunk() = default;

	/**
     * Set the terrain render entity for vertex updates of this mesh.
     *
     * @param entity New terrain render entity.
     * @param size Size of the chunk in tiles.
     * @param offset Offset of the chunk from origin in tiles.
     */
	void set_render_entity(const std::shared_ptr<TerrainRenderEntity> &entity);

	/**
     * Fetch updates from the render entity.
     *
     * @param time Current simulation time.
     */
	void fetch_updates(const time::time_t &time = 0.0);

	/**
     * Update the uniforms of the meshes.
     *
     * @param time Current simulation time.
     */
	void update_uniforms(const time::time_t &time = 0.0);

	/**
     * Get the meshes composing the terrain.
     *
     * @return Vector of terrain meshes.
     */
	const std::vector<std::shared_ptr<TerrainRenderMesh>> &get_meshes() const;

	/**
     * Get the size of the chunk in tiles.
     *
     * @return Size of the chunk (in tiles).
     */
	util::Vector2s &get_size();

	/**
     * Get the offset of the chunk from origin in tiles.
     *
     * @return Offset of the chunk (in tiles).
     */
	coord::scene2_delta &get_offset();

private:
	/**
	 * Create a terrain mesh from the data provided by the render entity.
	 *
	 * @return New terrain mesh.
	 */
	std::shared_ptr<TerrainRenderMesh> create_mesh();

	/**
     * Size of the chunk in tiles (width x height).
     */
	util::Vector2s size;

	/**
     * Offset of the chunk from origin in tiles (x, y).
     */
	coord::scene2_delta offset;

	/**
	 * Meshes composing the terrain. Each mesh represents a drawable vertex surface
	 * and a texture.
	 */
	std::vector<std::shared_ptr<TerrainRenderMesh>> meshes;

	/**
	 * Asset manager for central accessing and loading textures.
     */
	std::shared_ptr<renderer::resources::AssetManager> asset_manager;

	/**
     * Source for ingame terrain coordinates. These coordinates are translated into
     * our render vertex mesh when \p update() is called.
     */
	std::shared_ptr<TerrainRenderEntity> render_entity;
};


} // namespace terrain
} // namespace openage::renderer
