#pragma once

#include <stddef.h>
#include <stdint.h>

namespace console {

constexpr uintptr_t VGA_BUFFER_ADDRESS = 0xB8000u;
constexpr size_t VGA_WIDTH = 80u;
constexpr size_t VGA_HEIGHT = 25u;

// Default is white text on blue background
constexpr uint8_t DEFAULT_FG = 0x0Fu;
constexpr uint8_t DEFAULT_BG = 0x01u;

void clear();
void putch(char ch, uint8_t fg = DEFAULT_FG, uint8_t bg = DEFAULT_BG);
void print(const char *s);
char getch();
void readline(char *buffer, size_t max_length);

} // namespace console
