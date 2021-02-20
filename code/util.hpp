#ifndef __UTIL__HPP__
#define __UTIL__HPP__

#include <string>
#include <vector>

namespace util{

    std::vector<std::string> split(std::string s, string delimiter = " "){
        size_t pos = 0;
        std::string token;
        std::vector<std::string> res;
        while ((pos = s.find(delimiter)) != std::string::npos)
        {
            token = s.substr(0, pos);
            res.push_back(token);
            s.erase(0, pos + delimiter.length());
            std::cout << token << std::endl;
        }

        res.push_back(s);

        return res;
    }
}

#endif