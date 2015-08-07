#ifndef UTIL_H
#define UTIL_H

#include <vector>
#include <string>
#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace Util {
    std::vector<bool> int2BoolVec(int input,unsigned int numBits);
    int boolVec2Int(const std::vector<bool> &input);
    void clearFile(std::string filename);
    std::string buildSplitFilename(std::string filename, int counter);
    int removeSplitFilename(
            const std::string & filename,
            std::string & base_filename);
    std::string int2BinaryString(int input, unsigned int numBits);
    int binaryString2int(std::string input);
    std::string getExecuatableDirectory();
    int StringToBool(const std::string & str, bool & val);
    std::vector<int> BuildOrderedVector(
            int size,
            bool increasing=true,
            int start=0);
    double GetTimeOfDay();
    std::string FormatUnixTime(double time);
    std::string GetFormattedDateAndTime();
    std::vector<int> InvertMappingVector(std::vector<int> map);
    std::vector<int> strvec2intvec(std::vector<std::string> strvec);
    double str2double(std::string str);
    float str2float(std::string str);
    int str2int(std::string str);

    // Delays
    void sleep_ns(long long ns);
    void sleep_us(long long us);
    void sleep_ms(long long ms);

    // Bit and Byte Manipulation
    std::vector<bool> Byte2BoolVec(uint8_t input, unsigned int numBits = 8);
    uint8_t boolVec2Byte(const std::vector<bool> &input);
    std::vector<uint8_t> BoolVec2ByteVec(const std::vector<bool> & input,
                                         unsigned int numBitsPer = 8,
                                         bool reverse=false);

    void PrintHexVector(const std::vector<unsigned char> & data);
    void PrintHexVectorLine(
            const std::vector<unsigned char> & data,
            std::string LineStart = "",
            bool EndLine=true);

    void PrintBoolVector(const std::vector<bool> & data);
    void PrintBoolVectorLine(
            const std::vector<bool> & data,
            std::string LineStart,
            bool EndLine=true);

    /*!
     * \brief Turns a vector into a string separated by spaces
     *
     * Takes a vector and uses string stream to create a string of the vector
     * elements separated by a space.
     *
     * \return string vector of the vector elements separated by a space
     */
    template <class T>
    std::string vec2String(const std::vector<T> &input) {
        std::stringstream ss;
        for (unsigned int ii=0; ii<input.size(); ii++) {
            if (ii != 0) {
                ss << " ";
            }
            ss << input[ii];
        }
        return ss.str();
    }

    /*!
     * \brief Turns a string of elements separated by spaces into a vector
     *
     * Takes a string of elements separated by spaces and using stringstream to
     * parse string, turns it into a vector of the type that is specified by the
     * user. The type must be compatible with stringstream.
     *
     * \param str The string of bytes to be read in
     * \param result The vector in which the bytes from the string are placed
          This vector is cleared during the function
     */
    template <class T>
    void string2Vec (
            const std::string &str,
            std::vector<T> &result,
            bool hex = false)
    {
        result.clear();
        T element;
        std::stringstream ss;
        if (hex) {
            ss << std::hex;
        }
        ss << str;
        while (ss >> element) {
            result.push_back(element);
        }
    }

    template <>
    void string2Vec<uint8_t> (
            const std::string &str,
            std::vector<uint8_t> &result,
            bool hex);

    template <class T>
    T StringToNumber(const std::string &str) {
        T element;
        std::stringstream ss(str);
        ss >> element;
        return(element);
    }

    template <class T>
    std::vector<T> StringVecToNumberVec(const std::vector<std::string> & vec) {
        std::vector<T> ret;
        for (std::vector<std::string>::const_iterator it = vec.begin();
             it != vec.end();
             ++it)
        {
            ret.push_back(StringToNumber<T>(*(it)));
        }
        return(ret);
    }
}

#endif // UTIL_H
