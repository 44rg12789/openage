// Copyright 2022-2023 the openage authors. See copying.md for legal info.

#include "render_factory.h"

#include "coord/phys.h"
#include "renderer/stages/terrain/terrain_render_entity.h"
#include "renderer/stages/terrain/terrain_renderer.h"
#include "renderer/stages/world/world_render_entity.h"
#include "renderer/stages/world/world_renderer.h"

namespace openage::renderer {
RenderFactory::RenderFactory(const std::shared_ptr<terrain::TerrainRenderer> terrain_renderer,
                             const std::shared_ptr<world::WorldRenderer> world_renderer) :
	terrain_renderer{terrain_renderer},
	world_renderer{world_renderer} {
}

std::shared_ptr<terrain::TerrainRenderEntity> RenderFactory::add_terrain_render_entity(const util::Vector2s chunk_size,
                                                                                       const coord::tile_delta chunk_offset) {
	auto entity = std::make_shared<terrain::TerrainRenderEntity>();
	this->terrain_renderer->add_render_entity(entity, chunk_size, chunk_offset.to_phys2().to_scene2());

	return entity;
}

std::shared_ptr<world::WorldRenderEntity> RenderFactory::add_world_render_entity() {
	auto entity = std::make_shared<world::WorldRenderEntity>();
	this->world_renderer->add_render_entity(entity);

	return entity;
}

} // namespace openage::renderer
