// Copyright 2023-2023 the openage authors. See copying.md for legal info.

#include "terrain_chunk.h"

#include "renderer/resources/assets/asset_manager.h"
#include "renderer/resources/mesh_data.h"
#include "renderer/stages/terrain/terrain_mesh.h"
#include "renderer/stages/terrain/terrain_render_entity.h"


namespace openage::renderer::terrain {

TerrainChunk::TerrainChunk(const std::shared_ptr<renderer::resources::AssetManager> &asset_manager,
                           const util::Vector2s size,
                           const coord::scene2_delta offset) :
	size{size},
	offset{offset},
	asset_manager{asset_manager} {}

void TerrainChunk::set_render_entity(const std::shared_ptr<TerrainRenderEntity> &entity) {
	this->render_entity = entity;
}

void TerrainChunk::fetch_updates(const time::time_t & /* time */) {
	// TODO: Don't create model if render entity is not set
	if (not this->render_entity) {
		return;
	}

	// Check render entity for updates
	if (not this->render_entity->is_changed()) {
		return;
	}
	// TODO: Change mesh instead of recreating it
	// TODO: Multiple meshes
	auto new_mesh = this->create_mesh();
	new_mesh->create_model_matrix(this->offset);
	this->meshes.clear();
	this->meshes.push_back(new_mesh);

	// Indicate to the render entity that its updates have been processed.
	this->render_entity->clear_changed_flag();
}

void TerrainChunk::update_uniforms(const time::time_t &time) {
	for (auto &mesh : this->meshes) {
		mesh->update_uniforms(time);
	}
}

const std::vector<std::shared_ptr<TerrainRenderMesh>> &TerrainChunk::get_meshes() const {
	return this->meshes;
}

std::shared_ptr<TerrainRenderMesh> TerrainChunk::create_mesh() {
	// Update mesh
	auto size = this->render_entity->get_size();
	auto src_verts = this->render_entity->get_vertices();

	// dst_verts places vertices in order
	// (left to right, bottom to top)
	std::vector<float> dst_verts{};
	dst_verts.reserve(src_verts.size() * 5);
	for (auto v : src_verts) {
		// Transform to scene coords
		auto v_vec = v.to_world_space();
		dst_verts.push_back(v_vec[0]);
		dst_verts.push_back(v_vec[1]);
		dst_verts.push_back(v_vec[2]);
		// TODO: Texture scaling
		dst_verts.push_back((v.ne / 10).to_float());
		dst_verts.push_back((v.se / 10).to_float());
	}

	// split the grid into triangles using an index array
	std::vector<uint16_t> idxs;
	idxs.reserve((size[0] - 1) * (size[1] - 1) * 6);
	// iterate over all tiles in the grid by columns, i.e. starting
	// from the left corner to the bottom corner if you imagine it from
	// the camera's point of view
	for (size_t i = 0; i < size[0] - 1; ++i) {
		for (size_t j = 0; j < size[1] - 1; ++j) {
			// since we are working on tiles, we split each tile into two triangles
			// with counter-clockwise vertex order
			idxs.push_back(j + i * size[1]); // bottom left
			idxs.push_back(j + 1 + i * size[1]); // bottom right
			idxs.push_back(j + size[1] + i * size[1]); // top left
			idxs.push_back(j + 1 + i * size[1]); // bottom right
			idxs.push_back(j + size[1] + 1 + i * size[1]); // top right
			idxs.push_back(j + size[1] + i * size[1]); // top left
		}
	}

	resources::VertexInputInfo info{
		{resources::vertex_input_t::V3F32, resources::vertex_input_t::V2F32},
		resources::vertex_layout_t::AOS,
		resources::vertex_primitive_t::TRIANGLES,
		resources::index_t::U16};

	auto const vert_data_size = dst_verts.size() * sizeof(float);
	std::vector<uint8_t> vert_data(vert_data_size);
	std::memcpy(vert_data.data(), dst_verts.data(), vert_data_size);

	auto const idx_data_size = idxs.size() * sizeof(uint16_t);
	std::vector<uint8_t> idx_data(idx_data_size);
	std::memcpy(idx_data.data(), idxs.data(), idx_data_size);

	resources::MeshData meshdata{std::move(vert_data), std::move(idx_data), info};

	// Update textures
	auto tex_manager = this->asset_manager->get_texture_manager();

	// TODO: Support multiple textures per chunk

	auto terrain_mesh = std::make_shared<TerrainRenderMesh>(
		this->asset_manager,
		this->render_entity->get_terrain_path(),
		std::move(meshdata));

	return terrain_mesh;
}

util::Vector2s &TerrainChunk::get_size() {
	return this->size;
}

coord::scene2_delta &TerrainChunk::get_offset() {
	return this->offset;
}

} // namespace openage::renderer::terrain
