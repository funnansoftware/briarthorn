#pragma once

#include <cstddef>
#include <vector>

#include <game/Entity.hpp>

namespace bt::game
{
    /// The world's entity database and its one lifecycle owner: every Entity that
    /// exists is created and destroyed here. Structural changes (spawn/despawn)
    /// go through the command buffer and land at tick boundaries, never mid-step,
    /// so a system can iterate [entities] without the vector reallocating under
    /// it. A trimmed port of `lib/sim/model/world_model.dart`.
    class World
    {
    public:
        /// Add [entity] to the world, assigning it a fresh id (its `id` field is
        /// overwritten). Returns the new id.
        auto spawn(Entity entity) -> EntityId;

        /// Remove the entity with [id], if present. Returns whether one was.
        auto despawn(EntityId id) -> bool;

        /// The live entity for [id], or nullptr if none. Linear scan. The pointer
        /// is valid only until the next spawn/despawn — hold ids, not pointers.
        [[nodiscard]] auto find(EntityId id) -> Entity*;
        [[nodiscard]] auto find(EntityId id) const -> const Entity*;

        /// All entities, spawn order — the list systems iterate each tick.
        [[nodiscard]] auto entities() -> std::vector<Entity>&;
        [[nodiscard]] auto entities() const -> const std::vector<Entity>&;

        /// The entity the world revolves around (ownship); NullEntity if none.
        [[nodiscard]] auto getPlayer() const -> EntityId;
        auto setPlayer(EntityId id) -> void;

    private:
        std::vector<Entity> entities_;
        EntityId nextId_{1};
        EntityId player_{NullEntity};
    };
}
