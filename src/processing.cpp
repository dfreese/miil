#include <miil/process/processing.h>
#include <miil/SystemConfiguration.h>
#include <cmath>

using namespace std;

namespace {
#define DEFAULT_NO_READ_ADC_VALUE 0
/*!
 * An array where adc values can be stored.  At a maximum, there can be 24 ADC
 * values that can be read off of the Rena per module, with 4 modules per rena.
 * An additional unit to the array is added to serve as the default value to be
 * referenced when the value is not read out from the rena.  The __thread
 * keyword is a GCC extension to make the value local to the thread, thus this
 * implementation is thread safe.
*/
__thread short adc_value_storage[24 * 4 + 1] = {DEFAULT_NO_READ_ADC_VALUE};
}

/*!
 * \brief Decode the raw data stream into its components
 *
 * Take a section of the raw byte stream that should start and stop with an 0x80
 * and 0x81 respectively and put it into a DaqPacket data structure to be
 * handled more easily.  Return error codes when the packet does not fit the
 * required protocol.
 *
 * \param packet_byte_stream The data stream from the ethernet port to be parsed
 * \param packet_info Where the data stream information is returned
 *
 * \return 0 if successful, less than zero otherwise
 *         -1 Empty Bytestream
 *         -2 Incorrect start byte
 *         -3 Empty trigger code (no modules triggered)
 *         -4 Incorrect packet size
 *         -5 Invalid Address Byte
 */
int DecodePacketByteStream(
        const deque<char>::iterator begin,
        const deque<char>::iterator end,
        SystemConfiguration const * const system_config,
        std::vector<EventRaw> & events)
{
    // Protect from errors later on
    if (distance(begin,end) < 3) {
        return(-1);
    }

    // first byte of packet needs to be 0x80 and end 0x81
    if ((*begin != (char) 0x80) && (*(end-1) != (char) 0x81)) {
        return(-2);
    }

    int backend_address = ((*(begin + 1) & 0x7C) >> 2);
    int daq_board = ((*(begin + 1) & 0x03) >> 0);
    int fpga = ((*(begin + 2) & 0x30) >> 4);
    int rena = (2 * fpga) + ((*(begin + 2) & 0x40) >> 6);

    int trigCode = ((*(begin + 2) & 0x0F) >> 0);

    // The packet doesn't show any modules having triggered so return error
    if (trigCode == 0) {
        return(-3);
    }

    int panel = 0;
    int cartridge = 0;
    if (system_config->lookupPanelCartridge(
            backend_address, panel, cartridge) < 0)
    {
        return(-5);
    }

    int expected_packet_size =
            system_config->packet_size[panel][cartridge][daq_board]
                                      [rena]
                                      [trigCode];

    // Make sure the packet is the size expected from its header information
    if (distance(begin, end) != expected_packet_size) {
        return(-4);
    }

    // Generate the timestamp from the next 6 bytes
    long timestamp = 0;
    for (int ii = 3; ii < 9; ii++) {
        timestamp = (timestamp << 7);
        timestamp += long(*(begin + ii) & 0x7F);
    }

    // Remaining bytes are ADC data for each channel
    int store_idx(0);
    for (int ii = 9; ii < (expected_packet_size - 1); ii += 2) {
        short value(((*(begin + ii) & 0x3F) << 6) +
                     (*(begin + ii + 1) & 0x3F));
        adc_value_storage[store_idx++] = value;
    }

    ADCValueLocation const * const adc_locations =
            system_config->adc_value_locations[panel][cartridge]
                    [daq_board]
                    [rena]
                    [trigCode].data();
    for (int ii = 0; ii < system_config->modules_per_rena; ii++) {
        if (adc_locations[ii].triggered) {
            EventRaw event;
            event.ct = timestamp;
            event.panel = panel;
            event.cartridge = cartridge;
            event.daq = daq_board;
            event.rena = rena;
            event.module = ii;
            event.a = adc_value_storage[adc_locations[ii].a];
            event.b = adc_value_storage[adc_locations[ii].b];
            event.c = adc_value_storage[adc_locations[ii].c];
            event.d = adc_value_storage[adc_locations[ii].d];
            event.u0 = adc_value_storage[adc_locations[ii].u0];
            event.u1 = adc_value_storage[adc_locations[ii].u1];
            event.u0h = adc_value_storage[adc_locations[ii].u0h];
            event.u1h = adc_value_storage[adc_locations[ii].u1h];
            event.v0 = adc_value_storage[adc_locations[ii].v0];
            event.v1 = adc_value_storage[adc_locations[ii].v1];
            event.v0h = adc_value_storage[adc_locations[ii].v0h];
            event.v1h = adc_value_storage[adc_locations[ii].v1h];
            event.com0 = adc_value_storage[adc_locations[ii].com0];
            event.com1 = adc_value_storage[adc_locations[ii].com1];
            event.com0h = adc_value_storage[adc_locations[ii].com0h];
            event.com1h = adc_value_storage[adc_locations[ii].com1h];
            events.push_back(event);
        }
    }

    return(0);
}

/*!
 * \brief Calculate the fine timestampe from the UV Circle
 *
 * Calculates the fine timestamp from the uv circle by subtracting the circle
 * centers, and then taking the arc tangent.  The value is then normalized on a
 * scale of [0, uv_period_ns).
 *
 * \param u The u value of the timing signal
 * \param v The v value of the timing signal
 * \param u_cent The u coordinate of the circle center
 * \param v_cent The v coordinate of the circle center
 * \param uv_period_ns The period of the uv signal representing a full circle
 *
 * \return The fine timestamp on a scale of [0, uv_period_ns)
 */
float FineCalc(short u, short v, float u_cent, float v_cent, float uv_period_ns)
{
    float tmp = std::atan2((float) u - u_cent, (float) v - v_cent);
    if (tmp < 0.0) {
        tmp += 2 * M_PI;
    }
    tmp /= 2 * M_PI;
    tmp *= uv_period_ns;
    return(tmp);
}

/*!
 * \brief Assign crystal using nearest neighbor
 *
 * Takes an event and assigns it to a crystal by finding the crystal peak that
 * is closest to the crystal.  A distance calculation is made for each crystal.
 *
 * \param x The x anger logic position of the event
 * \param y The x anger logic position of the event
 * \param apd_cals Pointer to array of CrystalCalibration structs holding the
 *        crystal locations
 *
 * \return The id of the closest crystal on success.
 *         - -1 if the crystal couldn't be identified correctly
 *         - -2 if the event is out of bounds of anger logic.
 */
int GetCrystalID(
        float x,
        float y,
        const std::vector<CrystalCalibration> & apd_cals)
{
    double min(__DBL_MAX__);
    int crystal_id(-1);

    if ((std::abs(x) > 1) || (std::abs(y) > 1)) {
        return(-2);
    }
    for (int crystal = 0; crystal < 64; crystal++) {
        double dist = std::pow(apd_cals[crystal].x_loc - x, 2) +
                      std::pow(apd_cals[crystal].y_loc - y, 2);
        if (dist < min) {
            crystal_id = crystal;
            min = dist;
        }
    }
    return(crystal_id);
}


/*!
 * \brief Calculates the x, y, and energy for an event
 *
 * For use when a full calibration is not necessary, and only some pedestal
 * corrected processing is needed.
 *
 * Places the common channel energy into the event.E
 *
 * \param event Where the calibrated event is returned
 * \param rawevent The non-pedestal corrected event decoded from the bitstream
 * \param system_config Pointer to the system configuration to be used
 * \param reject_threshold Boolean flag to reject events below threshold
 * \param reject_double bool flag to reject double trigger events
 *
 * \return
 *     -  0 on success
 *     - -1 if the event is below the hit threshold for the module
 *     - -2 if the other apd is above the double trigger threshold
 *     - -5 If the conversion from PCDRM to PCFM indexing fails
 */
int CalculateXYandEnergy(
        EventCal & event,
        const EventRaw & rawevent,
        SystemConfiguration const * const system_config,
        bool reject_threshold,
        bool reject_double)
{
    int module = 0;
    int fin = 0;
    if (system_config->convertPCDRMtoPCFM(rawevent.panel, rawevent.cartridge,
                                          rawevent.daq, rawevent.rena,
                                          rawevent.module, fin, module) < 0)
    {
      return(-5);
    }

    const ModulePedestals & module_pedestals =
            system_config->pedestals[rawevent.panel][rawevent.cartridge]
                     [rawevent.daq][rawevent.rena][rawevent.module];

    const ModuleChannelConfig & module_config =
            system_config->module_configs[rawevent.panel][rawevent.cartridge]
                                           [fin][module].channel_settings;


    // Assume APD 0, unless the signal is greater on the APD 1 common channels.
    // Greater in this case is less than, because the common signals go negative
    // i.e. start (zero) at roughly 3000 and max out at roughly 1000 or so.
    int apd = 0;
    short primary_common = rawevent.com0h - module_pedestals.com0h;
    short secondary_common = rawevent.com1h - module_pedestals.com1h;
    if (primary_common > secondary_common) {
        apd = 1;
        swap(primary_common, secondary_common);
    }
    if (reject_threshold & (primary_common > module_config.hit_threshold)) {
        return(-1);
    }
    if (reject_double &
            (secondary_common < module_config.double_trigger_threshold))
    {
        return(-2);
    }

    event.ct = rawevent.ct;

    float a = (float) rawevent.a - module_pedestals.a;
    float b = (float) rawevent.b - module_pedestals.b;
    float c = (float) rawevent.c - module_pedestals.c;
    float d = (float) rawevent.d - module_pedestals.d;


    event.spat_total = a + b + c + d;
    event.x = (c + d - (b + a)) / (event.spat_total);
    event.y = (a + d - (b + c)) / (event.spat_total);

    if (apd == 0) {
        event.E = module_pedestals.com0 - rawevent.com0;
    } else if (apd == 1) {
        event.E = module_pedestals.com1 - rawevent.com1;
        event.y *= -1;
    }

    event.panel = rawevent.panel;
    event.cartridge = rawevent.cartridge;
    event.fin = fin;
    event.module = module;
    event.apd = apd;
    event.daq = rawevent.daq;
    event.rena = rawevent.rena;

    return(0);
}

/*!
 * \brief Calculates the x, y, and energy for an event
 *
 * For use when a full calibration is not necessary, and only some pedestal
 * corrected processing is needed.
 *
 * \param rawevent The non-pedestal corrected event decoded from the bitstream
 * \param system_config Pointer to the system configuration to be used
 * \param x where the x flood position is returned
 * \param y where the y flood position is returned
 * \param energy where the energy (sum of spatials) is returned
 * \param apd where the apd is returned
 * \param module where the module local to the fin is stored
 * \param fin where the fin is stored
 *
 * \return
 *     -  0 on success
 *     - -1 if the event is below the hit threshold for the module
 *     - -2 if the other apd is above the double trigger threshold
 *     - -5 If the conversion from PCDRM to PCFM indexing fails
 */
int CalculateXYandEnergy(
        const EventRaw & rawevent,
        SystemConfiguration const * const system_config,
        float & x,
        float & y,
        float & energy,
        int & apd,
        int & module,
        int & fin,
        bool reject_threshold,
        bool reject_double)
{
    EventCal event;
    int status = CalculateXYandEnergy(
                event, rawevent, system_config,
                reject_threshold, reject_double);

    x = event.x;
    y = event.y;
    energy = event.spat_total;
    apd = event.apd;
    module = event.module;
    fin = event.fin;

    return(status);
}

/*!
 * Overload of CalculateXYandEnergy where module and fin values are ignored.
 */
int CalculateXYandEnergy(
        const EventRaw & rawevent,
        SystemConfiguration const * const system_config,
        float & x,
        float & y,
        float & energy,
        int & apd,
        bool reject_threshold,
        bool reject_double)
{
    int module, fin;
    return(CalculateXYandEnergy(
               rawevent, system_config, x, y, energy, apd, module, fin,
               reject_threshold, reject_double));
}

/*!
 * Overload of CalculateXYandEnergy where apd, module, and fin values are
 * ignored.
 */
int CalculateXYandEnergy(
        const EventRaw & rawevent,
        SystemConfiguration const * const system_config,
        float & x,
        float & y,
        float & energy,
        bool reject_threshold,
        bool reject_double)
{
    int apd, module, fin;
    return(CalculateXYandEnergy(
               rawevent, system_config, x, y, energy, apd, module, fin,
               reject_threshold, reject_double));
}

/*!
 * \brief Converts a Rena event into a calibrated Event using the system config
 *
 * Places the common channel energy into the event.E
 *
 * \param event Where the calibrated event is returned
 * \param rawevent The non-pedestal corrected event decoded from the bitstream
 * \param system_config Pointer to the system configuration to be used
 *
 * \return
 *     -  0 on success
 *     - -1 if the event is below the hit threshold for the module
 *     - -2 if the other apd is above the double trigger threshold
 *     - -3 if the crystal could not be correctly identified
 *     - -4 if the identified crystal has been marked as invalid
 *     - -5 If the conversion from PCDRM to PCFM indexing fails
 */
int CalculateID(
        EventCal & event,
        const EventRaw & rawevent,
        SystemConfiguration const * const system_config)
{
    int status = CalculateXYandEnergy(event, rawevent, system_config);
    if (status < 0) {
        return(status);
    }

    const std::vector<CrystalCalibration> & apd_cals =
            system_config->calibration[event.panel][rawevent.cartridge]
                                      [event.fin][event.module][event.apd];

    int crystal = GetCrystalID(event.x, event.y, apd_cals);

    if (crystal < 0) {
        return(-3);
    }

    const CrystalCalibration & crystal_cal = apd_cals[crystal];

    if (!crystal_cal.use) {
        return(-4);
    }
    event.crystal = crystal;

    return(0);
}

/*!
 * \brief Converts a Rena event into a calibrated Event using the system config
 *
 *
 * \param rawevent The non-pedestal corrected event decoded from the bitstream
 * \param event Where the calibrated event is returned
 * \param system_config Pointer to the system configuration to be used
 *
 * \return
 *     -  0 on success
 *     - -1 if the event is below the hit threshold for the module
 *     - -2 if the other apd is above the double trigger threshold
 *     - -3 if the crystal could not be correctly identified
 *     - -4 if the identified crystal has been marked as invalid
 *     - -5 If the conversion from PCDRM to PCFM indexing fails
 */
int RawEventToEventCal(
        const EventRaw & rawevent,
        EventCal & event,
        SystemConfiguration const * const system_config)
{
    int module = 0;
    int fin = 0;
    if (system_config->convertPCDRMtoPCFM(rawevent.panel, rawevent.cartridge,
                                          rawevent.daq, rawevent.rena,
                                          rawevent.module, fin, module) < 0)
    {
      return(-5);
    }

    const ModulePedestals & module_pedestals =
            system_config->pedestals[rawevent.panel][rawevent.cartridge]
                     [rawevent.daq][rawevent.rena][rawevent.module];

    const ModuleChannelConfig & module_config =
            system_config->module_configs[rawevent.panel][rawevent.cartridge]
                                           [fin][module].channel_settings;


    // Assume APD 0, unless the signal is greater on the APD 1 common channels.
    // Greater in this case is less than, because the common signals go negative
    // i.e. start (zero) at roughly 3000 and max out at roughly 1000 or so.
    int apd = 0;
    short primary_common = rawevent.com0h - module_pedestals.com0h;
    short secondary_common = rawevent.com1h - module_pedestals.com1h;
    if (primary_common > secondary_common) {
        apd = 1;
        swap(primary_common, secondary_common);
    }
    if (primary_common > module_config.hit_threshold) {
        return(-1);
    }
    if (secondary_common < module_config.double_trigger_threshold) {
        return(-2);
    }

    event.ct = rawevent.ct;

    float a = (float) rawevent.a - module_pedestals.a;
    float b = (float) rawevent.b - module_pedestals.b;
    float c = (float) rawevent.c - module_pedestals.c;
    float d = (float) rawevent.d - module_pedestals.d;


    event.spat_total = a + b + c + d;
    event.x = (c + d - (b + a)) / (event.spat_total);
    event.y = (a + d - (b + c)) / (event.spat_total);
    if (apd == 1) {
        event.y *= -1;
        event.ft = FineCalc(rawevent.u1h,
                            rawevent.v1h,
                            module_pedestals.u1h,
                            module_pedestals.v1h,
                            system_config->uv_period_ns);
    } else {
        event.ft = FineCalc(rawevent.u0h,
                            rawevent.v0h,
                            module_pedestals.u0h,
                            module_pedestals.v0h,
                            system_config->uv_period_ns);
    }

    const std::vector<CrystalCalibration> & apd_cals =
            system_config->calibration[rawevent.panel][rawevent.cartridge]
                                      [fin][module][apd];

    int crystal = GetCrystalID(event.x, event.y, apd_cals);

    if (crystal < 0) {
        return(-3);
    }

    const CrystalCalibration & crystal_cal = apd_cals[crystal];

    if (!crystal_cal.use) {
        return(-4);
    }

    event.panel = rawevent.panel;
    event.cartridge = rawevent.cartridge;
    event.fin = fin;
    event.module = module;
    event.apd = apd;
    event.crystal = crystal;
    event.daq = rawevent.daq;
    event.rena = rawevent.rena;

    event.E = event.spat_total / crystal_cal.gain_spat * 511;

    // Changed the convention from the cal_offset programs, so that we subtract
    // from both panels, rather than adding to the right hand side.
    event.ft -= crystal_cal.time_offset;
    event.ft -= (event.E - 511.0) * crystal_cal.time_offset_edep;
    // Ensure that the fine timestamp is wrapped correctly
    while (event.ft < 0) {
        event.ft += system_config->uv_period_ns;
    }
    while (event.ft >= system_config->uv_period_ns) {
        event.ft -= system_config->uv_period_ns;
    }

    return(0);
}

/*!
 * \brief Checks if an event is in a given energy window
 *
 * Checks if an event is in a given energy window, inclusive of the limits.
 *
 * \param event The event to be evaluated
 * \param low The low energy threshold
 * \param high The high energy threshold
 *
 * \return True if in the window, false otherwise
 */
bool InEnergyWindow(const EventCal & event, float low, float high) {
    if ((event.E < low) || (event.E > high)) {
        return(false);
    } else {
        return(true);
    }
}

/*!
* \brief Calculate the time between two calibrated events
*
* Calculate the time difference between two calibrated events.  The ct_period_ns
* is used to calculate how many full uv periods have transpired between the
* events.  This is multiplied by uv_period_ns, and added to the difference of
* the two fine timestamps.  The difference between the fine timestamps is
* wrapped to (-uv_period_ns, uv_period_ns), to make safe the assumption that the
* fine timestamps are compared along the same uv circle.
*
* \param arg1 The reference event that is subtracted from
* \param arg2 The event subtracted from the reference
* \param uv_period_ns The period of the uv circle in nanoseconds
* \param ct_period_ns The period in nanoseconds of each coarse timestamp tick
*
* \return The time difference in nanoseconds
*/
float EventCalTimeDiff(
        const EventCal & arg1,
        const EventCal & arg2,
        float uv_period_ns,
        float ct_period_ns)
{
    // Assume the fine timestamps should always be compared as if they are in
    // the same period, so wrap them to +/- this period after comparing.  This
    // safe guards against the off chance that one ft value is not on
    // [0, uv_period_ns).
    float difference = arg1.ft - arg2.ft;
    while (difference > uv_period_ns) {
        difference -= uv_period_ns;
    }
    while (difference < -uv_period_ns) {
        difference += uv_period_ns;
    }
    // Add in a number of uv periods that hasn't already been compared with
    // using the fine timestamps.
    difference += uv_period_ns *
            trunc((ct_period_ns * (arg1.ct - arg2.ct)) / uv_period_ns);
    return(difference);
}

/*!
 * \brief Calculate if time(event 1) < time(event 2)
 *
 * Effectively returns (EventCalTimeDiff(arg1, arg2) < 0)
 *
 * \param arg1 The reference event that is subtracted from
 * \param arg2 The event subtracted from the reference
 * \param uv_period_ns The period of the uv circle in nanoseconds
 * \param ct_period_ns The period in nanoseconds of each coarse timestamp tick
 *
 * \return bool indicating if time(event 1) < time(event 2)
 */
bool EventCalLessThan(
        const EventCal & arg1,
        const EventCal & arg2,
        float uv_period_ns,
        float ct_period_ns)
{
    float difference = EventCalTimeDiff(arg1, arg2, uv_period_ns, ct_period_ns);
    return(difference < 0);
}

/*!
 * \brief Calculate if time(event 1) < time(event 2)
 *
 * Effectively returns (EventCalLessThan(arg1, arg2) called with ct_only = true.
 * Useful for functions like std::lower_bound that accept a binary comparison
 * function.
 *
 * \param arg1 The reference event that is compared
 * \param arg2 The event compared against the reference
 *
 * \return bool indicating if time(event 1) < time(event 2)
 */
bool EventCalLessThanOnlyCt(const EventCal & arg1, const EventCal & arg2) {
    if (arg1.ct < arg2.ct) {
        return(true);
    } else {
        return(false);
    }
}

EventCoinc MakeCoinc(
        const EventCal & event_left,
        const EventCal & event_right,
        float uv_period_ns,
        float ct_period_n)
{
    EventCoinc event;
    event.ct0 = event_left.ct;
    event.dct = event_left.ct - event_right.ct;

    event.ft0 = event_left.ft;
    event.dtf = EventCalTimeDiff(event_left, event_right,
                                 uv_period_ns, ct_period_n);

    event.E0 = event_left.E;
    event.E1 = event_right.E;

    event.spat_total0 = event_left.spat_total;
    event.spat_total1 = event_right.spat_total;

    event.x0 = event_left.x;
    event.x1 = event_right.x;

    event.y0 = event_left.y;
    event.y1 = event_right.y;

    event.cartridge0 = event_left.cartridge;
    event.cartridge1 = event_right.cartridge;

    event.fin0 = event_left.fin;
    event.fin1 = event_right.fin;

    event.module0 = event_left.module;
    event.module1 = event_right.module;

    event.apd0 = event_left.apd;
    event.apd1 = event_right.apd;

    event.crystal0 = event_left.crystal;
    event.crystal1 = event_right.crystal;

    event.daq0 = event_left.daq;
    event.daq1 = event_right.daq;

    event.rena0 = event_left.rena;
    event.rena1 = event_right.rena;

    return(event);
}

int TimeCalCoincEvent(
        EventCoinc & event,
        SystemConfiguration const * const config)
{
    const CrystalCalibration & cal0 =
            config->calibration[0][event.cartridge0][event.fin0][event.module0]
                              [event.apd0][event.crystal0];

    const CrystalCalibration & cal1 =
            config->calibration[1][event.cartridge1][event.fin1][event.module1]
                              [event.apd1][event.crystal1];

    event.ft0 -= cal0.time_offset;
    event.dtf -= cal0.time_offset;
    event.dtf += cal1.time_offset;
    event.ft0 -= (event.E0 - 511.0) * cal0.time_offset_edep;
    event.dtf -= (event.E0 - 511.0) * cal0.time_offset_edep;
    event.dtf += (event.E1 - 511.0) * cal1.time_offset_edep;

    // Ensure that the fine timestamp is wrapped correctly
    while (event.ft0 < 0) {
        event.ft0 += config->uv_period_ns;
    }
    while (event.ft0 >= config->uv_period_ns) {
        event.ft0 -= config->uv_period_ns;
    }
    return(0);
}
