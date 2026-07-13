#pragma once

#include <game/Vec2.hpp>

namespace bt::game
{
    /// The one home for the world's coordinate convention and the trig that
    /// derives from it. Position is in metres (1 px = 1 m); the origin is where
    /// ownship starts. +x is east, +y is south (screen-down), so north is -y.
    /// Angles — headings, bearings — are degrees clockwise from north (0 = N,
    /// 90 = E), kept in [0, 360). Everything that turns positions into angles, or
    /// angles into displacements, routes through here so the sign convention has
    /// one authoritative home. Ports `lib/sim/geo.dart`.
    class Geo
    {
    public:
        /// Wrap an angle in degrees onto [0, 360) — the canonical heading range.
        [[nodiscard]] static auto wrap360(float deg) -> float;

        /// Wrap an angle in degrees onto [-180, 180) — the shortest-turn range:
        /// negative is a left turn, positive a right turn.
        [[nodiscard]] static auto wrap180(float deg) -> float;

        /// The +x (east / screen-right) component of moving [distance] along
        /// [angle] degrees clockwise from north.
        [[nodiscard]] static auto offsetX(float angle, float distance) -> float;

        /// The +y (south / screen-down) component of the same displacement —
        /// negative when heading north, since north is -y.
        [[nodiscard]] static auto offsetY(float angle, float distance) -> float;

        /// The world displacement of moving [distance] along [angle].
        [[nodiscard]] static auto offset(float angle, float distance) -> Vec2;
    };
}
