#pragma once

#include <game/Vec2.hpp>

namespace bt::game
{
    /// @brief The one home for the world's coordinate convention and the trig that
    /// derives from it.
    ///
    /// Position is in metres (1 px = 1 m); the origin is where
    /// ownship starts. +x is east, +y is south (screen-down), so north is -y.
    /// Angles — headings, bearings — are degrees clockwise from north (0 = N,
    /// 90 = E), kept in [0, 360). Everything that turns positions into angles, or
    /// angles into displacements, routes through here so the sign convention has
    /// one authoritative home. Ports `lib/sim/geo.dart`.
    class Geo
    {
    public:
        /// @brief Wrap an angle in degrees onto [0, 360) — the canonical heading range.
        /// @param deg The angle in degrees to wrap.
        /// @return The angle wrapped onto [0, 360).
        [[nodiscard]] static auto wrap360(float deg) -> float;

        /// @brief Wrap an angle in degrees onto [-180, 180) — the shortest-turn range.
        ///
        /// Negative is a left turn, positive a right turn.
        /// @param deg The angle in degrees to wrap.
        /// @return The angle wrapped onto [-180, 180).
        [[nodiscard]] static auto wrap180(float deg) -> float;

        /// @brief The +x (east / screen-right) component of moving @p distance along
        /// @p angle degrees clockwise from north.
        /// @param angle The heading in degrees clockwise from north.
        /// @param distance The distance moved.
        /// @return The +x (east / screen-right) component of the displacement.
        [[nodiscard]] static auto offsetX(float angle, float distance) -> float;

        /// @brief The +y (south / screen-down) component of the same displacement.
        ///
        /// Negative when heading north, since north is -y.
        /// @param angle The heading in degrees clockwise from north.
        /// @param distance The distance moved.
        /// @return The +y (south / screen-down) component of the displacement.
        [[nodiscard]] static auto offsetY(float angle, float distance) -> float;

        /// @brief The world displacement of moving @p distance along @p angle.
        /// @param angle The heading in degrees clockwise from north.
        /// @param distance The distance moved.
        /// @return The world displacement as a Vec2.
        [[nodiscard]] static auto offset(float angle, float distance) -> Vec2;
    };
}
