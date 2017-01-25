#ifndef SPLIT_H_
#define SPLIT_H_

#include <string>
#include <sstream>
#include <vector>

template<typename Out>
void split(const std::string &s, char delim, Out result) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

std::vector<std::string> split(const std::string &s, char delim);

#endif /* SPLIT_H_ */
