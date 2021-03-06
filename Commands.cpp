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

bool _isAppend(char* cmd_line){
    const string str(cmd_line);
    // find character
    unsigned int idx = str.find('>');
    // if all characters are spaces then return
    if (idx == string::npos) {
        return false;
    }
    // if the command line does not end with & then return
    if ((cmd_line[idx]) != '>') {
        return false;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';

    if ((cmd_line[idx+1]) == '>') {
        return true;
    }
    return false;
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
bool SmallShell::getIfFork() {return is_allready_fork;}
void SmallShell::setIfFork(bool is_fork) { this->is_allready_fork = is_fork;}
shared_ptr<Command> SmallShell::CreateCommand(const char* cmd_line) {

  string cmd_s = string(cmd_line);
  size_t pos = cmd_s.find_first_not_of(" ");
  if(pos != string::npos) {
      cmd_s.erase(0, pos);
  }
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
          string cmd_array[COMMAND_MAX_ARGS];
          int new_num = _parseCommandLine(new_cmd, cmd_array);
          change_dir->changeCmdArray(cmd_array);
          change_dir->changeNumOfArg(new_num);

      }
      return change_dir;
  }
  else if(cmd_s.find("jobs") == 0){
      return make_shared<JobsCommand>(cmd_line, *job_list);
  }
  else if(cmd_s.find("kill") == 0){
      return make_shared<KillCommand>(cmd_line, *job_list);
  }
  else if(cmd_s.find("fg") == 0){
      return make_shared<ForegroundCommand>(cmd_line, *job_list);
  }
  else if(cmd_s.find("bg") == 0){
      return make_shared<BackgroundCommand>(cmd_line, *job_list);
  }
  else if(cmd_s.find("quit") == 0){
      return make_shared<QuitCommand>(cmd_line, *job_list);
  }
  else if(cmd_s.find("cp") == 0){
      return make_shared<CopyCommand>(cmd_line);
  }
  //TODO: add a condition for timeout!

  else {
//      cmd_name == "External";
    return make_shared<ExternalCommand>(cmd_line);
  }

    return nullptr;
}

bool SmallShell::executeCommand(const char *cmd_line) {
    job_list->removeFinishedJobs();
    shared_ptr<Command> cmd = CreateCommand(cmd_line);
    if(_isCharInComamnd(cmd_line, "|")) {
        SmallShell::getInstance().setPipe(true);
        if (_isBackgroundComamnd(cmd_line)) {
            pid_t pid = fork();
            if (pid == 0) {
                SmallShell::getInstance().setPipe(true);
                setpgrp();
                char cmd_temp[(cmd->cmd_line).length()];
                strcpy(reinterpret_cast<char *>(cmd_temp), cmd->cmd_line.c_str());
                _removeBackgroundSign(cmd_temp);
                if (_isCharInComamnd(cmd_temp, "&")) {
                    shared_ptr<PipeCommand> pipe = make_shared<PipeCommand>(cmd_line, 1, true);
                    SmallShell::getInstance().setForegrounfPid(getppid());
                    SmallShell::getInstance().setIfFork(true);
                    SmallShell::getInstance().setPipe(true);
                    pipe->execute();
                    exit(0);
                } else {
                    shared_ptr<PipeCommand> pipe = make_shared<PipeCommand>(cmd_line, 0, true);
                    SmallShell::getInstance().setForegrounfPid(getppid());
                    SmallShell::getInstance().setIfFork(true);
                    SmallShell::getInstance().setPipe(true);
                    pipe->execute();
                    exit(0);
                }
                exit(0);
            }
            if (pid < 0) {
                perror("smash error: fork failed");
                return true;
            } else {
                SmallShell::getInstance().setPipe(true);
                SmallShell::getInstance().get_job_list()->addJob(cmd_line, pid, BACKGROUND, 0, NONE, true);
            }
        } else {
            pid_t pid = fork();
            if (pid < 0) {
                perror("smash error: fork failed");
                return true;
            }
            if (pid == 0) {
                SmallShell::getInstance().setPipe(true);
                setpgrp();
                if (_isCharInComamnd(cmd_line, "&")) {
                    shared_ptr<PipeCommand> pipe = make_shared<PipeCommand>(cmd_line, 1, false);
                    SmallShell::getInstance().setForegrounfPid(getppid());
                    SmallShell::getInstance().setIfFork(true);
                    SmallShell::getInstance().setPipe(true);
                    pipe->execute();
                    exit(0);
                } else {
                    shared_ptr<PipeCommand> pipe = make_shared<PipeCommand>(cmd_line, 0, false);
                    SmallShell::getInstance().setForegrounfPid(getppid());
                    SmallShell::getInstance().setIfFork(true);
                    SmallShell::getInstance().setPipe(true);
                    pipe->execute();
                    exit(0);
                }
                exit(0);
            } else {
                // smash waits for child
                SmallShell::getInstance().setPipe(true);
                SmallShell::getInstance().setForegrounfPid(pid);
                int status;
                waitpid(pid, &status, WUNTRACED);
                SmallShell::getInstance().setForegrounfPid(0);
                if (WIFSTOPPED(status)) {
                    SmallShell::getInstance().get_job_list()->addJob(cmd_line, pid, STOPPED, 0, NONE, true);
                }
            }
        }
    }else if (_isCharInComamnd(cmd->cmd_line.c_str(), ">")) {
        if(_isBackgroundComamnd(cmd_line)){
            shared_ptr<RedirectionCommand> rd = make_shared<RedirectionCommand>(cmd_line,true);
            rd->execute();
        }else {
            shared_ptr<RedirectionCommand> rd = make_shared<RedirectionCommand>(cmd_line,false);
            rd->execute();
        }

    }else {
        if(SmallShell::getInstance().getForegrounfPid() == 0) {
            SmallShell::getInstance().setForegrounfPid(getpid());
        }
        cmd->execute();
        SmallShell::getInstance().setForegrounfPid(0);
    }
    string cmd_s = string(cmd_line);
    if((cmd_s.find("quit") == 0)){
        return false;
    }
    job_list->removeFinishedJobs();


    return true;

    // Please note that you must fork smash process for some commands (e.g., external commands....)

    //TODO: maybe need to check if wasnt added to job list?


}


Command::Command(const char *cmd_line):cmd_line(cmd_line)
{
    string input = devideCmdLine(cmd_line);
    num_arg = _parseCommandLine(input, cmd_array);

}


void Command::changeNumOfArg(int new_num_arg) { num_arg = new_num_arg; }

void Command::changeCmdArray(string new_cmd_array[COMMAND_MAX_ARGS]) {
    for (int i = 0; i < COMMAND_MAX_ARGS; ++i) {
        cmd_array[i] = new_cmd_array[i];
    }
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
        cerr << "smash error: cd: too many arguments"<< endl;
        return false;
    }
    string* temp_cmd_array = getCmdArray();
    if(last_pwd == "" && temp_cmd_array[1] == "-"){
        cerr << "smash error: cd: OLDPWD not set"<< endl;
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
    pid_t pid = SmallShell::getInstance().getPid();
    cout << "smash pid is " << to_string(pid) << endl;
}

//TODO: understand how to cast the string to char* for execv
void RedirectionCommand::execute() {
    string cmd_temp_array[COMMAND_MAX_ARGS];
    char cmd_temp[(cmd_line).length()];
    strcpy(reinterpret_cast<char *>(cmd_temp), cmd_line.c_str());
    if(is_background){
        _removeBackgroundSign(cmd_temp);
    }
    _parseCommandLineByChar(cmd_temp, cmd_temp_array, '>');
    int indicator = _removeChar(cmd_temp,'>');
    int std_fd = dup(1);
    int write_fd;
    if (indicator == 0){
        close(1);
        mode_t mode = 0666;
        string path[COMMAND_MAX_ARGS];
        int j=0;
        for (int i = 0; i <COMMAND_MAX_ARGS ; ++i) {
            if(cmd_temp_array[i] != ""){
                path[j] = cmd_temp_array[i];
                j++;
            }
        }
        size_t pos = path[1].find_first_not_of(" ");
        if(pos != string::npos) {
            path[1].erase(0, pos);
        }
        size_t pos_last = path[1].find_last_of(" ");
        size_t first_pos = path[1].find_first_of(" ");
        if(first_pos != string::npos) {
            path[1].erase(first_pos, pos_last);
        }
        write_fd = open(path[1].c_str(), O_WRONLY|O_TRUNC|O_CREAT, mode);
        if (write_fd == -1) { /* Check if file opened */
            perror("smash error: open failed");
            dup2(std_fd,1);
            return;
        }
    } else{
        close(1);
        mode_t mode = 0666;
        string path[COMMAND_MAX_ARGS];
        int j=0;
        for (int i = 0; i <COMMAND_MAX_ARGS ; ++i) {
            if(cmd_temp_array[i] != ""){
                path[j] = cmd_temp_array[i];
                j++;

            }
        }
        size_t pos = path[1].find_first_not_of(" ");
        if(pos != string::npos) {
            path[1].erase(0, pos);
        }
        size_t pos_last = path[1].find_last_of(" ");
        size_t first_pos = path[1].find_first_of(" ");
        if(first_pos != string::npos) {
            path[1].erase(first_pos, pos_last);
        }
        write_fd = open(path[1].c_str(), O_WRONLY|O_APPEND|O_CREAT, mode);
        if (write_fd == -1) { /* Check if file opened */
            perror("smash error: open failed");
            dup2(std_fd,1);
            return;
        }
    }
    if(is_background){
        cmd_temp_array[0] = cmd_temp_array[0] + "&";
        SmallShell::getInstance().executeCommand(cmd_temp_array[0].c_str());
    } else{
        SmallShell::getInstance().executeCommand(cmd_temp_array[0].c_str());
    }
    close(write_fd);
    dup2(std_fd,1);
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
    if (std_err_indicator == 1) {
        char cmd_temp[(cmd_line).length()];
        strcpy(reinterpret_cast<char *>(cmd_temp), cmd_line.c_str());
        _removeChar(cmd_temp, '|');
        _parseCommandLineByChar(cmd_temp, cmd_temp_array, '&');
        int fd[2];
        pipe(fd);
        pid_t child1 = fork();
        if (child1 < 0) {
            perror("smash error: fork failed");
            return;
        }
        if (child1 == 0) {
            // first child


            dup2(fd[1], 2);
            close(fd[1]);
            close(fd[0]);
            if (is_background) {
                cmd_temp_array[0] = cmd_temp_array[0] + "&";
            }
            SmallShell::getInstance().setIfFork(true);
            SmallShell::getInstance().setPipe(true);
            SmallShell::getInstance().executeCommand(cmd_temp_array[0].c_str());

            exit(0);
        }

        pid_t child2 = fork();
        if (child2 < 0) {
            perror("smash error: fork failed");
            return;
        }
        if (child2 == 0) {
            // second child


            dup2(fd[0], 0);
            close(fd[1]);
            close(fd[0]);
            if (is_background) {
                cmd_temp_array[1] = cmd_temp_array[1] + "&";
            }
            SmallShell::getInstance().setIfFork(true);
            SmallShell::getInstance().setPipe(true);
            SmallShell::getInstance().executeCommand(cmd_temp_array[1].c_str());
            exit(0);
        }
        SmallShell::getInstance().setPipe(true);
        close(fd[0]);
        close(fd[1]);
        int status;
        waitpid(child1, &status, WUNTRACED);
        waitpid(child2, &status, WUNTRACED);
        exit(0);


    } else {
        char cmd_temp[(cmd_line).length()];
        strcpy(reinterpret_cast<char *>(cmd_temp), cmd_line.c_str());
        if (_isCharInComamnd(cmd_temp, "&")) {
            _removeChar(cmd_temp, '&');
        }
        _parseCommandLineByChar(cmd_temp, cmd_temp_array, '|');
        int fd[2];
        pipe(fd);
        pid_t child1 = fork();
        if (child1 < 0) {
            perror("smash error: fork failed");
            return;
        }
        if (child1 == 0) {

            // first child

            dup2(fd[1], 1);
            close(fd[1]);
            close(fd[0]);
            if (is_background) {
                cmd_temp_array[0] = cmd_temp_array[0] + "&";
            }
            SmallShell::getInstance().setIfFork(true);
            SmallShell::getInstance().setPipe(true);
            SmallShell::getInstance().executeCommand(cmd_temp_array[0].c_str());
            exit(0);
        }

        pid_t child2 = fork();
        if (child2 < 0) {
            perror("smash error: fork failed");
            return;
        }
        if (child2 == 0) {
            // second child
            dup2(fd[0], 0);
            close(fd[1]);
            close(fd[0]);
            if (is_background) {
                cmd_temp_array[1] = cmd_temp_array[1] + "&";
            }
            SmallShell::getInstance().setIfFork(true);
            SmallShell::getInstance().setPipe(true);
            SmallShell::getInstance().executeCommand(cmd_temp_array[1].c_str());
            exit(0);
        }

        SmallShell::getInstance().setPipe(true);
        close(fd[0]);
        close(fd[1]);
        int status;
        waitpid(child1, &status, WUNTRACED);
        waitpid(child2, &status, WUNTRACED);
        exit(0);
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
            char temp_1[COMMAND_ARGS_MAX_LENGTH];
            char temp_2[COMMAND_ARGS_MAX_LENGTH];
            realpath(cmd_temp_array[1].c_str(),temp_1);
            realpath(cmd_temp_array[2].c_str(),temp_2);
            int read_fd;
            int write_fd;
            char buffer[1024];
            ssize_t count;
            read_fd = open(cmd_temp_array[1].c_str(), O_RDONLY);
            if (read_fd == -1) { /* Check if file opened */
                perror("smash error: open failed");
                exit(0);
            }
            if(strcmp(temp_1,temp_2) == 0){
                cout <<"smash: "<< cmd_temp_array[1].c_str()<< " was copied to "<< cmd_temp_array[2].c_str() << endl;
                close(read_fd);
                exit(0);
            }
            mode_t mode = 0666;
            write_fd = open(cmd_temp_array[2].c_str(), O_WRONLY | O_TRUNC |O_CREAT ,mode);
            if (write_fd == -1) /* Check if file opened*/
            {
                close(read_fd);
                perror("smash error: open failed");
                exit(0);
            }
            while ((count = read(read_fd, buffer, sizeof(buffer))) != 0){
                write(write_fd, buffer, count);

            }
            cout <<"smash: "<< cmd_temp_array[1].c_str()<<  " was copied to "<< cmd_temp_array[2].c_str() << endl;
            close(write_fd);
            close(read_fd);
            exit(0);
        }
        if (pid < 0) {
            perror("smash error: fork failed");
            return;
        } else {

            SmallShell::getInstance().get_job_list()->addJob(cmd_line,pid,BACKGROUND, 0, NONE, false);
        }
    } else {
        pid_t pid = fork();
        if (pid < 0) {
            perror("smash error: fork failed");
            return;
        }
        if (pid == 0) {
            setpgrp();
            string * command = getCmdArray();
            char temp_1[COMMAND_ARGS_MAX_LENGTH];
            char temp_2[COMMAND_ARGS_MAX_LENGTH];
            realpath(command[1].c_str(),temp_1);
            realpath(command[2].c_str(),temp_2);
            int read_fd;
            int write_fd;
            char buffer[1024];
            ssize_t count;
            read_fd = open(command[1].c_str(), O_RDONLY);
            if (read_fd == -1) { /* Check if file opened */
                perror("smash error: open failed");
                exit(0);
            }
            if(strcmp(temp_1,temp_2) == 0){
                cout <<"smash: "<< command[1].c_str()<< " was copied to "<< command[2].c_str() << endl;
                close(read_fd);
                exit(0);
            }
            mode_t mode = 0666;
            write_fd = open(command[2].c_str(), O_WRONLY |O_TRUNC | O_CREAT ,mode);
            if (write_fd == -1) /* Check if file opened*/
            {
                close(read_fd);
                perror("smash error: open failed");
                exit(0);
            }
            while ((count = read(read_fd, buffer, sizeof(buffer))) != 0){
                write(write_fd, buffer, count);

            }
            cout <<"smash: "<< command[1].c_str()<< " was copied to "<< command[2].c_str() << endl;
            close(write_fd);
            close(read_fd);
            exit(0);
        } else {
            // smash waits for child
            SmallShell::getInstance().setForegrounfPid(pid);
            int status;
            waitpid(pid,&status, WUNTRACED);
            SmallShell::getInstance().setForegrounfPid(0);
            if(WIFSTOPPED(status)){
                SmallShell::getInstance().get_job_list()->addJob(cmd_temp,pid, STOPPED, 0, NONE, false);
            }
        }


    }

}

void ExternalCommand::execute() {
    char cmd_temp[(cmd_line).length()];
    strcpy(reinterpret_cast<char *>(cmd_temp), cmd_line.c_str());
    if (SmallShell::getInstance().getIfFork()) {
        SmallShell::getInstance().setIfFork(true);
        if (_isBackgroundComamnd(cmd_line.c_str())) {
            _removeBackgroundSign(cmd_temp);
        }
        char *char_array[] = {(char *) "/bin/bash", (char *) "-c", cmd_temp, NULL};
        execv(char_array[0], char_array);
        exit(0);
    } else {
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

                SmallShell::getInstance().get_job_list()->addJob(cmd_line, pid, BACKGROUND, 0, NONE, false);

            }
        } else {
            pid_t pid = fork();

            if (pid < 0) {
                perror("smash error: fork failed");
                return;
            }
            if (pid == 0) {
                setpgrp();
                char *char_array[] = {(char *) "/bin/bash", (char *) "-c", cmd_temp, NULL};
                execv(char_array[0], char_array);
                exit(0);
            } else {

                // smash waits for child
                SmallShell::getInstance().setForegrounfPid(pid);
                int status;
                waitpid(pid, &status, WUNTRACED);
                SmallShell::getInstance().setForegrounfPid(0);
                if (WIFSTOPPED(status)) {
                    SmallShell::getInstance().get_job_list()->addJob(cmd_line, pid, STOPPED, 0, NONE, false);
                }
            }


        }
    }
}


int JobEntry::getJobID() { return jobID; }

int JobEntry::getJobPid() { return  job_pid; }

string JobEntry::getInputCmd() { return input_cmd; }

JobStatus JobEntry::getJobStatus() { return  job_status; }


bool JobsCommand::checkArgInput() { return true; }

time_t JobEntry::getTime() { return job_time; }

void JobsCommand::execute() {
    if(checkArgInput() == false){ return; }
    jobs.printJobsList();
}

void JobsList::setNextId(int id) {
    next_job_id = id;
}

int JobsList::getNextId() {
    return next_job_id;
}
void JobsList::addJob(string input_cmd, int job_pid, JobStatus job_status,int job_id, JobStatus last_status, bool pipe) {
    int new_jobID;
    if(job_id != 0){
        new_jobID = job_id;
    }
    else if (list_jobs.empty()) {
        new_jobID = 1;
    } else {
        int max_id = 0;
        for(auto &cur_job : list_jobs){
            if(cur_job.getJobID() > max_id){
                max_id = cur_job.getJobID();
            }
        }
        new_jobID = max_id + 1;
    }
    time_t temp_time;
    time(&temp_time);
    JobEntry new_job(job_pid, new_jobID, job_status, last_status, input_cmd, temp_time,  pipe);
    list_jobs.push_back(new_job);
//    SmallShell::getInstance().get_job_list()->setNextId(new_jobID + 1);
}


bool cmpareJobs(JobEntry job1, JobEntry job2){
    return job1.getJobID() < job2.getJobID();
};

//to check if print w.r.t
void JobsList::printJobsList() {
    sort(list_jobs.begin(), list_jobs.end(),cmpareJobs);
    for(auto &cur_job : list_jobs) {
        JobStatus temp_s = cur_job.getJobStatus();
        if (temp_s == FOREGROUND) { continue; } //dont print the jobs ths run in it the front

        cout << '[' << cur_job.getJobID() << ']' << ' '
             << cur_job.getInputCmd()
             << " : " << to_string(cur_job.getJobPid()) << " " << difftime(time(NULL), cur_job.getTime())
             << " secs";

        if (cur_job.getJobStatus() == STOPPED) { cout << " (stopped)"; }
        cout << endl;
    }
    return;
}

JobEntry* JobsList::getJobById(int jobId) {

    for(auto &cur_job : list_jobs){
        if( cur_job.getJobID() == jobId ){
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
        cerr <<"smash error: kill: invalid arguments" << endl;
        return false;
    }

    string* cmd_array = getCmdArray();

      if((cmd_array[2].substr(0,1) == "-") && (checkInputRealNum(cmd_array[2].substr(1, cmd_array[2].size())))){
          cerr << "smash error: kill: job-id " << stoi(cmd_array[2]) << " does not exist"
                   << endl;
              return false;
          }

    if((checkInputRealNum(cmd_array[2]) == false) || (stoi(cmd_array[2]) <= 0) ){
        cerr <<"smash error: kill: invalid arguments" << endl;
        return false;
    }

    if((num_arg == 1) && (cmd_array[0]!="kill")){
        cerr <<"smash error: kill: invalid arguments" << endl;
        return false;
    }




    string::iterator i_it = cmd_array[1].begin();
    string::iterator end_it = cmd_array[1].end();
    i_it++; // skip '-'
    while ((std::isdigit(*i_it))&&(i_it!=end_it)){
        i_it++;
    }
    if(i_it != end_it){
        cerr <<"smash error: kill: invalid arguments" << endl;
        return false; }

    //check the arguments before '-', start in the right place
    size_t temp = cmd_array[1].find_last_of('-');
    if ((temp == string::npos)||(temp != 0)){
        cerr <<"smash error: kill: invalid arguments" << endl;
        return false;
    }


    int jobID = stoi(cmd_array[2]);
    if(!jobs.getJobById(jobID)){
        cerr << "smash error: kill: job-id " << jobID << " does not exist"
             << endl;
        return false;
    }
    return true;

}


void KillCommand::execute(){
    if(checkArgInput() == false){ return;}

    string* cmd_array = getCmdArray();
//    pid_t group_pid = getpgid(jobs.getJobById(stoi(cmd_array[2]))->getJobPid());
    if(jobs.getJobById(stoi(cmd_array[2]))->getPipe()){
        if(killpg(jobs.getJobById(stoi(cmd_array[2]))->getJobPid(),
                stoi(cmd_array[1].substr(1, cmd_array[1].size()))) == -1){
            perror("smash error: kill failed");
            return;
        }
    }else if(kill(jobs.getJobById(stoi(cmd_array[2]))->getJobPid(),
            stoi(cmd_array[1].substr(1, cmd_array[1].size()))) == -1){
        perror("smash error: kill failed");
        return;
    }

    cout << "signal number " << stoi(cmd_array[1].substr(1, cmd_array[1].size()))
    << " was sent to pid " << jobs.getJobById(stoi(cmd_array[2]))->getJobPid()
    << endl;

}




void JobsList::killAllJobs() {
    cout << "smash: sending SIGKILL signal to " << list_jobs.size()
         << " jobs:" <<endl;
    for (auto &cur_job : list_jobs){
        string temp_input_cmd = cur_job.getInputCmd();
        cout << cur_job.getJobPid() << ": " << temp_input_cmd << endl;
        if(cur_job.getPipe()){
            if(killpg(cur_job.getJobPid(),SIGKILL) == -1){
                perror("smash error: kill failed");
                return;
            }
        }else if(kill(cur_job.getJobPid(),SIGKILL) == -1){
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


void JobEntry::changeLastStatusOfJob(JobStatus last_status) {
    this->last_status = last_status;
}


void JobsList::removeJobById(int jobId) {
    vector<JobEntry>::iterator i_it = list_jobs.begin();
    for(unsigned int j = 0; j < list_jobs.size(); j++){
        if(i_it->getJobID() == jobId){
            list_jobs.erase(i_it);
            break;
        }
        i_it++;
    }
}
void JobsList::removeJobByPid(int pid) {
    vector<JobEntry>::iterator i_it = list_jobs.begin();
    for(unsigned long j = 0; j < list_jobs.size(); j++){
        if(i_it->getJobPid() == pid){
            list_jobs.erase(i_it);
            break;
        }
        i_it++;
    }
}
// to check if it is possible to get fg -3, means to get the job id
// with the sign '-'
bool ForegroundCommand::checkArgInput() {
    int num_arg = getNumOfArg();
    if (num_arg > 2) {
        cerr << "smash error: fg: invalid arguments" << endl;
        return false;
    }

    string *cmd_array = getCmdArray();
    if ((num_arg == 2) && (cmd_array[1].substr(0, 1) == "-") &&
        ((checkInputRealNum(cmd_array[1].substr(1, cmd_array[1].size()))))) {
        cerr << "smash error: fg: job-id " << stoi(cmd_array[1])
             << " does not exist" << endl;
        return false;
    }

    if ((num_arg > 2) || (checkInputRealNum(cmd_array[1])) == false) {
        cerr << "smash error: fg: invalid arguments" << endl;
        return false;
    }
    //check for empty job list and no arguments
    JobEntry *temp_last_job = jobs.getLastJob();
    if ((num_arg == 1) && (!temp_last_job)) {
        cerr << "smash error: fg: jobs list is empty" << endl;
        return false;
    }
    //check if the job exist
    if (num_arg == 2){
        JobEntry *temp_job = jobs.getJobById(stoi(cmd_array[1]));
        if (temp_job == nullptr) {
            cerr << "smash error: fg: job-id " << stoi(cmd_array[1])
                 << " does not exist" << endl;
            return false;
        }
    }
    return true;

}
int JobsList::getMaxJovId() {
    int max_id = 0;
    for (auto &cur_job : list_jobs) {
        if (cur_job.getJobID() > max_id) {
            max_id = cur_job.getJobID();
        }
    }
    return max_id;
}

void ForegroundCommand::execute() {

    if (checkArgInput() == false) { return; }
    JobEntry *temp_job;

    string *cmd_array = getCmdArray();
    int num_arg = getNumOfArg();

    if (num_arg == 1) {
        int job_id = SmallShell::getInstance().get_job_list()->getMaxJovId();
        temp_job = SmallShell::getInstance().get_job_list()->getJobById(job_id);
    } else {
        temp_job = SmallShell::getInstance().get_job_list()->getJobById(stoi(cmd_array[1]));

    }
    int job_pid = temp_job->getJobPid();
    int job_id = temp_job->getJobID();
    string cmd = temp_job->getInputCmd();
    string input_cmd = temp_job->getInputCmd();
    JobStatus last_status = temp_job->getJobStatus();
    bool pipe = temp_job->getPipe();
    cout << input_cmd << " : " << temp_job->getJobPid() << endl;
    SmallShell::getInstance().setForegrounfPid(temp_job->getJobPid());
    JobStatus temp_old_status = temp_job->getJobStatus();
    if (temp_old_status == STOPPED) {
        if(temp_job->getPipe()){
            SmallShell::getInstance().setPipe(true);
            if (killpg(temp_job->getJobPid(), SIGCONT) == -1) {
                perror("smash error: kill failed");
            }
        }else if (kill(temp_job->getJobPid(), SIGCONT) == -1) {
            perror("smash error: kill failed");
        }
    }
    int check = -1;
    SmallShell::getInstance().setForegrounfPid(temp_job->getJobPid());
    waitpid(temp_job->getJobPid(), &check, WUNTRACED);//wait for the process to chage statusdfgfg
    if (WIFSTOPPED(check)) {

        SmallShell::getInstance().get_job_list()->removeJobByPid(temp_job->getJobPid());
        SmallShell::getInstance().get_job_list()->addJob(cmd, job_pid,STOPPED, job_id, last_status, pipe);
        return;
    } //wifdtopped check if stopped
    else{
        SmallShell::getInstance().get_job_list()->removeJobById(temp_job->getJobID());
    }
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
        cerr << "smash error: bg: invalid arguments" << endl;
        return false;
    }
    JobEntry* last_job_stop = jobs.getLastStoppedJob();
    if((num_arg == 1) && (last_job_stop == nullptr)){
        cerr << "smash error: bg: there is no stopped jobs to resume" << endl;
        return false;
    }

    string *cmd_array = getCmdArray();
    if((num_arg == 2) && (cmd_array[1].substr(0,1) == "-") && ((checkInputRealNum(cmd_array[1].substr(1,cmd_array[1].size()))))){
        cerr << "smash error: bg: job-id " << stoi(cmd_array[1])
             << " does not exist" << endl;
        return false;
    }

    if((num_arg == 2) && (!checkInputRealNum(cmd_array[1]))){
        cerr << "smash error: bg: invalid arguments" << endl;
        return false;
    }
    if(num_arg == 2) {
        JobEntry *temp_job = jobs.getJobById(stoi(cmd_array[1]));
        if ((num_arg == 2) && (temp_job == nullptr)) {
            cerr << "smash error: bg: job-id " << stoi(cmd_array[1])
                 << " does not exist" << endl;
            return false;
        }
    }
    if(num_arg == 2){
        JobEntry *temp_job = jobs.getJobById(stoi(cmd_array[1]));
        JobStatus temp_job_status = temp_job->getJobStatus();
        if((num_arg == 2) && (temp_job_status == BACKGROUND )){
            cerr << "smash error: bg: job-id " << temp_job->getJobID()
                 << " is already running in the background" << endl;
            return false;
        }
    }

    return true;
}

void BackgroundCommand::execute() {
    if (checkArgInput() == false) { return; }

    JobEntry *temp_job;
    int num_arg = getNumOfArg();
    string *cmd_array = getCmdArray();
    if (num_arg == 1) {
        temp_job = SmallShell::getInstance().get_job_list()->getLastStoppedJob();
    } else {
        temp_job = SmallShell::getInstance().get_job_list()->getJobById(stoi(cmd_array[1]));
    }

    string input_cmd = temp_job->getInputCmd();
    int temp_pid = temp_job->getJobPid();
    int temp_id = temp_job->getJobID();
    bool pipe = temp_job->getPipe();
    cout << input_cmd << " : " << temp_pid << endl;
    JobStatus last_status = temp_job->getJobStatus();
    SmallShell::getInstance().get_job_list()->removeJobById(temp_id);
    SmallShell::getInstance().get_job_list()->addJob(input_cmd, temp_pid, BACKGROUND, temp_id, last_status, pipe);
    temp_job->changeLastStatusOfJob(last_status);
    if(pipe){
        if (killpg(temp_pid, SIGCONT) == -1) {
            perror("smash error: kill failed");
        }
    }else if (kill(temp_pid, SIGCONT) == -1) {
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
        jobs.killAllJobs();
    }
}

void JobsList::setNextKill(int id) {
    next_kill = id;
}

int JobsList::getNextKill() {
    return next_kill;
}

void JobsList::removeFinishedJobs() {
    int jobs_id_finished[PROCESS_MAX], num_finished = 0;
    bool to_remove = false;

    for(auto &cur_job : list_jobs){
        int flag_one = -1;
        int job_pid = cur_job.getJobPid();
        bool job_terminated = false;
        if(waitpid(job_pid,NULL,WNOHANG)==job_pid){
             job_terminated = true;
        }

        waitpid(job_pid,&flag_one,WNOHANG);
        bool job_signaled = WIFSIGNALED((flag_one));
//        waitpid(job_pid, &flag_second, WNOHANG);
//        bool job_terminated = WIFEXITED((flag_second));

        if(job_terminated || job_signaled){
            jobs_id_finished[num_finished] = cur_job.getJobID();
            num_finished +=1;
            to_remove = true;
        }
    }

    if(to_remove){
        int i;
        int max = 0;
        for(i = 0; i < num_finished; i++){
            int jobID_to_remove = jobs_id_finished[i];
            if(jobID_to_remove > max){
                max = jobID_to_remove;
            }
            removeJobById(jobID_to_remove);
        }
        if(max > getNextKill()){
            setNextKill(max);
        }
    }
}


