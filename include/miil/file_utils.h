#ifndef FILE_UTILS_H_
#define FILE_UTILS_H_

#include <map>
#include <string>
#include <vector>
#include <sstream>


namespace Util {

    int LoadSettingsFile(
            std::map<std::string,std::string> & values,
            const std::string & settings_file);
    int LoadListSettingsFile(
            std::map<std::string,std::vector<std::string> > & values,
            const std::string & settings_file);
    int LoadNestedSettingsFile(
            std::map<std::string,std::map<std::string,std::string> > & values,
            const std::string & settings_file);
    int checkForParameters(
            const std::vector<std::string> & param,
            const std::map<std::string,std::string> & map);
    int checkForParameters(
            const std::vector<std::string> & param,
            const std::map<std::string,std::vector<std::string> > & map);

    template<class T>
    std::map<std::string,T> StringMapToNumber(const std::map<std::string,std::string> & val) {
        std::map<std::string,T> ret;
        std::map<std::string,std::string>::const_iterator it = val.begin();

        while (it != val.end()) {
            std::stringstream ss(it->second);
            T val;
            ss >> val;
            ret[it->first] = val;
            ++it;
        }
        return ret;
    }
}

#endif // FILE_UTILS_H_
