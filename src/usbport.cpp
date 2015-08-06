#include <usbport.h>

USBPort::USBPort() :
    totalSent(0),
    totalReceived(0),
    totalRetransmit(0),
    portState(false)
{
}
