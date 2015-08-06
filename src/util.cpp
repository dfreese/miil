#include <miil/util.h>
#include <fstream>
#include <algorithm>
#include <sys/time.h>

/*! \brief Turns a string into a boolean value
 *
 * Takes a string, and checks to see if it is either a 1 or a 0 and then turns
 * it into a true or false.  If it is neither of these things, the value is not
 * modified and an error is returned
 *
 * \param str The string to be converted into a boolean value (either 0 or 1)
 * \param val The variable to which a valid boolean value is stored.
 *
 * \return A 0 if successful, less than zero otherwise.
 */
int Util::StringToBool(const std::string & str, bool & val) {
    if (str == "0") {
        val = false;
        return(0);
    } else if(str == "1") {
        val = true;
        return(0);
    } else {
        return(-1);
    }
}

/*! \brief Creates an inverse of a mapping vector
 *
 * Takes a vector of integers where each integer is used to map to
 * an index in a different vector or array and inverts it by creating
 * a vector whos ith index maps to the integer i in the given vector
 *
 * \param vector of integers that maps to the indicies of a different array
 *
 * \return an inverse of the mapping vector, or a map of the map. Unmapped
           values are set to -1.
 */
std::vector<int> Util::InvertMappingVector(std::vector<int> map) {
    std::vector<int> ret(*std::max_element(map.begin(), map.end()), -1);
    for (size_t i = 0; i < map.size(); i++) {
        if (map[i] >= 0) {
            ret[map[i]] = i;
        }
    }
    return(ret);
}

/*! \brief Turns vector of strings into a vector of integers
 *
 * Takes a vector of strings, where the strings represent integers and
 * converts it into a vector of integers using Util::str2int(std::string), which
 * uses a stringstream to convert the numbers.
 *
 * \param strvec A vector of strings that represent integers
 *
 * \return A vector of integers that were represented by the strings
 */
std::vector<int> Util::strvec2intvec(std::vector<std::string> strvec) {
    std::vector<int> ret;
    ret.reserve(strvec.size());
    for (size_t i = 0; i < strvec.size(); i++) {
        ret.push_back(str2int(strvec[i]));
    }
    return(ret);
}

/*! \brief Turns a string representing a number into a double value
 *
 * This turns a string that represents a number into a double floating
 * point number using stringstream to convert
 *
 * \param str The string representing the number
 *
 * \return The number represented by the string
 */
double Util::str2double(std::string str) {
    double num;
    std::stringstream ss(str);
    ss >> num;
    return num;
}

/*! \brief Turns a string representing a number into a float value
 *
 * This turns a string that represents a number into a floating
 * point number using stringstream to convert
 *
 * \param str The string representing the number
 *
 * \return The number represented by the string
 */
float Util::str2float(std::string str) {
    float num;
    std::stringstream ss(str);
    ss >> num;
    return num;
}

/*! \brief Turns a string representing a number into an integer value
 *
 * This turns a string that represents a number into an integer using
 * stringstream to convert
 *
 * \param str The string representing the number
 *
 * \return The number represented by the string
 */
int Util::str2int(std::string str) {
    int num;
    std::stringstream ss(str);
    ss >> num;
    return num;
}

/*! \brief Turns an integer into vector of boolean bits MSB first
 *
 * Takes an integer and turns it into a vector of boolean bits starting
 * with the more significant bits of the integer.  The first bit to be
 * converted is specified by the numBits parameter, so if one was converting
 * a byte to a bit vector, 8 would be specified, and the 8th least
 * significant bit would be the first in the returned vector and the LSB
 * would be the final bit in the returned vector
 *
 * \param input The integer to be converted into a bit vector
 * \param numBits The number of bits to be taken from the supplied integer
 *
 * \return A bit vector representing the bits of the integer specified to
           be taken out
 */
std::vector<bool> Util::int2BoolVec(int input, unsigned int numBits) {
    // Most significant bit is returned in index 0 of the vector.
    if (numBits > (8 * sizeof(int))) {
        return(std::vector<bool>());
    }

    int inputMask=1;
    inputMask = inputMask << (numBits-1);

    std::vector<bool> output(numBits,0);
    int inputShifted = input;
    for (unsigned int ii=0; ii<numBits; ii++) {
        if((inputShifted&inputMask)!=0){
            output[ii]=1;
        }
        inputShifted = inputShifted << 1;
    }
    return(output);
}

/*! \brief Converts a bit vector into an integer, assuming MSB first
 *
 * Takes the supplied bit vector and converts it into an integer by
 * shifting in each bit using a multiply times 2 and adding in a 1
 * or zero depending on the next bit to be added into the integer
 *
 * \warning Does not check the size of the bit vector against an
            integer, so overflows can occur.
 *
 * \param The bit vector to be converted into an integer
 *
 * \return integer that was represented by the bit vector
 */
int Util::boolVec2Int(const std::vector<bool> &input) {
    // Most significant bit should be located at index 0 of the vector.
    int output=0;
    for (unsigned int ii=0; ii<input.size(); ii++) {
        output*=2;
        output+=int(input[ii]);
    }
    return(output);
}

/*! \brief Erases a given file
 *
 * Takes the given file and clears it by opening and closing it
 * using ofstream
 *
 * \param filename The string representing the file path
 */
void Util::clearFile(std::string filename) {
    std::ofstream file;
    file.open(filename.c_str(), std::ios::out | std::ios::trunc);
    file.close();
}

/*! \brief Returns a filename with a number appended
 *
 * Takes the given filename and appends an underscore '_' and then
 * a number that is provided to the function before the last '.'
 * that is contained within the string
 *
 * \param filename The base filename that will have the number added
 * \param counter The number that will be added into the filename
 *
 * \return A filename with a _# added before the last '.'
 */
std::string Util::buildSplitFilename(std::string filename, int counter) {
    std::stringstream ss;
    ss << filename.substr(0,filename.find_last_of('.'))
       << "_" << counter << filename.substr(filename.find_last_of('.'));
    return(ss.str());
}

/*! \brief Returns a filename with the file number removed
 *
 * Takes the given filename and removes an appended underscore '_' and
 * a number before the last '.'.  This has an upper limit of 9999 for the
 * counter that was used to build the split file.
 *
 * \param filename The filename with a number
 * \param base_filename The location the base filename extracted from filename
 *                      is stored.  This is not modified unless the function is
 *                      successful.
 *
 * \return 0 if the function is successful.
 *         -1 if the '.' is not found.
 *         -2 if the '_' is not found.
 *         -3 if the '.' is before the '_'.
 *         -4 if the counter would not be of the appropriate length '0' to
 *            '9999'
 */
int Util::removeSplitFilename(
        const std::string & filename,
        std::string & base_filename)
{
    size_t period_position = filename.find_last_of('.');
    size_t underscore_position = filename.find_last_of('_');

    if (period_position == std::string::npos) {
        return(-1);
    }
    if (underscore_position == std::string::npos) {
        return(-2);
    }
    if (period_position < underscore_position) {
        return(-3);
    }
    size_t difference(period_position - underscore_position);
    if ((difference < 1) || (difference > 4)) {
        return(-4);
    }

    base_filename = filename.substr(0,underscore_position) +
                    filename.substr(period_position);
    return(0);
}

/*! \brief Converts an number into a string of bits, MSB first
 *
 * This takes the input integer and converts it into a boolean vector
 * using Util::int2BoolVec(int,unsigned int), and then displays it
 * as a series of 1s and 0s from MSB to LSB.
 *
 * \param input The integer to be converted to a binary string
 * \param numBits The number of bits to be displayed, starting from LSB
 *
 * \return A string of 1s and 0s representing the integer
 */
std::string Util::int2BinaryString(int input, unsigned int numBits)
{
    std::string output = "";
    std::vector<bool> boolVec = int2BoolVec(input, numBits);
    for (size_t ii=0; ii<boolVec.size(); ii++) {
        if (boolVec[ii]) {
            output += '1';
        } else {
            output += '0';
        }
    }
    return output;
}

/*! \brief Takes a string of 1s and 0s and converts it to an integer
 *
 * Takes the input string and converts each 0 into a false and everything
 * else into a true.  Assumes that the MSB is first, and converts it
 * using Util::boolVec2Int(const std::vector<bool> & ).
 *
 * \param input The string of bits from MSB to LSB
 *
 * \return The integer represented by the bit string
 */
int Util::binaryString2int(std::string input) {
    std::vector<bool> boolVec;
    for (size_t ii=0; ii<input.size(); ii++) {
        if (input[ii] == '0') {
            boolVec.push_back(false);
        } else {
            boolVec.push_back(true);
        }
    }
    return boolVec2Int(boolVec);
}

/*! \brief Returns the directory the executable is located in
 *
 * This function returns the global system directory in which the executable is
 * located in, so that paths can be referenced from the executable, rather than
 * from PWD.
 *
 * \return string containing global path to executable
 */
std::string Util::getExecuatableDirectory() {
    char buff[1024];
    ssize_t len = ::readlink("/proc/self/exe", buff, sizeof(buff)-1);
    if (len != -1) {
        buff[len] = '\0';
        std::string ret(buff);
        ret = ret.substr(0,ret.rfind('/')+1);
        return ret;
        //return string(buff);
    } else {
        /* handle error condition */
        return std::string();
    }
}

/*! \brief Builds a vector of integers in accending or descending order
 *
 * Builds a vector of integers of the requested size either counting up from
 * zero to (size-1) or from (size-1) to zero depending for increasing flag being
 * true or false respectively.
 *
 * \param size The size of the vector that is returned
 * \param increasing An optional argument to sets the order of the counting.
                     Default is true
 *
 * \return Vector of integers counting up from or down to zero
 */
std::vector<int> Util::BuildOrderedVector(int size, bool increasing) {
    std::vector<int> ret(size,0);
    if (!increasing) ret.front() = (int) (ret.size()-1);
    std::vector<int>::iterator it = ret.begin();
    do {
        it++;
        if (increasing) {
            *(it) = *(it-1)+1;
        } else {
            *(it) = *(it-1)-1;
        }
    } while (it != ret.end());
    return(ret);
}

/*! \brief Returns Unix Time with microsecond precision
 *
 * Returns a double value that represents seconds since the epoc with
 * microsecond precision. The function uses gettimeofday to retrieve the time
 * using a struct timeval.  This is then converted into double value.
 *
 * \return Double representing seconds from the epoc
 */
double Util::GetTimeOfDay() {
    struct timeval tim;
    gettimeofday(&tim, NULL);
    double time_point = (double)tim.tv_sec+(double)tim.tv_usec/1e6;
    return(time_point);
}

/*! \brief Formatted string representing seconds from the epoc with milliseconds
 *
 * Takes the number of seconds from the epoc in double form and uses
 * stringstream and setprecision to format it as the number of seconds with 3
 * decimals of precision to display milliseconds since the epoc
 *
 * \return string formatted to three decimal places for the time
 */
std::string Util::FormatUnixTime(double time) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(3) << time;
    return(ss.str());
}

/*! \brief Date string formatted for filenames with a specific creation
 *
 * Retrieves the current date and time using ::time and ::localtime to provide a
 * string with a date formated with YYYYmmddHHMMSS using ::strftime which then
 * returns a basic_string
 *
 * \return String with the date formated for filenames
 */
std::string Util::GetFormattedDateAndTime() {
    char temp[128];
    time_t curr_time = time(NULL);
    strftime(temp,sizeof(temp),"%Y%m%d%H%M%S",localtime(&curr_time));

    return(std::string(temp));
}

/*! \brief Prints out a vector of bits in byte format
 *
 * Takes a vector of bytes and prints it out in sets of 8. This assumes the most
 * significant bit is in the data[0] position.
 *
 * \param data The boolean vector to be printed out
 *
 */
void Util::PrintBoolVector(const std::vector<bool> & data) {
    size_t remainder = data.size() % 8;
    std::vector<bool>::const_iterator it = data.begin();

    while( (it != data.end()) && (it != (data.begin()+remainder) ) ) {
        std::cout << *(it++);
    }

    int counter = 0;
    while( (it != data.end()) ) {
        if( (counter % 8) == 0)
            std::cout << " ";
        if( (counter % 16) == 0)
            std::cout << " ";
        std::cout << *(it++);
        counter++;
    }

}


/*! \brief Prints out a vector of bits as ones and zeros grouped as nibbles
 *
 * \param data The boolean vector of bits to be printed out
 * \param LineStart The string that preceeds the line of hex characters to
                    printed out
 * \param EndLine A flag that indicates whether or not to append a line break to
                  the end of the statement
 *
 */
void Util::PrintBoolVectorLine(
        const std::vector<bool> & data,
        std::string LineStart,
        bool EndLine)
{
    std::cout << LineStart;
    PrintBoolVector(data);
    if (EndLine)
        std::cout << std::endl;
}

/*! \brief Prints out a vector of bytes in hex format
 *
 * Wraps each byte in a vector with ostream commands so that each byte is
 * printed as a series of hex formatted bytes separated by a space.
 *
 * \param data Vector of bytes to be printed out
 *
 */
void Util::PrintHexVector(const std::vector<unsigned char> & data) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(2);
    for (std::vector<unsigned char>::const_iterator it=data.begin();
         it!=data.end();
         ++it)
    {
        ss << (int) *it << " ";
    }
    std::cout << ss;
}


/*! \brief Prints out a introduction string followed by a the bytes in hex format
 *
 * Prints a line of bytes in a vector in hex format separated by a space by
 * wrapping each byte with ostream commands.  Prepends the line with the string
 * specified by LineStart and adds a line break based on user input.
 *
 * \param data Vector of bytes to be printed out
 * \param LineStart The string that preceeds the line of hex characters to
                    printed out
 * \param EndLine A flag that indicates whether or not to append a line break to
                  the end of the statement
 *
 */
void Util::PrintHexVectorLine(
    const std::vector<unsigned char> & data,
    std::string LineStart,
    bool EndLine)
{
    std::cout << LineStart;
    PrintHexVector(data);
    if (EndLine)
        std::cout << std::endl;
}

/*! \brief Sleeps a specified number of nanoseconds
 *
 * A wrapper around nanosleep to cause the program to sleep for a specified
 * amount of nanoseconds
 *
 * \param ns Specifies the number of nanoseconds the program should sleep
 *
 */
void Util::sleep_ns(long long ns) {
    struct timespec req = {0,0};
    while (ns > 999999999) {
        req.tv_sec = req.tv_sec + 1;
        ns -= 1000000000;
    }
    req.tv_nsec = (long) ns;
    nanosleep(&req,(struct timespec *) NULL);
}

/*! \brief Sleeps a specified number of microseconds
 *
 * A wrapper around nanosleep to cause the program to sleep for a specified
 * amount of microseconds
 *
 * \param ns Specifies the number of microseconds the program should sleep
 *
 */
void Util::sleep_us(long long us) {
    sleep_ns(1000L*us);
}

/*! \brief Sleeps a specfied number of milliseconds
 *
 * A wrapper around nanosleep to cause the program to sleep for a specified
 * amount of milliseconds
 *
 * \param ns Specifies the number of milliseconds the program should sleep
 *
 */
void Util::sleep_ms(long long ms) {
    sleep_ns(1000000L*ms);
}

/*! \brief Shifts a specified number of bits from byte into a vector
 *
 * This function takes a byte and places the specified number of bits into a
 * vector starting with the MSB and working right.  If the number of bits is
 * less than the size of the byte supplied, then numBits of the least
 * significant bits will be palced into the vector.  If the number of bits
 * greater than 8 is specified then the function will throw a -1.
 *
 * \param input The byte that will be placed into a boolean vector
 * \param numBits The number of bits from the byte that should be placed into
                  the vector starting with the MSB
 *
 * \return A boolean vector containing the bits starting with the MSB from the
           input byte
 *
 */
std::vector<bool> Util::Byte2BoolVec(uint8_t input,unsigned int numBits) {
    // Most significant bit is returned in index 0 of the vector.
    if (numBits>8*sizeof(uint8_t)) throw (-1);

    int inputMask=1;
    inputMask = inputMask << (numBits-1);

    std::vector<bool> output(numBits,0);
    int inputShifted = input;
    for (unsigned int ii=0; ii<numBits; ii++){
        if((inputShifted&inputMask)!=0){
            output[ii]=1;
        }
        inputShifted = inputShifted << 1;
    }
    return output;
}

/*! \brief Turns a bit vector into a byte
 *
 * Turns a vector of bits into a byte.  The bits are left shifted into the byte
 * one at a time for each bit in the input vector, meaning that the MS bits in
 * the returned byte are zeros if there
 * is not enough bits to fill the byte
 *
 * \param input The vector of bits with the most significant bit is first.
 *
 * \return a byte to represent the vector of bits
 *
 */
uint8_t Util::boolVec2Byte(const std::vector<bool> &input)
{
    // Most significant bit should be located at index 0 of the vector.
    int output=0;
    for (unsigned int ii=0; ii<input.size(); ii++) {
        output*=2;
        output+=uint8_t(input[ii]);
    }
    return output;
}

/*! \brief Takes a bool vector and splits it up into a vector of integers
 *
 * This function takes a bool vector of bits and splits it up into integers. If
 * a number less that 8 is selected for the numBitsPer, then each of the bytes
 * is padded on the MSB end with zeros.
 *
 * \param input Vector of bits with the MSB at input[0]
 * \param numBitsPer The number of bits that are put into each integer of the
                     output vector
 * \param reverse Determines the order that the bits are added to the vector of
                  bytes. False indicates MSB of return[0] is input[0],
                  true means MSB of return[end] is input[0]
 *
 * \return vector of integers filled out will all of the bits supplied by input
 *
 */
std::vector<uint8_t> Util::BoolVec2ByteVec(
        const std::vector<bool> & input,
        unsigned int numBitsPer,
        bool reverse)
{
    std::vector<uint8_t> ret;

    // If vector isn't evenly divisible, then return an empty vector
    if ( input.size() % numBitsPer != 0 ) {
        return std::vector<uint8_t>();
    }

    int counter=0;
    for ( size_t i=0; i<(input.size()/numBitsPer); i++) {
        std::vector<bool> part;
        for (unsigned int j=0; j<numBitsPer; j++) {
            part.push_back(input[counter++]);
        }
        if (reverse) {
            ret.insert(ret.begin(),boolVec2Byte(part));
        } else {
            ret.push_back(boolVec2Byte(part));
        }
    }

    return ret;
}

/*! \brief An explict version of string2Vec to read in numbers as bytes
 *
 * Because stringstream doesn't handle unsigned char the same as an integer
 * to read in a string number as a byte, this temlate required an explicit
 * specialization of uint8_t that first reads in the elements as an integer
 * and then reads in the bottom byte of the integer to place it in the
 * vector.  This allows a string of numbers to be read in as a byte stream
 *
 * \param str The string of bytes to be read in
 * \param result The vector in which the bytes from the string are placed
          This vector is cleared during the function
 * \param hex This is an optional flag that if true tells the function as
          if the characters are written in base hex.  Default is false
 */
template <>
void Util::string2Vec<uint8_t> (
        const std::string &str,
        std::vector<uint8_t> &result,
        bool hex)
{
    result.clear();
    int element;
    std::stringstream ss;
    if (hex) ss << std::hex;
    ss << str;
    while (ss >> element) {
        uint8_t ele = element;
        result.push_back(ele);
    }
}
