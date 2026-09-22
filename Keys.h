#pragma once

#include <cstdint>

// The key ids every input driver reports and every game asks about:
//
//     if (DekiInput::DekiInput::IsKeyPressed(DekiInput::Keys::Space)) ...
//
// A key that types a printable character is that character's ASCII code, in
// lower case for letters; the rest have ids of their own. These are the values
// the engine has always used (its own list is private to it), which every
// driver used to restate and every game used to write as bare numbers.
//
// A key is not a character. Holding shift and pressing the A key is the key
// Keys::A typing 'A': InputEvent::key carries the first, InputEvent::character
// the second.

namespace DekiInput::Keys
{

inline constexpr uint32_t Backspace = 8;
inline constexpr uint32_t Tab = 9;
inline constexpr uint32_t Enter = 13;
inline constexpr uint32_t Esc = 27;
inline constexpr uint32_t Space = 32;
inline constexpr uint32_t Delete = 127;

inline constexpr uint32_t Up = 1001;
inline constexpr uint32_t Down = 1002;
inline constexpr uint32_t Left = 1003;
inline constexpr uint32_t Right = 1004;

inline constexpr uint32_t Num0 = '0';
inline constexpr uint32_t Num1 = '1';
inline constexpr uint32_t Num2 = '2';
inline constexpr uint32_t Num3 = '3';
inline constexpr uint32_t Num4 = '4';
inline constexpr uint32_t Num5 = '5';
inline constexpr uint32_t Num6 = '6';
inline constexpr uint32_t Num7 = '7';
inline constexpr uint32_t Num8 = '8';
inline constexpr uint32_t Num9 = '9';

inline constexpr uint32_t A = 'a';
inline constexpr uint32_t B = 'b';
inline constexpr uint32_t C = 'c';
inline constexpr uint32_t D = 'd';
inline constexpr uint32_t E = 'e';
inline constexpr uint32_t F = 'f';
inline constexpr uint32_t G = 'g';
inline constexpr uint32_t H = 'h';
inline constexpr uint32_t I = 'i';
inline constexpr uint32_t J = 'j';
inline constexpr uint32_t K = 'k';
inline constexpr uint32_t L = 'l';
inline constexpr uint32_t M = 'm';
inline constexpr uint32_t N = 'n';
inline constexpr uint32_t O = 'o';
inline constexpr uint32_t P = 'p';
inline constexpr uint32_t Q = 'q';
inline constexpr uint32_t R = 'r';
inline constexpr uint32_t S = 's';
inline constexpr uint32_t T = 't';
inline constexpr uint32_t U = 'u';
inline constexpr uint32_t V = 'v';
inline constexpr uint32_t W = 'w';
inline constexpr uint32_t X = 'x';
inline constexpr uint32_t Y = 'y';
inline constexpr uint32_t Z = 'z';

/// The key a typed character sits on, for a keyboard that reports characters
/// rather than keys: letters fold to lower case, everything else is itself.
/// A shifted symbol stays the symbol - which key carries '!' is the layout's
/// business, and a device that sends '!' has not said.
inline constexpr uint32_t ForCharacter(uint32_t character)
{
    return (character >= 'A' && character <= 'Z') ? character + ('a' - 'A') : character;
}

}  // namespace DekiInput::Keys
