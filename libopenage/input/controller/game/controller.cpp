// Copyright 2021-2023 the openage authors. See copying.md for legal info.

#include "controller.h"

#include "event/event_loop.h"
#include "event/evententity.h"
#include "event/state.h"
#include "gamestate/component/internal/commands/types.h"
#include "gamestate/event/send_command.h"
#include "gamestate/event/spawn_entity.h"
#include "gamestate/game.h"
#include "gamestate/game_state.h"
#include "gamestate/simulation.h"
#include "input/controller/game/binding_context.h"
#include "time/clock.h"
#include "time/time_loop.h"

#include "coord/phys.h"

namespace openage::input::game {

Controller::Controller(const std::unordered_set<size_t> &controlled_factions,
                       size_t active_faction_id) :
	controlled_factions{controlled_factions},
	active_faction_id{active_faction_id},
	outqueue{} {}

void Controller::set_control(size_t faction_id) {
	std::unique_lock lock{this->mutex};

	if (this->controlled_factions.find(faction_id) != this->controlled_factions.end()) {
		this->active_faction_id = faction_id;
	}
}

size_t Controller::get_controlled() const {
	std::unique_lock lock{this->mutex};

	return this->active_faction_id;
}

const std::vector<gamestate::entity_id_t> &Controller::get_selected() const {
	std::unique_lock lock{this->mutex};

	return this->selected;
}

void Controller::set_selected(std::vector<gamestate::entity_id_t> ids) {
	std::unique_lock lock{this->mutex};

	this->selected = ids;
}

bool Controller::process(const event_arguments &ev_args, const std::shared_ptr<BindingContext> &ctx) {
	std::unique_lock lock{this->mutex};

	if (not ctx->is_bound(ev_args.e)) {
		return false;
	}

	// TODO: check if action is allowed
	auto bind = ctx->lookup(ev_args.e);
	auto controller = this->shared_from_this();
	auto game_event = bind.transform(ev_args, controller);

	switch (bind.action_type) {
	case forward_action_t::SEND:
		this->outqueue.push_back(game_event);
		for (auto event : this->outqueue) {
			// TODO: Send gamestate event
		}
		break;

	case forward_action_t::QUEUE:
		this->outqueue.push_back(game_event);
		break;

	case forward_action_t::CLEAR:
		this->outqueue.clear();
		break;

	default:
		throw Error{MSG(err) << "Unrecognized action type."};
	}

	return true;
}

void setup_defaults(const std::shared_ptr<BindingContext> &ctx,
                    const std::shared_ptr<time::TimeLoop> &time_loop,
                    const std::shared_ptr<openage::gamestate::GameSimulation> &simulation,
                    const std::shared_ptr<renderer::camera::Camera> &camera) {
	binding_func_t create_entity_event{[&](const event_arguments &args,
	                                       const std::shared_ptr<Controller> controller) {
		auto mouse_pos = args.mouse.to_phys3(camera);
		event::EventHandler::param_map::map_t params{
			{"position", mouse_pos},
			{"owner", controller->get_controlled()},
			// TODO: Remove
			{"select_cb", std::function<void(gamestate::entity_id_t id)>{[controller](gamestate::entity_id_t id) {
				 controller->set_selected({id});
			 }}},
		};

		auto event = simulation->get_event_loop()->create_event(
			"game.spawn_entity",
			simulation->get_spawner(),
			simulation->get_game()->get_state(),
			time_loop->get_clock()->get_time(),
			params);
		return event;
	}};

	binding_action create_entity_action{forward_action_t::SEND, create_entity_event};
	Event ev_mouse_lmb{event_class::MOUSE_BUTTON, Qt::MouseButton::LeftButton, Qt::NoModifier, QEvent::MouseButtonRelease};

	ctx->bind(ev_mouse_lmb, create_entity_action);

	binding_func_t move_entity{[&](const event_arguments &args,
	                               const std::shared_ptr<Controller> controller) {
		auto mouse_pos = args.mouse.to_phys3(camera);
		event::EventHandler::param_map::map_t params{
			{"type", gamestate::component::command::command_t::MOVE},
			{"target", mouse_pos},
			{"entity_ids", controller->get_selected()},
		};

		auto event = simulation->get_event_loop()->create_event(
			"game.send_command",
			simulation->get_commander(),
			simulation->get_game()->get_state(),
			time_loop->get_clock()->get_time(),
			params);
		return event;
	}};

	binding_action move_entity_action{forward_action_t::SEND, move_entity};
	Event ev_mouse_rmb{event_class::MOUSE_BUTTON, Qt::MouseButton::RightButton, Qt::NoModifier, QEvent::MouseButtonRelease};

	ctx->bind(ev_mouse_rmb, move_entity_action);
}


} // namespace openage::input::game
