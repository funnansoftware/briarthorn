#include <chrono>

#include <gtest/gtest.h>

#include <game/CommandBuffer.hpp>
#include <game/Duration.hpp>
#include <game/Entity.hpp>
#include <game/Geo.hpp>
#include <game/World.hpp>
#include <game/systems/Movement.hpp>

namespace
{
    using bt::game::CommandBuffer;
    using bt::game::Duration;
    using bt::game::Entity;
    using bt::game::Geo;
    using bt::game::Movement;
    using bt::game::World;

    TEST(DurationTest, ToSecondsConvertsFromAnyChronoDuration)
    {
        EXPECT_FLOAT_EQ(Duration{std::chrono::milliseconds{500}}.toSeconds().count(), 0.5F);
        EXPECT_FLOAT_EQ(Duration{std::chrono::seconds{2}}.toSeconds().count(), 2.0F);
        EXPECT_FLOAT_EQ(Duration{}.toSeconds().count(), 0.0F);
    }

    TEST(DurationTest, AddAndSubtractAccumulate)
    {
        auto total = Duration{};
        total += Duration{std::chrono::milliseconds{150}};
        total += Duration{std::chrono::milliseconds{150}};
        total -= Duration{std::chrono::milliseconds{100}};

        EXPECT_FLOAT_EQ(total.toSeconds().count(), 0.2F);
        EXPECT_TRUE(total >= Duration{std::chrono::milliseconds{100}});
        EXPECT_FALSE(total >= Duration{std::chrono::milliseconds{300}});
    }

    TEST(WorldTest, SpawnAssignsUniqueIdsAndFindsThem)
    {
        auto world = World{};
        const auto first = world.spawn(Entity{});
        const auto second = world.spawn(Entity{});

        EXPECT_NE(first, second);
        ASSERT_NE(world.find(first), nullptr);
        EXPECT_EQ(world.find(first)->id, first);
        EXPECT_EQ(world.entities().size(), 2U);
    }

    TEST(WorldTest, DespawnRemovesTheEntity)
    {
        auto world = World{};
        const auto id = world.spawn(Entity{});

        EXPECT_TRUE(world.despawn(id));
        EXPECT_EQ(world.find(id), nullptr);
        EXPECT_FALSE(world.despawn(id)); // already gone
    }

    TEST(CommandBufferTest, FlushAppliesClampedIntentThenClears)
    {
        auto world = World{};
        const auto id = world.spawn(Entity{});

        auto commands = CommandBuffer{};
        commands.throttle(id, 5.0F); // over-range -> clamps to 1
        commands.steer(id, -3.0F);   // under-range -> clamps to -1
        EXPECT_EQ(commands.size(), 2U);

        commands.flush(world);

        EXPECT_FLOAT_EQ(world.find(id)->controls.throttle, 1.0F);
        EXPECT_FLOAT_EQ(world.find(id)->controls.steer, -1.0F);
        EXPECT_TRUE(commands.empty()); // flush drains the buffer
    }

    TEST(CommandBufferTest, DamageIsAppliedAndFloorsAtZero)
    {
        auto world = World{};
        auto entity = Entity{};
        entity.health = 30.0F;
        const auto id = world.spawn(entity);

        auto commands = CommandBuffer{};
        commands.damage(id, 100.0F);
        commands.flush(world);

        EXPECT_FLOAT_EQ(world.find(id)->health, 0.0F);
    }

    TEST(CommandBufferTest, CommandsNamingADespawnedEntityAreSkipped)
    {
        auto world = World{};
        const auto id = world.spawn(Entity{});

        auto commands = CommandBuffer{};
        commands.despawn(id);
        commands.throttle(id, 1.0F); // applied after the despawn -> a safe no-op

        commands.flush(world);

        EXPECT_EQ(world.find(id), nullptr);
        EXPECT_EQ(world.entities().size(), 0U);
    }

    TEST(MovementTest, ThrottleAcceleratesAlongHeading)
    {
        auto world = World{};
        auto entity = Entity{};
        entity.heading = 90.0F; // due east: +x
        entity.controls.throttle = 1.0F;
        entity.limits.acceleration = 50.0F; // distinct from the other limits
        const auto id = world.spawn(entity);

        auto movement = Movement{};
        movement.update(world, 1.0F); // one second

        const auto* out = world.find(id);
        ASSERT_NE(out, nullptr);
        EXPECT_FLOAT_EQ(out->speed, 50.0F); // full throttle * the acceleration limit
        EXPECT_GT(out->position.x, 0.0F);          // moved east
        EXPECT_NEAR(out->position.y, 0.0F, 1e-2F); // not north/south
    }

    TEST(MovementTest, SteerTurnsWithinTheRate)
    {
        auto world = World{};
        auto entity = Entity{};
        entity.controls.steer = 1.0F; // full right
        entity.limits.turnRate = 90.0F;
        const auto id = world.spawn(entity);

        auto movement = Movement{};
        movement.update(world, 0.5F);

        EXPECT_FLOAT_EQ(world.find(id)->heading, 45.0F); // 90 deg/s * 0.5 s
    }

    TEST(MovementTest, BrakeDeceleratesAndFloorsAtZero)
    {
        auto world = World{};
        auto entity = Entity{};
        entity.speed = 100.0F;
        const auto id = world.spawn(entity);

        auto commands = CommandBuffer{};
        commands.brake(id, 5.0F); // over-range -> clamps to 1
        commands.flush(world);
        EXPECT_FLOAT_EQ(world.find(id)->controls.brake, 1.0F);

        auto movement = Movement{};
        movement.update(world, 0.5F);
        EXPECT_FLOAT_EQ(world.find(id)->speed, 40.0F); // 100 - 120 m/s^2 * 0.5 s

        movement.update(world, 0.5F);
        EXPECT_FLOAT_EQ(world.find(id)->speed, 0.0F); // floors at zero
    }

    TEST(MovementTest, BoostLiftsTheTopSpeedCeiling)
    {
        auto world = World{};
        const auto id = world.spawn(Entity{});

        auto commands = CommandBuffer{};
        commands.throttle(id, 1.0F);
        commands.boost(id, 1.0F);
        commands.flush(world);

        auto movement = Movement{};
        movement.update(world, 1.0F);
        movement.update(world, 1.0F);
        movement.update(world, 1.0F); // enough to hit the boosted ceiling

        // At full boost the ceiling is limits.topSpeed * Movement's 1.6
        // afterburner multiplier.
        const auto* out = world.find(id);
        ASSERT_NE(out, nullptr);
        EXPECT_FLOAT_EQ(out->speed, out->limits.topSpeed * 1.6F);

        commands.boost(id, 0.0F); // release the afterburner
        commands.flush(world);
        movement.update(world, 1.0F);

        EXPECT_FLOAT_EQ(world.find(id)->speed, out->limits.topSpeed); // re-clamped
    }

    TEST(MovementTest, ControlsStayLatchedAcrossSteps)
    {
        auto world = World{};
        const auto id = world.spawn(Entity{});

        auto commands = CommandBuffer{};
        commands.throttle(id, 1.0F);
        commands.flush(world); // one flush; no further commands

        auto movement = Movement{};
        movement.update(world, 0.5F);
        const auto speedAfterOneStep = world.find(id)->speed;
        movement.update(world, 0.5F); // the lever persists until countermanded

        EXPECT_GT(speedAfterOneStep, 0.0F);
        EXPECT_GT(world.find(id)->speed, speedAfterOneStep);
    }

    TEST(EntityTest, TopSpeedFoldsTheLiveBoostMultiplier)
    {
        auto entity = Entity{};
        entity.limits.topSpeed = 100.0F;
        entity.speedBoost = 1.5F;

        EXPECT_FLOAT_EQ(entity.topSpeed(), 150.0F);
    }

    TEST(GeoTest, WrapsAnglesOntoTheirCanonicalRange)
    {
        EXPECT_FLOAT_EQ(Geo::wrap360(370.0F), 10.0F);
        EXPECT_FLOAT_EQ(Geo::wrap360(-10.0F), 350.0F);
        EXPECT_FLOAT_EQ(Geo::wrap180(190.0F), -170.0F);
    }
}
