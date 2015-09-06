#ifndef SYSTEM_CONFIGURATION_H
#define SYSTEM_CONFIGURATION_H
#include <vector>
#include <string>

/*!
 * A rena has 36 channels (32 which are used) that can have up to 3 values read
 * out for it (value, u, v).  32 * 3 = 96.  Used in the decoding process.
 */
#define MAX_NO_ADC_VALUES 96

/*!
 * A structure to hold the pedestal values for each of the channels of the
 * module.  The u and v values represent the centers of the uv circle.
 */
struct ModulePedestals {
    float a;
    float b;
    float c;
    float d;
    float com1;
    float com2;
    float com1h;
    float com2h;
    float u1h;
    float v1h;
    float u2h;
    float v2h;
};

/*!
 * A structure containing all of the information required to identify an event
 * to a particular crystal, convert the signal into an energy, and then
 * apply a time calibration to the event.
 */
struct CrystalCalibration {
    bool use;
    float gain_spat;
    float gain_comm;
    float eres_spat;
    float eres_comm;
    float x_loc;
    float y_loc;
    float time_offset;
};

/*!
 * A structure to store where within a packet received from the system a
 * particular channels value will be located amongst the ADC values (i.e. after
 * the packet header).  This structure needs to be calculated for every module
 * in the system under every possible trigger code, since modules that trigger
 * at the same time are combined within the same packet.  If an ADC value is not
 * present in the packet, it is initialized to MAX_NO_ADC_VALUES, pointing to a
 * default value at the end of the array where the ADC values are read into.
 * this means that the channels that are not read out are set to the value at
 * array[MAX_NO_ADC_VALUES].
 *
 * This alternatively could be done using a boolean flag, or -1, but that would
 * require checking this flag prior to loading the value for each channel.
 */
struct ADCValueLocation {
    bool triggered;
    int com1;
    int com2;
    int com1h;
    int com2h;
    int u1;
    int v1;
    int u2;
    int v2;
    int u1h;
    int v1h;
    int u2h;
    int v2h;
    int a;
    int a_u;
    int a_v;
    int b;
    int b_u;
    int b_v;
    int c;
    int c_u;
    int c_v;
    int d;
    int d_u;
    int d_v;

    /*!
     * Default constructor that initializes everything to MAX_NO_ADC_VALUES
     */
    ADCValueLocation() :
            triggered(false),
            com1(MAX_NO_ADC_VALUES),
            com2(MAX_NO_ADC_VALUES),
            com1h(MAX_NO_ADC_VALUES),
            com2h(MAX_NO_ADC_VALUES),
            u1(MAX_NO_ADC_VALUES),
            v1(MAX_NO_ADC_VALUES),
            u2(MAX_NO_ADC_VALUES),
            v2(MAX_NO_ADC_VALUES),
            u1h(MAX_NO_ADC_VALUES),
            v1h(MAX_NO_ADC_VALUES),
            u2h(MAX_NO_ADC_VALUES),
            v2h(MAX_NO_ADC_VALUES),
            a(MAX_NO_ADC_VALUES),
            a_u(MAX_NO_ADC_VALUES),
            a_v(MAX_NO_ADC_VALUES),
            b(MAX_NO_ADC_VALUES),
            b_u(MAX_NO_ADC_VALUES),
            b_v(MAX_NO_ADC_VALUES),
            c(MAX_NO_ADC_VALUES),
            c_u(MAX_NO_ADC_VALUES),
            c_v(MAX_NO_ADC_VALUES),
            d(MAX_NO_ADC_VALUES),
            d_u(MAX_NO_ADC_VALUES),
            d_v(MAX_NO_ADC_VALUES)
    {}
};

/*!
 * A structure to hold all of the potential settings that an individual rena
 * channel could be programmed with
 */
struct RenaChannelConfig {
    int fast_daq_threshold;
    ///! The channel readout flag is used on the FPGA, not on the Rena
    bool fast_hit_readout;
    bool fast_powerdown;
    bool fast_trig_enable;
    bool feedback_cap;
    bool feedback_resistor;
    bool feedback_type;
    bool fet_size;
    bool follower;
    int gain;
    bool polarity;
    bool pole_zero_enable;
    bool powerdown;
    int shaping_time;
    int slow_daq_threshold;
    ///! The channel readout flag is used on the FPGA, not on the Rena
    bool slow_hit_readout;
    bool slow_trig_enable;
    bool test_enable;
    bool vref;
    ///! The module number local to the rena the channel is associated with
    int module;
};

/*!
 * This is a structure which holds all of the information that is relevant to a
 * modules channels.  This structure is the data structure that gathers all of
 * the more specific settings as the load() function goes down the settings
 * tree.  If "channel_settings" is specified at any level below the root of the
 * json level, the values at that level will be used for everything belonging to
 * that level (i.e. all of the modules of a fin).
 */
struct ModuleChannelConfig {
    int hit_threshold;
    int double_trigger_threshold;
    RenaChannelConfig comL;
    RenaChannelConfig comH;
    RenaChannelConfig spatA;
    RenaChannelConfig spatB;
    RenaChannelConfig spatC;
    RenaChannelConfig spatD;
};

/*!
 * \brief APD Calibration Information
 *
 * Contains all of the calibration information about the PSAPD
 */
struct ApdConfig {
    /*!
     * The average common photopeak position over all of the crystals in the
     * module.  Calculated in loadCalibration.
     */
    float gain_comm_avg;
    /*!
     * The minimum common photopeak position over all of the crystals in the
     * module.  Calculated in loadCalibration.
     */
    float gain_comm_min;
    /*!
     * The maximum common photopeak position over all of the crystals in the
     * module.  Calculated in loadCalibration.
     */
    float gain_comm_max;
    /*!
     * The average common energy resolution over all of the crystals in the
     * module.  Calculated in loadCalibration.
     */
    float eres_comm_avg;
    /*!
     * The minimum common energy resolution over all of the crystals in the
     * module.  Calculated in loadCalibration.
     */
    float eres_comm_min;
    /*!
     * The maximum common energy resolution over all of the crystals in the
     * module.  Calculated in loadCalibration.
     */
    float eres_comm_max;
};

/*!
 * Contains all of the information about the individual module and it's testing
 * information.
 */
struct ModuleConfig {
    std::string name;
    float bias_voltage;
    float leakage_current;
    float system_temperature;
    float system_bias_resistor;
    float module_test_voltage;
    float module_test_current;
    float module_test_temperature;
    float module_test_bias_resistor;
    ModuleChannelConfig channel_settings;
    /*!
     * The average photopeak position over all of the crystals in the module.
     * Calculated in loadCalibration.
     */
    float gain_spat_avg;
    /*!
     * The minimum photopeak position over all of the crystals in the module.
     * Calculated in loadCalibration.
     */
    float gain_spat_min;
    /*!
     * The maximum photopeak position over all of the crystals in the module.
     * Calculated in loadCalibration.
     */
    float gain_spat_max;
    /*!
     * The average spatial energy resolution over all of the crystals in the
     * module.  Calculated in loadCalibration.
     */
    float eres_spat_avg;
    /*!
     * The minimum spatial energy resolution over all of the crystals in the
     * module.  Calculated in loadCalibration.
     */
    float eres_spat_min;
    /*!
     * The maximum spatial energy resolution over all of the crystals in the
     * module.  Calculated in loadCalibration.
     */
    float eres_spat_max;
    ApdConfig apd_configs[2];
};

/*!
 * Contains all configuration information related to that fin.
 */
struct FinConfig {
    /// If true, exclude the thermistors value from the panel temperature calc.
    bool exclude_thermistor_value;
};

/*!
 * Configuration information for the Hv Floating boards stored in the crate.
 */
struct HvFloatingBoardConfig {
    std::string usb_port_name;
    int usb_port_baud_rate;
};

/*!
 * \brief runtime information about the front end FPGAs.
 */
struct FrontendFpgaConfig {
    //! Whether the rena should readout information from its renas
    bool readout_enable;
    //! Enabling overrides the coincidence logic between the panels aka singles
    bool coinc_override;
    //! Forces the rena to trigger on noise
    bool force_trigger;
    //! Debug only, readout the triggers of individual channels not the ct
    bool read_triggers_not_timestamps;
};

/*!
 * Configuration information for the backend board that reads out four daq
 * boards.
 */
struct BackendBoardConfig {
    //! The delay that should be applied to the coincidence logic signal input
    int input_delay;
    /*!
     * The delay that should be applied to the board's  coincidence logic
     * output
     */
    int output_delay;
    /*!
     * The length, in coarse timestamp ticks, that should be used for the board.
     */
    int coinc_window;
    //! Flag for if the board's ethernet port is being read out
    bool ethernet_readout;
    //! The ID that the board is hardwired to send packets out with.
    int daqboard_id;
    //! The name of the port on the computer that the board's connected to.
    std::string port_name;
};

/*!
 * Contains the information relevant to the particular panel in the cartridge,
 * including the configuration of the backend board for that cartridge.
 */
struct CartridgeConfig {
    BackendBoardConfig backend_board_config;
    //! The bias voltage that the cartridge should be assumed to be set at.
    float bias_voltage;
    //! The hv channel on the crate to which the cartrige is connected.
    int hv_power_supply_channel;
    //! The lv channel on the crate to which the cartrige is connected.
    int lv_power_supply_channel;
    //! The id of the hv floating board to which the cartridge is connected.
    int hv_floating_board_id;
    //! The sloat on the hv floating board to which the cartridge is connected.
    int hv_floating_board_slot;
};

/*!
 * Contains configuration information relevant to the entire panel.
 */
struct PanelConfig {
    std::string usb_port_name;
    int usb_port_baud_rate;
};

/*!
 * The class into which json configuration file and calibration files are loaded
 * into.  This is used to decode and calibrated the events coming from the
 * system.
 */
class SystemConfiguration {
public:
    SystemConfiguration(const std::string & filename = "");

    int load(const std::string & filename);

    int loadPedestals(const std::string & filename);
    int loadUVCenters(const std::string & filename);
    int loadCalibration(const std::string & filename);
    int loadTimeCalibration(const std::string & filename);

    int lookupPanelCartridge(
            int backend_address,
            int & panel,
            int & cartridge) const;

    int convertPCDRMtoPCFM(
            int panel,
            int cartridge,
            int daq,
            int rena,
            int rena_local_module,
            int & fin,
            int & module) const;

    int convertPCFMtoPCDRM(
            int panel,
            int cartridge,
            int fin,
            int module,
            int & daq,
            int & rena,
            int & rena_local_module) const;

    int createChannelMap();

    /*!
     * \brief Check if pedestals have been loaded
     *
     * \return bool flag indicating if pedestals have been loaded
     */
    bool pedestalsLoaded() const {
        return(pedestals_loaded_flag);
    }

    /*!
     * \brief Check if calibration settings have been loaded
     *
     * \return bool flag indicating if calibration settings have been loaded
     */
    bool calibrationLoaded() const {
        return(calibration_loaded_flag);
    }

    /*!
     * \brief Check if the uv circle centers have been loaded
     *
     * \return bool flag indicating if uv circle centers have been loaded
     */
    bool uvCentersLoaded() const {
        return(uv_centers_loaded_flag);
    }

    /*!
     * \brief Check if the time offset calibration has been loaded
     *
     * \return bool indicating if the time offset calibration has been loaded
     */
    bool timeCalibrationLoaded() const {
        return(time_calibration_loaded_flag);
    }

    //! The number of panels in the system (should always be 2)
    int panels_per_system;
    //! The number of cartridges in each panel of the system
    int cartridges_per_panel;
    //! The number of daq boards (a.k.a four-up-boards) in each cartridge
    int daqs_per_cartridge;
    /*!
     * The number of renas on each daq board.  Should be 8 unless the module
     * testing setup is being used, where it is 2.
     */
    int renas_per_daq;
    //! The number of modules connected to each rena chip (should be 4)
    int modules_per_rena;
    //! The number of fins per cartridge.  Should be 8, 1 if module testing.
    int fins_per_cartridge;
    //! The number of modules connected to each fin (system design is 16)
    int modules_per_fin;
    //! The number of PSAPDS in each module.  Will always be 2
    int apds_per_module;
    //! The number of crystals on each PSAPD.  Will always be 64
    int crystals_per_apd;
    /*!
     * Number of ethernet ports used to read out each panel.  Can be 1 to
     * cartridges_per_panel.
     */
    int ethernets_per_panel;
    //! The number of hv floating boards in the system. 3 slots each.
    int hv_floating_boards_per_system;
    /*!
     * The number of micro controllers on the discrete boards that are daisy-
     * chained together to collect slow control information.
     * \note The daisy chain extends between cartridges.
     */
    int scmicros_per_cartridge;
    //! The number of modules controlled by each chip on the hv bias board.
    int modules_per_dac;
    //! The number of channels on each rena
    int channels_per_rena;
    //! The number of renas associated with a given front FPGA (2)
    int renas_per_fpga;
    //! The number of front end FPGAs on each daq board
    int fpgas_per_daq;

    //! Array index by panel id holding the panel config information.
    std::vector<PanelConfig> panel_configs;
    //! Array indexed Panel, Cartridge holding the config for each cartridge.
    std::vector<std::vector<CartridgeConfig> > cartridge_configs;
    /*!
     * Array indexed Panel, Cartridge, Fin holding the configuration for each
     * fin.
     */
    std::vector<std::vector<std::vector<FinConfig> > > fin_configs;
    /*!
     * Array indexed Panel, Cartridge, Fin, Module holding the configuration for
     * each module.
     */
    std::vector<std::vector<std::vector<
            std::vector<ModuleConfig> > > > module_configs;

    std::vector<HvFloatingBoardConfig> hv_volting_board_configs;

    /*!
     * Array indexed Panel, Cartridge, DAQ_Board, Rena, Trigger Code that
     * holds the expected packet size with that combination of rena and trigger
     * code.
     */
    std::vector<std::vector<std::vector<
            std::vector<std::vector<int> > > > > packet_size;
    /*!
     * Array indexed Panel, Cartridge, DAQ_Board, Rena, Trigger Code, Module
     * holding the ADC value location information for each module and trigger
     * code combination.
     */
    std::vector<std::vector<std::vector<
            std::vector<std::vector<std::vector<
            ADCValueLocation> > > > > > adc_value_locations;

    /*!
     * Array indexed Panel, Cartridge, DAQ_Board, Rena, Module holding
     * the pedestal information for each module.
     */
    std::vector<std::vector<std::vector<
            std::vector<std::vector<ModulePedestals> > > > > pedestals;

    /*!
     * Array indexed Panel, Cartridge, Fin, Module, APD, Crystal holding
     * the calibration information for each crystal.
     */
    std::vector<std::vector<std::vector<
            std::vector<std::vector<std::vector<
            CrystalCalibration> > > > > > calibration;

    /*!
     * Array index  Panel, Cartridge, DAQ_Board, Rena, Channel holding pointers
     * to the appropriate channel settings structure
     */
    std::vector<std::vector<std::vector<std::vector<std::vector<
            RenaChannelConfig *> > > > > channel_map;

    /*!
     * Array index  Panel, Cartridge, DAQ_Board, FPGA holding the runtime
     * information for each front end FPGA
     */
    std::vector<std::vector<std::vector<std::vector<
            FrontendFpgaConfig> > > > fpga_configs;

    //! configuration for the unused channels on the RENA
    RenaChannelConfig unused_channel_config;

private:
    bool backend_address_valid[32];
    int backend_address_panel_lookup[32];
    int backend_address_cartridge_lookup[32];
    bool pedestals_loaded_flag;
    bool calibration_loaded_flag;
    bool uv_centers_loaded_flag;
    bool time_calibration_loaded_flag;
};

#endif // SYSTEM_CONFIGURATION_H
