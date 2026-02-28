#include "console.h"
#include "shell.h"

extern "C" void kmain(void) {
  console::clear();
  shell::printBanner();

  for (;;) {
    char buffer[shell::COMMAND_BUFFER_SIZE];
    shell::printPrompt();
    console::readline(buffer, sizeof(buffer));
    console::print("\n");

    if (shell::handleExit(buffer)) {
      break;
    }

    if (shell::handleHelp(buffer)) {
      continue;
    }

    if (shell::handleClear(buffer)) {
      continue;
    }

    if (shell::handleEcho(buffer)) {
      continue;
    }

    if (shell::isEmptyCommand(buffer)) {
      continue;
    } else {
      shell::handleUnknown(buffer);
    }
  }

  console::print("\nGoodbye!\n");
}
