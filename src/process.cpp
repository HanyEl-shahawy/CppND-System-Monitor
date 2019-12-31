#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cmath>

#include "process.h"
#include <utils.h>
#include <format.h>

using std::sort;
using std::string;
using std::to_string;
using std::vector;

using namespace std;


Process::Process(int pid, long Hertz) : pid_(pid), Hertz_(Hertz) {
    vector<string> cpuNumbers = ReadFile(pid);

    utime_ = stof(cpuNumbers[13]);
    stime_ = stof(cpuNumbers[14]);
    cutime_ = stof(cpuNumbers[15]);
    cstime_ = stof(cpuNumbers[16]);
    starttime_ = stof(cpuNumbers[21]);
}

int Process::Pid() { return pid_; }

double Process::CpuUtilization() {
    long uptime = LinuxParser::UpTime();
    double total_time = utime_ + stime_ + cutime_ + cstime_;

    double seconds = uptime - (starttime_ / Hertz_);
    double cpu_usage = (total_time / Hertz_) / seconds;

    return cpu_usage;
}

string Process::Command() {
    auto cmd{GetValue<string>(to_string(pid_) +
                                                LinuxParser::kCmdlineFilename).value_or("")};
    size_t maxSize{50};
    if(cmd.size() > maxSize) {
        cmd.resize(maxSize - 3);
        cmd = cmd + "...";
    }
    return cmd;
}

float Process::RawRam() {
    return GetValueByKey<float>(
            LinuxParser::filterProcMem,
            to_string(pid_) + LinuxParser::kStatusFilename).value_or(0.0);
}

string Process::Ram() {
    auto memInKB{RawRam()};
    return Format::KBisMB(memInKB);
}

string Process::User() {
    auto UID{GetValueByKey<int>(
            LinuxParser::filterUID, to_string(pid_) + LinuxParser::kStatusFilename)};

    string user = LinuxParser::UserByUID(UID.value_or(0));
    return user;
}

long int Process::UpTime() {
    long uptime = LinuxParser::UpTime();
    long seconds = uptime - floor(starttime_ / Hertz_);

    return seconds;
}

vector<string> Process::ReadFile(int pid) {
    string line, skip;

    std::ifstream stream(LinuxParser::kProcDirectory + to_string(pid) +
                         LinuxParser::kStatFilename);

    getline(stream, line);
    istringstream linestream(line);
    istream_iterator<string> beg(linestream), end;
    vector<string> cpuNumbers(beg, end);
    return cpuNumbers;
};


bool compareProcesses(Process& p1, Process& p2) {
    return (p1.RawRam() > p2.RawRam());
};

ProcessesClass::ProcessesClass() {
    Hertz = sysconf(_SC_CLK_TCK);
    UpdateProcesses();
}

vector<Process>& ProcessesClass::GetProcesses() {
    UpdateProcesses();
    return processes_;
}

void ProcessesClass::UpdateProcesses() {
    current_pids_ = ReadFolders();

    bool changed = false;

    AddNewProcesses(changed);
    RemoveFinishedProcesses(changed);

    if (changed) {
        sort(processes_.begin(), processes_.end(), compareProcesses);
    }
}

vector<int> ProcessesClass::ReadFolders() { return LinuxParser::Pids(); };

void ProcessesClass::AddNewProcesses(bool& changed) {
    for (std::size_t i = 0; i < current_pids_.size(); ++i) {
        int current_pid = current_pids_[i];

        if (std::find_if(processes_.begin(), processes_.end(),
                         [current_pid](Process& n) {
                             return n.Pid() == current_pid;
                         }) == processes_.end()) {
            changed = true;
            Process process(current_pids_[i], Hertz);

            processes_.emplace_back(process);
        }
    }
}

void ProcessesClass::RemoveFinishedProcesses(bool& changed) {
    for (size_t i = 0; i < processes_.size(); i++) {
        int current_pid = processes_[i].Pid();

        if (std::find(current_pids_.begin(), current_pids_.end(), current_pid) ==
            current_pids_.end()) {
            changed = true;

            processes_.erase(processes_.begin() + i);
        }
    }
}