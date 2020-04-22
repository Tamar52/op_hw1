#include <unistd.h>
#include <string.h>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"

#define MAXPATH 4096
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

// TODO: Add your implementation for classes in Commands.h 

SmallShell::SmallShell() {
// TODO: add your implementation
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {
	// For example:
/*
  string cmd_s = string(cmd_line);
  if (cmd_s.find("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else if ...
  .....
  else {
    return new ExternalCommand(cmd_line);
  }
  */
  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
  // TODO: Add your implementation here
  // for example:
  // Command* cmd = CreateCommand(cmd_line);
  // cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
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
  char temp_pwd[MAXPATH];
  getcwd(temp_pwd,MAXPATH);
  cout<< temp_pwd <<endl;
}

bool GetCurrDirCommand::checkArgInput() { return true; }



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


void ChangeDirCommand::execute() {
  if(checkArgInput() == false){
    return;
  }

  int check;
  string *cmd_array = getCmdArray();
  char temp[MAXPATH], array[last_pwd.size() + 1];
  string new_pwd = getcwd(temp, MAXPATH);
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

void RedirectionCommand::execute() {
    //TODO: change to working with string?
    char* command = getCommand();
    if (command[1] == '>'){
        pid_t pid = fork();
        if (pid == 0) {
            close(1);
            open(command[2], O_WRONLY|O_CREAT,666 );
            char* args[] ={command[0], nullptr};
            execv(args[0],args);
        } else {
            wait(NULL);
        }

    } else{
        pid_t pid = fork();
        if (pid == 0) {
            close(1);
            open(command[2], O_APPEND|O_CREAT, 666);
            char* args[] ={command[0], nullptr};
            execv(args[0],args);
        } else {
            wait(NULL);
        }
    }
}

void ChpromptCommand::execute() {
    //TODO: change to working with string?
    char* command = getCommand();
    if(command[1]){
        prompt = command[1];
    }
    else{
        prompt = "smash";
    }
}