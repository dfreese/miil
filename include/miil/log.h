#ifndef LOG_H
#define LOG_H

#include <string>
#include <fstream>

class Log {
	public:
		Log(const std::string & loc,const std::string & base, bool open = false, int max_lines = 44000);
		~Log();
		void close();
		int open();
		void setLength(int newlength);
		void WriteLine(std::string line);

	private:
		std::ofstream logfile;
		std::string logfilename;
		std::string logfileloc;
		std::string logfilebase;
		bool logfileopen;
		int numberOfLines;
		int maxlines;
		void checklength();
};

#endif // LOG_H
