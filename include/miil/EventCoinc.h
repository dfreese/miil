#ifndef EVENTCOINC_H
#define EVENTCOINC_H
#include <stdint.h>
#include <climits>

static_assert(sizeof(float) * CHAR_BIT == 32, "Require 32 bit floats");

/*!
 * \brief An event from the DAQ system which has been calibrated
 *
 * The output structure of the event that has been calibrated for both energy
 * and time, and has been assigned a crystal location within the system.
 */
struct EventCoinc {
    //! The course timestamp recorded by the FPGA controlling the left rena
    int64_t ct0;
    //! Difference in coarse timestamps (left - right)
    int64_t dct;
    //! The fine timestamp calculated from the left UV timing circle
    float ft0;
    //! Difference in fine timestamps (left - right)
    float dtf;
    //! Calibrated Energy of the left event
    float E0;
    //! Calibrated Energy of the right event
    float E1;
    //! The total of the left spatial channels and anger logic denominator
    float spat_total0;
    //! The total of the right spatial channels and anger logic denominator
    float spat_total1;
    //! The x position of the left event within the flood histogram
    float x0;
    //! The x position of the right event within the flood histogram
    float x1;
    //! The y position of the left event within the flood histogram
    float y0;
    //! The y position of the right event within the flood histogram
    float y1;
    //! The cartridge of the left event
    int8_t cartridge0;
    //! The cartridge of the right event
    int8_t cartridge1;
    //! The fin of the left event
    int8_t fin0;
    //! The fin of the right event
    int8_t fin1;
    //! The module of the left event
    int8_t module0;
    //! The module of the right event
    int8_t module1;
    //! The apd of the left event
    int8_t apd0;
    //! The apd of the right event
    int8_t apd1;
    //! The crystal of the left event
    int8_t crystal0;
    //! The crystal of the right event
    int8_t crystal1;
    //! DAQ Board of the left event
    int8_t daq0;
    //! DAQ Board of the right event
    int8_t daq1;
    //! Rena of the left event
    int8_t rena0;
    //! Rena of the right event
    int8_t rena1;
    /*!
     * The structure will be padded to 72 bytes by the compiler, due to the 8
     * byte alignment requirement for the int64_t (long) so flags were added as
     * space for storing information in the future that may be useful for
     * calibration or processing purposes.  Their current uses are:
     *     - 0: none
     *     - 1: none
     */
    int8_t flags[2];
};

static_assert(sizeof(EventCoinc) == 72, "Require 72 byte EventCoinc structure");

#endif // EVENTCOINC_H
