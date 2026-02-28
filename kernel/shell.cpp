#include "shell.h"

namespace shell {

bool isEmptyCommand(const char *input) noexcept {
  return input != nullptr && input[0] == '\0';
}

void printBanner() {
  console::print(
      "\x02\x01\x0f\x01  ______    _     _             ____   _____ \n"
      " |  ____|  (_)   | |           / __ \\ / ____|\n"
      " | |__ _ __ _  __| | __ _  ___| |  | | (___  \n"
      " |  __| '__| |/ _` |/ _` |/ _ \\ |  | |\\___ \\\n"
      " | |  | |  | | (_| | (_| |  __/ |__| |____) |\n"
      " |_|  |_|  |_|\\__,_|\\__, |\\___|\\____/|_____/ \n"
      "                     __/ |                   \n"
      "                    |___/                    \n");
}

void printPrompt() { console::print("> "); }

bool handleExit(const char *input) {
  return kstring::startsWith(input, "exit");
}

bool handleHelp(const char *input) {
  if (!kstring::startsWith(input, "help")) {
    return false;
  }

  console::print("Available commands: exit, help, clear, echo <message>\n");

  return true;
}

bool handleClear(const char *input) {
  if (!kstring::startsWith(input, "clear")) {
    return false;
  }

  console::clear();

  return true;
}

bool handleEcho(const char *input) {
  if (!kstring::startsWith(input, "echo")) {
    return false;
  }

  console::print("[ECHO] ");
  console::print(input + ECHO_PREFIX_LENGTH);
  console::print("\n");

  return true;
}

void handleUnknown(const char *input) {
  console::print("Unknown command: ");
  console::print(input);
  console::print("\n");
}

} // namespace shell
