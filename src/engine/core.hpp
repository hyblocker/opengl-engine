#pragma once

#if _DEBUG
#define ASSERT_BREAK() (__debugbreak())
#define ASSERT(s) do { if (!(s)) { ASSERT_BREAK();} } while (0)
#else
#define ASSERT_BREAK()
#define ASSERT(s) 
#endif