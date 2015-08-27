#include <miil/SystemConfiguration.h>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <cstring>
#include <cmath>
#include <jsoncpp/json/json.h>
#include <cassert>

// Anonymous Namespace for helper functions to keep them local to object file
namespace {
/*!
 * \brief Fill out lookup tables for backend board address bytes
 *
 * The configuration structure has three lookup tables that need to be populated
 * once the address of the backend boards are known.  Since the address byte is
 * only 5 bits, only 32 values are valid.
 *
 * \param config The system configuration to be used a reference
 * \param backend_address_valid an array of 32 bools showing if that backend
 *        address has been configured and can be looked up.
 * \param backend_address_panel_lookup an array of 32 integers that stores the
 *        panel number associated with the address.
 * \param backend_address_cartridge_lookup an array of 32 integers that stores
 *        the cartridge number associated with the address.
 *
 * \return 0 on success, less than otherwise.
 *         -1 on an out of range id value.
 */
int populateBackendAddressReverseLookup(
    SystemConfiguration const * const config,
    bool * backend_address_valid,
    int * backend_address_panel_lookup,
    int * backend_address_cartridge_lookup)
{
    for (int p = 0; p < config->panels_per_system; p++) {
        for (int c = 0; c < config->cartridges_per_panel; c++) {
            int backend_address = config->
                    cartridge_configs[p][c].backend_board_config.daqboard_id;
            if (backend_address < 0 || backend_address >= 32) {
                return(-1);
            }
            backend_address_valid[backend_address] = true;
            backend_address_panel_lookup[backend_address] = p;
            backend_address_cartridge_lookup[backend_address] = c;
        }
    }
    return(0);
}

/*!
 * \brief Walk over all spatial channels to determine their ADC location
 *
 * The packets sent by the system contain ADC data for all modules that trigger
 * at the same time.  The packet contains the ADC data in order of the rena
 * channel.  The order of the channels for the spatials is A, B, C, D on even
 * renas, and D, C, B, A on odd renas. The module order does not change (0, 1,
 * 2, 3).  Example, on rena 1, the order of the channels would be D0, C0, B0,
 * A0, D1, C1, etc.  For each channel an energy channel and timing channels can
 * be read out.  These are refered to as slow_hit and fast_hit readouts
 * respectively.  If the slow_hit is set to be read out, the value of that
 * channel is read out.  If the fast_hit is set to be read out, the u and v
 * values from the timing circle are read out.  For each channel the order is
 * value, u, v.  The channel value's location within the packet is calculated by
 * walking through all of the potential channels in the order expected and
 * incrementing a counter if that channel is set to be read out.  The location
 * to start with, current_value, is passed by reference because the common
 * channels can be walked afterwards starting with the ending value here.
 *
 * This function also sets whether or not the module triggered in the
 * ADCValueLocation structure using the trigger code.
 *
 * \param config Pointer to the system config used for reference
 * \param p The panel currently being evaluated
 * \param c The cartridge currently being evaluated
 * \param d The daq board currently being evaluated
 * \param r The rena currently being evaluated
 * \param t The trigger code currently being evaluated
 * \param current_value The adc value readout location to start at, which is
 *        passed back to the calling function if common channels need to be
 *        walked.
 * \param adc_value_locations A PCDRTM array that stores the calculated location
 *        that an adc value for a particular channel will be stored at given the
 *        specific module, it's settings, and the trigger code.
 */
void walkModulesSpatials(
    SystemConfiguration const * const config,
    int p, int c, int d, int r, int t,
    int & current_value,
    std::vector<std::vector<std::vector<
            std::vector<std::vector<std::vector<
            ADCValueLocation> > > > > > & adc_value_locations)
{
    for (int m = 0; m < config->modules_per_rena; m++) {
        ADCValueLocation * loc =
                &adc_value_locations[p][c][d][r][t][m];
        int f = 0;
        int module = 0;
        config->convertPCDRMtoPCFM(p, c, d, r, m, f, module);
        const ModuleChannelConfig settings =
                config->module_configs[p][c][f][module].channel_settings;
        if (0x01 & (t >> m)) {
            loc->triggered = true;
            if (r % 2) {
                // If the rena is odd, read out D, C, B, A
                if (settings.spatD.slow_hit_readout) {
                    loc->d = current_value++;
                }
                if (settings.spatD.fast_hit_readout) {
                    loc->d_u = current_value++;
                    loc->d_v = current_value++;
                }
                if (settings.spatC.slow_hit_readout) {
                    loc->c = current_value++;
                }
                if (settings.spatC.fast_hit_readout) {
                    loc->c_u = current_value++;
                    loc->c_v = current_value++;
                }
                if (settings.spatB.slow_hit_readout) {
                    loc->b = current_value++;
                }
                if (settings.spatB.fast_hit_readout) {
                    loc->b_u = current_value++;
                    loc->b_v = current_value++;
                }
                if (settings.spatA.slow_hit_readout) {
                    loc->a = current_value++;
                }
                if (settings.spatA.fast_hit_readout) {
                    loc->a_u = current_value++;
                    loc->a_v = current_value++;
                }
            } else {
                // If the rena is even, read out A, B, C, D
                if (settings.spatA.slow_hit_readout) {
                    loc->a = current_value++;
                }
                if (settings.spatA.fast_hit_readout) {
                    loc->a_u = current_value++;
                    loc->a_v = current_value++;
                }
                if (settings.spatB.slow_hit_readout) {
                    loc->b = current_value++;
                }
                if (settings.spatB.fast_hit_readout) {
                    loc->b_u = current_value++;
                    loc->b_v = current_value++;
                }
                if (settings.spatC.slow_hit_readout) {
                    loc->c = current_value++;
                }
                if (settings.spatC.fast_hit_readout) {
                    loc->c_u = current_value++;
                    loc->c_v = current_value++;
                }
                if (settings.spatD.slow_hit_readout) {
                    loc->d = current_value++;
                }
                if (settings.spatD.fast_hit_readout) {
                    loc->d_u = current_value++;
                    loc->d_v = current_value++;
                }
            }
        }
    }
}

/*!
 * \brief Walk over all common channels to determine their ADC location
 *
 * The packets sent by the system contain ADC data for all modules that trigger
 * at the same time.  The packet contains the ADC data in order of the rena
 * channel.  The order of the channels for the common channels is H0, L0, H1, L1
 * for PSAPD 0 (front) and PSAPD 1 (back).  The modules are read in order from 0
 * to 4.  For each channel an energy channel and timing channels can be read
 * out.  These are refered to as slow_hit and fast_hit readouts respectively.
 * If the slow_hit is set to be read out, the value of that channel is read out.
 * If the fast_hit is set to be read out, the u and v values from the timing
 * circle for that channel are read out.  For each channel the order is
 * value, u, v.  The channel value's location within the packet is calculated by
 * walking through all of the potential channels in the order expected and
 * incrementing a counter if that channel is set to be read out.  The location
 * to start with, current_value, is passed by reference because the spatial
 * channels can be walked afterwards starting with the ending value here.
 *
 * This function also sets whether or not the module triggered in the
 * ADCValueLocation structure using the trigger code.
 *
 * \param config Pointer to the system config used for reference
 * \param p The panel currently being evaluated
 * \param c The cartridge currently being evaluated
 * \param d The daq board currently being evaluated
 * \param r The rena currently being evaluated
 * \param t The trigger code currently being evaluated
 * \param current_value The adc value readout location to start at, which is
 *        passed back to the calling function if common channels need to be
 *        walked.
 * \param adc_value_locations A PCDRTM array that stores the calculated location
 *        that an adc value for a particular channel will be stored at given the
 *        specific module, it's settings, and the trigger code.
 */
void walkModulesCommons(
    SystemConfiguration const * const config,
    int p, int c, int d, int r, int t,
    int & current_value,
    std::vector<std::vector<std::vector<
            std::vector<std::vector<std::vector<
            ADCValueLocation> > > > > > & adc_value_locations)
{
    for (int m = 0; m < config->modules_per_rena; m++) {
        ADCValueLocation * loc =
                &adc_value_locations[p][c][d][r][t][m];
        int f = 0;
        int module = 0;
        config->convertPCDRMtoPCFM(p, c, d, r, m, f, module);
        ModuleChannelConfig settings =
                config->module_configs[p][c][f][module].channel_settings;
        if (0x01 & (t >> m)) {
            loc->triggered = true;
            if (settings.comH.slow_hit_readout) {
                loc->com1h = current_value++;
            }
            if (settings.comH.fast_hit_readout) {
                loc->u1h = current_value++;
                loc->v1h = current_value++;
            }
            if (settings.comL.slow_hit_readout) {
                loc->com1 = current_value++;
            }
            if (settings.comL.fast_hit_readout) {
                loc->u1 = current_value++;
                loc->v1 = current_value++;
            }
            if (settings.comH.slow_hit_readout) {
                loc->com2h = current_value++;
            }
            if (settings.comH.fast_hit_readout) {
                loc->u2h = current_value++;
                loc->v2h = current_value++;
            }
            if (settings.comL.slow_hit_readout) {
                loc->com2 = current_value++;
            }
            if (settings.comL.fast_hit_readout) {
                loc->u2 = current_value++;
                loc->v2 = current_value++;
            }
        }
    }
}

/*!
 * \brief Generate a lookup table for a channel's ADC value in a DAQ packet.
 *
 * The packets sent by the system contain ADC data for all modules that trigger
 * at the same time.  The packet contains the ADC data in order of the rena
 * channel.  To efficiently parse packets and get the ADC data assigned to the
 * correct channel in software, this location is precalculated based on the
 * specific rena's settings, and the trigger code within that packet.  For each
 * rena, the value, u, and v are read out (if specified) for the commons (0H,
 * 0L, 1H, 1L) and then the spatials (A, B, C, D).  For odd modules, the spatial
 * and common channels are swaped.  The order of the spatial channels is also
 * reversed (D, C, B, A).
 *
 * This function also sets whether or not the module triggered in the
 * ADCValueLocation structure using the trigger code.
 *
 * \param config Pointer to the system config used for reference
 * \param adc_value_locations A PCDRTM array that stores the calculated location
 *        that an adc value for a particular channel will be stored at given the
 *        specific module, it's settings, and the trigger code.
 *
 * \return 0 if no error, less than otherwise.
 */
int populateADCLocationLookup(
    SystemConfiguration const * const config,
    std::vector<std::vector<std::vector<
            std::vector<std::vector<std::vector<
            ADCValueLocation> > > > > > & adc_value_locations)
{
    adc_value_locations.resize(config->panels_per_system);
    for (int p = 0; p < config->panels_per_system; p++) {
        adc_value_locations[p].resize(config->cartridges_per_panel);
        for (int c = 0; c < config->cartridges_per_panel; c++) {
            adc_value_locations[p][c].resize(config->daqs_per_cartridge);
            for (int d = 0; d < config->daqs_per_cartridge; d++) {
                adc_value_locations[p][c][d].resize(config->renas_per_daq);
                for (int r = 0; r < config->renas_per_daq; r++) {
                    // There are four modules, so there are 2^4 = 16
                    // combinations, of which 0 shouldn't be used anyways
                    adc_value_locations[p][c][d][r].resize(16);
                    for (int t = 0; t < 16; t++) {
                        adc_value_locations[p][c][d][r][t].resize(
                                config->renas_per_daq);
                        int current_value = 0;
                        if (r % 2) {
                            // On odd channels, read out the spatials first.
                            // walkModulesSpatials handles the spatial order
                            // flip
                            walkModulesSpatials(config, p, c, d, r, t,
                                                current_value,
                                                adc_value_locations);
                            walkModulesCommons(config, p, c, d, r, t,
                                               current_value,
                                               adc_value_locations);
                        } else {
                            // On odd channels, read out the commons first.
                            // walkModulesSpatials handles the spatial order
                            // flip
                            walkModulesCommons(config, p, c, d, r, t,
                                               current_value,
                                               adc_value_locations);
                            walkModulesSpatials(config, p, c, d, r, t,
                                                current_value,
                                                adc_value_locations);
                        }
                    }
                }
            }
        }
    }
    return(0);
}

/*!
 * \brief Generate a lookup table for a channel's ADC value in a DAQ packet.
 *
 * Before the ADC values contained within a packet can be sorted to their
 * respective channels, the packet size needs to be verified against the system
 * configuration.  To do this efficiently, the size is calculated beforehand.
 * A packet has a header of 10 bytes.  Each 12 bit ADC value is split over two
 * bytes.  Reading out timing information causes two channels to be read out.
 * Enabling readout for a common channel enables it for both APD 0 and APD 1,
 * doubling the size that the channel value and timing values add.
 *
 * \param config Pointer to the system config used for reference
 * \param packet_size A PCDRT array that stores the calculated packet size for a
 *        specific rena, it's settings and the trigger code.
 *
 * \return 0 if no error, less than otherwise.
 */
int populatePacketSizeLookup(
    SystemConfiguration const * const config,
    std::vector<std::vector<std::vector<
            std::vector<std::vector<int> > > > > & packet_size)
{
    packet_size.resize(config->panels_per_system);
    for (int p = 0; p < config->panels_per_system; p++) {
        packet_size[p].resize(config->cartridges_per_panel);
        for (int c = 0; c < config->cartridges_per_panel; c++) {
            packet_size[p][c].resize(config->daqs_per_cartridge);
            for (int d = 0; d < config->daqs_per_cartridge; d++) {
                packet_size[p][c][d].resize(config->renas_per_daq);
                for (int r = 0; r < config->renas_per_daq; r++) {
                    // There are four modules, so there are 2^4 = 16
                    // combinations, of which 0 shouldn't be used anyways
                    // The header for the packet is 10, so default to that
                    packet_size[p][c][d][r].resize(16, 10);
                    for (int t = 0; t < 16; t++) {
                        for (int m = 0; m < config->modules_per_rena; m++) {
                            // If the module would have triggered, add in it's
                            // channels that would be read out.
                            if (t & (0x0001 << m)) {
                                int fin = 0;
                                int module = 0;
                                config->convertPCDRMtoPCFM(p, c, d, r, m,
                                                           fin, module);
                                ModuleChannelConfig module_channel_settings =
                                        config->module_configs[p][c]
                                                [fin][module].channel_settings;
                                RenaChannelConfig comH =
                                        module_channel_settings.comH;
                                RenaChannelConfig comL =
                                        module_channel_settings.comL;
                                RenaChannelConfig spatA =
                                        module_channel_settings.spatA;
                                RenaChannelConfig spatB =
                                        module_channel_settings.spatB;
                                RenaChannelConfig spatC =
                                        module_channel_settings.spatC;
                                RenaChannelConfig spatD =
                                        module_channel_settings.spatD;
                                // Each channel that is readout from the Rena is
                                // 2 bytes.  The slow hit readout enables one
                                // channel to be read out. Fast hit readout
                                // enables reading out the two timing channels,
                                // u and v.  The common channel readout enable
                                // flag enables both apd 0 and 1, doubling the
                                // number of channels.
                                if (comH.fast_hit_readout) {
                                    packet_size[p][c][d][r][t] += 8;
                                }
                                if (comH.slow_hit_readout) {
                                    packet_size[p][c][d][r][t] += 4;
                                }
                                if (comL.fast_hit_readout) {
                                    packet_size[p][c][d][r][t] += 8;
                                }
                                if (comL.slow_hit_readout) {
                                    packet_size[p][c][d][r][t] += 4;
                                }
                                if (spatA.fast_hit_readout) {
                                    packet_size[p][c][d][r][t] += 4;
                                }
                                if (spatA.slow_hit_readout) {
                                    packet_size[p][c][d][r][t] += 2;
                                }
                                if (spatB.fast_hit_readout) {
                                    packet_size[p][c][d][r][t] += 4;
                                }
                                if (spatB.slow_hit_readout) {
                                    packet_size[p][c][d][r][t] += 2;
                                }
                                if (spatC.fast_hit_readout) {
                                    packet_size[p][c][d][r][t] += 4;
                                }
                                if (spatC.slow_hit_readout) {
                                    packet_size[p][c][d][r][t] += 2;
                                }
                                if (spatD.fast_hit_readout) {
                                    packet_size[p][c][d][r][t] += 4;
                                }
                                if (spatD.slow_hit_readout) {
                                    packet_size[p][c][d][r][t] += 2;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return(0);
}

/*!
 * \brief Resize a PC array to the proper size
 *
 * For arrays indexed Panel, Cartridge.
 *
 * \param config The system configuration to be used as reference
 * \param vect The vector to be resized.
 */
template<class T>
void resizePCArray(
        SystemConfiguration const * const config,
        std::vector<std::vector<T> > & vect)
{
    vect.resize(config->panels_per_system);
    for (int p = 0; p < config->panels_per_system; p++) {
        vect[p].resize(config->cartridges_per_panel);
    }
}

/*!
 * \brief Resize a PCF array to the proper size
 *
 * For arrays indexed Panel, Cartridge, Fin.
 *
 * \param config The system configuration to be used as reference
 * \param vect The vector to be resized.
 */
template<class T>
void resizePCFArray(
        SystemConfiguration const * const config,
        std::vector<std::vector<std::vector<T> > > & vect)
{
    vect.resize(config->panels_per_system);
    for (int p = 0; p < config->panels_per_system; p++) {
        vect[p].resize(config->cartridges_per_panel);
        for (int c = 0; c < config->cartridges_per_panel; c++) {
            vect[p][c].resize(config->fins_per_cartridge);
        }
    }
}

/*!
 * \brief Resize a PCFM array to the proper size
 *
 * For arrays indexed Panel, Cartridge, Fin, Module.
 *
 * \param config The system configuration to be used as reference
 * \param vect The vector to be resized.
 */
template<class T>
void resizePCFMArray(
        SystemConfiguration const * const config,
        std::vector<std::vector<std::vector<std::vector<T> > > > & vect)
{
    vect.resize(config->panels_per_system);
    for (int p = 0; p < config->panels_per_system; p++) {
        vect[p].resize(config->cartridges_per_panel);
        for (int c = 0; c < config->cartridges_per_panel; c++) {
            vect[p][c].resize(config->fins_per_cartridge);
            for (int f = 0; f < config->fins_per_cartridge; f++) {
                vect[p][c][f].resize(config->modules_per_fin);
            }
        }
    }
}

/*!
 * \brief Resize a PCFMAX array to the proper size
 *
 * For arrays indexed Panel, Cartridge, Fin, Module, APD, Crystal.
 *
 * \param config The system configuration to be used as reference
 * \param vect The vector to be resized.
 */
template<class T>
void resizePCFMAXArray(
        SystemConfiguration const * const config,
        std::vector<std::vector<std::vector<std::vector<
                std::vector<std::vector<T> > > > > > & vect)
{
    vect.resize(config->panels_per_system);
    for (int p = 0; p < config->panels_per_system; p++) {
        vect[p].resize(config->cartridges_per_panel);
        for (int c = 0; c < config->cartridges_per_panel; c++) {
            vect[p][c].resize(config->fins_per_cartridge);
            for (int f = 0; f < config->fins_per_cartridge; f++) {
                vect[p][c][f].resize(config->modules_per_fin);
                for (int m = 0; m < config->modules_per_fin; m++) {
                    vect[p][c][f][m].resize(config->apds_per_module);
                    for (int a = 0; a < config->apds_per_module; a++) {
                        vect[p][c][f][m][a].resize(config->crystals_per_apd);
                    }
                }
            }
        }
    }
}

/*!
 * \brief Resize a PCDRM array to the proper size
 *
 * For arrays indexed Panel, Cartridge, DAQ Board, Rena, Module.
 *
 * \param config The system configuration to be used as reference
 * \param vect The vector to be resized.
 */
template<class T>
void resizePCDRMArray(
        SystemConfiguration const * const config,
        std::vector<std::vector<std::vector<
                std::vector<std::vector<T> > > > > & vect)
{
    vect.resize(config->panels_per_system);
    for (int p = 0; p < config->panels_per_system; p++) {
        vect[p].resize(config->cartridges_per_panel);
        for (int c = 0; c < config->cartridges_per_panel; c++) {
            vect[p][c].resize(config->daqs_per_cartridge);
            for (int d = 0; d < config->daqs_per_cartridge; d++) {
                vect[p][c][d].resize(config->renas_per_daq);
                for (int r = 0; r < config->renas_per_daq; r++) {
                    vect[p][c][d][r].resize(config->modules_per_rena);
                }
            }
        }
    }
}

/*!
 * \brief Resize a PCDRM array to the proper size
 *
 * For arrays indexed Panel, Cartridge, DAQ_Board, Rena, Channel.
 *
 * \param config The system configuration to be used as reference
 * \param vect The vector to be resized.
 */
template<class T>
void resizePCDRCArray(
        SystemConfiguration const * const config,
        std::vector<std::vector<std::vector<
                std::vector<std::vector<T> > > > > & vect)
{
    vect.resize(config->panels_per_system);
    for (int p = 0; p < config->panels_per_system; p++) {
        vect[p].resize(config->cartridges_per_panel);
        for (int c = 0; c < config->cartridges_per_panel; c++) {
            vect[p][c].resize(config->daqs_per_cartridge);
            for (int d = 0; d < config->daqs_per_cartridge; d++) {
                vect[p][c][d].resize(config->renas_per_daq);
                for (int r = 0; r < config->renas_per_daq; r++) {
                    vect[p][c][d][r].resize(config->channels_per_rena);
                }
            }
        }
    }
}

/*!
 * \brief Resize a PCDRM array to the proper size
 *
 * For arrays indexed Panel, Cartridge, DAQ_Board, Rena, Channel.
 *
 * \param config The system configuration to be used as reference
 * \param vect The vector to be resized.
 */
template<class T>
void resizePCDRCArray(
        SystemConfiguration const * const config,
        std::vector<std::vector<std::vector<
                std::vector<std::vector<T> > > > > & vect,
        T default_value)
{
    vect.resize(config->panels_per_system);
    for (int p = 0; p < config->panels_per_system; p++) {
        vect[p].resize(config->cartridges_per_panel);
        for (int c = 0; c < config->cartridges_per_panel; c++) {
            vect[p][c].resize(config->daqs_per_cartridge);
            for (int d = 0; d < config->daqs_per_cartridge; d++) {
                vect[p][c][d].resize(config->renas_per_daq);
                for (int r = 0; r < config->renas_per_daq; r++) {
                    vect[p][c][d][r].resize(config->channels_per_rena,
                                            default_value);
                }
            }
        }
    }
}

/*!
 * \brief Load System Configuration Settings from Json Object
 *
 * Takes the root json system configuration object and searches for
 *     -"NUM_PANEL_PER_DEVICE"
 *     -"NUM_CART_PER_PANEL"
 *     -"NUM_DAQ_PER_CART",
 *     -"NUM_RENA_PER_DAQ"
 *     -"NUM_MODULE_PER_RENA"
 *     -"NUM_FIN_PER_CARTRIDGE",
 *     -"NUM_MODULE_PER_FIN"
 *     -"NUM_ETH_READOUTS_PER_PANEL"
 *     -"NUM_HV_FLOATING_BOARDS",
 *     -"NUM_SCMICRO_PER_CART"
 *     -"NUM_MODULE_PER_DAC"
 * and places their values inside the SystemConfiguration object.
 *
 * The function was not made public to keep the jsoncpp header local to the
 * object file.
 *
 * \param config The system configuration object to be modified
 * \param root The Json object to be searched
 *
 * \return 0 is successful, less than otherwise
 *        -1 if there is no "system_config" member in the json object
 *        -2 if not all of the required objects are found
 */
int loadSystemSize(
    SystemConfiguration * const config,
    const Json::Value & root)
{
    if (!root.isMember("system_config")) {
        return (-1);
    }
    config->panels_per_system =
            root["system_config"]["NUM_PANEL_PER_DEVICE"].asInt();
    config->cartridges_per_panel =
            root["system_config"]["NUM_CART_PER_PANEL"].asInt();
    config->daqs_per_cartridge =
            root["system_config"]["NUM_DAQ_PER_CART"].asInt();
    config->renas_per_daq =
            root["system_config"]["NUM_RENA_PER_DAQ"].asInt();
    config->modules_per_rena =
            root["system_config"]["NUM_MODULE_PER_RENA"].asInt();
    config->fins_per_cartridge =
            root["system_config"]["NUM_FIN_PER_CARTRIDGE"].asInt();
    config->modules_per_fin =
            root["system_config"]["NUM_MODULE_PER_FIN"].asInt();
    config->ethernets_per_panel =
            root["system_config"]["NUM_ETH_READOUTS_PER_PANEL"].asInt();
    config->hv_floating_boards_per_system =
            root["system_config"]["NUM_HV_FLOATING_BOARDS"].asInt();
    config->scmicros_per_cartridge =
            root["system_config"]["NUM_SCMICRO_PER_CART"].asInt();
    config->modules_per_dac =
            root["system_config"]["NUM_MODULE_PER_DAC"].asInt();

    // If these are not found, then asInt() will return a 0.  Check that all
    // of the settings are not zero to indicate a successful parse.
    if (config->panels_per_system && config->cartridges_per_panel &&
        config->daqs_per_cartridge && config->renas_per_daq &&
        config->modules_per_rena && config->fins_per_cartridge &&
        config->modules_per_fin && config->ethernets_per_panel &&
        config->hv_floating_boards_per_system &&
        config->scmicros_per_cartridge && config->modules_per_dac)
    {
        return(0);
    } else {
        return(-2);
    }
}

/*!
 * \brief Load Panel Configuration Settings from Json Object
 *
 * Takes a panel json object and searches for "usb_name" and "usb_baud_rate" and
 * puts them into a PanelConfig object.   Config object can be modified even
 * with errors.
 *
 * The function was not made public to keep the jsoncpp header local to the
 * object file.
 *
 * \param panel_config The configuration object to be modified
 * \param panel_json The Json object to be searched
 * \param require_all If true, return an error if not all of the objects are
 *        found.
 *
 * \return 0 is successful, less than otherwise
 *        -1 if not all of the required objects are found
 */
int loadPanelSettings(
    PanelConfig & panel_config,
    const Json::Value & panel_json,
    bool require_all = true)
{
    int not_found = 0;
    if (panel_json.isMember("usb_name")) {
        panel_config.usb_port_name = panel_json["usb_name"].asString();
    } else {
        not_found++;
    }
    if (panel_json.isMember("usb_baud_rate")) {
        panel_config.usb_port_baud_rate = panel_json["usb_baud_rate"].asInt();
    } else {
        not_found++;
    }
    if (require_all && (not_found > 0)) {
        return(-1);
    }
    return(0);
}

/*!
 * \brief Load Cartridge Configuration Settings from Json Object
 *
 * Takes a cartridge json object and searches for:
 *     -"bias"
 *     -"input_delay"
 *     -"output_delay"
 *     -"coinc_window"
 *     -"daqboard_id"
 *     -"ethernet_readout",
 *     -"ethernet_port"
 *     -"lv_power_supply_channel"
 *     -"hv_power_supply_channel"
 *     -"hv_floating_board_id"
 * and puts them into a CartridgeConfig object.  Config object can be modified
 * even with errors.
 *
 * The function was not made public to keep the jsoncpp header local to the
 * object file.
 *
 * \param cartridge_config The configuration object to be modified
 * \param cartridge_json The Json object to be searched
 * \param require_all If true, return an error if not all of the objects are
 *        found.
 *
 * \return 0 is successful, less than otherwise
 *        -1 if not all of the required objects are found
 */
int loadCartridgeSettings(
    CartridgeConfig & cartridge_config,
    const Json::Value & cartridge_json,
    bool require_all)
{
    int not_found = 0;
    if (cartridge_json.isMember("bias")) {
        cartridge_config.bias_voltage = cartridge_json["bias"].asFloat();
    } else {
        not_found++;
    }
    if (cartridge_json.isMember("input_delay")) {
        cartridge_config.backend_board_config.input_delay =
                cartridge_json["input_delay"].asInt();
    } else {
        not_found++;
    }
    if (cartridge_json.isMember("output_delay")) {
        cartridge_config.backend_board_config.output_delay =
                cartridge_json["output_delay"].asInt();
    } else {
        not_found++;
    }
    if (cartridge_json.isMember("coinc_window")) {
        cartridge_config.backend_board_config.coinc_window =
                cartridge_json["coinc_window"].asInt();
    } else {
        not_found++;
    }
    if (cartridge_json.isMember("daqboard_id")) {
        cartridge_config.backend_board_config.daqboard_id =
                cartridge_json["daqboard_id"].asInt();
    } else {
        not_found++;
    }
    if (cartridge_json.isMember("ethernet_readout")) {
        cartridge_config.backend_board_config.ethernet_readout =
                cartridge_json["ethernet_readout"].asBool();
    } else {
        not_found++;
    }
    if (cartridge_config.backend_board_config.ethernet_readout) {
        if (cartridge_json.isMember("ethernet_port")) {
            cartridge_config.backend_board_config.port_name =
                    cartridge_json["ethernet_port"].asString();
        } else {
            not_found++;
        }
    }
    if (cartridge_json.isMember("lv_power_supply_channel")) {
        cartridge_config.lv_power_supply_channel =
                cartridge_json["lv_power_supply_channel"].asInt();
    } else {
        not_found++;
    }
    if (cartridge_json.isMember("hv_power_supply_channel")) {
        cartridge_config.hv_power_supply_channel =
                cartridge_json["hv_power_supply_channel"].asInt();
    } else {
        not_found++;
    }
    if (cartridge_json.isMember("hv_floating_board_id")) {
        cartridge_config.hv_floating_board_id =
                cartridge_json["hv_floating_board_id"].asInt();
    } else {
        not_found++;
    }
    if (cartridge_json.isMember("hv_floating_board_slot")) {
        cartridge_config.hv_floating_board_slot =
                cartridge_json["hv_floating_board_slot"].asInt();
    } else {
        not_found++;
    }
    if (require_all && (not_found > 0)) {
        return(-1);
    }
    return(0);
}

/*!
 * \brief Load Fin Configuration Settings from Json Object
 *
 * Takes a fin json object and searches for "exclude_thermistor"  and puts that
 * value into a FinConfig object.  Config object can be modified if an error
 * occurs.
 *
 * The function was not made public to keep the jsoncpp header local to the
 * object file.
 *
 * \param fin_config The configuration object to be modified
 * \param fin_json The Json object to be searched
 * \param require_all If true, return an error if not all of the objects are
 *        found.
 *
 * \return 0 is successful, less than otherwise
 *        -1 if not all of the required objects are found
 */
int loadFinSettings(
    FinConfig & fin_config,
    const Json::Value & fin_json,
    bool require_all = true)
{
    int not_found = 0;
    if (fin_json.isMember("exclude_thermistor")) {
        fin_config.exclude_thermistor_value =
                fin_json["exclude_thermistor"].asBool();
    } else {
        not_found++;
    }
    if (require_all && (not_found > 0)) {
        return(-1);
    }
    return(0);
}

/*!
 * \brief Load Rena Channel Settings Configuration from Json Object
 *
 * Takes a channel settings json object and searches for:
 *     -"Fast_DAC"
 *     -"Fast_Hit_Readout"
 *     -"Fast_Powerdown"
 *     -"Fast_Trig_Enable"
 *     -"Feedback_Cap"
 *     -"Feedback_Resistor"
 *     -"Feedback_Type"
 *     -"Fet_Size"
 *     -"Follower"
 *     -"Gain"
 *     -"Polarity"
 *     -"Pole_Zero_Enable"
 *     -"Powerdown"
 *     -"Shaping_Time"
 *     -"Slow_DAC"
 *     -"Slow_Hit_Readout"
 *     -"Slow_Trig_Enable"
 *     -"Test_Enable"
 *     -"VRef"
 * The values are put into a RenaChannelConfig object.  Config object can be
 * modified if an error occurs.  A RenaChannelConfig object previously set can
 * also be given to this function to have more specific settings changed.  This
 * is used to set rena settings for individual fins, modules, panels, etc.
 * require_all should be set to false to do this.
 *
 * The function was not made public to keep the jsoncpp header local to the
 * object file.
 *
 * \param config The configuration object to be modified
 * \param channel_settings The Json object to be searched
 * \param require_all If true, return an error if not all of the objects are
 *        found.
 *
 * \return 0 is successful, less than otherwise
 *        -1 if not all of the required objects are found
 */
int loadChannelSettings(
    RenaChannelConfig & config,
    const Json::Value & channel_settings,
    bool require_all = false)
{
    int not_found = 0;
    if (channel_settings.isMember("Fast_DAC")) {
        config.fast_daq_threshold =
                channel_settings["Fast_DAC"].asInt();
    } else {
        not_found++;
    }
    if (channel_settings.isMember("Fast_Hit_Readout")) {
        config.fast_hit_readout =
                channel_settings["Fast_Hit_Readout"].asBool();
    } else {
        not_found++;
    }
    if (channel_settings.isMember("Fast_Powerdown")) {
        config.fast_powerdown =
                channel_settings["Fast_Powerdown"].asBool();
    } else {
        not_found++;
    }
    if (channel_settings.isMember("Fast_Trig_Enable")) {
        config.fast_trig_enable =
                channel_settings["Fast_Trig_Enable"].asBool();
    } else {
        not_found++;
    }
    if (channel_settings.isMember("Feedback_Cap")) {
        config.feedback_cap =
                channel_settings["Feedback_Cap"].asBool();
    } else {
        not_found++;
    }
    if (channel_settings.isMember("Feedback_Resistor")) {
        config.feedback_resistor =
                channel_settings["Feedback_Resistor"].asBool();
    } else {
        not_found++;
    }
    if (channel_settings.isMember("Feedback_Type")) {
        config.feedback_type =
                channel_settings["Feedback_Type"].asBool();
    } else {
        not_found++;
    }
    if (channel_settings.isMember("Fet_Size")) {
        config.fet_size =
                channel_settings["Fet_Size"].asBool();
    } else {
        not_found++;
    }
    if (channel_settings.isMember("Follower")) {
        config.follower =
                channel_settings["Follower"].asBool();
    } else {
        not_found++;
    }
    if (channel_settings.isMember("Gain")) {
        config.gain =
                channel_settings["Gain"].asInt();
    } else {
        not_found++;
    }
    if (channel_settings.isMember("Polarity")) {
        config.polarity =
                channel_settings["Polarity"].asBool();
    } else {
        not_found++;
    }
    if (channel_settings.isMember("Pole_Zero_Enable")) {
        config.pole_zero_enable =
                channel_settings["Pole_Zero_Enable"].asBool();
    } else {
        not_found++;
    }
    if (channel_settings.isMember("Powerdown")) {
        config.powerdown =
                channel_settings["Powerdown"].asBool();
    } else {
        not_found++;
    }
    if (channel_settings.isMember("Shaping_Time")) {
        config.shaping_time =
                channel_settings["Shaping_Time"].asBool();
    } else {
        not_found++;
    }
    if (channel_settings.isMember("Slow_DAC")) {
        config.slow_daq_threshold =
                channel_settings["Slow_DAC"].asInt();
    } else {
        not_found++;
    }
    if (channel_settings.isMember("Slow_Hit_Readout")) {
        config.slow_hit_readout =
                channel_settings["Slow_Hit_Readout"].asBool();
    } else {
        not_found++;
    }
    if (channel_settings.isMember("Slow_Trig_Enable")) {
        config.slow_trig_enable =
                channel_settings["Slow_Trig_Enable"].asBool();
    } else {
        not_found++;
    }
    if (channel_settings.isMember("Test_Enable")) {
        config.test_enable =
                channel_settings["Test_Enable"].asBool();
    } else {
        not_found++;
    }
    if (channel_settings.isMember("VRef")) {
        config.vref =
                channel_settings["VRef"].asBool();
    } else {
        not_found++;
    }

    if (require_all && (not_found > 0)) {
        return(-1);
    }
    return(0);
}

/*!
 * \brief Load a Modules Rena Channel Settings Configuration from Json Object
 *
 * Takes a channel settings json object and searches for:
 *     -"hit_threshold"
 *     -"double_trigger_threshold"
 *     -"ComH_Channels"
 *     -"ComL_Channels"
 *     -"Spat_Channels"
 * The values are put into a ModuleChannelConfig object.  Config object can be
 * modified if an error occurs.  A ModuleChannelConfig object previously set can
 * also be given to this function to have more specific settings changed.  This
 * is used to set rena settings for individual fins, modules, panels, etc.
 * require_all should be set to false to do this.
 *
 * The function was not made public to keep the jsoncpp header local to the
 * object file.
 *
 * \param module_config The configuration object to be modified
 * \param module_channel_settings The Json object to be searched
 * \param require_all If true, return an error if not all of the objects are
 *        found.
 *
 * \return 0 is successful, less than otherwise
 *        -1 if not all of the required objects are found
 */
int loadModuleChannelSettings(
    ModuleChannelConfig & module_config,
    const Json::Value & module_channel_settings,
    bool require_all = false)
{
    int not_found = 0;
    if (module_channel_settings.isMember("hit_threshold")) {
        module_config.hit_threshold =
                module_channel_settings["hit_threshold"].asInt();
    } else {
        not_found++;
    }
    if (module_channel_settings.isMember("double_trigger_threshold")) {
        module_config.double_trigger_threshold =
                module_channel_settings["double_trigger_threshold"].asInt();
    } else {
        not_found++;
    }
    if (module_channel_settings.isMember("ComH_Channels")) {
        loadChannelSettings(module_config.comH,
                            module_channel_settings["ComH_Channels"],
                            require_all);
    } else {
        not_found++;
    }
    if (module_channel_settings.isMember("ComL_Channels")) {
        loadChannelSettings(module_config.comL,
                            module_channel_settings["ComL_Channels"],
                            require_all);
    } else {
        not_found++;
    }
    if (module_channel_settings.isMember("Spat_Channels")) {
        loadChannelSettings(module_config.spatA,
                            module_channel_settings["Spat_Channels"],
                            require_all);
        loadChannelSettings(module_config.spatB,
                            module_channel_settings["Spat_Channels"],
                            require_all);
        loadChannelSettings(module_config.spatC,
                            module_channel_settings["Spat_Channels"],
                            require_all);
        loadChannelSettings(module_config.spatD,
                            module_channel_settings["Spat_Channels"],
                            require_all);
    } else {
        not_found++;
    }

    if (require_all && (not_found > 0)) {
        return(-1);
    }
    return(0);
}

/*!
 * \brief Checks for and loads a module channel setttings from Json Object
 *
 * Takes a json object and searches for "channel_settings".  If the object is
 * found, it calls loadModuleChannelSettings.  If it is not found, it returns
 * an error or success depending on the require_all flag.  This function is
 * called at every level of the system configuration to check for additional
 * settings that are applied to that level and downwards.  Config object can be
 * modified if an error occurs.  A ModuleChannelConfig object previously set can
 * also be given to this function to have more specific settings changed.  This
 * is used to set rena settings for individual fins, modules, panels, etc.
 * require_all should be set to false to do this.
 *
 * The function was not made public to keep the jsoncpp header local to the
 * object file.
 *
 * \param module_config The configuration object to be modified
 * \param json_object The Json object to be searched
 * \param require_all If true, return an error if not all of the objects are
 *        found.
 *
 * \return 0 is successful, less than otherwise
 *        -1 if "channel_settings" is required but not found
 *        -2 if loadModuleChannelSettings fails
 */
bool checkAndLoadChannelSettings(
        ModuleChannelConfig & module_config,
        const Json::Value & json_object,
        bool require_all = false)
{
    if (!json_object.isMember("channel_settings")) {
        if (require_all) {
            return(-1);
        }
    }
    int channel_load_status =
            loadModuleChannelSettings(module_config,
                                      json_object["channel_settings"],
                                      require_all);
    if (channel_load_status < 0) {
        return(-2);
    }
    return(0);
}

/*!
 * \brief Load a Modules Rena Channel Settings Configuration from Json Object
 *
 * Takes a module settings json object and searches for:
 *     -"name"
 *     -"bias"
 *     -"current"
 *     -"temp"
 *     -"system_resistor"
 *     -"module_test_voltage"
 *     -"module_test_current"
 *     -"module_test_temp"
 *     -"module_test_resistor"
 * The values are put into a ModuleConfig object.  Config object can be
 * modified if an error occurs.  A ModuleChannelConfig object previously set can
 * also be given to this function to have more specific settings changed.  This
 * is used to set rena settings for individual fins, modules, panels, etc.
 * require_all should be set to false to do this.  Can optionally require all of
 * the settings for a module to be given, except for the channel settings.
 * Relies on checkAndLoadChannelSettings to load the ModuleChannelConfig within
 * the ModuleConfig object.
 *
 * The function was not made public to keep the jsoncpp header local to the
 * object file.
 *
 * \param module_config The configuration object to be modified
 * \param default_channel_config the module channel settings to start with
 * \param module_json The Json object to be searched
 * \param require_non_channel_settings return error if the
 * \param require_all If true, return an error if not all of the objects are
 *        found.
 *
 * \return 0 is successful, less than otherwise
 *        -1 if not all of the required non-channel related objects are found
 *        -2 if checkAndLoadChannelSettings errors out
 */
int loadModuleSettings(
    ModuleConfig & module_config,
    const ModuleChannelConfig & default_channel_config,
    const Json::Value & module_json,
    bool require_non_channel_settings = true,
    bool require_all = false)
{
    int not_found = 0;
    if (module_json.isMember("name")) {
        module_config.name = module_json["name"].asString();
    } else {
        not_found++;
    }
    if (module_json.isMember("bias")) {
        module_config.bias_voltage = module_json["bias"].asFloat();
    } else {
        not_found++;
    }
    if (module_json.isMember("current")) {
        module_config.leakage_current = module_json["current"].asFloat();
    } else {
        not_found++;
    }
    if (module_json.isMember("temp")) {
        module_config.system_temperature = module_json["temp"].asFloat();
    } else {
        not_found++;
    }
    if (module_json.isMember("system_resistor")) {
        module_config.system_bias_resistor =
                module_json["system_resistor"].asFloat();
    } else {
        not_found++;
    }
    if (module_json.isMember("module_test_voltage")) {
        module_config.module_test_voltage =
                module_json["module_test_voltage"].asFloat();
    } else {
        not_found++;
    }
    if (module_json.isMember("module_test_current")) {
        module_config.module_test_current =
                module_json["module_test_current"].asFloat();
    } else {
        not_found++;
    }
    if (module_json.isMember("module_test_temp")) {
        module_config.module_test_temperature =
                module_json["module_test_temp"].asFloat();
    } else {
        not_found++;
    }
    if (module_json.isMember("module_test_resistor")) {
        module_config.module_test_bias_resistor =
                module_json["module_test_resistor"].asFloat();
    } else {
        not_found++;
    }
    if ((require_all || require_non_channel_settings) && (not_found > 0)) {
        return(-1);
    }
    // Set the module to the default
    module_config.channel_settings = default_channel_config;
    // And then load
    int check_status = checkAndLoadChannelSettings(
            module_config.channel_settings,
            module_json,
            require_all);

    if (check_status < 0) {
        return(-2);
    }

    return(0);
}

/*!
 * \brief Load all of the Hv Floating Board settings from root Json Object
 *
 * Takes the root json object and searches for "hv_floating_boards".  Assuming
 * that hv_floating_boards_per_system has already been set by loadSystemSize, it
 * searches for hv_floating_boards_per_system number of json objects, each with:
 *     -"usb_name"
 *     -"usb_baud_rate"
 * If these are found, then a HvFloatingBoardConfig is pushed into the
 * SystemConfiguration hv_volting_board_configs vector.
 *
 * The function was not made public to keep the jsoncpp header local to the
 * object file.
 *
 * \param config The system configuration object to be modified
 * \param root The Json object to be searched
 *
 * \return 0 is successful, less than otherwise
 *        -1 if "hv_floating_boards" is not found in the Json object
 *        -2 if a needed hv floating board json object is not found in the array
 *        -3 if "usb_name" is not found
 *        -4 if "usb_baud_rate" is not found
 */
int loadHvFloatingBoardSettings(
        SystemConfiguration * const config,
        const Json::Value & root)
{
    if (!root.isMember("hv_floating_boards")) {
        return(-1);
    }
    Json::Value hv_floating_boards = root["hv_floating_boards"];

    for (int b = 0; b < config->hv_floating_boards_per_system; b++) {
        Json::Value board = hv_floating_boards[b];
        if (board == Json::nullValue) {
            return(-2);
        }
        HvFloatingBoardConfig board_config;
        if (!board.isMember("usb_name")) {
            return(-3);
        }
        if (!board.isMember("usb_baud_rate")) {
            return(-4);
        }
        board_config.usb_port_name = board["usb_name"].asString();
        board_config.usb_port_baud_rate = board["usb_baud_rate"].asInt();
        config->hv_volting_board_configs.push_back(board_config);
    }
    return(0);
}
// end of the annoymous namespace
}

/*!
 * \brief Construct a System Configuration object from config file
 *
 * Sets up the object, and trys to load the configuration file if given.
 *
 * \param filename The system configuration file to be loaded.  By default this
 *        is blank and the load command must be called afterwards.
 *
 * \throws runtime_error if a load(filename) fails.  none if filename is blank.
 */
SystemConfiguration::SystemConfiguration(const std::string & filename) :
        apds_per_module(2),
        crystals_per_apd(64),
        channels_per_rena(36)
{
    std::memset(backend_address_panel_lookup, -1,
                sizeof(backend_address_panel_lookup));
    std::memset(backend_address_cartridge_lookup, -1,
                sizeof(backend_address_cartridge_lookup));
    std::memset(backend_address_valid, false,
                sizeof(backend_address_valid));

    if (filename != "") {
        int load_status = load(filename);
        if (load_status < 0) {
            std::stringstream ss;
            ss << "SystemConfiguration load(\"" << filename << "\") failed with"
               << " status: " << load_status;
            throw(std::runtime_error(ss.str()));
        }
    }
}

/*!
 * \brief Look up a Panel and Cartridge id given a backend board address
 *
 * \param backend_address The backend address to use in the lookup
 * \param panel Where the panel number is returned
 * \param panel Where the cartridge number is returned
 *
 * \returns 0 if successful, less than otherwise
 *         -1 if the address given could not be a valid id
 *         -2 if the address given has not been configured in the system config
 */
int SystemConfiguration::lookupPanelCartridge(
    int backend_address,
    int & panel,
    int & cartridge)
{
    if (backend_address < 0 || backend_address >= 32) {
        return(-1);
    }
    if (!backend_address_valid[backend_address]) {
        return(-2);
    }
    panel = backend_address_panel_lookup[backend_address];
    cartridge = backend_address_cartridge_lookup[backend_address];
    return(0);
}

/*!
 * \brief Convert PCDRM indexing to PCFM indexing
 *
 * Take a module that is indexed by panel, cartridge, daq board, rena, and
 * module (local to the rena) and then convert this indexing to panel,
 * cartridge, fin, module (local to the fin).
 *
 * \param panel the panel of the module
 * \param cartridge The cartridge of the module
 * \param daq The daq board of the module
 * \param rena The rena of the module
 * \param rena_local_module The module number local to the rena chip
 * \param fin Where the fin number is returned
 * \param module Where the module number local to the fin is returned
 *
 * \returns 0 if successful, less than otherwise
 *         -1 if the panel is invalid
 *         -2 if the cartridge is invalid
 *         -3 if the daq board id is invalid
 *         -4 if the rena number is invalid
 *         -5 if the module number local to the rena is invalid
 */
int SystemConfiguration::convertPCDRMtoPCFM(
        int panel,
        int cartridge,
        int daq,
        int rena,
        int rena_local_module,
        int & fin,
        int & module) const
{
    // Validate input
    if (panel < 0 || panel >= panels_per_system) {
        return(-1);
    }
    if (cartridge < 0 || cartridge >= cartridges_per_panel) {
        return(-2);
    }
    if (daq < 0 || daq >= daqs_per_cartridge) {
        return(-3);
    }
    if (rena < 0 || rena >= renas_per_daq) {
        return(-4);
    }
    if (rena_local_module < 0 || rena_local_module >= modules_per_rena) {
        return(-5);
    }
    // Convert
    fin = fins_per_cartridge - 1 - 2 * std::floor(rena / 2);
    module = rena_local_module;
    if (rena % 2) {
        module += modules_per_rena;
    }
    if (daq % 2) {
        module += (modules_per_fin / 2);
    }

    if (panel == 0) {
        if (daq < 2) {
            if (renas_per_daq > 2) {
                fin--;
            }
        }
    } else if (panel == 1) {
        if (daq >= 2) {
            if (renas_per_daq > 2) {
                fin--;
            }
        }
        module = modules_per_fin - 1 - module;
    }
    return(0);
}

/*!
 * \brief Convert PCFM indexing to PCDRM indexing
 *
 * Take a module that is indexed by panel, cartridge, fin, and module (local to
 * the fin) and then convert this indexing to panel, cartridge, daq board, rena,
 * and module (local to the rena) .
 *
 * \param panel the panel of the module
 * \param cartridge The cartridge of the module
 * \param fin The fin number of the module
 * \param module The module id local to the fin
 * \param daq The daq board number is returned
 * \param rena Where the rena id is returned
 * \param rena_local_module Where the module number local to the rena chip is
 *        returned
 *
 * \returns 0 if successful, less than otherwise
 *         -1 if the panel is invalid
 *         -2 if the cartridge is invalid
 *         -3 if the fin id is invalid
 *         -4 if the module number local to the fin is invalid
 */
int SystemConfiguration::convertPCFMtoPCDRM(
        int panel,
        int cartridge,
        int fin,
        int module,
        int & daq,
        int & rena,
        int & rena_local_module) const
{
    // Validate input
    if (panel < 0 || panel >= panels_per_system) {
        return(-1);
    }
    if (cartridge < 0 || cartridge >= cartridges_per_panel) {
        return(-2);
    }
    if (fin < 0 || fin >= fins_per_cartridge) {
        return(-3);
    }
    if (module < 0 || module >= modules_per_fin) {
        return(-4);
    }

    rena = 2 * std::floor((fins_per_cartridge - 1 - fin) / 2);
    daq = 0;
    if (panel == 0) {
        if (fin % 2) {
            if (renas_per_daq > 2) {
                daq += 2;
            }
        }
        if (module >= 8) {
            daq += 1;
        }
        if (module % 8 >= modules_per_rena) {
            rena += 1;
        }
        rena_local_module = (module % modules_per_rena);
    } else if (panel == 1) {
        if (!(fin % 2)) {
            if (renas_per_daq > 2) {
                daq += 2;
            }
        }
        if (module < 8) {
            daq += 1;
        }
        if (module % 8 < modules_per_rena) {
            rena += 1;
        }
        rena_local_module = modules_per_rena - 1 - (module % modules_per_rena);
    }
    return(0);
}

/*!
 * \brief Load in a system configuration JSON file
 *
 * Take a json-based system configuration file and parse it into the data
 * structure.  This loads everything relevant to decoding the datastream.
 *
 * \param filename The system configuration file to be loaded.
 *
 * \returns 0 if successful, less than otherwise
 *         -1 if jsoncpp cannot parse the file
 *         -2 if loadSystemSize failed
 *         -3 if checkAndLoadChannelSettings failed to load the default settings
 *         -4 if "panels" wasn't found in the root object
 *         -5 if a specific panel index was not found
 *         -6 if "cartridges" wasn't found in the panel settings
 *         -7 if the specific cartridge index was not found
 *         -8 if loadCartridgeSettings failed
 *         -9 if "fins" wasn't found in the cartridge settings
 *        -10 if a specific fin index wasn't found
 *        -11 if "modules" wasn't found in the fin settings
 *        -12 if a specific module index was not found
 *        -13 if loadModuleSettings failed
 *        -14 if populateBackendAddressReverseLookup failed
 *        -15 if loadHvFloatingBoardSettings failed
 *        -16 if loadPanelSettings failed
 *        -17 if loadFinSettings failed
 */
int SystemConfiguration::load(const std::string & filename) {
    std::ifstream json_in(filename.c_str());
    Json::Value root;
    Json::Reader reader;
    bool parsingSuccessful = reader.parse(json_in, root);
    json_in.close();
    if (!parsingSuccessful) {
        std::cerr  << "Failed to parse configuration\n"
                   << reader.getFormattedErrorMessages();
        return(-1);
    }
    // Load the system size
    if (loadSystemSize(this, root) < 0) {
        return(-2);
    }
    // Configure the arrays given the size
    resizePCFMArray(this, module_configs);
    resizePCArray(this, cartridge_configs);
    resizePCFArray(this, fin_configs);

    // Load in the HV Floating Board Settings
    if (loadHvFloatingBoardSettings(this, root) < 0) {
        return(-15);
    }

    ModuleChannelConfig system_default_channel_settings;
    if (checkAndLoadChannelSettings(system_default_channel_settings,
                                root, true) < 0)
    {
        return(-3);
    }

    if (loadChannelSettings(unused_channel_config,
                            root["channel_settings"]["Unused_Channels"],
                            true) < 0)
    {
        return(-3);
    }
    Json::Value panels = root["panels"];
    if (panels == Json::nullValue) {
        return(-4);
    }

    for (int p = 0; p < panels_per_system; p++) {
        Json::Value panel = panels[p];
        if (panel == Json::nullValue) {
            return(-5);
        }
        PanelConfig panel_config;
        if (loadPanelSettings(panel_config, panel) < 0) {
            return(-16);
        }
        panel_configs.push_back(panel_config);
        ModuleChannelConfig panel_default_channel_settings =
                system_default_channel_settings;
        checkAndLoadChannelSettings(panel_default_channel_settings, panel);
        Json::Value cartridges = panel["cartridges"];
        if (cartridges == Json::nullValue) {
            return(-6);
        }
        for (int c = 0; c < cartridges_per_panel; c++) {
            Json::Value cartridge = cartridges[c];
            if (cartridge == Json::nullValue) {
                return(-7);
            }
            if (loadCartridgeSettings(cartridge_configs[p][c],
                                      cartridge, true) < 0)
            {
                std::cerr << "P" << p << "C" << c
                          << " failed to load" << std::endl;
                return(-8);
            }

            ModuleChannelConfig cartridge_default_channel_settings =
                    panel_default_channel_settings;
            checkAndLoadChannelSettings(cartridge_default_channel_settings,
                                        cartridge);
            Json::Value fins = cartridge["fins"];
            if (fins == Json::nullValue) {
                return(-9);
            }
            for (int f = 0; f < fins_per_cartridge; f++) {
                Json::Value fin = fins[f];
                if (fin == Json::nullValue) {
                    return(-10);
                }
                if (loadFinSettings(fin_configs[p][c][f], fin) < 0) {
                    return(-17);
                }
                ModuleChannelConfig fin_default_channel_settings =
                        cartridge_default_channel_settings;
                checkAndLoadChannelSettings(fin_default_channel_settings,
                                            fin);

                Json::Value modules = fin["modules"];
                if (modules == Json::nullValue) {
                    return(-11);
                }
                for (int m = 0; m < modules_per_fin; m++) {
                    Json::Value module = modules[m];
                    if (module == Json::nullValue) {
                        return(-12);
                    }
                    if (loadModuleSettings(module_configs[p][c][f][m],
                                       fin_default_channel_settings,
                                       module, true) < 0)
                    {
                        std::cerr << "load failed on P" << p << "C" << c
                                  << "F" << f << "M" << m << std::endl;
                        return(-13);
                    }
                    // Set the module value for each of the channels in the
                    // module so that it can be used by the hit register config
                    int module_rena;
                    int rena;
                    int daq;
                    convertPCFMtoPCDRM(p, c, f, m, daq, rena, module_rena);
                    module_configs[p][c][f][m].channel_settings.comH.module =
                            module_rena;
                    module_configs[p][c][f][m].channel_settings.comL.module =
                            module_rena;
                    module_configs[p][c][f][m].channel_settings.spatA.module =
                            module_rena;
                    module_configs[p][c][f][m].channel_settings.spatB.module =
                            module_rena;
                    module_configs[p][c][f][m].channel_settings.spatC.module =
                            module_rena;
                    module_configs[p][c][f][m].channel_settings.spatD.module =
                            module_rena;
                }
            }
        }
    }

    if (populateBackendAddressReverseLookup(
                this,
                this->backend_address_valid,
                this->backend_address_panel_lookup,
                this->backend_address_cartridge_lookup) < 0)
    {
        std::cerr << "Invalid daq_address (must be 0-31)" << std::endl;
        return(-14);
    }

    if (populatePacketSizeLookup(this, this->packet_size) < 0) {
        std::cerr << "populatePacketSizeLookup failed" << std::endl;
    }

    populateADCLocationLookup(this, this->adc_value_locations);

    // Verify full round trip conversion on the mapping.  Should be in a test
    // case, but this will do for now.
    for (int p = 0; p < panels_per_system; p++) {
        for (int c = 0; c < cartridges_per_panel; c++) {
            for (int f = 0; f < fins_per_cartridge; f++) {
                for (int m = 0; m < modules_per_fin; m++) {
                    int daq = 0;
                    int rena = 0;
                    int module = 0;
                    convertPCFMtoPCDRM(p, c, f, m, daq, rena, module);
                    int test_f = 0;
                    int test_m = 0;
                    convertPCDRMtoPCFM(p, c, daq, rena, module, test_f, test_m);
                    assert(test_f == f);
                    assert(test_m == m);
                }
            }
        }
    }
    return(0);
}

/*!
 * \brief Load a pedestal value file into the system configuration
 *
 * Takes a pedestal value file pulls out the pedestal values to be stored in the
 * pedestals array of ModulePedestals objects.  This assumes a file of lines
 * with the following columns separated by any number amount of whitespace:
 *     - The PCDRM id for the line (i.e. "P0C1D13M2")
 *     - The number of events that were factored into the pedestal calculation
 *     - The Spatial A Channel Pedestal Value
 *     - The Spatial A Channel Pedestal RMS Value
 *     - The Spatial B Channel Pedestal Value
 *     - The Spatial B Channel Pedestal RMS Value
 *     - The Spatial C Channel Pedestal Value
 *     - The Spatial C Channel Pedestal RMS Value
 *     - The Spatial D Channel Pedestal Value
 *     - The Spatial D Channel Pedestal RMS Value
 *     - The PSAPD 0 Low Gain Common Channel Pedestal Value
 *     - The PSAPD 0 Low Gain Common Channel Pedestal RMS Value
 *     - The PSAPD 0 High Gain Common Channel Pedestal Value
 *     - The PSAPD 0 High Gain Common Channel Pedestal RMS Value
 *     - The PSAPD 1 Low Gain Common Channel Pedestal Value
 *     - The PSAPD 1 Low Gain Common Channel Pedestal RMS Value
 *     - The PSAPD 1 High Gain Common Channel Pedestal Value
 *     - The PSAPD 1 High Gain Common Channel Pedestal RMS Value
 *
 * \param filename The name of the file to be loaded.
 *
 * \returns 0 if successful, less than otherwise
 *         -1 if file could not be opened
 *         -2 if a PCDRM id load failed
 *         -3 if parsing a PCDRM id failed
 *         -4 if parsing the number of events in the pedestal calculation failed
 *         -5 if the values from the PCDRM id were out of range
 *         -6 if parsing an individual pedestal value failed
 *         -7 if parsing an individual pedestal rm value failed
 *         -8 if an incorrect number of lines was loaded from the file
 */
int SystemConfiguration::loadPedestals(const std::string & filename) {
    resizePCDRMArray(this, pedestals);

    std::ifstream pedestal_value_filestream;

    pedestal_value_filestream.open(filename.c_str());
    if (!pedestal_value_filestream) {
        return(-1);
    }

    int lines(0);
    std::string fileline;
    while (std::getline(pedestal_value_filestream, fileline)) {
        lines++;
        std::stringstream line_stream(fileline);
        std::string id_string;
        if ((line_stream >> id_string).fail()) {
            return(-2);
        }
        int panel;
        int cartridge;
        int chip;
        int module;
        int sscan_status(sscanf(id_string.c_str(),
                                "P%dC%dR%dM%d",
                                &panel, &cartridge, &chip, &module));

        if (sscan_status != 4) {
            return(-3);
        }

        int events;
        if((line_stream >> events).fail()) {
            return(-4);
        }

        int rena = chip % renas_per_daq;
        int daq = (chip - rena) / renas_per_daq;

        if ((rena >= renas_per_daq) || (rena < 0) ||
                (daq >= daqs_per_cartridge) || (daq < 0) ||
                (module >= modules_per_rena) || (module < 0) ||
                (cartridge >= cartridges_per_panel) || (cartridge < 0) ||
                (panel >= panels_per_system) || (panel < 0))
        {
            return(-5);
        }
        for (int ii = 0; ii < 8; ii++) {
            float channel_pedestal_value;
            if ((line_stream >> channel_pedestal_value).fail()) {
                return(-6);
            } else {
                if (ii == 0) {
                    pedestals[panel][cartridge][daq][rena][module].a =
                            channel_pedestal_value;
                } else if (ii == 1) {
                    pedestals[panel][cartridge][daq][rena][module].b =
                            channel_pedestal_value;
                } else if (ii == 2) {
                    pedestals[panel][cartridge][daq][rena][module].c =
                            channel_pedestal_value;
                } else if (ii == 3) {
                    pedestals[panel][cartridge][daq][rena][module].d =
                            channel_pedestal_value;
                } else if (ii == 4) {
                    pedestals[panel][cartridge][daq][rena][module].com1 =
                            channel_pedestal_value;
                } else if (ii == 5) {
                    pedestals[panel][cartridge][daq][rena][module].com1h =
                            channel_pedestal_value;
                } else if (ii == 6) {
                    pedestals[panel][cartridge][daq][rena][module].com2 =
                            channel_pedestal_value;
                } else if (ii == 7) {
                    pedestals[panel][cartridge][daq][rena][module].com2h =
                            channel_pedestal_value;
                }
            }

            float channel_pedestal_rms;
            if ((line_stream >> channel_pedestal_rms).fail()) {
                return(-7);
            }
        }
    }

    const int expected_lines(panels_per_system *
                             cartridges_per_panel *
                             daqs_per_cartridge *
                             renas_per_daq *
                             modules_per_rena);

    if (expected_lines != lines) {
        return(-8);
    } else {
        return(0);
    }
}

/*!
 * \brief Load a uv centers value file into the system configuration
 *
 * Takes a uv centers value file and loads the uv centers into the
 * pedestals array of ModulePedestals objects.  This assumes a file of lines
 * with the following columns separated by any number amount of whitespace:
 *     - u value of the circle center
 *     - v value of the circle center
 * The lines are assumed to be listed in C index order, and indexed by panel,
 * cartridge, fin, module, psapd.
 *
 * \param filename The name of the file to be loaded.
 *
 * \returns 0 if successful, less than otherwise
 *         -1 if file could not be opened
 *         -2 if parsing an individual u value failed
 *         -3 if parsing an individual v value failed
 *         -4 if the number of lines read was not expected by the system config
 */
int SystemConfiguration::loadUVCenters(const std::string &filename) {
    std::ifstream calibration_filestream;

    calibration_filestream.open(filename.c_str());
    if (!calibration_filestream) {
        return(-1);
    }

    const size_t expected_lines(panels_per_system *
                                cartridges_per_panel *
                                fins_per_cartridge *
                                modules_per_fin *
                                apds_per_module);

    std::vector<float> circles_u_read;
    std::vector<float> circles_v_read;

    std::string fileline;
    while (std::getline(calibration_filestream, fileline)) {
        std::stringstream line_stream(fileline);
        float u;
        float v;
        if((line_stream >> u).fail()) {
            return(-2);
        }
        circles_u_read.push_back(u);

        if((line_stream >> v).fail()) {
            return(-3);
        }
        circles_v_read.push_back(v);
    }

    if ((circles_u_read.size() != expected_lines) ||
            (circles_v_read.size() != expected_lines)) {
        return(-4);
    }
    int read_idx = 0;
    for (int p = 0; p < panels_per_system; p++) {
        for (int c = 0; c < cartridges_per_panel; c++) {
            for (int f = 0; f < fins_per_cartridge; f++) {
                for (int m = 0; m < modules_per_fin; m++) {
                    int daq = 0;
                    int rena = 0;
                    int module = 0;
                    convertPCFMtoPCDRM(p, c, f, m, daq, rena, module);
                    for (int a = 0; a < apds_per_module; a++) {
                        if (a == 0) {
                            pedestals[p][c][daq][rena][module].u1h =
                                    circles_u_read[read_idx];
                            pedestals[p][c][daq][rena][module].v1h =
                                    circles_v_read[read_idx];
                        } else if (a == 1) {
                            pedestals[p][c][daq][rena][module].u2h =
                                    circles_u_read[read_idx];
                            pedestals[p][c][daq][rena][module].v2h =
                                    circles_v_read[read_idx];
                        }
                        read_idx++;
                    }
                }
            }
        }
    }
    return(0);
}

/*!
 * \brief Load a calibration value file into the system configuration
 *
 * Takes a calibration value file and loads the values into the calibration
 * array's CrystalCalibration objects.  This assumes a file of lines with the
 * following columns separated by any number amount of whitespace:
 *     - Boolean value indicating wether the crystal should be used or not
 *     - x location of the crystal in the flood histogram
 *     - y location of the crystal in the flood histogram
 *     - the photopeak position of the spatial channels in adc values
 *     - the photopeak position of the common channel in adc values
 *     - the energy resolution of the spatial channels in percent
 *     - the energy resolution of the spatial channel in percent
 * The lines are assumed to be listed in C index order, and indexed by panel,
 * cartridge, fin, module, psapd, crystal.
 *
 * \param filename The name of the file to be loaded.
 *
 * \returns 0 if successful, less than otherwise
 *         -1 if file could not be opened
 *         -2 if the file has more lines that expected by the system config
 *         -3 if parsing an individual crystal use value failed
 *         -4 if parsing an individual x location value failed
 *         -5 if parsing an individual x location value failed
 *         -6 if parsing an individual spatial photopeak location value failed
 *         -7 if parsing an individual common photopeak location value failed
 *         -8 if parsing an individual spatial energy resolution value failed
 *         -9 if parsing an individual common energy resolution value failed
 *        -10 if the number of lines read was not expected by the system config
 */
int SystemConfiguration::loadCalibration(const std::string &filename) {
    std::ifstream calibration_filestream;

    calibration_filestream.open(filename.c_str());
    if (!calibration_filestream) {
        return(-1);
    }

    const int expected_lines(panels_per_system *
                             cartridges_per_panel *
                             fins_per_cartridge *
                             modules_per_fin *
                             apds_per_module *
                             crystals_per_apd);

    std::vector<bool> use_crystal_read(expected_lines, 0);
    std::vector<float> crystal_x_read(expected_lines, 0);
    std::vector<float> crystal_y_read(expected_lines, 0);
    std::vector<float> gain_spat_read(expected_lines, 0);
    std::vector<float> gain_comm_read(expected_lines, 0);
    std::vector<float> eres_spat_read(expected_lines, 0);
    std::vector<float> eres_comm_read(expected_lines, 0);

    int lines(0);
    std::string fileline;
    while (std::getline(calibration_filestream, fileline)) {
        if (lines >= expected_lines) {
            return(-2);
        }

        std::stringstream line_stream(fileline);

        bool use_crystal_val;
        if((line_stream >> use_crystal_val).fail()) {
            return(-3);
        } else {
            use_crystal_read[lines] = use_crystal_val;
        }

        if((line_stream >> crystal_x_read[lines]).fail()) {
            return(-4);
        }

        if((line_stream >> crystal_y_read[lines]).fail()) {
            return(-5);
        }

        if((line_stream >> gain_spat_read[lines]).fail()) {
            return(-6);
        }

        if((line_stream >> gain_comm_read[lines]).fail()) {
            return(-7);
        }

        if((line_stream >> eres_spat_read[lines]).fail()) {
            return(-8);
        }

        if((line_stream >> eres_comm_read[lines]).fail()) {
            return(-9);
        }

        lines++;
    }

    if (expected_lines != lines) {
        return(-10);
    }

    resizePCFMAXArray(this, this->calibration);
    int read_idx = 0;
    for (int p = 0; p < panels_per_system; p++) {
        for (int c = 0; c < cartridges_per_panel; c++) {
            for (int f = 0; f < fins_per_cartridge; f++) {
                for (int m = 0; m < modules_per_fin; m++) {
                    for (int a = 0; a < apds_per_module; a++) {
                        for (int x = 0; x < crystals_per_apd; x++) {
                            calibration[p][c][f][m][a][x].use =
                                    use_crystal_read[read_idx];
                            calibration[p][c][f][m][a][x].gain_spat =
                                    gain_spat_read[read_idx];
                            calibration[p][c][f][m][a][x].gain_comm =
                                    gain_comm_read[read_idx];
                            calibration[p][c][f][m][a][x].eres_spat =
                                    eres_spat_read[read_idx];
                            calibration[p][c][f][m][a][x].eres_comm =
                                    eres_comm_read[read_idx];
                            calibration[p][c][f][m][a][x].x_loc =
                                    crystal_x_read[read_idx];
                            calibration[p][c][f][m][a][x].y_loc =
                                    crystal_y_read[read_idx];
                            read_idx++;
                        }
                    }
                }
            }
        }
    }
    return(0);
}

/*!
 * \brief Load a time offset value file into the system configuration
 *
 * Takes a time offset value file and loads the values into the calibration
 * array's CrystalCalibration objects's time_offset value.  This assumes a file
 * of lines with single value representing the offset for an individual crystal.
 * The lines are assumed to be listed in C index order, and indexed by panel,
 * cartridge, fin, module, psapd, crystal.
 *
 * \param filename The name of the file to be loaded.
 *
 * \returns 0 if successful, less than otherwise
 *         -1 if file could not be opened
 *         -2 if the file has more lines that expected by the system config
 *         -3 if parsing an individual time offset value failed
 *         -4 if the number of lines read was not expected by the system config
 */
int SystemConfiguration::loadTimeCalibration(const std::string &filename) {
    std::ifstream calibration_filestream;

    calibration_filestream.open(filename.c_str());
    if (!calibration_filestream) {
        return(-1);
    }

    const int expected_lines(panels_per_system *
                             cartridges_per_panel *
                             fins_per_cartridge *
                             modules_per_fin *
                             apds_per_module *
                             crystals_per_apd);

    std::vector<float> time_offset_read(expected_lines, 0);

    int lines(0);
    std::string fileline;
    while (std::getline(calibration_filestream, fileline)) {
        if (lines >= expected_lines) {
            return(-2);
        }

        std::stringstream line_stream(fileline);

        if((line_stream >> time_offset_read[lines]).fail()) {
            return(-3);
        }
        lines++;
    }

    if (expected_lines != lines) {
        return(-4);
    }

    resizePCFMAXArray(this, this->calibration);
    int read_idx = 0;
    for (int p = 0; p < panels_per_system; p++) {
        for (int c = 0; c < cartridges_per_panel; c++) {
            for (int f = 0; f < fins_per_cartridge; f++) {
                for (int m = 0; m < modules_per_fin; m++) {
                    for (int a = 0; a < apds_per_module; a++) {
                        for (int x = 0; x < crystals_per_apd; x++) {
                            calibration[p][c][f][m][a][x].time_offset =
                                    time_offset_read[read_idx];
                            read_idx++;
                        }
                    }
                }
            }
        }
    }
    return(0);
}

/*!
 * \brief Create a default channel map for the system
 *
 * Map a channel number on a rena to a particular rena setting structure in the
 * module_configs PCFM array.  This assumes that channels 0, 1, 34, and 35 on
 * the rena are unused.  Assumes that on even number renas, commons are listed
 * first (H0, L0, H1, L1) for each module, then spatials (A, B, C, D) for each
 * module.  For odd numbered renas, the spatials are listed first (D, C, B, A)
 * for each module, then the commons (H0, L0, H1, L1) for each module.
 *
 * \note This assumes that the configuration is as hardwired. If this needs to
 *       change, then this function can be overloaded to find a particular file,
 *       however, the ADC readout function lookup table generation will need to
 *       change to use the channel map, and the RenaChannelSettings structure
 *       will need more information to identify it's type.
 */
int SystemConfiguration::createChannelMap() {
    // Initialize the entire array to be pointers to the unused channel since
    // channels 0, 1, 34, and 35 are not connected to anything.
    resizePCDRCArray(this, this->channel_map);
    // Make sure something in software doesn't screw up something that is always
    // true in hardware.
    assert((8 * modules_per_rena + 4) == channels_per_rena);
    for (int p = 0; p < panels_per_system; p++) {
        for (int c = 0; c < cartridges_per_panel; c++) {
            for (int d = 0; d < daqs_per_cartridge; d++) {
                for (int r = 0; r < renas_per_daq; r++) {
                    int channel = 0;
                    channel_map[p][c][d][r][channel++] =
                            &this->unused_channel_config;
                    channel_map[p][c][d][r][channel++] =
                            &this->unused_channel_config;
                    if (r % 2) {
                        // For Odd Renas, backward spatials first, then commons
                        for (int m = 0; m < modules_per_rena; m++) {
                            int module;
                            int fin;
                            convertPCDRMtoPCFM(p, c, d, r, m, fin, module);
                            ModuleConfig * configs =
                                    module_configs[p][c][fin].data();
                            channel_map[p][c][d][r][channel++] =
                                    &configs[module].channel_settings.spatD;
                            channel_map[p][c][d][r][channel++] =
                                    &configs[module].channel_settings.spatC;
                            channel_map[p][c][d][r][channel++] =
                                    &configs[module].channel_settings.spatB;
                            channel_map[p][c][d][r][channel++] =
                                    &configs[module].channel_settings.spatA;
                        }
                        for (int m = 0; m < modules_per_rena; m++) {
                            int module;
                            int fin;
                            convertPCDRMtoPCFM(p, c, d, r, m, fin, module);
                            ModuleConfig * configs =
                                    module_configs[p][c][fin].data();
                            channel_map[p][c][d][r][channel++] =
                                    &configs[module].channel_settings.comH;
                            channel_map[p][c][d][r][channel++] =
                                    &configs[module].channel_settings.comL;
                            channel_map[p][c][d][r][channel++] =
                                    &configs[module].channel_settings.comH;
                            channel_map[p][c][d][r][channel++] =
                                    &configs[module].channel_settings.comL;
                        }
                    } else {
                        // for even renas, commons first, then spatials.
                        for (int m = 0; m < modules_per_rena; m++) {
                            int module;
                            int fin;
                            convertPCDRMtoPCFM(p, c, d, r, m, fin, module);
                            ModuleConfig * configs =
                                    module_configs[p][c][fin].data();
                            channel_map[p][c][d][r][channel++] =
                                    &configs[module].channel_settings.comH;
                            channel_map[p][c][d][r][channel++] =
                                    &configs[module].channel_settings.comL;
                            channel_map[p][c][d][r][channel++] =
                                    &configs[module].channel_settings.comH;
                            channel_map[p][c][d][r][channel++] =
                                    &configs[module].channel_settings.comL;
                        }
                        for (int m = 0; m < modules_per_rena; m++) {
                            int module;
                            int fin;
                            convertPCDRMtoPCFM(p, c, d, r, m, fin, module);
                            ModuleConfig * configs =
                                    module_configs[p][c][fin].data();
                            channel_map[p][c][d][r][channel++] =
                                    &configs[module].channel_settings.spatA;
                            channel_map[p][c][d][r][channel++] =
                                    &configs[module].channel_settings.spatB;
                            channel_map[p][c][d][r][channel++] =
                                    &configs[module].channel_settings.spatC;
                            channel_map[p][c][d][r][channel++] =
                                    &configs[module].channel_settings.spatD;
                        }
                    }
                    channel_map[p][c][d][r][channel++] =
                            &this->unused_channel_config;
                    channel_map[p][c][d][r][channel++] =
                            &this->unused_channel_config;
                }
            }
        }
    }
    return(0);
}
