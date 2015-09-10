#ifndef EVENTRAW_H
#define EVENTRAW_H

#include <stdint.h>

/*!
 * \brief An event on the rena chip
 *
 * A structure designed to hold all of the information from an event on a Rena.
 */
struct EventRaw {
    //! The course timestamp recorded by the FPGA controlling the rena
    int64_t ct;
    //! The low gain common channel for APD 0
    int16_t com0;
    //! The low gain common channel for APD 1
    int16_t com1;
    //! The high gain common channel for APD 0
    int16_t com0h;
    //! The high gain common channel for APD 1
    int16_t com1h;
    //! The u (x) timing channel for the low gain common on APD 0
    int16_t u0;
    //! The v (y) timing channel for the low gain common on APD 0
    int16_t v0;
    //! The u (x) timing channel for the low gain common on APD 1
    int16_t u1;
    //! The v (y) timing channel for the low gain common on APD 1
    int16_t v1;
    //! The u (x) timing channel for the high gain common on APD 0
    int16_t u0h;
    //! The v (x) timing channel for the high gain common on APD 0
    int16_t v0h;
    //! The u (x) timing channel for the high gain common on APD 1
    int16_t u1h;
    //! The v (x) timing channel for the high gain common on APD 1
    int16_t v1h;
    //! The a spatial signal
    int16_t a;
    //! The b spatial signal
    int16_t b;
    //! The c spatial signal
    int16_t c;
    //! The d spatial signal
    int16_t d;
    //! The panel of the event
    int8_t panel;
    //! The cartridge of the event
    int8_t cartridge;
    //! DAQ Board of the event
    int8_t daq;
    //! Rena of the event
    int8_t rena;
    //! The module local to the rena of the event
    int8_t module;
    /*!
     * The structure will be padded to 48 bytes by the compiler, due to the 8
     * byte alignment requirement for the int64_t (long) so flags were added as
     * space for storing information in the future that may be useful for
     * calibration or processing purposes.  Their current uses are:
     *     - 0: none
     *     - 1: none
     *     - 2: none
     */
    int8_t flags[3];
};

static_assert(sizeof(EventRaw) == 48, "Require 48 byte EventRaw structure");

#endif // EVENTRAW_H
