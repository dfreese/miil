#include <miil/log.h>
#include <sstream>
#include <miil/util.h>

Log::Log(const std::string & loc, const std::string & base, bool open, int max_lines):
	logfileloc(loc),
	logfilebase(base),
	logfileopen(open),
	numberOfLines(0),
	maxlines(max_lines)
{
    // Add a slash to the directory name if isn't already there
    if (*(logfileloc.end()-1) != '/') {
        logfileloc += "/";
    }
    if (logfileopen) {
        this->open();
    }
}

Log::~Log() {
    close();
}

int Log::open() {
	logfilename = logfileloc + logfilebase + Util::GetFormattedDateAndTime() + ".txt";
    logfile.open(logfilename.c_str(),std::ios::out);
    if (logfile.good()) {
        logfileopen=true;
        return 0;
    } else {
        return -1;
    }
}

void Log::close() {
    if(logfileopen) {
        logfile.close();
    }
}

void Log::WriteLine(std::string line) {
	checklength();
	std::string time = Util::FormatUnixTime(Util::GetTimeOfDay());
	logfile << time << " " << line << std::endl;
	numberOfLines++;
}

void Log::checklength() {
    if(numberOfLines>=maxlines) {
        close();
        open();
        numberOfLines=0;
    }
}

void Log::setLength(int newlength) {
    maxlines = newlength;
}
