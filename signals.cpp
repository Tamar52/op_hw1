#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"
#include <unistd.h>

using namespace std;

void ctrlZHandler(int sig_num) {
    if(getpid() == SmallShell::getInstance().getPid()) {
        cout << "smash: got ctrl-Z" << endl;
    }
    pid_t foreground_pid = SmallShell::getInstance().getForegrounfPid();
    if(foreground_pid == 0){
        return;
    }
    if(kill(foreground_pid,SIGSTOP) == -1) {
        perror("smash error: kill failed");
        return;
    }
        //reset maybe time
        if(getpid() == SmallShell::getInstance().getPid()) {
            cout << "smash: process " << to_string(foreground_pid) << " was stopped" << endl;
        }
//    SmallShell::getInstance().setForegrounfPid(0);
}


void ctrlCHandler(int sig_num) {
    if(getpid() == SmallShell::getInstance().getPid()) {
        cout << "smash: got ctrl-C" << endl;
    }
    pid_t foreground_pid = SmallShell::getInstance().getForegrounfPid();
    if(foreground_pid == 0){
        return;
    }
    if(kill(foreground_pid, SIGKILL) == -1){
        perror("smash error: kill failed");
        return;
    }
    if(getpid() == SmallShell::getInstance().getPid()) {
        cout << "smash: process " << to_string(foreground_pid) << " was killed" << endl;
    }
//    SmallShell::getInstance().setForegrounfPid(0);
}

void alarmHandler(int sig_num) {
    cout << "smash got an alarm"<< endl;

//    //TODO: how can we find out who causes the alarm?
//    command =
//
//    cout << "smash:" << command << "timed out!\n";

}

