#include "console.h"

namespace console {

namespace internal {

constexpr uint16_t makeVgaEntry(char ch, uint8_t fg, uint8_t bg) {
  const uint16_t color = static_cast<uint16_t>((bg << 4u) | (fg & 0x0Fu));
  return static_cast<uint16_t>(static_cast<uint16_t>(ch) |
                               static_cast<uint16_t>(color << 8u));
}

inline uint8_t inb(uint16_t port) {
  uint8_t value;
  asm volatile("inb %1, %0" : "=a"(value) : "Nd"(port));

  return value;
}

volatile uint16_t *const buffer =
    reinterpret_cast<volatile uint16_t *>(VGA_BUFFER_ADDRESS);
size_t row = 0u;
size_t col = 0u;
bool shiftActive = false;
bool capsLockActive = false;

void scrollIfNeeded() {
  if (row < VGA_HEIGHT) {
    return;
  }

  for (size_t r = 1u; r < VGA_HEIGHT; ++r) {
    for (size_t c = 0u; c < VGA_WIDTH; ++c) {
      buffer[(r - 1u) * VGA_WIDTH + c] = buffer[r * VGA_WIDTH + c];
    }
  }

  const uint16_t blank = makeVgaEntry(' ', DEFAULT_FG, DEFAULT_BG);
  for (size_t c = 0u; c < VGA_WIDTH; ++c) {
    buffer[(VGA_HEIGHT - 1u) * VGA_WIDTH + c] = blank;
  }

  row = VGA_HEIGHT - 1u;
}

void putChar(char ch, uint8_t fg, uint8_t bg) {
  if (ch == '\n') {
    col = 0u;
    ++row;
    scrollIfNeeded();
    return;
  }

  if (ch == '\r') {
    col = 0u;
    return;
  }

  if (ch == '\b') {
    if (col > 0u) {
      --col;
    } else if (row > 0u) {
      --row;
      col = VGA_WIDTH - 1u;
    }
    buffer[row * VGA_WIDTH + col] = makeVgaEntry(' ', fg, bg);
    return;
  }

  buffer[row * VGA_WIDTH + col] = makeVgaEntry(ch, fg, bg);

  ++col;
  if (col >= VGA_WIDTH) {
    col = 0u;
    ++row;
    scrollIfNeeded();
  }
}

constexpr char decodeScancode(uint8_t scancode) noexcept {
  constexpr char UNKNOWN_KEY = '\0';

  struct ScancodeMapping {
    uint8_t code;
    char ch;
  };

  constexpr ScancodeMapping SCANCODE_MAPPINGS[] = {
      {0x01u, '\x1B'}, // Escape
      {0x02u, '1'},
      {0x03u, '2'},
      {0x04u, '3'},
      {0x05u, '4'},
      {0x06u, '5'},
      {0x07u, '6'},
      {0x08u, '7'},
      {0x09u, '8'},
      {0x0Au, '9'},
      {0x0Bu, '0'},
      {0x0Cu, '-'},
      {0x0Du, '='},
      {0x0Eu, '\b'},
      {0x0Fu, '\t'},
      {0x10u, 'q'},
      {0x11u, 'w'},
      {0x12u, 'e'},
      {0x13u, 'r'},
      {0x14u, 't'},
      {0x15u, 'y'},
      {0x16u, 'u'},
      {0x17u, 'i'},
      {0x18u, 'o'},
      {0x19u, 'p'},
      {0x1Au, '['},
      {0x1Bu, ']'},
      {0x1Cu, '\n'},
      {0x1Eu, 'a'},
      {0x1Fu, 's'},
      {0x20u, 'd'},
      {0x21u, 'f'},
      {0x22u, 'g'},
      {0x23u, 'h'},
      {0x24u, 'j'},
      {0x25u, 'k'},
      {0x26u, 'l'},
      {0x27u, ';'},
      {0x28u, '\''},
      {0x29u, '`'},
      {0x2Bu, '\\'},
      {0x2Cu, 'z'},
      {0x2Du, 'x'},
      {0x2Eu, 'c'},
      {0x2Fu, 'v'},
      {0x30u, 'b'},
      {0x31u, 'n'},
      {0x32u, 'm'},
      {0x33u, ','},
      {0x34u, '.'},
      {0x35u, '/'},
      {0x39u, ' '},
      // Numeric keypad
      {0x47u, '7'},
      {0x48u, '8'},
      {0x49u, '9'},
      {0x4Au, '-'},
      {0x4Bu, '4'},
      {0x4Cu, '5'},
      {0x4Du, '6'},
      {0x4Eu, '+'},
      {0x4Fu, '1'},
      {0x50u, '2'},
      {0x51u, '3'},
      {0x52u, '0'},
      {0x53u, '.'},
  };

  constexpr size_t SCANCODE_MAPPINGS_COUNT =
      sizeof(SCANCODE_MAPPINGS) / sizeof(SCANCODE_MAPPINGS[0]);

  for (size_t i = 0u; i < SCANCODE_MAPPINGS_COUNT; ++i) {
    if (SCANCODE_MAPPINGS[i].code == scancode) {
      return SCANCODE_MAPPINGS[i].ch;
    }
  }
  return UNKNOWN_KEY;
}

char readKeyBlocking() {
  constexpr uint16_t KEYBOARD_DATA_PORT = 0x60u;
  constexpr uint16_t KEYBOARD_STATUS_PORT = 0x64u;
  constexpr uint8_t KEYBOARD_OUTPUT_BUFFER_MASK = 0x01u;
  constexpr uint8_t KEYBOARD_BREAK_CODE_MASK = 0x80u;
  constexpr uint8_t LEFT_SHIFT_SCANCODE = 0x2Au;
  constexpr uint8_t RIGHT_SHIFT_SCANCODE = 0x36u;
  constexpr uint8_t CAPS_LOCK_SCANCODE = 0x3Au;

  for (;;) {
    const uint8_t status = inb(KEYBOARD_STATUS_PORT);
    if ((status & KEYBOARD_OUTPUT_BUFFER_MASK) == 0u) {
      continue;
    }

    const uint8_t rawScancode = inb(KEYBOARD_DATA_PORT);
    const bool isRelease = (rawScancode & KEYBOARD_BREAK_CODE_MASK) != 0u;
    const uint8_t scancode = static_cast<uint8_t>(
        rawScancode & static_cast<uint8_t>(~KEYBOARD_BREAK_CODE_MASK));

    if (scancode == LEFT_SHIFT_SCANCODE || scancode == RIGHT_SHIFT_SCANCODE) {
      shiftActive = !isRelease;
      continue;
    }

    if (scancode == CAPS_LOCK_SCANCODE) {
      if (!isRelease) {
        capsLockActive = !capsLockActive;
      }
      continue;
    }

    if (isRelease) {
      continue;
    }

    const char decoded = decodeScancode(scancode);
    if (decoded == '\0') {
      continue;
    }

    char result = decoded;
    const bool isLetter = (decoded >= 'a') && (decoded <= 'z');
    const bool makeUpper = isLetter && (shiftActive != capsLockActive);
    if (makeUpper) {
      result = static_cast<char>(decoded - 'a' + 'A');
    }

    return result;
  }
}

} // namespace internal

void putch(char ch, uint8_t fg, uint8_t bg) { internal::putChar(ch, fg, bg); }

void clear() {
  const uint16_t blank = internal::makeVgaEntry(' ', DEFAULT_FG, DEFAULT_BG);
  for (size_t r = 0u; r < VGA_HEIGHT; ++r) {
    for (size_t c = 0u; c < VGA_WIDTH; ++c) {
      internal::buffer[r * VGA_WIDTH + c] = blank;
    }
  }

  internal::row = 0u;
  internal::col = 0u;
}

void print(const char *s) {
  if (s == nullptr) {
    return;
  }

  constexpr char ColorPrefix = '\x02';
  constexpr char ColorMarker = '\x01';

  uint8_t current_fg = DEFAULT_FG;
  uint8_t current_bg = DEFAULT_BG;

  for (size_t i = 0u; s[i] != '\0'; ++i) {
    if (s[i] == ColorPrefix && s[i + 1] == ColorMarker && s[i + 2] != '\0' &&
        s[i + 3] != '\0') {
      current_fg = static_cast<uint8_t>(s[i + 2]);
      current_bg = static_cast<uint8_t>(s[i + 3]);
      i += 3;
      continue;
    }

    putch(s[i], current_fg, current_bg);
  }
}

char getch() { return internal::readKeyBlocking(); }

void readline(char *buffer, size_t max_length) {
  size_t index = 0u;

  for (;;) {
    const char ch = getch();
    if (ch == '\n' || ch == '\r') {
      buffer[index] = '\0';
      break;
    }
    if (ch == '\b') {
      if (index > 0u) {
        --index;
        putch('\b');
      }
      continue;
    }
    if (index < (max_length - 1u)) {
      buffer[index] = ch;
      ++index;
      putch(ch);
    }
  }
}

} // namespace console
