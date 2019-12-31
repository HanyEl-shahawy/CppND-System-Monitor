 #include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <cmath>

#include "linux_parser.h"

#include <utils.h>
using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

 float LinuxParser::MemoryUtilization() {
   float memory{};
   auto memTotal{GetValueByKey<int>(
           LinuxParser::filterMemTotalString, LinuxParser::kMeminfoFilename).value_or(0)};
   auto memFree{GetValueByKey<int>(
           LinuxParser::filterMemFreeString, LinuxParser::kMeminfoFilename).value_or(0)};
   if (memTotal != 0)
     memory = (static_cast<float>(memTotal - memFree) / memTotal);

   return memory;
 }

 long LinuxParser::UpTime() {
   auto upTime{GetValue<long>(LinuxParser::kUptimeFilename)};
   return upTime.value_or(0);
 }

 int LinuxParser::TotalProcesses() {
   return GetValueByKey<int>(LinuxParser::filterProcesses,
                                           LinuxParser::kStatFilename).value_or(0);
 }

 int LinuxParser::RunningProcesses() {
   return GetValueByKey<int>(LinuxParser::filterRunningProcesses,
                                           LinuxParser::kStatFilename).value_or(0);
 }

 string LinuxParser::UserByUID(int UID) {
   string line{};
   string user{};
   string x{};
   int fileUid{};

   std::ifstream filestream(LinuxParser::kPasswordPath);
   if (filestream.is_open()) {
     while (std::getline(filestream, line)) {
       std::replace(line.begin(), line.end(), ':', ' ');
       std::istringstream linestream(line);
       while (linestream >> user >> x >> fileUid) {
         if (fileUid == UID) {
           return user;
         }
       }
     }
   }
   return user;
 }