// A trackball's pulses and click, as arrow keys or as a pointer.
//
// The lines only say "a step happened"; how many arrive in a frame depends on
// how fast the ball was rolled. What a frame's worth of steps becomes - one
// press, a repeated press, a pointer move that stops at the edge - is decided
// here and nowhere else.

#include <gtest/gtest.h>

#include <Trackball.h>
#include <Keys.h>

#include <vector>

using namespace DekiInput;

namespace
{

struct Bench
{
    Trackball::Sample next;
    uint32_t now = 0;
    std::vector<InputEvent> events;
    Trackball ball;

    Bench()
        : ball(
              [this]()
              {
                  const Trackball::Sample s = next;
                  next = Trackball::Sample{};
                  next.pressed = s.pressed;  // a switch stays where it is
                  return s;
              },
              [this]() { return now; })
    {
        ball.RegisterEventCallback([this](const InputEvent& e) { events.push_back(e); });
        ball.SetPointerArea(320, 240);
        EXPECT_TRUE(ball.Initialize());
    }

    void Frame()
    {
        now += 16;
        ball.Update();
    }
    std::vector<InputEvent> Take()
    {
        std::vector<InputEvent> out;
        out.swap(events);
        return out;
    }
};

}  // namespace

TEST(TrackballKeys, AStepIsAnArrowPressedThenReleased)
{
    Bench b;
    b.next.up = 1;
    b.Frame();
    auto e = b.Take();
    ASSERT_EQ(e.size(), 1u);
    EXPECT_EQ(e[0].type, InputEventType::KEY_DOWN);
    EXPECT_EQ(e[0].key, Keys::Up);
    EXPECT_TRUE(b.ball.IsKeyPressed(Keys::Up));

    b.Frame();
    e = b.Take();
    ASSERT_EQ(e.size(), 1u);
    EXPECT_EQ(e[0].type, InputEventType::KEY_UP);
    EXPECT_EQ(e[0].key, Keys::Up);
    EXPECT_FALSE(b.ball.IsKeyPressed(Keys::Up));
}

TEST(TrackballKeys, SeveralStepsInAFrameAreOnePressAndRollingRepeatsIt)
{
    Bench b;
    b.next.right = 5;
    b.Frame();
    EXPECT_EQ(b.Take().size(), 1u) << "a fast roll is not five presses in one frame";

    b.next.right = 3;
    b.Frame();
    auto e = b.Take();
    ASSERT_EQ(e.size(), 2u);
    EXPECT_EQ(e[0].type, InputEventType::KEY_UP);
    EXPECT_EQ(e[1].type, InputEventType::KEY_DOWN) << "still rolling: pressed again this frame";
}

TEST(TrackballKeys, OppositeStepsCancelAndDiagonalsAreTwoKeys)
{
    Bench b;
    b.next.left = 2;
    b.next.right = 2;
    b.Frame();
    EXPECT_TRUE(b.Take().empty());

    b.next.up = 1;
    b.next.left = 1;
    b.Frame();
    auto e = b.Take();
    ASSERT_EQ(e.size(), 2u);
    EXPECT_TRUE(b.ball.IsKeyPressed(Keys::Up));
    EXPECT_TRUE(b.ball.IsKeyPressed(Keys::Left));
}

TEST(TrackballKeys, TheClickIsEnterHeldForAsLongAsItIsPressed)
{
    Bench b;
    b.next.pressed = true;
    b.Frame();
    auto e = b.Take();
    ASSERT_EQ(e.size(), 1u);
    EXPECT_EQ(e[0].type, InputEventType::KEY_DOWN);
    EXPECT_EQ(e[0].key, Keys::Enter);

    b.Frame();
    EXPECT_TRUE(b.Take().empty()) << "a held switch is one press, not one per frame";
    EXPECT_TRUE(b.ball.IsKeyPressed(Keys::Enter));

    b.next.pressed = false;
    b.Frame();
    e = b.Take();
    ASSERT_EQ(e.size(), 1u);
    EXPECT_EQ(e[0].type, InputEventType::KEY_UP);
    EXPECT_FALSE(b.ball.IsKeyPressed(Keys::Enter));
}

TEST(TrackballPointer, StartsInTheMiddleAndMovesPixelsPerStep)
{
    Bench b;
    b.ball.SetMode(Trackball::Mode::Pointer);
    b.ball.SetPixelsPerStep(4);
    b.ball.Initialize();

    int32_t x = 0, y = 0;
    ASSERT_TRUE(b.ball.GetPointerPosition(&x, &y));
    EXPECT_EQ(x, 160);
    EXPECT_EQ(y, 120);

    b.next.right = 2;
    b.next.up = 1;
    b.Frame();
    auto e = b.Take();
    ASSERT_EQ(e.size(), 1u);
    EXPECT_EQ(e[0].type, InputEventType::MOUSE_MOVE);
    EXPECT_EQ(e[0].x, 168);
    EXPECT_EQ(e[0].y, 116);
    EXPECT_FALSE(b.ball.IsKeyPressed(Keys::Right)) << "a pointer does not press arrows";
}

TEST(TrackballPointer, StopsAtTheEdgeAndSaysNothingWhenItCannotMove)
{
    Bench b;
    b.ball.SetMode(Trackball::Mode::Pointer);
    b.ball.SetPixelsPerStep(100);
    b.ball.Initialize();

    b.next.left = 5;
    b.Frame();
    auto e = b.Take();
    ASSERT_EQ(e.size(), 1u);
    EXPECT_EQ(e[0].x, 0);

    b.next.left = 1;
    b.Frame();
    EXPECT_TRUE(b.Take().empty());
}

TEST(TrackballPointer, TheClickIsAMouseButtonAtThePointer)
{
    Bench b;
    b.ball.SetMode(Trackball::Mode::Pointer);
    b.ball.Initialize();

    b.next.pressed = true;
    b.Frame();
    auto e = b.Take();
    ASSERT_EQ(e.size(), 1u);
    EXPECT_EQ(e[0].type, InputEventType::MOUSE_BUTTON_DOWN);
    EXPECT_EQ(e[0].x, 160);
    EXPECT_EQ(e[0].y, 120);

    b.next.pressed = false;
    b.Frame();
    e = b.Take();
    ASSERT_EQ(e.size(), 1u);
    EXPECT_EQ(e[0].type, InputEventType::MOUSE_BUTTON_UP);
}
