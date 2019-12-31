
#include "format.h"
#include <iomanip>
#include <sstream>
#include <string>
using std::string;
using std::to_string;

string Format::Format(long time) {
    auto timeAsString{to_string(time)};
    return string(2 - timeAsString.length(), '0') + timeAsString;
}

string Format::ElapsedTime(const long seconds) {
    auto hour{seconds / 3600};
    auto secs{seconds % 3600};
    auto mins{secs / 60};
    secs = secs % 60;
    return Format(hour) + ':' + Format(mins) + ':' + Format(secs);
}

string Format::KBisMB(float kb) {
    float mb = kb / 1024;
    std::stringstream mb_stream;
    mb_stream << std::fixed << std::setprecision(1) << mb;
    return mb_stream.str();
}