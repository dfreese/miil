#include <miil/file_utils.h>
#include <iostream>

/*! \brief loads a file of typed varible settings
 *
 * This opens a file with a given filename, and then assumes a structure of
 * "VARIABLE_NAME = VALUE" for each line of the file.  The value is loaded in
 * as a string, and if it is enclosed in quotations, then it will load in
 * values with spaces, otherwise values with spaces will have everything but
 * the first word ignored.  If the map varaible values is passed as an empty
 * map, then every line in the file will be loaded.  If the map is passed with
 * some keys created, the file will be searched for variable names EXACTlY
 * matching the key string.  If it is found in the file, it will be loaded into
 * the key's pair.
 *
 * \param values The map that the values from the file will be loaded into. If
 *               it is passed to the function empty, all variables will be
 *               loaded, otherwise, only the existing keys will be loaded if
 *               found
 * \param settings_file The name and path to the file to be loaded
 *
 * \return 0 if the file is loaded successfully, negative otherwise
 */
int Util::LoadSettingsFile(std::map<std::string,std::string> & values,const std::string & settings_file) {
	std::ifstream inputFile(settings_file.c_str(), std::ifstream::in);

	if (!inputFile.is_open()) {
		return(-1);
    }

    bool read_all = false;
    if (values.empty()) {
        read_all = true;
    }

	std::string fileline="";
	while(getline(inputFile,fileline)) {
        if (fileline.size() == 0) {
            // Ignore blank lines.
            continue;
        } else if (fileline.size() > 0) {
            if (fileline[0] == '#') {
                // If a line starts with a pound sign, it is ignored
                continue;
            }
        }
		std::string equal;
		std::string value;
		std::stringstream ss(fileline);
		std::string parameter;
		ss >> parameter;

        // If reading in all values, create key in map, or reinitialize it to be ""
        if (read_all) {
            values[parameter] = "";
        }

		std::map<std::string,std::string>::iterator pair_iter = values.find(parameter);

		if (pair_iter != values.end() ) {
			ss >> equal;
	        if (equal.compare("=")!=0){
                std::cerr << "In file \"" << settings_file << "\":" << std::endl;
                std::cerr << "\tInvalid declaration of \"" << parameter << "\""<< std::endl;
				inputFile.close();
				return(-1);
			}
			ss >> value;

			// Allow for processing of strings enclosed in quotations
			if (value.find('"') == 0 ) {
                // Subtract off quotation
				value = value.substr(1,value.length()-1);
                // Check that there's not a quotation in the word extracted
                size_t quote_check = value.find('"');
                if (quote_check == std::string::npos) {
                    // No quotations were found, add characters one by one
                    // until one is found
                    char next = ss.get();
                    while (next != '"') {
                        value += next;
                        // Get next character if there's any left, otherwise
                        // give the rest of the line
                        if (ss.good()) {
                            next = ss.get();
                        } else {
                            break;
                        }
                    }
                } else {
                    // A quotation was found in the word, hopefully just a
                    // string with no spaces enclosed in quotations. Either way
                    // just throw away everything after the quotation mark
                    value = value.substr(0,quote_check);
                }
			}

			pair_iter->second = value;
		}
		else {
			std::cout << "Parameter: " << parameter << " - In File, but not specified" << std::endl;
		}
	}

	inputFile.close();
	return(0);
}

int Util::LoadListSettingsFile(std::map<std::string,std::vector<std::string> > & values, const std::string & settings_file) {
	std::ifstream inputFile(settings_file.c_str(), std::ifstream::in);

	if (!inputFile.is_open()) {
		return(-1);
    }

    bool read_all = false;
    if (values.empty()) {
        read_all = true;
    }

	std::string fileline="";
	while(getline(inputFile,fileline)) {
        if (fileline.size() == 0) {
            // Ignore blank lines.
            continue;
        } else if (fileline.size() > 0) {
            if (fileline[0] == '#') {
                // If a line starts with a pound sign, it is ignored
                continue;
            }
        }
		std::string equal;
		std::string value;
		std::stringstream ss(fileline);
		std::string parameter;
		ss >> parameter;

        // If reading in all values, create key in map, or reinitialize it
        if (read_all) {
            values[parameter] = std::vector<std::string>();
        }

		std::map<std::string,std::vector<std::string> >::iterator pair_iter = values.find(parameter);

		if (pair_iter != values.end() ) {
			ss >> equal;
	        if (equal.compare("=")!=0){
                std::cerr << "In file \"" << settings_file << "\":" << std::endl;
                std::cerr << "\tInvalid declaration of \"" << parameter << "\""<< std::endl;
				inputFile.close();
				return(-1);
			}

			while(ss.good()) {
				ss >> value;

				// Allow for processing of strings enclosed in quotations
				if (value.find('"') == 0 ) {
					value = value.substr(1,value.length()-1);
					std::string sub;
					ss >> sub;
					while (sub.rfind('"') != (sub.length()-1) ) {
						if (!ss.good()) break; // In case of a forgotten trailing quotation
						value = value + " " + sub;
						ss >> sub;
					}
					value = value + " " + sub;
					value = value.substr(0,value.length()-1);
				}

				//pair_iter->second = value;
				pair_iter->second.push_back(value);
			}
		}
		else {
			std::cout << "Parameter: " << parameter << " - In File, but not specified" << std::endl;
		}
	}

	inputFile.close();
	return(0);
}


int Util::LoadNestedSettingsFile(
        std::map<std::string,std::map<std::string,std::string> > & values,
        const std::string & settings_file) {
	std::ifstream inputFile(settings_file.c_str(), std::ifstream::in);

	if (!inputFile.is_open()) {
		return(-1);
    }

    // If an empty map is given, fill it with everything in the file
    // Otherwise, fill it with the given parameters
    bool fill_all = true;
    if (values.size() != 0) {
        fill_all = false;
    }

    std::string fileline="";
    bool block_open = false;
    bool block_fill_all = true;
    std::string block_name = "";
    while(getline(inputFile,fileline)) {
        if (fileline.size() == 0) {
            // Ignore blank lines.
            continue;
        } else if (fileline.size() > 0) {
            if (fileline[0] == '#') {
                // If a line starts with a pound sign, it is ignored
                continue;
            }
        }
        std::stringstream ss(fileline);
        if (block_open) {
            if (inputFile.good()) {
                std::string parameter;
                ss >> parameter;

                if (parameter == "}") {
                    block_open = false;
                } else {

                    bool read_in_parameter = true;
                    if (!block_fill_all) {
                        std::map<std::string,std::string>::iterator pair_iter = values[block_name].find(parameter);
                        if (pair_iter == values[block_name].end()) {
                            read_in_parameter = false;
                        }
                    }
                    //std::cout << "read_in_parameter: " << read_in_parameter << std::endl;
                    if (read_in_parameter) {
                        std::string equal;
                        ss >> equal;
                        if (equal == "=") {
                            std::string value;
                            ss >> value;
                            values[block_name][parameter] = value;
                        } else {
                            // Parameter not specified correctly
                            //std::cout << "Parameter: " << parameter << " not specified correctly.\n";
                        }
                    } else {
                        // Parameter specified, but not read in
                        //std::cout << "Parameter: " << parameter << " specified, but not read in.\n";
                    }
                }
            } else {
                std::cout << "File finished without closing block\n";
                // File finished without closing block
                inputFile.close();
                return(-1);
            }
        } else {
            std::string block_start;
            ss >> block_name;
            ss >> block_start;

            if (block_start == "{") {
                if (fill_all) {
                    values[block_name] = std::map<std::string,std::string>();
                    block_open = true;
                    block_fill_all = true;
                } else {
                    std::map<std::string, std::map<std::string,std::string> >::iterator pair_iter = values.find(block_name);
                    if (pair_iter != values.end()) {
                        block_open = true;
                        if (pair_iter->second.size() == 0) {
                            block_fill_all = true;
                        } else {
                            block_fill_all = false;
                        }
                    }
                }

            } else {
                // Block not opened properly
            }
        }
    }

    inputFile.close();
    return(0);
}

/*! \brief Look through map for series of parameters
 *
 * file_utils uses the map construct to store values from settings files.  If
 * the parameter values to be looked for are not specified beforehand, then all
 * of the parameters from the file are loaded.  This function takes a list of
 * parameters that are needed and looks through a given map file to see if they
 * are there or not
 *
 * \param param A vector of strings specifying the parameters that are required
 * \param map The map of values through which the function look for the params
 *
 * \return A 0 if there are no missing parameters, and negative number
 *         representing the number of missing parameters.
 */
int Util::checkForParameters(
        const std::vector<std::string> & param,
        const std::map<std::string,std::string> & map)
{
    int result = 0;
    for (size_t i = 0; i < param.size(); ++i) {
        std::map<std::string,std::string>::const_iterator it = map.find(param[i]);
        if (map.end() == it) {
            std::cerr << "Parameter: \"" << param[i] << "\" was not found\n";
            --result;
        }
    }
    return(result);
}

int Util::checkForParameters(
        const std::vector<std::string> & param,
        const std::map<std::string,std::vector<std::string> > & map)
{
    int result = 0;
    for (size_t i = 0; i < param.size(); ++i) {
        std::map<std::string,std::vector<std::string> >::const_iterator it =
                map.find(param[i]);
        if (map.end() == it) {
            std::cerr << "Parameter: \"" << param[i] << "\" was not found\n";
            --result;
        }
    }
    return(result);
}
