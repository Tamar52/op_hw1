#include <unistd.h>
#include <string.h>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <fcntl.h>
#include <memory>



using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";


#if 0
#define FUNC_ENTRY()  \
  cerr << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cerr << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

#define DEBUG_PRINT cerr << "DEBUG: "

#define EXEC(path, arg) \
  execvp((path), (arg));

string _ltrim(const std::string& s)
{
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
    return _rtrim(_ltrim(s));
}


string devideCmdLine(const char* cmd_input){
    string fix_input;
    int first = 0, last = 0;
    while(cmd_input[last] != '\0'){
        last++;
    }
    last--;
    while((cmd_input[first] == ' ') && (first < last)){
        first++;
    }
    while((cmd_input[last] == ' ') && (first < last)){
        last--;
    }
    for(int i = 0; first<=last; i++){
        fix_input.append(1,cmd_input[first++]);
    }
    string result;
    for(int i = 0; i<(int)fix_input.size();i++){
        if(fix_input[i] != ' '){
            result.append(1, fix_input[i]);
        }
        else {
            result.append(1, ' ');
            while (fix_input[i + 1] == ' ') { i++; }
        }
    }
    return result;
}







int _parseCommandLine(const string& cmd_line, string args[]) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(cmd_line.c_str());
    string intermediate, first;
    while(getline(iss, intermediate, ' ')){ args[i++] = intermediate; }
    return i;
    /* for(std::string s; iss >> s; ) {
       args[i] = (char*)malloc(s.length()+1);
       memset(args[i], 0, s.length()+1);
       strcpy(args[i], s.c_str());
       args[++i] = NULL;
     }
     return i;
   */
    FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) {
    const string str(cmd_line);
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos) {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&') {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}


/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
shared_ptr<Command> SmallShell::CreateCommand(const char* cmd_line) {

  string cmd_s = string(cmd_line);

  if (cmd_s.find("chprompt") == 0) {
      return make_shared<ChpromptCommand>(cmd_line);
  }
  else if(cmd_s.find("showpid") == 0){
      return make_shared<ShowPidCommand>(cmd_line);
  }
  else if(cmd_s.find("pwd") == 0){
      return make_shared<GetCurrDirCommand>(cmd_line);
  }
  else if(cmd_s.find("cd") == 0){
      if(change_dir == nullptr) {
          change_dir = make_shared<ChangeDirCommand>(cmd_line);
      } else {
          string new_cmd = devideCmdLine(cmd_line);
          string *old_cmd_array = change_dir->getCmdArray();
          _parseCommandLine(new_cmd, old_cmd_array);

      }
      return change_dir;
  }
//  else if(cmd_s.find("jobs") == 0){
//      return new JobsCommand(cmd_line);
//  }
//  else if(cmd_s.find("kill") == 0){
//      return new KillCommand(cmd_line);
//  }
//  else if(cmd_s.find("fg") == 0){
//      return new ForegroundCommand(cmd_line);
//  }
//  else if(cmd_s.find("bg") == 0){
//      return new BackgroundCommand(cmd_line);
//  }
//  else if(cmd_s.find("quit") == 0){
//      return new QuitCommand(cmd_line);
//  }
  else if(cmd_s.find("cp") == 0){
      return make_shared<CopyCommand>(cmd_line);
  }
  //TODO: add a condition for timeout!

  else {
    return make_shared<ExternalCommand>(cmd_line);
  }

    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    shared_ptr<Command> cmd = CreateCommand(cmd_line);
    cmd->execute();
    // Please note that you must fork smash process for some commands (e.g., external commands....)

    //TODO: maybe need to check if wasnt added to job list?

}


Command::Command(const char *cmd_line)
{
    string input = devideCmdLine(cmd_line);
    num_arg = _parseCommandLine(input, cmd_array);

}

Command::~Command(){}

int Command::getNumOfArg() { return num_arg; }

string* Command::getCmdArray() { return cmd_array; }

void GetCurrDirCommand::execute() {
    char temp_pwd[COMMAND_ARGS_MAX_LENGTH];
    getcwd(temp_pwd,COMMAND_ARGS_MAX_LENGTH);
    std::string current_working_dir(temp_pwd);
    cout<< current_working_dir <<endl;
}

bool GetCurrDirCommand::checkArgInput() { return true; }

string GetCurrDirCommand::getCurrentDir() {
    char temp_pwd[COMMAND_MAX_ARGS];
    getcwd(temp_pwd,sizeof(temp_pwd));
    std::string current_working_dir(temp_pwd);
    return current_working_dir;
}


bool ChangeDirCommand::checkArgInput() {
    int temp_num_arg = getNumOfArg();
    if(temp_num_arg > 2){
        cout << "smash error: cd: too many arguments\n";
        return false;
    }
    string* temp_cmd_array = getCmdArray();
    if(last_pwd == "" && temp_cmd_array[1] == "-"){
        cout << "smash error: cd: OLDPWD not set\n";
        return false;
    }
    return true;
}

//To check if .. is working
void ChangeDirCommand::execute() {
    if(checkArgInput() == false){
        return;
    }

    int check;
    string *cmd_array = getCmdArray();
    char temp[COMMAND_ARGS_MAX_LENGTH], array[last_pwd.size() + 1];
    char* new_pwd = getcwd(temp, COMMAND_ARGS_MAX_LENGTH);
    char *path;
    if (cmd_array[1] == "-") {
        strcpy(array, last_pwd.c_str());
        path = array;
        check = chdir(path);
        if (check != 0) {
            perror("smash error: chdir failed");
        } else { last_pwd = new_pwd; }
    } else {
        array[cmd_array[1].size() + 1];
        strcpy(array, cmd_array[1].c_str());
        path = array;
        check = chdir(path);
        if (check != 0) {
            perror("smash error: chdir failed");
        }else{ last_pwd = new_pwd; }
    }
}


bool ShowPidCommand::checkArgInput() { return true; }

//TODO: implement constructor and destructor
void ShowPidCommand::execute() {
    pid_t pid  = getpid();
    cout << "smash pid is " << to_string(pid) << endl;
}

//TODO: understand how to cast the string to char* for execv
void RedirectionCommand::execute() {
    string* command = getCmdArray();
    if (command[1] == ">"){
        pid_t pid = fork();
        if(pid<0){
            perror("smash error: fork failed");
        }
//        if (pid == 0) {
//            setpgrp();
//            close(1);
//            mode_t mode = 0666;
//            int write_fd = open(command[2], O_WRONLY|O_CREAT, mode);
//            if (write_fd == -1) { /* Check if file opened */
//                perror("smash error: open failed");
//
//            }
//            char* args[] ={command[0], nullptr};
//            execv(args[0],args);
//        } else {
//            wait(NULL);
//        }

    } else{
        pid_t pid = fork();
        if(pid<0){
            perror("smash error: fork failed");
        }
        if (pid == 0) {
            setpgrp();
            close(1);
            mode_t mode = 0666;
//            int write_fd = open(command[2], O_APPEND|O_CREAT, mode);
//            if (write_fd == -1) { /* Check if file opened */
//                perror("smash error: open failed");
//
//            }
//            char* args[] ={command[0], nullptr};
//            execv(args[0],args);
        } else {
            wait(NULL);
        }
    }
}

void ChpromptCommand::execute() {
    string * command = getCmdArray();
    if(!command[1].empty()){
       prompt_message = command[1];
    }
    else{
       prompt_message = "smash";
    }
}

//TODO: understand how to cast the string to char* for execv
void PipeCommand::execute() {
    string * command = getCmdArray();
    if(command[2] == "&"){
        int fd[2];
        pipe(fd);
        pid_t child1 = fork();
        if(child1<0){
            perror("smash error: fork failed");
        }
//        if (child1 == 0) {
//            // first child
//            setpgrp();
//            dup2(fd[1],2);
//            close(fd[0]);
//            close(fd[1]);
//            char* args[] ={command[0], nullptr};
//            execv(args[0],args);
//        }

        pid_t child2 = fork();
        if(child2<0){
            perror("smash error: fork failed");
        }
//        if (child2 == 0) {
//            // second child
//            setpgrp();
//            dup2(fd[0],0);
//            close(fd[0]);
//            close(fd[1]);
//            char* args[] ={command[2], nullptr};
//            execv(args[0],args);
//        }

        close(fd[0]);
        close(fd[1]);
    }
    else{
        int fd[2];
        pipe(fd);
        pid_t child1 = fork();
        if(child1<0){
            perror("smash error: fork failed");
        }
//        if (child1 == 0) {
//            // first child
//            setpgrp();
//            dup2(fd[1],1);
//            close(fd[0]);
//            close(fd[1]);
//            char* args[] ={command[0], nullptr};
//            execv(args[0],args);
//        }

        pid_t child2 = fork();
        if(child2<0){
            perror("smash error: fork failed");
        }
//        if (child2 == 0) {
//            // second child
//            setpgrp();
//            dup2(fd[0],0);
//            close(fd[0]);
//            close(fd[1]);
//            char* args[] ={command[1], nullptr};
//            execv(command[1],args);
//        }

        close(fd[0]);
        close(fd[1]);

    }
}

void CopyCommand::execute() {
    //TODO: check if this is the right way of doing this- i used a pipe + should we close both files we've open?
    string * command = getCmdArray();
    int read_fd;
    int write_fd;
    char buffer[1024];
    ssize_t count;
//    read_fd = open(command[0], O_RDONLY);
//    if (read_fd == -1) { /* Check if file opened */
//        perror("smash error: open failed");
//
//    }
    //TODO: are these neede? - S_IRUSR | S_IWUSR
    mode_t mode = 0666;
//    write_fd = open(command[2], O_WRONLY | O_CREAT ,mode);
//    if (write_fd == -1) /* Check if file opened*/
//    {
//        close(read_fd);
//        perror("smash error: open failed");
//    }
    while ((count = read(read_fd, buffer, sizeof(buffer))) != 0){
        write(write_fd, buffer, count);

    }

}

//TODO: understand how to cast the string to char* for execv
void ExternalCommand::execute() {
    string *command = getCmdArray();
    pid_t pid = fork();
    if(pid<0){
        perror("smash error: fork failed");
    }
    if (pid > 0) {
        // smash waits for child
        wait(NULL);
    }
    if(pid == 0){
        string bash_command = "/bin/bash" + command[0];
        command[0] = " ";
        execv( bash_command,command[0]);
    }


}


int JobEntry::getJobID() { return jobID; }

int JobEntry::getJobPid() { return  job_pid; }

string JobEntry::getInputCmd() { return input_cmd; }

JobStatus JobEntry::getJobStatus() { return  job_status; }

JobStatus JobEntry::getJobLastStatus() { return last_status; }

bool JobsCommand::checkArgInput() { return true; }

void JobsCommand::execute() {
    if(checkArgInput() == false){ return; }
    jobs.printJobsList();
}


void JobsList::addJob(string input_cmd, int job_pid, JobStatus job_status) {
    int new_jobID;
    if(list_jobs.empty()){
        new_jobID = 1;
    } else {
        new_jobID = list_jobs.back().getJobID() + 1;
    }
    time_t temp_time;
    time(&temp_time);
    JobEntry new_job(job_pid,new_jobID,job_status,input_cmd,temp_time);
    list_jobs.push_back(new_job);

}

//to check if print w.r.t
void JobsList::printJobsList() {
    for(auto &cur_job : list_jobs) {
        int cur_status = -1;
        JobStatus temp_s = cur_job.getJobStatus();
        if (temp_s == FOREGROUND) { continue; } //dont print the jobs ths run in it the front

        cout << '[' << cur_job.getJobID() << ']' << ' '
             << cur_job.getInputCmd()
             << " : " << cur_job.getJobPid() << " " << cur_job.getTime()
             << " secs";

        if (cur_job.getJobStatus() == STOPPED) { cout << " (stopped)"; }
        cout << endl;
    }
}

JobEntry* JobsList::getJobById(int jobId) {

    for(auto &cur_job : list_jobs){
        if(jobId == cur_job.getJobID()){
            return &cur_job;
        }
    }
    return nullptr;
}




bool KillCommand::checkArgInput() {
    int num_arg = getNumOfArg();
    if(num_arg != 3){ cout <<"smash error: kill: invalid arguments"; }

}


void KillCommand::execute(){
    if(checkArgInput() == false){ return;}

    string* cmd_array = getCmdArray();

    if(kill(jobs->getJobById(stoi(cmd_array[2]))->getJobPid(),
            stoi(cmd_array[1].substr(1, cmd_array[1].size()))) == -1){
        perror("smash error: kill failed");
        return;
    }
    cout << "signal number " << stoi(cmd_array[1].substr(1, cmd_array[1].size()))
         << " was sent to pid " << jobs->getJobById(stoi(cmd_array[2]))->getJobPid()
         << endl;
    usleep(20000);

}





