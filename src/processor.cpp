#include "processor.h"
#include <linux_parser.h>
using std::string;
using std::vector;

double Processor::Utilization() {
    vector<double> values = ReadFile();
    auto user = values[0];
    auto nice = values[1];
    auto system = values[2];
    auto idle = values[3];
    auto iowait = values[4];
    auto irq = values[5];
    auto softirq = values[6];
    auto steal = values[7];

    auto PrevIdle = previdle + previowait;
    auto Idle = idle + iowait;

    auto PrevNonIdle =
            prevuser + prevnice + prevsystem + previrq + prevsoftirq + prevsteal;
    auto NonIdle = user + nice + system + irq + softirq + steal;

    auto PrevTotal = PrevIdle + PrevNonIdle;
    auto Total = Idle + NonIdle;

    auto totald = Total - PrevTotal;

    auto idled = Idle - PrevIdle;

    auto CPU_Percentage = (totald - idled) / totald;

    AssignPrevValues(values);
    return CPU_Percentage;
}

void Processor::AssignPrevValues(vector<double> newValues) {
    prevuser = newValues[0];
    prevnice = newValues[1];
    prevsystem = newValues[2];
    previdle = newValues[3];
    previowait = newValues[4];
    previrq = newValues[5];
    prevsoftirq = newValues[6];
    prevsteal = newValues[7];
}

vector<double> Processor::ReadFile() {
    string line, key;
    double value;
    vector<double> cpuNumbers;
    std::ifstream stream(LinuxParser::kProcDirectory +
                         LinuxParser::kStatFilename);
    if (stream.is_open()) {
        while (std::getline(stream, line)) {
            std::istringstream linestream(line);
            while (linestream >> key) {
                if (key == LinuxParser::filterCpu) {
                    while (linestream >> value) {
                        cpuNumbers.emplace_back(value);
                    }
                }
            }
        }
    }
    return cpuNumbers;
}