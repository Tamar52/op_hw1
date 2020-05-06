#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <string.h>
#include <string>
#include <vector>
#include <memory>
#include <unistd.h>
#include <bits/stdc++.h>
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (21)
#define PROCESS_MAX (100)


extern std::string prompt_message;
using namespace std;

typedef enum{FOREGROUND,STOPPED,BACKGROUND,NONE,ZOMBIE} JobStatus;
class Command {
    // cmd_line_name; option of the name of command
    int num_arg; //number of arg in cmd_line
    string cmd_array[COMMAND_MAX_ARGS]; //array of the arg from cmd_line

public:
    void changeNumOfArg(int new_num_arg);
    void changeCmdArray(string new_cmd_array[COMMAND_MAX_ARGS]);
    string cmd_line; // original cmd_line
    explicit Command(const char* cmd_line);
    virtual ~Command();
    virtual void execute() = 0;
    int getNumOfArg();
    string* getCmdArray();
    Command(const Command & command) = default;
    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
public:
    explicit BuiltInCommand(const char* cmd_line) : Command(cmd_line){};
    ~BuiltInCommand() override = default;
    virtual bool checkArgInput() = 0; //function to check each command
};

class ExternalCommand : public Command {

public:
    explicit ExternalCommand(const char* cmd_line): Command(cmd_line){};
    ~ExternalCommand() override = default;
    void execute() override;
};

class PipeCommand : public Command {
    int std_err_indicator;
    bool is_background;
public:
    explicit PipeCommand(const char* cmd_line, int std_err, bool is_background): Command(cmd_line){
        std_err_indicator = std_err;
        is_background = is_background;
    };
    ~PipeCommand() override = default;
    void execute() override;
};

class RedirectionCommand : public Command {
    bool is_background;

public:
    RedirectionCommand(const char* cmd_line, bool is_background): Command(cmd_line), is_background(is_background){}
    ~RedirectionCommand() override = default;
    void execute() override;
    //void prepare() override;
    //void cleanup() override;
};


//TODO: is it really a builtincommand?
class CopyCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    explicit CopyCommand(const char* cmd_line): BuiltInCommand(cmd_line){};
    ~CopyCommand() override = default;
    void execute() override;
    bool checkArgInput() override{ return true;};
};

class ChangeDirCommand : public BuiltInCommand {
    string last_pwd;
// TODO: Add your data members public:
public:
    explicit ChangeDirCommand(const char* cmd_line) : BuiltInCommand(cmd_line){
        last_pwd = "";
    };
    ~ChangeDirCommand() override = default;
    void execute() override;
    bool checkArgInput() override;
};

class GetCurrDirCommand : public BuiltInCommand {
public:
    explicit GetCurrDirCommand(const char* cmd_line) : BuiltInCommand(cmd_line){};
    ~GetCurrDirCommand() override = default;
    void execute() override;
    bool checkArgInput()override ;
    static string getCurrentDir();
};

class ShowPidCommand : public BuiltInCommand {
public:
    explicit ShowPidCommand(const char* cmd_line) : BuiltInCommand(cmd_line){};
    ~ShowPidCommand() override = default;
    void execute() override;
    bool checkArgInput()override ;
};



class JobsList;


class JobEntry {
    // TODO: Add your data members
    int job_pid;
    int jobID; //sequence id
    JobStatus job_status;
    JobStatus last_status;
    string input_cmd;
    time_t job_time;
public:
    JobEntry(int job_pid, int jobID, JobStatus job_status,
             string input_cmd, time_t job_time) :
            job_pid(job_pid), jobID(jobID),job_status(job_status),
            input_cmd(input_cmd), job_time(job_time){ last_status = NONE; };
    int getJobID();
    int getJobPid();
    JobStatus getJobStatus();
    JobStatus getJobLastStatus();
    void changeStatusOfJob(JobStatus status);
    void changeLastStatusOfJob(JobStatus last_status);

    time_t getTime();
    void setTime(time_t new_time);
    void resetTime();
    string getInputCmd();
    ~JobEntry() = default;
};

class JobsList {
    vector<JobEntry> list_jobs;
    // TODO: Add your data members
    int next_job_id;
    int next_kill = 0;
public:
    JobsList() = default;
    ~JobsList() = default;
    void addJob(string input_cmd, int job_pid, JobStatus job_status, int job_id);
    void addJob(JobEntry job);
    JobEntry* getJobWithStatusForegroind();
    void updateJobsStatus();
    void printJobsList();
    void killAllJobs();
    void removeFinishedJobs();
    JobEntry * getJobById(int jobId);
    int getNextJobIdForKill();
    void removeJobById(int jobId);
    void removeJobByPid(int pid);
    JobEntry * getLastJob();
    JobEntry *getLastStoppedJob();
    void setNextId(int id);
    int getNextId();
    void setNextKill(int id);
    int getNextKill();
    // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
    // TODO: Add your data members
    JobsList jobs;
public:
    JobsCommand(const char* cmd_line, JobsList& jobs) : BuiltInCommand(cmd_line),jobs(jobs){};
    virtual ~JobsCommand() {}
    void execute() override;
    bool checkArgInput() override;
};

class KillCommand : public BuiltInCommand {
    // TODO: Add your data members
    JobsList jobs;
public:
    KillCommand(const char* cmd_line, JobsList& jobs) : BuiltInCommand(cmd_line), jobs(jobs){};
    virtual ~KillCommand() = default;
    void execute() override;
    bool checkArgInput() override;
};

class ForegroundCommand : public BuiltInCommand {
    // TODO: Add your data members
    JobsList jobs;
public:
    ForegroundCommand(const char* cmd_line, JobsList& jobs) :
            BuiltInCommand(cmd_line), jobs(jobs){};
    virtual ~ForegroundCommand() = default;
    void execute() override;
    bool checkArgInput();
};

class BackgroundCommand : public BuiltInCommand {
    // TODO: Add your data members
    JobsList jobs;
public:
    BackgroundCommand(const char* cmd_line, JobsList& jobs) :
            BuiltInCommand(cmd_line),jobs(jobs){};
    virtual ~BackgroundCommand() = default;
    void execute() override;
    bool checkArgInput();
};

class QuitCommand : public BuiltInCommand {
// TODO: Add your data members public:
    JobsList jobs;
public:
    QuitCommand(const char* cmd_line, JobsList &jobs) :
            BuiltInCommand(cmd_line),jobs(jobs){};
    virtual ~QuitCommand()  = default;
    void execute() override;
    bool checkArgInput();
};

// TODO: add more classes if needed
// maybe chprompt , timeout ?
class ChpromptCommand : public BuiltInCommand {
public:
    ChpromptCommand(const char* cmd_line): BuiltInCommand(cmd_line){};
    ~ChpromptCommand() override = default;
    void execute() override;
    bool checkArgInput() override { return true;};
};

class SmallShell {
private:
    // TODO: Add your data members
    shared_ptr<ChangeDirCommand> change_dir;
    shared_ptr<JobsList> job_list;
    pid_t foreground_command;
    pid_t smash_pid;
    bool is_allready_fork;
//    string cmd_name;
    SmallShell(){
          job_list = make_shared<JobsList>();
          change_dir = nullptr;
          foreground_command = 0;
          setpgrp();
          smash_pid = getpgrp();
          is_allready_fork = false;
    }
public:
    void setForegrounfPid(pid_t pid){ foreground_command = pid;}
    pid_t getForegrounfPid(){ return foreground_command;}
    pid_t getPid(){return smash_pid;}
    std::shared_ptr<Command> CreateCommand(const char* cmd_line);

    SmallShell(SmallShell const&)      = delete; // disable copy ctor
    void operator=(SmallShell const&)  = delete; // disable = operator
    static SmallShell& getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
    shared_ptr<JobsList> get_job_list(){return job_list;};
    ~SmallShell(){}
    void setIfFork(bool is_fork);
    bool getIfFork();
    bool executeCommand(const char* cmd_line);
    // TODO: add extra methods as needed
};


#endif //SMASH_COMMAND_H_
