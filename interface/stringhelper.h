#ifndef APE_COMMON_STRING_HELPER_H
#define APE_COMMON_STRING_HELPER_H
#include <string>
#include <vector>
namespace ape {
namespace common {
static inline void split(const std::string &line, char delim, std::vector<std::string > *ret) {
    std::string::size_type begin = 0, end = -1;
    while (std::string::npos != (end = line.find(delim, begin))) {
        ret->push_back(line.substr(begin, end - begin));
        begin = end + 1;
    }
    ret->push_back(line.substr(begin));
}
}
}
#endif