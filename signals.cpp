#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"


using namespace std;

void ctrlZHandler(int sig_num) {
    cout << "smash: got ctrl-Z\n";
//    //TODO: DOSE THIS ONE EXIST? NEEDS TO BE IMPLEMENTED? add an if for no pid returned
//    pid_t foreground_pid = getForgroundPid();
//
//    //TODO: should it really be with the kill function?
//    kill(foreground_pid, SIGSTOP);
//    cout << "smash: process"<< to_string(foreground_pid) << "was stopped\n";
}

void ctrlCHandler(int sig_num) {
    cout << "smash: got ctrl-C\n";
//    //TODO: DOSE THIS ONE EXIST? NEEDS TO BE IMPLEMENTED?add an if for no pid returned
//    pid_t foreground_pid = getForgroundPid();
//
//    //TODO: should it really be with the kill function?
//    kill(foreground_pid, SIGKILL);
//    cout << "smash: process" << to_string(foreground_pid) << "was killed\n";
}

void alarmHandler(int sig_num) {
    cout << "smash got an alarm\n";

//    //TODO: how can we find out who causes the alarm?
//    command =
//
//    cout << "smash:" << command << "timed out!\n";

}

