#pragma once

#if _DEBUG
#define ASSERT_BREAK() (__debugbreak())
#define ASSERT(s) do { if (!(s)) { ASSERT_BREAK();} } while (0)
#else
#define ASSERT_BREAK()
#define ASSERT(s) 
#endif

// helpers to convert radians to degrees and back
constexpr float DEG2RAD = (3.14159265 / 180.0);
constexpr float RAD2DEG = (180 / 3.14159265);