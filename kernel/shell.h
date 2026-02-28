#pragma once

#include <stddef.h>

#include "console.h"
#include "string.h"

namespace shell {

constexpr size_t COMMAND_BUFFER_SIZE = 256u;
constexpr size_t ECHO_PREFIX_LENGTH = 5u; // "echo "

bool isEmptyCommand(const char *input) noexcept;
void printBanner();
void printPrompt();
bool handleExit(const char *input);
bool handleHelp(const char *input);
bool handleClear(const char *input);
bool handleEcho(const char *input);
void handleUnknown(const char *input);

} // namespace shell
