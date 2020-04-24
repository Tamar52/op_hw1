#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <string.h>
#include <string>
#include <vector>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (21)
#define HISTORY_MAX_RECORDS (50)

std::string prompt = "smash";
using namespace std;

class Command {
    // cmd_line_name; option of the name of command
    int num_arg; //number of arg in cmd_line
    string cmd_array[COMMAND_MAX_ARGS]; //array of the arg from cmd_line
 public:
    explicit Command(const char* cmd_line);
  virtual ~Command();
  virtual void execute() = 0;
  int getNumOfArg();
  string* getCmdArray();
  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
 public:
  explicit BuiltInCommand(const char* cmd_line) : Command(cmd_line){};
  virtual ~BuiltInCommand() {}
  virtual bool checkArgInput() = 0; //function to check each command
};

class ExternalCommand : public Command {
 public:
  ExternalCommand(const char* cmd_line): Command(cmd_line){};
  virtual ~ExternalCommand() {}
  void execute() override;
};

class PipeCommand : public Command {
  // TODO: Add your data members
 public:
  PipeCommand(const char* cmd_line): Command(cmd_line){};
  virtual ~PipeCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command {
 // TODO: Add your data members
 public:
  explicit RedirectionCommand(const char* cmd_line): Command(cmd_line){};
  virtual ~RedirectionCommand() {}
  void execute() override;
  //void prepare() override;
  //void cleanup() override;
};


//TODO: is it really a builtincommand?
class CopyCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    CopyCommand(const char* cmd_line): BuiltInCommand(cmd_line){};
    virtual ~CopyCommand() {}
    void execute() override;
    bool checkArgInput() override;
};

class ChangeDirCommand : public BuiltInCommand {
    string& last_pwd;
// TODO: Add your data members public:
    public:
    ChangeDirCommand(const char* cmd_line, string& plastPwd) :
            BuiltInCommand(cmd_line), last_pwd(plastPwd){};
    virtual ~ChangeDirCommand() {}
    void execute() override;
    bool checkArgInput() override;
};

class GetCurrDirCommand : public BuiltInCommand {
 public:
  GetCurrDirCommand(const char* cmd_line) : BuiltInCommand(cmd_line){};
  virtual ~GetCurrDirCommand() {}
  void execute() override;
  bool checkArgInput()override ;
};

class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(const char* cmd_line) : BuiltInCommand(cmd_line){};
  virtual ~ShowPidCommand() {}
  void execute() override;
  bool checkArgInput()override ;
};

class JobsList;
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members public:
  QuitCommand(const char* cmd_line, JobsList* jobs);
  virtual ~QuitCommand() {}
  void execute() override;
};


class JobsList {
 public:
  class JobEntry {
   // TODO: Add your data members
  };
 // TODO: Add your data members
 public:
  JobsList();
  ~JobsList();
  void addJob(Command* cmd, bool isStopped = false);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry * getJobById(int jobId);
  void removeJobById(int jobId);
  JobEntry * getLastJob(int* lastJobId);
  JobEntry *getLastStoppedJob(int *jobId);
  // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  JobsCommand(const char* cmd_line, JobsList* jobs);
  virtual ~JobsCommand() {}
  void execute() override;
};

class KillCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  KillCommand(const char* cmd_line, JobsList* jobs);
  virtual ~KillCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  ForegroundCommand(const char* cmd_line, JobsList* jobs);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  BackgroundCommand(const char* cmd_line, JobsList* jobs);
  virtual ~BackgroundCommand() {}
  void execute() override;
};

// TODO: add more classes if needed
// maybe chprompt , timeout ?
class ChpromptCommand : public BuiltInCommand {
public:
    ChpromptCommand(const char* cmd_line): BuiltInCommand(cmd_line){};
    virtual ~ChpromptCommand() {}
    void execute() override;
    bool checkArgInput() override;
};

class SmallShell {
 private:
  // TODO: Add your data members
  SmallShell();
 public:
  Command *CreateCommand(const char* cmd_line);
  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell();
  void executeCommand(const char* cmd_line);
  // TODO: add extra methods as needed
};



#endif //SMASH_COMMAND_H_
