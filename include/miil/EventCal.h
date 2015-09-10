#ifndef EVENTCAL_H
#define EVENTCAL_H

#include <stdint.h>
#include <climits>

static_assert(sizeof(float) * CHAR_BIT == 32, "Require 32 bit floats");

/*!
 * \brief An event from the DAQ system which has been calibrated
 *
 * The output structure of the event that has been calibrated for both energy
 * and time, and has been assigned a crystal location within the system.
 */
struct EventCal {
    //! The course timestamp recorded by the FPGA controlling the rena
    int64_t ct;
    //! The fine timestamp calculated from the UV timing circle
    float ft;
    //! Calibrated Energy of the event
    float E;
    //! The total of the spatial channels and anger logic denominator
    float spat_total;
    //! The x position of the event within the flood histogram
    float x;
    //! The y position of the event within the flood histogram
    float y;
    //! The panel of the event
    int8_t panel;
    //! The cartridge of the event
    int8_t cartridge;
    //! The fin of the event
    int8_t fin;
    //! The module of the event
    int8_t module;
    //! The apd of the event
    int8_t apd;
    //! The crystal of the event
    int8_t crystal;
    //! DAQ Board of the event
    int8_t daq;
    //! Rena of the event
    int8_t rena;
    /*!
     * The structure will be padded to 40 bytes by the compiler, due to the 8
     * byte alignment requirement for the int64_t (long) so flags were added as
     * space for storing information in the future that may be useful for
     * calibration or processing purposes.  Their current uses are:
     *     - 0: none
     *     - 1: none
     *     - 2: none
     *     - 3: none
     */
    int8_t flags[4];
};

static_assert(sizeof(EventCal) == 40, "Require 40 byte EventCal structure");

#endif
