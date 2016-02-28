#ifndef FILE_UTILS_H_
#define FILE_UTILS_H_

#include <map>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <deque>
#include <vector>


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
    int loadFilelistFile(
            const std::string & filename,
            std::vector<std::string> & files);

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

    /*!
     * \brief Reads a binary file of type into a deque
     *
     * \param filename Name of file to be read
     * \param container The deque to read into
     * \param read_buff_size Size of chunks to read in bytes
     *
     * \return
     *      - 0 on success
     *      - -1 when unable to open the file
     */
    template<typename T>
    int readFileIntoDeque(
            const std::string & filename,
            std::deque<T> & container,
            size_t read_buff_size = 1048576)
    {
        std::ifstream file(filename.c_str(), std::ios::binary);
        if (!file.good()) {
            return(-1);
        }

        size_t chunk_size = read_buff_size / sizeof(T);
        std::vector<T> chunk(chunk_size);

        while (file.read((char*) chunk.data(), chunk.size()) || file.gcount()) {
            container.insert(
                        container.end(),
                        chunk.begin(),
                        chunk.begin() + file.gcount());
        }
        return(0);
    }

    template<typename T>
    int readFileIntoVector(
            const std::string & filename,
            std::vector<T> & container,
            size_t read_buff_size = 1048576)
    {
        std::ifstream file(filename.c_str(), std::ios::binary);
        if (!file.good()) {
            return(-1);
        }

        size_t chunk_size = read_buff_size / sizeof(T);
        std::vector<T> chunk(chunk_size);

        while (file.read((char*) chunk.data(), chunk.size()) || file.gcount()) {
            container.insert(
                        container.end(),
                        chunk.begin(),
                        chunk.begin() + file.gcount());
        }
        return(0);
    }

}

#endif // FILE_UTILS_H_
