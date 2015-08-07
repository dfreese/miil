#ifndef MONITOR_H
#define MONITOR_H

#include <string>

class USBPort1;

class TempRhMonitor {

public:
    TempRhMonitor(int portid);
	std::string getName();
    bool Open(int portid);
	bool Open();
	void Close();
	int getTempAndRH(float & temp, float & rh);

private:
    USBPort1 * port;
    int portnumber;
    std::string portname;
	float temperature;
	float relhumidity;
};

#endif /* MONITOR_H */
