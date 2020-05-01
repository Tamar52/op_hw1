#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"


using namespace std;

void ctrlZHandler(int sig_num) {
    cout << "smash: got ctrl-Z\n";
    JobEntry* cur_foreground_job = SmallShell::getInstance().get_job_list()->getJobWithStatusForegroind();
    if(cur_foreground_job){
        cur_foreground_job->changeLastStatusOfJob(FOREGROUND);
        cur_foreground_job->changeStatusOfJob(STOPPED);
        if(kill(cur_foreground_job->getJobPid(),SIGSTOP) == -1) {
            perror("smash error: kill failed");
            return;
        }
        //reset maybe time
        cout << "smash: process"<< to_string(cur_foreground_job->getJobPid()) << "was stopped\n";
    }
}

void ctrlCHandler(int sig_num) {
    cout << "smash: got ctrl-C\n";
    JobEntry* cur_foreground_job = SmallShell::getInstance().get_job_list()->getJobWithStatusForegroind();
    if(cur_foreground_job){
        if(kill(cur_foreground_job->getJobPid(), SIGINT) == -1){
            perror("smash error: kill failed");
            return;
        }
    }
    cout << "smash: process" << to_string(cur_foreground_job->getJobPid()) << "was killed\n";
}

void alarmHandler(int sig_num) {
    cout << "smash got an alarm\n";

//    //TODO: how can we find out who causes the alarm?
//    command =
//
//    cout << "smash:" << command << "timed out!\n";

}

