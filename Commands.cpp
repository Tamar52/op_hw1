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
void _parseCommandLineByChar(const string& cmd_line, string args[], char sign) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(cmd_line.c_str());
    string intermediate, first;
    while (getline(iss, intermediate, sign)) { args[i++] = intermediate; }
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

bool _isCharInComamnd(const char* cmd_line, const char* sign) {
    const string str(cmd_line);
    if(str.find(sign) == string::npos){
        return false;
    }
    return true;
}
 int _removeChar(char* cmd_line,char sign ) {
    const string str(cmd_line);
    // find character
    int double_sign =0;
    unsigned int idx = str.find(sign);
    // if all characters are spaces then return
    if (idx == string::npos) {
        return -1;
    }
    // if the command line does not end with & then return
    if ((cmd_line[idx]) != sign) {
        return -1;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';

    if ((cmd_line[idx+1]) == sign) {
        cmd_line[idx+1] = ' ';
        double_sign = 1;
    }
    return double_sign;
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
      cmd_name = "External";
    return make_shared<ExternalCommand>(cmd_line);
  }

    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    shared_ptr<Command> cmd = CreateCommand(cmd_line);
    string* command = cmd->getCmdArray();
    int array_size = cmd->getNumOfArg();
    if(_isCharInComamnd(cmd_line, "|")) {
        if (_isBackgroundComamnd(cmd_line)) {
//            pid_t pid = fork();
//            if (pid == 0) {
//                setpgrp();
//                char cmd_temp[(cmd->cmd_line).length()];
//                strcpy(reinterpret_cast<char *>(cmd_temp), cmd->cmd_line.c_str());
//                _removeBackgroundSign(cmd_temp);
//                if (_isCharInComamnd(cmd_line, "&")){
//                    shared_ptr<PipeCommand> pipe = make_shared<PipeCommand>(cmd_temp,1);
//                    pipe->execute();
//                }else {
//                    shared_ptr<PipeCommand> pipe = make_shared<PipeCommand>(cmd_line, 0);
//                    pipe->execute();
//                }
//
//            }
//            if (pid < 0) {
//                perror("smash error: fork failed");
//                return;
//            } else {
//                //TODO: DELETE THE WAIT AND ADD TO JOB LIST
//                wait(nullptr);
//            }
        } else {
            pid_t pid = fork();
            if (pid > 0) {
                // smash waits for child
                wait(nullptr);
            }
            if (pid < 0) {
                perror("smash error: fork failed");
                return;
            }
            if (pid == 0) {
                setpgrp();
                if (_isCharInComamnd(cmd_line, "&")) {
                    shared_ptr<PipeCommand> pipe = make_shared<PipeCommand>(cmd_line, 1);
                    pipe->execute();
                } else {
                    shared_ptr<PipeCommand> pipe = make_shared<PipeCommand>(cmd_line, 0);
                    pipe->execute();
                }
            }
        }

    }else if (_isCharInComamnd(cmd->cmd_line.c_str(), ">")) {
        char cmd_temp[(cmd->cmd_line).length()];
        strcpy(reinterpret_cast<char *>(cmd_temp), cmd->cmd_line.c_str());
        int indicator = _removeChar(cmd_temp, '>');
        string cmd_temp_array[COMMAND_MAX_ARGS];
        string input = devideCmdLine(cmd_temp);
        _parseCommandLine(input, cmd_temp_array);
        if (indicator == 0){
            pid_t pid = fork();
            if (pid < 0) {
                perror("smash error: fork failed");
                return;
             }
            if (pid == 0) {
                setpgrp();
                shared_ptr<RedirectionCommand> rd = make_shared<RedirectionCommand>(" ", 1, command[array_size-1]);
                rd->execute();
                cmd->cmd_line = input;
                size_t pos = cmd->cmd_line.find(command[array_size-1]);
                if (pos != std::string::npos)
                {
                    cmd->cmd_line.erase(pos, command[array_size-1].length());
                }
                cmd->execute();
                exit(0);
            } else {
                wait(NULL);
            }

        }else{
            pid_t pid = fork();
            if (pid < 0) {
                perror("smash error: fork failed");
                return;
            }
            if (pid == 0) {
                setpgrp();
                shared_ptr<RedirectionCommand> rd = make_shared<RedirectionCommand>(" ", 2, command[array_size-1]);
                rd->execute();
                cmd->cmd_line = input;
                size_t pos = cmd->cmd_line.find(command[array_size-1]);
                if (pos != std::string::npos)
                {
                    cmd->cmd_line.erase(pos, command[array_size-1].length());
                }
                cmd->execute();
                exit(0);
            } else {
                wait(NULL);
            }
        }

    }else {
        cmd->execute();
    }

    // Please note that you must fork smash process for some commands (e.g., external commands....)

    //TODO: maybe need to check if wasnt added to job list?

}


Command::Command(const char *cmd_line):cmd_line(cmd_line)
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
            return;
        } else { last_pwd = new_pwd; }
    } else {
        array[cmd_array[1].size() + 1];
        strcpy(array, cmd_array[1].c_str());
        path = array;
        check = chdir(path);
        if (check != 0) {
            perror("smash error: chdir failed");
            return;
        }else{ last_pwd = new_pwd; }
    }
}


bool ShowPidCommand::checkArgInput() { return true; }

//TODO: implement constructor and destructor
void ShowPidCommand::execute() {
    pid_t pid  = getpid();
    string *command = getCmdArray();
    cout << "smash pid is " << to_string(pid) << endl;
}

//TODO: understand how to cast the string to char* for execv
void RedirectionCommand::execute() {
    if (sign == 1){
        close(1);
        mode_t mode = 0666;
        int write_fd = open(path.c_str(), O_WRONLY|O_TRUNC|O_CREAT, mode);
        if (write_fd == -1) { /* Check if file opened */
            perror("smash error: open failed");
            return;
        }
    } else{
        close(1);
        mode_t mode = 0666;
        int write_fd = open(path.c_str(), O_WRONLY|O_APPEND|O_CREAT, mode);
        if (write_fd == -1) { /* Check if file opened */
            perror("smash error: open failed");
            return;
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


void PipeCommand::execute() {
    string cmd_temp_array[COMMAND_MAX_ARGS];

    if(std_err_indicator == 1){
        char cmd_temp[(cmd_line).length()];
        strcpy(reinterpret_cast<char *>(cmd_temp), cmd_line.c_str());
        _removeChar(cmd_temp,'|');
        _parseCommandLineByChar(cmd_temp, cmd_temp_array, '&');
        int fd[2];
        pipe(fd);
        pid_t child1 = fork();
        if(child1<0){
            perror("smash error: fork failed");
            return;
        }
        if (child1 == 0) {
            // first child
            setpgrp();
            close(fd[0]);
            dup2(fd[1],2);
            close(fd[1]);
            SmallShell::getInstance().executeCommand(cmd_temp_array[0].c_str());
            exit(0);
        }

        pid_t child2 = fork();
        if(child2<0){
            perror("smash error: fork failed");
            return;
        }
        if (child2 == 0) {
            // second child
            setpgrp();
            close(fd[1]);
            dup2(fd[0],0);
            close(fd[0]);
            SmallShell::getInstance().executeCommand(cmd_temp_array[1].c_str());
            exit(0);
        }

        close(fd[0]);
        close(fd[1]);
        while(wait(NULL)>0);
    }
    else{
        _parseCommandLineByChar(cmd_line, cmd_temp_array, '|');
        int fd[2];
        pipe(fd);
        pid_t child1 = fork();
        if(child1<0){
            perror("smash error: fork failed");
            return;
        }
        if (child1 == 0) {
            // first child
            setpgrp();
            close(fd[0]);
            dup2(fd[1],1);
            close(fd[1]);
            SmallShell::getInstance().executeCommand(cmd_temp_array[0].c_str());
            exit(0);
        }

        pid_t child2 = fork();
        if(child2<0){
            perror("smash error: fork failed");
            return;
        }
        if (child2 == 0) {
            // second child
            setpgrp();
            close(fd[1]);
            dup2(fd[0],0);
            close(fd[0]);
            SmallShell::getInstance().executeCommand(cmd_temp_array[1].c_str());
            exit(0);
        }
        close(fd[0]);
        close(fd[1]);
        while(wait(NULL)>0);
    }
}

void CopyCommand::execute() {
    char cmd_temp[(cmd_line).length()];
    strcpy(reinterpret_cast<char *>(cmd_temp), cmd_line.c_str());
    if (_isBackgroundComamnd(cmd_line.c_str())) {
        pid_t pid = fork();
        if (pid == 0) {
            setpgrp();
            _removeBackgroundSign(cmd_temp);
            string cmd_temp_array[COMMAND_MAX_ARGS];
            string input = devideCmdLine(cmd_temp);
            _parseCommandLine(input, cmd_temp_array);
            int read_fd;
            int write_fd;
            char buffer[1024];
            ssize_t count;
            read_fd = open(cmd_temp_array[1].c_str(), O_RDONLY);
            if (read_fd == -1) { /* Check if file opened */
                perror("smash error: open failed");
                return;
            }
            mode_t mode = 0666;
            write_fd = open(cmd_temp_array[2].c_str(), O_WRONLY | O_TRUNC |O_CREAT ,mode);
            if (write_fd == -1) /* Check if file opened*/
            {
                close(read_fd);
                perror("smash error: open failed");
                return;
            }
            while ((count = read(read_fd, buffer, sizeof(buffer))) != 0){
                write(write_fd, buffer, count);

            }
            close(write_fd);
            close(read_fd);
        }
        if (pid < 0) {
            perror("smash error: fork failed");
            return;
        } else {
            //TODO: DELETE THE WAIT AND ADD TO JOB LIST
            wait(nullptr);
        }
    } else {
        pid_t pid = fork();
        if (pid > 0) {
            // smash waits for child
            wait(nullptr);
        }
        if (pid < 0) {
            perror("smash error: fork failed");
            return;
        }
        if (pid == 0) {
            setpgrp();
            string * command = getCmdArray();
            int read_fd;
            int write_fd;
            char buffer[1024];
            ssize_t count;
            read_fd = open(command[1].c_str(), O_RDONLY);
            if (read_fd == -1) { /* Check if file opened */
                perror("smash error: open failed");
                return;
            }
            mode_t mode = 0666;
            write_fd = open(command[2].c_str(), O_WRONLY |O_TRUNC | O_CREAT ,mode);
            if (write_fd == -1) /* Check if file opened*/
            {
                close(read_fd);
                perror("smash error: open failed");
                return;
            }
            while ((count = read(read_fd, buffer, sizeof(buffer))) != 0){
                write(write_fd, buffer, count);

            }
            close(write_fd);
            close(read_fd);
        }


    }

}

void ExternalCommand::execute() {
    char cmd_temp[(cmd_line).length()];
    strcpy(reinterpret_cast<char *>(cmd_temp), cmd_line.c_str());
    if (_isBackgroundComamnd(cmd_line.c_str())) {
        pid_t pid = fork();
        if (pid == 0) {
            setpgrp();
            _removeBackgroundSign(cmd_temp);
            char *char_array[] = {(char *) "/bin/bash", (char *) "-c", cmd_temp, NULL};
            execv(char_array[0], char_array);
            exit(0);
        }
        if (pid < 0) {
            perror("smash error: fork failed");
            return;
        } else {
            //TODO: DELETE THE WAIT AND ADD TO JOB LIST
            wait(nullptr);
        }
    } else {
        pid_t pid = fork();
        if (pid > 0) {
            // smash waits for child
            wait(nullptr);
        }
        if (pid < 0) {
            perror("smash error: fork failed");
            return;
        }
        if (pid == 0) {
            setpgrp();
            char *char_array[] = {(char *) "/bin/bash", (char *) "-c", cmd_temp, NULL};
            execv(char_array[0], char_array);
            exit(0);
        }


    }
}


int JobEntry::getJobID() { return jobID; }

int JobEntry::getJobPid() { return  job_pid; }

string JobEntry::getInputCmd() { return input_cmd; }

JobStatus JobEntry::getJobStatus() { return  job_status; }

JobStatus JobEntry::getJobLastStatus() { return last_status; }

bool JobsCommand::checkArgInput() { return true; }

time_t JobEntry::getTime() { return job_time; }

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






//function to check if the input is real number
int checkInputRealNum(string num){
    string::iterator i_it = num.begin();
    string::iterator end_it = num.end();

    while ((std::isdigit(*i_it))&&(i_it != end_it)){ i_it++; }

    if(i_it != end_it){ return false; }

    return true;
}


bool KillCommand::checkArgInput() {
    int num_arg = getNumOfArg();
    if(num_arg != 3){
        cout <<"smash error: kill: invalid arguments" << endl;
        return false;
    }

    string* cmd_array = getCmdArray();
    // to check if to add, check of the second arg include '-'
    /*  if(cmd_array[2].substr(0,1) == "-"){
          if(checkInputRealNum(cmd_array[2].substr(1, cmd_array[2].size()))){
              cout << "smash error: kill: job-id " << stoi(cmd_array[2]) << " does not exist"
                   << endl;
              return false;
          }
      }
  */

    if((checkInputRealNum(cmd_array[2]) == false) || (stoi(cmd_array[2]) <= 0) ){
        cout <<"smash error: kill: invalid arguments" << endl;
        return false;
    }

    string::iterator i_it = cmd_array[1].begin();
    string::iterator end_it = cmd_array[1].end();
    i_it++; // skip '-'
    while ((std::isdigit(*i_it))&&(i_it!=end_it)){
        i_it++;
    }
    if(i_it != end_it){
        cout <<"smash error: kill: invalid arguments" << endl;
        return false; }

    //check the arguments before '-', start in the right place
    size_t temp = cmd_array[1].find_last_of('-');
    if ((temp == string::npos)||(temp != 0)){
        cout <<"smash error: kill: invalid arguments" << endl;
        return false;
    }


    int jobID = stoi(cmd_array[2]);
    if(jobs->getJobById(jobID)){
        cout << "smash error: kill: job-id " << jobID << " does not exist"
             << endl;
        return false;
    }
    return true;

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




void JobsList::killAllJobs() {
    cout << "smash: sending SIGKILL signal to " << list_jobs.size()
         << "jobs:" <<endl;
    for (auto &cur_job : list_jobs){
        string temp_input_cmd = cur_job.getInputCmd();
        cout << cur_job.getJobPid() << ": " << temp_input_cmd << endl;
        if(kill(cur_job.getJobPid(),SIGKILL) == -1){
            perror("smash error: kill failed");
            return;
        }
    }
    list_jobs.clear();

}

//get the last job by JobID
JobEntry* JobsList::getLastJob(){
    if(list_jobs.empty()){
        return nullptr;
    }
    return  getJobById(list_jobs.back().getJobID());
}

void JobEntry::changeStatusOfJob(JobStatus status) {
    this->job_status = status;
}

void JobEntry::changeLastStatusOfJob(JobStatus last_status) {
    this->last_status = last_status;
}


void JobsList::removeJobById(int jobId) {
    vector<JobEntry>::iterator i_it = list_jobs.begin();
    for(unsigned long j = 0; j < list_jobs.size(); j++){
        if(i_it->getJobID() == jobId){
            break;
        } i_it++;
    }
}

// to check if it is possible to get fg -3, means to get the job id
// with the sign '-'
bool ForegroundCommand::checkArgInput() {
    int num_arg = getNumOfArg();
    string *cmd_array = getCmdArray();
    if ((num_arg > 2) || (checkInputRealNum(cmd_array[1])) == false) {
        cout << "smash error: fg: invalid arguments" << endl;
        return false;
    }
    //check for empty job list and no arguments
    JobEntry *temp_last_job = jobs->getLastJob();
    if ((num_arg == 1) && (!temp_last_job)) {
        cout << "smash error: fg: jobs list is empty" << endl;
        return false;
    }
    //check if the job exist
    JobEntry *temp_job = jobs->getJobById(stoi(cmd_array[1]));
    if (temp_job == nullptr) {
        cout << "smash error: fg: job-id " << stoi(cmd_array[1])
             << " does not exist" << endl;
        return false;
    }
    return true;
}


void ForegroundCommand::execute() {
    if (checkArgInput() == false) { return; }

    JobEntry *temp_job;
    string *cmd_array = getCmdArray();
    int num_arg = getNumOfArg();

    if (num_arg == 1) {
        temp_job = jobs->getLastJob();
    } else {
        temp_job = jobs->getJobById(stoi(cmd_array[1]));
    }
    string input_cmd = temp_job->getInputCmd();
    cout << input_cmd << " : " << temp_job->getJobPid() << endl;
    JobStatus temp_old_status = temp_job->getJobStatus();
    temp_job->changeStatusOfJob(FOREGROUND);
    temp_job->changeLastStatusOfJob(temp_old_status);
    if (temp_old_status == STOPPED) {
        if (kill(temp_job->getJobPid(), SIGCONT) == -1) {
            perror("smash error: kill failed");
        }
    }
    int check = -1;
    waitpid(temp_job->getJobPid(),&check,WUNTRACED);//wait for the process to chage status
    if (WIFSTOPPED(check)) { return; } //wifdtopped check if stopped

    jobs->removeJobById(temp_job->getJobID());

}

JobEntry* JobsList::getLastStoppedJob() {
    if(list_jobs.empty()){ return nullptr; }

    auto cur_job = --list_jobs.end();
    for(unsigned long j=0; j < list_jobs.size(); j++){
        if(cur_job->getJobStatus() == STOPPED){
            return getJobById(cur_job->getJobID());
        }
        cur_job--;
    }
    return nullptr;
}

bool BackgroundCommand::checkArgInput() {
    int num_arg = getNumOfArg();
    if(num_arg > 2) {
        cout << "smash error: bg: invalid arguments" << endl;
        return false;
    }
    JobEntry* last_job_stop = jobs->getLastStoppedJob();
    if((num_arg == 1) && (last_job_stop == nullptr)){
        cout << "smash error: bg: there is no stopped jobs to resume" << endl;
        return false;
    }

    string* cmd_array = getCmdArray();
    if((num_arg == 2) && (checkInputRealNum(cmd_array[1]))){
        cout << "smash error: bg: invalid arguments" << endl;
        return false;
    }
    JobEntry* temp_job = jobs->getJobById(stoi(cmd_array[1]));
    if((num_arg == 2) && (temp_job == nullptr)){
        cout << "smash error: bg: job-id " << stoi(cmd_array[1])
             << " does not exist" << endl;
        return false;
    }
    JobStatus temp_job_status = temp_job->getJobStatus();
    if((num_arg == 2) && (temp_job_status == BACKGROUND )){
        cout << "smash error: bg: job-id " << temp_job->getJobID()
             << " is already running in the background" << endl;
        return false;
    }
    return true;
}

void BackgroundCommand::execute() {
    if (checkArgInput() == false) { return; }

    JobEntry *temp_job;
    int num_arg = getNumOfArg();
    string *cmd_array = getCmdArray();
    if (num_arg == 1) {
        temp_job = jobs->getLastStoppedJob();
    } else {
        temp_job = jobs->getJobById(stoi(cmd_array[1]));
    }

    string input_cmd = temp_job->getInputCmd();
    int temp_pid = temp_job->getJobPid();
    cout << input_cmd << " : " << temp_pid << endl;
    JobStatus last_status = temp_job->getJobStatus();
    temp_job->changeStatusOfJob(BACKGROUND);
    temp_job->changeLastStatusOfJob(last_status);
    if (kill(temp_pid, SIGCONT) == -1) {
        perror("smash error: kill failed");
    }

}

bool QuitCommand::checkArgInput() {
    return true;
}


void QuitCommand::execute() {
    string* cmd_array = getCmdArray();
    int num_arg = getNumOfArg();
    if((num_arg == 2 ) && (cmd_array[1] == "kill")){
        jobs->killAllJobs();
    }
}


void JobsList::removeFinishedJobs() {
    int jobs_id_finished[PROCESS_MAX], num_finished = 0;
    bool to_remove = false;

    for(auto &cur_job : list_jobs){
        int flag_one = -1, flag_second = -1;
        int job_pid = cur_job.getJobPid();

        waitpid(job_pid,&flag_one,WNOHANG);
        bool job_signaled = WIFSIGNALED((flag_one));
        waitpid(job_pid, &flag_second, WNOHANG);
        bool job_terminated = WIFEXITED((flag_second));

        if(job_terminated || job_signaled){
            jobs_id_finished[num_finished];
            num_finished +=1;
            to_remove = true;
        }
    }

    if(to_remove){
        int i;
        for(i = 0; i < num_finished; i++){
            int jobID_to_remove = jobs_id_finished[i];
            removeJobById(jobID_to_remove);
        }
    }
}

void JobsList::updateJobsStatus() {

    int temp_status = -1;
    for(JobEntry &cur_job :  list_jobs){
        int temp_pid = cur_job.getJobPid();
        waitpid(temp_pid, &temp_status, WNOHANG | WUNTRACED);
        //bool check_stopped_1 = WIFSTOPPED((temp_status));
        if(WIFSTOPPED((temp_status))){
            JobStatus temp_cur_status_1 = cur_job.getJobStatus();
            cur_job.changeLastStatusOfJob(temp_cur_status_1);
            cur_job.changeStatusOfJob(STOPPED);
        }
        waitpid(temp_pid,&temp_status,WNOHANG | WCONTINUED);
        bool check_continued = WIFCONTINUED(temp_status);
        if((check_continued) && (cur_job.getJobStatus() !=STOPPED)) { return; }
        if(check_continued){
            JobStatus temp_cur_status_2 = cur_job.getJobStatus();
            cur_job.changeStatusOfJob(temp_cur_status_2);
            cur_job.changeLastStatusOfJob(STOPPED);
            if((temp_cur_status_2) == FOREGROUND){
                int temp;
                waitpid(temp_pid,&temp,WUNTRACED);
                bool check_stopped_2 = WIFSTOPPED(temp);
                if(check_stopped_2) {
                    break; }
                int temp_jobID = cur_job.getJobID();
                removeJobById(temp_jobID);
                break;
            }
        }
        temp_status =-1;
    }



}


