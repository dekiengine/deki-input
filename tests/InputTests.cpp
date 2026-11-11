// Gesture ownership, and the coordinate flip in the box hit test.
//
// Two things here are easy to get wrong in a way that compiles. The gesture
// claim is process-wide mutable state with an ownership protocol, so a missed
// release leaves every consuming collider dead until restart. And HitTest
// converts world coordinates (Y-up, origin at the object's centre) into
// bounds-local ones (Y-down, origin top-left) -- a sign slip there puts the
// hit box above the sprite instead of on it, and padding compounds it because
// top and bottom are separate fields.

#include <gtest/gtest.h>

#include <InputCollider.h>
#include <InputDispatch.h>

#include <deki/Object.h>

// The package's types live in its namespace; tests name them unqualified.
using namespace DekiInput;

namespace
{

// Releases the claim however a test ends, so one failure cannot cascade into
// every later test through the shared static.
class GestureFixture : public ::testing::Test
{
protected:
    void SetUp() override { InputDispatch::ReleaseGesture(); }
    void TearDown() override { InputDispatch::ReleaseGesture(); }
};

// A collider centred on the origin unless the test moves the object.
struct Box
{
    Deki::Object object{ "box" };
    InputCollider* collider = nullptr;

    Box(float w, float h)
    {
        collider = object.AddComponent<InputCollider>();
        collider->width = w;
        collider->height = h;
    }
};

}  // namespace

TEST_F(GestureFixture, NothingIsClaimedToBeginWith)
{
    EXPECT_FALSE(InputDispatch::IsGestureClaimed());
}

TEST_F(GestureFixture, AskingWithANullOwnerReadsAsOwnershipWhileNothingIsClaimed)
{
    // Pinning a sharp edge rather than endorsing it. The unclaimed state IS a
    // null owner, so IsGestureClaimedBy(nullptr) is true exactly when nobody
    // holds the gesture. Callers pass `this` and never hit it, but a caller
    // that passed a null owner would read as the claimant and skip the
    // suppression check. IsGestureClaimed() is the question to ask when the
    // owner is not known to be non-null.
    EXPECT_TRUE(InputDispatch::IsGestureClaimedBy(nullptr));

    int a = 0;
    InputDispatch::ClaimGesture(&a);
    EXPECT_FALSE(InputDispatch::IsGestureClaimedBy(nullptr));
}

TEST_F(GestureFixture, ClaimingMakesTheOwnerAndOnlyTheOwnerTheClaimant)
{
    int a = 0, b = 0;
    InputDispatch::ClaimGesture(&a);

    EXPECT_TRUE(InputDispatch::IsGestureClaimed());
    EXPECT_TRUE(InputDispatch::IsGestureClaimedBy(&a));
    EXPECT_FALSE(InputDispatch::IsGestureClaimedBy(&b));
}

TEST_F(GestureFixture, AClaimIsNotReentrantAndTheLastClaimantWins)
{
    // Whoever claims last owns it; there is no stack. A component that claims
    // over another's gesture takes it, which is why scroll claims once on
    // drag start rather than every frame.
    int a = 0, b = 0;
    InputDispatch::ClaimGesture(&a);
    InputDispatch::ClaimGesture(&b);

    EXPECT_TRUE(InputDispatch::IsGestureClaimedBy(&b));
    EXPECT_FALSE(InputDispatch::IsGestureClaimedBy(&a));
}

TEST_F(GestureFixture, ReleaseClearsTheClaimWhoeverCallsIt)
{
    int a = 0;
    InputDispatch::ClaimGesture(&a);
    InputDispatch::ReleaseGesture();

    EXPECT_FALSE(InputDispatch::IsGestureClaimed());
    EXPECT_FALSE(InputDispatch::IsGestureClaimedBy(&a));
}

TEST_F(GestureFixture, IsGestureClaimedByNullIsFalseWhileSomethingOwnsIt)
{
    // Otherwise a caller passing a null owner would read as the claimant and
    // walk straight through the suppression check.
    int a = 0;
    InputDispatch::ClaimGesture(&a);
    EXPECT_FALSE(InputDispatch::IsGestureClaimedBy(nullptr));
}

TEST(InputColliderHitTest, WithoutAnOwnerNothingIsHit)
{
    InputCollider loose;
    loose.width = 100.0f;
    loose.height = 100.0f;
    EXPECT_FALSE(loose.HitTest(0.0f, 0.0f));
}

TEST(InputColliderHitTest, TheBoxIsCentredOnTheObject)
{
    // Position is the visual centre (pivot 0.5), so a 100x50 box at the
    // origin spans -50..50 in x and -25..25 in y.
    Box box(100.0f, 50.0f);

    EXPECT_TRUE(box.collider->HitTest(0.0f, 0.0f));
    EXPECT_TRUE(box.collider->HitTest(-49.0f, -24.0f));
    EXPECT_TRUE(box.collider->HitTest(49.0f, 24.0f));

    EXPECT_FALSE(box.collider->HitTest(-51.0f, 0.0f));
    EXPECT_FALSE(box.collider->HitTest(51.0f, 0.0f));
    EXPECT_FALSE(box.collider->HitTest(0.0f, -26.0f));
    EXPECT_FALSE(box.collider->HitTest(0.0f, 26.0f));
}

TEST(InputColliderHitTest, TheBoxFollowsTheObject)
{
    Box box(20.0f, 20.0f);
    box.object.SetWorldPosition(100.0f, -40.0f);

    EXPECT_TRUE(box.collider->HitTest(100.0f, -40.0f));
    EXPECT_TRUE(box.collider->HitTest(109.0f, -31.0f));
    EXPECT_FALSE(box.collider->HitTest(0.0f, 0.0f)) << "the box did not move with its object";
}

TEST(InputColliderHitTest, PaddingExtendsEachEdgeIndependently)
{
    // The asymmetry is the point: a padding that leaks from one edge to
    // another still passes a symmetric test.
    Box box(10.0f, 10.0f);
    box.collider->paddingLeft = 20.0f;

    EXPECT_TRUE(box.collider->HitTest(-24.0f, 0.0f)) << "left padding not applied";
    EXPECT_FALSE(box.collider->HitTest(24.0f, 0.0f)) << "left padding leaked to the right edge";
}

TEST(InputColliderHitTest, TopPaddingExtendsUpwardsInWorldSpace)
{
    // World Y is up; bounds-local Y is down. This is the conversion that a
    // sign slip inverts, putting the padded region below the box.
    Box box(10.0f, 10.0f);
    box.collider->paddingTop = 20.0f;

    EXPECT_TRUE(box.collider->HitTest(0.0f, 20.0f)) << "top padding did not extend upwards";
    EXPECT_FALSE(box.collider->HitTest(0.0f, -20.0f)) << "top padding extended downwards instead";
}

TEST(InputColliderHitTest, BottomPaddingExtendsDownwardsInWorldSpace)
{
    Box box(10.0f, 10.0f);
    box.collider->paddingBottom = 20.0f;

    EXPECT_TRUE(box.collider->HitTest(0.0f, -20.0f)) << "bottom padding did not extend downwards";
    EXPECT_FALSE(box.collider->HitTest(0.0f, 20.0f)) << "bottom padding extended upwards instead";
}

TEST(InputColliderHitTest, EdgesAreInclusive)
{
    // A point exactly on the boundary counts as inside, so two colliders
    // sharing an edge both accept it rather than leaving a one-unit dead line.
    Box box(10.0f, 10.0f);
    EXPECT_TRUE(box.collider->HitTest(-5.0f, 0.0f));
    EXPECT_TRUE(box.collider->HitTest(5.0f, 0.0f));
    EXPECT_TRUE(box.collider->HitTest(0.0f, -5.0f));
    EXPECT_TRUE(box.collider->HitTest(0.0f, 5.0f));
}

TEST(InputColliderHitTest, AZeroSizedColliderStillHitsItsOwnPoint)
{
    // Degenerate but reachable: a collider whose size has not been set yet.
    // It must not hit the whole screen, and must not throw.
    Box box(0.0f, 0.0f);
    EXPECT_TRUE(box.collider->HitTest(0.0f, 0.0f));
    EXPECT_FALSE(box.collider->HitTest(1.0f, 0.0f));
}

// ---- Input sources and sessions ----

#include <DekiInput.h>

namespace
{
// An input source that keeps its event callback, so a test can send events.
class FakeSource : public IDekiInput
{
public:
    explicit FakeSource(bool* shutDown = nullptr) : m_ShutDown(shutDown) {}
    bool Initialize() override { return true; }
    void Shutdown() override
    {
        if (m_ShutDown)
        {
            *m_ShutDown = true;
        }
    }
    void Update() override {}
    void RegisterEventCallback(const InputEventCallback& callback) override { send = callback; }
    bool IsInitialized() const override { return true; }
    bool GetPointerPosition(int32_t*, int32_t*) const override { return false; }
    bool IsKeyPressed(uint32_t) const override { return false; }

    InputEventCallback send;

private:
    bool* m_ShutDown;
};

InputEvent KeyDown()
{
    InputEvent e{};
    e.type = InputEventType::KeyDown;
    e.pressed = true;
    return e;
}
}  // namespace

TEST(InputSources, OneCanBeRemovedByName)
{
    DekiInput::DekiInput::Shutdown();
    bool shutDown = false;
    DekiInput::DekiInput::SetInput(std::make_unique<FakeSource>(&shutDown), "Gone");
    EXPECT_TRUE(DekiInput::DekiInput::RemoveInput("Gone"));
    EXPECT_TRUE(shutDown);
    EXPECT_EQ(DekiInput::DekiInput::GetInput("Gone"), nullptr);
    EXPECT_FALSE(DekiInput::DekiInput::RemoveInput("Gone")) << "nothing left to remove";
    DekiInput::DekiInput::Shutdown();
}

TEST(InputSources, WhatASessionAddedGoesWhenItEnds)
{
    DekiInput::DekiInput::Shutdown();
    auto host = std::make_unique<FakeSource>();
    FakeSource* hostSource = host.get();
    DekiInput::DekiInput::SetInput(std::move(host), "Host");
    int hostHeard = 0, gameHeard = 0;
    DekiInput::DekiInput::RegisterEventCallback([&](const InputEvent&) { ++hostHeard; });

    DekiInput::DekiInput::BeginSession();
    bool gameShutDown = false;
    DekiInput::DekiInput::SetInput(std::make_unique<FakeSource>(&gameShutDown), "Game");
    DekiInput::DekiInput::RegisterEventCallback([&](const InputEvent&) { ++gameHeard; });
    hostSource->send(KeyDown());
    EXPECT_EQ(hostHeard, 1);
    EXPECT_EQ(gameHeard, 1);

    DekiInput::DekiInput::EndSession();
    EXPECT_EQ(DekiInput::DekiInput::GetInput("Game"), nullptr);
    EXPECT_TRUE(gameShutDown);
    EXPECT_NE(DekiInput::DekiInput::GetInput("Host"), nullptr) << "the host's own source stays";
    hostSource->send(KeyDown());
    EXPECT_EQ(hostHeard, 2) << "the host's callback stays";
    EXPECT_EQ(gameHeard, 1) << "the game's callback is gone";
    DekiInput::DekiInput::Shutdown();
}

TEST(InputSources, AHostCallbackInASlotTheGameFreedStays)
{
    DekiInput::DekiInput::Shutdown();
    auto host = std::make_unique<FakeSource>();
    FakeSource* hostSource = host.get();
    DekiInput::DekiInput::SetInput(std::move(host), "Host");

    DekiInput::DekiInput::BeginSession();
    const size_t game = DekiInput::DekiInput::RegisterEventCallback([](const InputEvent&) {});
    DekiInput::DekiInput::UnregisterEventCallback(game);
    DekiInput::DekiInput::EndSession();

    int heard = 0;
    DekiInput::DekiInput::RegisterEventCallback([&](const InputEvent&) { ++heard; });  // may reuse the slot
    DekiInput::DekiInput::BeginSession();
    DekiInput::DekiInput::EndSession();
    hostSource->send(KeyDown());
    EXPECT_EQ(heard, 1);
    DekiInput::DekiInput::Shutdown();
}
