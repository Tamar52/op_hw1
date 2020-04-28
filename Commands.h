#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <string.h>
#include <string>
#include <vector>
#include <memory>


#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (21)


extern std::string prompt_message;
using namespace std;

typedef enum{FOREGROUND,STOPPED,BACKGROUND,NONE,ZOMBIE} JobStatus;
class Command {
    // cmd_line_name; option of the name of command
    int num_arg; //number of arg in cmd_line
    string cmd_array[COMMAND_MAX_ARGS]; //array of the arg from cmd_line

public:
    const char* cmd_line; // original cmd_line
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
    // TODO: Add your data members
public:
    explicit PipeCommand(const char* cmd_line): Command(cmd_line){};
    ~PipeCommand() override = default;
    void execute() override;
};

class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    explicit RedirectionCommand(const char* cmd_line): Command(cmd_line){};
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
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members public:
    JobsList* jobs;
public:
    QuitCommand(const char* cmd_line, JobsList* jobs) :
            BuiltInCommand(cmd_line),jobs(jobs){};
    ~QuitCommand() override = default;
    void execute() override;
    bool checkArgInput();
};

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
    time_t getTime() { return 2;};
//    void resetTime();
    string getInputCmd();
    ~JobEntry() = default;
};

class JobsList {
    vector<JobEntry> list_jobs;
    // TODO: Add your data members
public:
    JobsList() = default;
    ~JobsList() = default;
    void addJob(string input_cmd, int job_pid, JobStatus job_status);


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
    JobsList& jobs;
public:
    JobsCommand(const char* cmd_line, JobsList& jobs) : BuiltInCommand(cmd_line),jobs(jobs){};
    ~JobsCommand() override = default;
    void execute() override;
    bool checkArgInput() override;
};

class KillCommand : public BuiltInCommand {
    // TODO: Add your data members
    JobsList* jobs;
public:
    KillCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line), jobs(jobs){};
    ~KillCommand() override = default;
    void execute() override;
    bool checkArgInput() override;
};

class ForegroundCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    ForegroundCommand(const char* cmd_line, JobsList* jobs);
    ~ForegroundCommand() override = default;
    void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    BackgroundCommand(const char* cmd_line, JobsList* jobs);
    ~BackgroundCommand() override = default;
    void execute() override;
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
    JobsList* jobs;
    string cmd_name;
    SmallShell(){
        shared_ptr<JobsList> jobs = make_shared<JobsList>();
        shared_ptr<ChangeDirCommand>  change_dir = nullptr;
    }
public:
    std::shared_ptr<Command> CreateCommand(const char* cmd_line);

    SmallShell(SmallShell const&)      = delete; // disable copy ctor
    void operator=(SmallShell const&)  = delete; // disable = operator
    static SmallShell& getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
    ~SmallShell(){
        delete jobs;
    }
    void executeCommand(const char* cmd_line);
    // TODO: add extra methods as needed
};



#endif //SMASH_COMMAND_H_
