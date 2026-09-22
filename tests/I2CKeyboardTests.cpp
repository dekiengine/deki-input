// A keyboard that reports characters, turned into key events.
//
// The device says what was typed and nothing else: no releases, no key ids,
// and nothing at all until its own controller has started. Each of those is a
// decision made in I2CKeyboard, and each is wrong in a way that still looks
// like a working keyboard for a while - a key that never releases, a shifted
// letter no game recognises, a bus timeout every frame on a board without one.

#include <gtest/gtest.h>

#include <I2CKeyboard.h>
#include <Keys.h>

#include <deque>
#include <vector>

using namespace DekiInput;

namespace
{

// A keyboard on a bench: what it will answer, whether it is there, the time.
struct Bench
{
    std::deque<uint8_t> typed;
    bool present = true;
    bool answers = true;
    uint32_t now = 0;
    int probes = 0;
    int reads = 0;

    std::vector<InputEvent> events;
    I2CKeyboard keyboard;

    Bench()
        : keyboard(
              [this](uint8_t& out)
              {
                  ++reads;
                  if (!answers) return false;
                  out = 0;
                  if (!typed.empty())
                  {
                      out = typed.front();
                      typed.pop_front();
                  }
                  return true;
              },
              [this]()
              {
                  ++probes;
                  return present;
              },
              [this]() { return now; })
    {
        keyboard.RegisterEventCallback([this](const InputEvent& e) { events.push_back(e); });
        EXPECT_TRUE(keyboard.Initialize());
    }

    void Frame(uint32_t ms = 16)
    {
        now += ms;
        keyboard.Update();
    }

    // Through the first update, which is the one that finds the device.
    void Found()
    {
        Frame();
        ASSERT_EQ(keyboard.GetState(), I2CKeyboard::State::Active);
    }
};

}  // namespace

TEST(I2CKeyboard, ACharacterIsAPressThenARelease)
{
    Bench b;
    b.Found();

    b.typed.push_back('a');
    b.Frame();
    ASSERT_EQ(b.events.size(), 1u);
    EXPECT_EQ(b.events[0].type, InputEventType::KEY_DOWN);
    EXPECT_EQ(b.events[0].key, Keys::A);
    EXPECT_EQ(b.events[0].character, uint32_t('a'));
    EXPECT_TRUE(b.events[0].pressed);
    EXPECT_TRUE(b.keyboard.IsKeyPressed(Keys::A));

    // The device never says the key came up, so the next update does.
    b.Frame();
    ASSERT_EQ(b.events.size(), 2u);
    EXPECT_EQ(b.events[1].type, InputEventType::KEY_UP);
    EXPECT_EQ(b.events[1].key, Keys::A);
    EXPECT_FALSE(b.events[1].pressed);
    EXPECT_FALSE(b.keyboard.IsKeyPressed(Keys::A));

    b.Frame();
    EXPECT_EQ(b.events.size(), 2u) << "an idle keyboard says nothing";
}

TEST(I2CKeyboard, AShiftedLetterIsTheSameKeyTypingAnotherCharacter)
{
    Bench b;
    b.Found();

    b.typed.push_back('A');
    b.Frame();
    ASSERT_EQ(b.events.size(), 1u);
    EXPECT_EQ(b.events[0].key, Keys::A) << "a game asking about the A key must not care about shift";
    EXPECT_EQ(b.events[0].character, uint32_t('A'));
}

TEST(I2CKeyboard, EnterAndBackspaceAreKeysThatTypeNothing)
{
    Bench b;
    b.Found();

    b.typed.push_back(13);
    b.Frame();
    b.typed.push_back(8);
    b.Frame();

    ASSERT_EQ(b.events.size(), 3u);  // down, up + down
    EXPECT_EQ(b.events[0].key, Keys::Enter);
    EXPECT_EQ(b.events[0].character, 0u);
    EXPECT_EQ(b.events[2].key, Keys::Backspace);
    EXPECT_EQ(b.events[2].character, 0u);
}

TEST(I2CKeyboard, TheSameKeyTwiceIsTwoPresses)
{
    Bench b;
    b.Found();

    b.typed.push_back('l');
    b.typed.push_back('l');
    b.Frame();
    b.Frame();
    b.Frame();

    ASSERT_EQ(b.events.size(), 4u);
    EXPECT_EQ(b.events[0].type, InputEventType::KEY_DOWN);
    EXPECT_EQ(b.events[1].type, InputEventType::KEY_UP);
    EXPECT_EQ(b.events[2].type, InputEventType::KEY_DOWN);
    EXPECT_EQ(b.events[3].type, InputEventType::KEY_UP);
}

TEST(I2CKeyboard, AKeyboardStillStartingIsFoundLater)
{
    Bench b;
    b.present = false;

    b.Frame();
    EXPECT_EQ(b.keyboard.GetState(), I2CKeyboard::State::Searching);
    EXPECT_EQ(b.reads, 0) << "nothing is read from a device that has not answered a probe";

    // Not probed every frame: an unanswered probe is a bus timeout.
    for (int i = 0; i < 10; ++i)
        b.Frame(16);
    EXPECT_LE(b.probes, 2);

    b.present = true;
    b.Frame(I2CKeyboard::kProbeIntervalMs);
    EXPECT_EQ(b.keyboard.GetState(), I2CKeyboard::State::Active);

    b.typed.push_back('k');
    b.Frame();
    ASSERT_EQ(b.events.size(), 1u);
    EXPECT_EQ(b.events[0].key, Keys::K);
}

TEST(I2CKeyboard, ABoardWithoutOneStopsBeingAsked)
{
    Bench b;
    b.present = false;

    for (uint32_t t = 0; t <= I2CKeyboard::kGiveUpAfterMs + 1000; t += 100)
        b.Frame(100);
    EXPECT_EQ(b.keyboard.GetState(), I2CKeyboard::State::GaveUp);

    const int probes = b.probes;
    for (int i = 0; i < 100; ++i)
        b.Frame(100);
    EXPECT_EQ(b.probes, probes);
    EXPECT_EQ(b.reads, 0);
    EXPECT_TRUE(b.events.empty());
}

TEST(I2CKeyboard, OneThatStopsAnsweringIsLookedForAgainAndAHeldKeyIsReleased)
{
    Bench b;
    b.Found();

    b.typed.push_back('x');
    b.Frame();
    ASSERT_TRUE(b.keyboard.IsKeyPressed(Keys::X));

    b.answers = false;
    b.present = false;
    for (int i = 0; i < I2CKeyboard::kLostAfterFailures + 1; ++i)
        b.Frame();

    EXPECT_FALSE(b.keyboard.IsKeyPressed(Keys::X)) << "a key must not stay down because the bus went quiet";
    EXPECT_EQ(b.keyboard.GetState(), I2CKeyboard::State::Searching);

    b.answers = true;
    b.present = true;
    b.Frame(I2CKeyboard::kProbeIntervalMs);
    EXPECT_EQ(b.keyboard.GetState(), I2CKeyboard::State::Active);
}

TEST(Keys, ACharacterFoldsToItsKey)
{
    EXPECT_EQ(Keys::ForCharacter('Q'), Keys::Q);
    EXPECT_EQ(Keys::ForCharacter('q'), Keys::Q);
    EXPECT_EQ(Keys::ForCharacter('7'), Keys::Num7);
    EXPECT_EQ(Keys::ForCharacter(' '), Keys::Space);
    EXPECT_EQ(Keys::ForCharacter('!'), uint32_t('!'));
}
