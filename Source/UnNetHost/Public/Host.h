#pragma once

#ifdef _WIN32
typedef wchar_t unnet_char_t;
#else
typedef char_t unnet_char_t;
#endif

int UnNet_Execute(int argc, unnet_char_t const* argv[]);