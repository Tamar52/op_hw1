#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"

std::string prompt_message = "smash";

int main(int argc, char* argv[]) {
    if(signal(SIGTSTP , ctrlZHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-Z handler");
    }
    if(signal(SIGINT , ctrlCHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }

    //TODO: setup sig alarm handler

    SmallShell& smash = SmallShell::getInstance();
    while(true) {
        std::cout << prompt_message << "> ";
        std::string cmd_line;
        std::getline(std::cin, cmd_line);
        if(!(smash.executeCommand(cmd_line.c_str()))){ // to change the shell execute to bool
            break;
        }
    }
    exit(0);
}