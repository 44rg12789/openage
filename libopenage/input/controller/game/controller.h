// Copyright 2021-2023 the openage authors. See copying.md for legal info.

#pragma once

#include <memory>
#include <mutex>
#include <unordered_set>

#include "curve/discrete.h"
#include "gamestate/types.h"
#include "input/event.h"

namespace openage {

namespace gamestate {
class GameSimulation;
}

namespace time {
class TimeLoop;
}

namespace input::game {

class BindingContext;

/**
 * Interface for game controllers.
 *
 * Controllers handle inputs from outside of a game (e.g. GUI, AI, scripts, ...)
 * and pass the resulting events to game entities. They also act as a form of
 * access control for using in-game functionality of game entities.
 *
 * TODO: Connection to engine
 */
class Controller : public std::enable_shared_from_this<Controller> {
public:
	Controller(const std::unordered_set<size_t> &controlled_factions,
	           size_t active_faction_id);

	~Controller() = default;

	/**
     * Switch the actively controlled faction by the controller.
     * The ID must be in the list of controlled factions.
     *
     * @param faction_id ID of the new active faction.
     */
	void set_control(size_t faction_id);

	/**
     * Get the ID of the faction actively controlled by the controller.
     *
     * @return ID of the active faction.
     */
	size_t get_controlled() const;

	/**
      * Get the currently selected entities.
      *
      * @return Selected entities.
      */
	const std::vector<gamestate::entity_id_t> &get_selected() const;

	/**
      * Set the currently selected entities.
      *
      * @param ids Selected entities.
      */
	void set_selected(std::vector<gamestate::entity_id_t> ids);

	/**
     * Process an input event from the input manager.
     *
     * @param ev_args Input event and arguments.
     * @param ctx Binding context for looking up the event transformation.
     *
     * @return true if the event is accepted, else false.
     */
	bool process(const event_arguments &ev_args, const std::shared_ptr<BindingContext> &ctx);

private:
	/**
     * List of factions controllable by this controller.
     */
	std::unordered_set<size_t> controlled_factions;

	/**
     * ID of the currently active faction.
     */
	size_t active_faction_id;

	/**
      * Currently selected entities.
      */
	std::vector<gamestate::entity_id_t> selected;

	/**
     * Queue for gamestate events generated from inputs.
     */
	std::vector<std::shared_ptr<event::Event>> outqueue;

	/**
      * Mutex for threaded access.
      */
	mutable std::recursive_mutex mutex;
};

/**
 * Setup default controller action bindings:
 *
 * - Mouse click: Create game entity.
 *
 * @param ctx Binding context the actions are added to.
 * @param time_loop Time loop for getting simulation time.
 * @param simulation Game simulation.
 * @param camera Active game camera.
 */
void setup_defaults(const std::shared_ptr<BindingContext> &ctx,
                    const std::shared_ptr<time::TimeLoop> &time_loop,
                    const std::shared_ptr<openage::gamestate::GameSimulation> &simulation,
                    const std::shared_ptr<renderer::camera::Camera> &camera);

} // namespace input::game
} // namespace openage
