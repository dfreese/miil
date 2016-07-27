import numpy as np
import json
import os
from scipy.sparse import csc_matrix
from scipy.optimize import curve_fit


default_system_shape = [2, 3, 8, 16, 2, 64]
default_panel_sep = 64.262
default_x_crystal_pitch = 1.0
default_y_crystal_pitch = 1.0
default_z_crystal_pitch = 1.0
default_x_module_pitch = 0.405 * 25.4
default_y_apd_pitch = (0.32 + 0.079) * 25.4
default_y_apd_offset = 1.51
default_z_pitch = 0.0565 * 25.4

ped_dtype = np.dtype([
        ('name', 'S32'),
        ('events', int),
        ('spatA', float),
        ('spatA_std', float),
        ('spatB', float),
        ('spatB_std', float),
        ('spatC', float),
        ('spatC_std', float),
        ('spatD', float),
        ('spatD_std', float),
        ('comL0', float),
        ('comL0_std', float),
        ('comH0', float),
        ('comH0_std', float),
        ('comL1', float),
        ('comL1_std', float),
        ('comH1', float),
        ('comH1_std', float)])

uv_dtype = np.dtype([
        ('u', float),
        ('v', float)])

loc_dtype = np.dtype([
        ('use', bool),
        ('x', float),
        ('y', float)])

cal_dtype = np.dtype([
        ('use', bool),
        ('x', float),
        ('y', float),
        ('gain_spat', float),
        ('gain_comm', float),
        ('eres_spat', float),
        ('eres_comm', float)])

tcal_dtype = np.dtype([
        ('offset', float),
        ('edep_offset', float)])

eventraw_dtype = np.dtype([
        ('ct', np.int64),
        ('com0', np.int16),
        ('com1', np.int16),
        ('com0h', np.int16),
        ('com1h', np.int16),
        ('u0', np.int16),
        ('v0', np.int16),
        ('u1', np.int16),
        ('v1', np.int16),
        ('u0h', np.int16),
        ('v0h', np.int16),
        ('u1h', np.int16),
        ('v1h', np.int16),
        ('a', np.int16),
        ('b', np.int16),
        ('c', np.int16),
        ('d', np.int16),
        ('panel', np.int8),
        ('cartridge', np.int8),
        ('daq', np.int8),
        ('rena', np.int8),
        ('module', np.int8),
        ('flags', np.int8, (3,))],
        align=True)

eventcal_dtype = np.dtype([
        ('ct', np.int64),
        ('ft', np.float32),
        ('E', np.float32),
        ('spat_total', np.float32),
        ('x', np.float32),
        ('y', np.float32),
        ('panel', np.int8),
        ('cartridge', np.int8),
        ('fin', np.int8),
        ('module', np.int8),
        ('apd', np.int8),
        ('crystal', np.int8),
        ('daq', np.int8),
        ('rena', np.int8),
        ('flags', np.int8, (4,))],
        align=True)

eventcoinc_dtype = np.dtype([
        ('ct0', np.int64),
        ('dtc', np.int64),
        ('ft0', np.float32),
        ('dtf', np.float32),
        ('E0', np.float32),
        ('E1', np.float32),
        ('spat_total0', np.float32),
        ('spat_total1', np.float32),
        ('x0', np.float32),
        ('x1', np.float32),
        ('y0', np.float32),
        ('y1', np.float32),
        ('cartridge0', np.int8),
        ('cartridge1', np.int8),
        ('fin0', np.int8),
        ('fin1', np.int8),
        ('module0', np.int8),
        ('module1', np.int8),
        ('apd0', np.int8),
        ('apd1', np.int8),
        ('crystal0', np.int8),
        ('crystal1', np.int8),
        ('daq0', np.int8),
        ('daq1', np.int8),
        ('rena0', np.int8),
        ('rena1', np.int8),
        ('flags', np.int8, (2,))],
        align=True)

cudarecon_type0_dtype = np.dtype([
        ('x0', np.float32),
        ('y0', np.float32),
        ('z0', np.float32),
        ('dt', np.float32),
        ('randoms_est', np.float32),
        ('x1', np.float32),
        ('y1', np.float32),
        ('z1', np.float32),
        ('tof_scatter_est', np.float32),
        ('scatter_est', np.float32)],
        align=True)

cudarecon_type0_vec_dtype = np.dtype([
        ('pos0', np.float32, (3,)),
        ('dt', np.float32),
        ('randoms_est', np.float32),
        ('pos1', np.float32, (3,)),
        ('tof_scatter_est', np.float32),
        ('scatter_est', np.float32)],
        align=True)

cudarecon_type1_dtype = np.dtype([
        ('x0', np.float32),
        ('y0', np.float32),
        ('z0', np.float32),
        ('weight', np.float32),
        ('e0', np.float32), # Appears to be unused
        ('x1', np.float32),
        ('y1', np.float32),
        ('z1', np.float32),
        ('weight1', np.float32), # Appears to be unused
        ('e1', np.float32)], # Appears to be unused
        align=True)

cudarecon_type1_vec_dtype = np.dtype([
        ('pos0', np.float32, (3,)),
        ('weight', np.float32),
        ('e0', np.float32),  # Appears to be unused
        ('pos1', np.float32, (3,)),
        ('weight1', np.float32),  # Appears to be unused
        ('e1', np.float32)],  # Appears to be unused
        align=True)

cuda_vox_file_header_dtype = np.dtype([
        ('magic_number', np.int32),
        ('version_number', np.int32),
        ('size', np.int32, (3,))],
        align=True)

def load_cuda_vox(filename, return_header=False):
    '''
    Takes a cuda vox image file and loads it into a (X,Y,Z) shaped numpy array.
    if return_header is True, then a header object is returned with a
    cuda_vox_file_header_dtype.

    Assumes image was written as [x][z][y] on disk as float32 values.
    '''
    fid = open(filename, 'rb')
    header = np.fromfile(fid, dtype=cuda_vox_file_header_dtype, count=1)[0]
    image = np.fromfile(fid, dtype=np.float32)
    fid.close()
    image = np.reshape(image, (header['size'][0],
            header['size'][2], header['size'][1]))
    image = np.swapaxes(image, 1, 2)
    if return_header:
        return image, header
    else:
        return image

def write_cuda_vox(image, filename, magic_number=65531, version_number=1):
    '''
    Takes a cuda vox image in the form of a numpy array shaped (X,Y,Z) and
    writes it to disk in the cuda vox image header format (see
    cuda_vox_file_header_dtype).  Header defaults are based on projection
    defaults.

    Writes the image as [x][z][y] on the disk.  Values are written as
    float32.
    '''
    header = np.zeros((1,), dtype=cuda_vox_file_header_dtype)
    header['magic_number'] = magic_number
    header['version_number'] = version_number
    header['size'] = image.shape
    with open(filename, 'wb') as fid:
        header.tofile(fid)
        image.swapaxes(1,2).astype(np.float32).tofile(fid)

def load_amide(filename, size):
    '''
    Loads an amide image file of size (X, Y, Z).

    Assumes image was written as [z][y][x] on disk.  Amide format has no header
    information, so the image size must be known.  Assumes values are written as
    float32.
    '''
    size_zyx = (size[2], size[1], size[0])
    return np.fromfile(filename,
            dtype=np.float32).reshape(size[::-1]).swapaxes(0,2)

def write_amide(image, filename):
    '''
    Writes an amide image file of size (X, Y, Z) in [z][y][x] order as float32.
    Amide format has no header information, so the image size must be known.
    '''
    image.swapaxes(0,2).astype(np.float32).tofile(filename)

def load_decoded(filename, count = -1):
    '''
    Load a decode file.  This is a binary file of eventraw_dtype objects.  If
    count is -1 all events will be loaded, otherwise count events will be
    loaded.
    '''
    with open(filename, 'rb') as fid:
        data = np.fromfile(fid, dtype=eventraw_dtype, count=count)
    return data

def load_calibrated(filename, count = -1):
    '''
    Load a calibrate file.  This is a binary file of eventcal_dtype objects.  If
    count is -1 all events will be loaded, otherwise count events will be
    loaded.
    '''
    with open(filename, 'rb') as fid:
        data = np.fromfile(fid, dtype=eventcal_dtype, count=count)
    return data

def load_coincidence(filename, count = -1):
    '''
    Load a calibrate file.  This is a binary file of eventcoinc_dtype objects.
    If count is -1 all events will be loaded, otherwise count events will be
    loaded.
    '''
    with open(filename, 'rb') as fid:
        data = np.fromfile(fid, dtype=eventcoinc_dtype, count=count)
    return data

def get_filenames_from_filelist(filename):
    '''
    Helper function to load in filelist.  Corrects file paths as relative to the
    filelist if the paths that are listed are not absolute.

    Takes the path of the filelist.

    Returns a list of corrected filenames.
    '''
    # Get all of the lines out of the file
    with open(filename, 'r') as f:
        files = f.read().splitlines()
    # Get the directory of the filelist
    filename_path = os.path.dirname(filename)

    # Assume each line in the coinc filelist is either an absolute directory or
    # referenced to the directory of the file.
    full_files = []
    for f in files:
        if os.path.isabs(f):
            full_files.append(f)
        else:
            if not filename_path:
                full_files.append(f)
            else:
                full_files.append(filename_path + '/' + f)
    # Now we have a list of files fully corrected relative to their filelist
    return full_files

def load_decoded_filelist(filename, count = -1):
    '''
    Call load_decoded for every file in the given filelist.  count decode
    events are loaded from each file in the filelist.
    '''
    files = get_filenames_from_filelist(filename)
    data = np.hstack([load_decoded(f, count) for f in files])
    return data

def load_calib_filelist(filename, count = -1):
    '''
    Call load_calibrated for every file in the given filelist.  count calibrate
    events are loaded from each file in the filelist.
    '''
    files = get_filenames_from_filelist(filename)
    data = np.hstack([load_calibrated(f, count) for f in files])
    return data

def load_coinc_filelist(filename, count = -1):
    '''
    Call load_coincidence for every file in the given filelist.  count coinc
    events are loaded from each file in the filelist.
    '''
    files = get_filenames_from_filelist(filename)
    data = np.hstack([load_coincidence(f, count) for f in files])
    return data

def load_pedestals(filename):
    '''
    Loads a pedestal file.  Should be a text file with space separated columns
    for each value in ped_dtype.
    '''
    return np.loadtxt(filename, dtype=ped_dtype)

def load_locations(filename):
    '''
    Loads a crystal location file (typically .loc).  Should be a text file with
    space separated columns for each value in loc_dtype.
    '''
    return np.loadtxt(filename, dtype=loc_dtype)

def write_locations(cal, filename):
    '''
    Writes a crystal location file (typically .loc) to filename.
    '''
    return np.savetxt(filename, cal, '%d %0.6f %0.6f')

def load_calibration(filename):
    '''
    Loads a crystal calibraiton file (typically .cal).  Should be a text file
    with space separated columns for each value in cal_dtype.
    '''
    return np.loadtxt(filename, dtype=cal_dtype)

def write_calibration(cal, filename):
    '''
    Writes a crystal calibration file (typically .cal) to filename.  Writes the
    following columns

    - use %d
    - x %0.6f
    - y %0.6f
    - gain_spat %0.0f
    - gain_comm %0.0f
    - eres_spat %0.4f
    - eres_comm %0.4f
    '''
    return np.savetxt(filename, cal, '%d %0.6f %0.6f %0.0f %0.0f %0.4f %0.4f')

def load_time_calibration(filename):
    '''
    Loads a crystal time calibraiton file (typically .tcal).  Should be a text
    file with space separated columns for each value in tcal_dtype.
    '''
    return np.loadtxt(filename, dtype=tcal_dtype)

def load_system_shape_pcfmax(filename):
    '''
    From a json system configuration file, load the PCFMAX shape of the system.
    Assumes that there are 2 apds per module and 64 crystals per apd, which is
    fixed in hardware.
    '''
    with open(filename, 'r') as f:
        config = json.load(f)
    system_config = config['system_config']
    system_shape = [system_config['NUM_PANEL_PER_DEVICE'],
                    system_config['NUM_CART_PER_PANEL'],
                    system_config['NUM_FIN_PER_CARTRIDGE'],
                    system_config['NUM_MODULE_PER_FIN'],
                    2, 64,]
    return system_shape

def load_uv_freq(filename):
    '''
    From a json system configuration file, load the uv_frequency.
    '''
    with open(filename, 'r') as f:
        config = json.load(f)
    return config['uv_frequency']

def load_uv_period(filename):
    '''
    Use load_uv_freq to calculate the uv period from a json system configuration
    file.
    '''
    uv_freq = load_uv_freq(filename)
    return 1.0 / uv_freq

# For Calibrated Events
def get_global_cartridge_number(events, system_shape=default_system_shape):
    '''
    Takes eventcal_dtype events and returns the global catridge number for each
    event given system_shape.
    '''
    global_cartridge = events['cartridge'].astype(int) + \
                       system_shape[1] * events['panel'].astype(int)
    return global_cartridge

def get_global_fin_number(events, system_shape=default_system_shape):
    '''
    Takes eventcal_dtype events and returns the global fin number for each
    event given system_shape.  Uses get_global_cartridge_number as a base.
    '''
    global_cartridge = get_global_cartridge_number(events, system_shape)
    global_fin = events['fin'] + system_shape[2] * global_cartridge
    return global_fin

def get_global_module_number(events, system_shape=default_system_shape):
    '''
    Takes eventcal_dtype events and returns the global module number for each
    event given system_shape.  Uses get_global_fin_number as a base.
    '''
    global_fin = get_global_fin_number(events, system_shape)
    global_module = events['module'] + system_shape[3] * global_fin
    return global_module

def get_global_apd_number(events, system_shape=default_system_shape):
    '''
    Takes eventcal_dtype events and returns the global apd number for each
    event given system_shape.  Uses get_global_module_number as a base.
    '''
    global_module = get_global_module_number(events, system_shape)
    global_apd = events['apd'] + system_shape[4] * global_module
    return global_apd

def get_global_crystal_number(events, system_shape=default_system_shape):
    '''
    Takes eventcal_dtype events and returns the global crystal number for each
    event given system_shape.  Uses get_global_apd_number as a base.
    '''
    global_apd = get_global_apd_number(events, system_shape)
    global_crystal = events['crystal'] + system_shape[5] * global_apd
    return global_crystal

# For Coincidence Events
def get_global_cartridge_numbers(events, system_shape=default_system_shape):
    '''
    Takes eventcoinc_dtype events and returns the left and right global
    cartridge number for each event given system_shape.
    '''
    global_cartridge0 = events['cartridge0'].astype(int)
    global_cartridge1 = events['cartridge1'].astype(int) + system_shape[1]
    return global_cartridge0, global_cartridge1

def get_global_fin_numbers(events, system_shape=default_system_shape):
    '''
    Takes eventcoinc_dtype events and returns the left and right global
    fin number for each event given system_shape.  Uses
    get_global_cartridge_numbers as a base.
    '''
    global_cartridge0, global_cartridge1 = \
            get_global_cartridge_numbers(events, system_shape)
    global_fin0 = events['fin0'] + system_shape[2] * global_cartridge0
    global_fin1 = events['fin1'] + system_shape[2] * global_cartridge1
    return global_fin0, global_fin1

def get_global_module_numbers(events, system_shape=default_system_shape):
    '''
    Takes eventcoinc_dtype events and returns the left and right global
    module number for each event given system_shape.  Uses
    get_global_fin_numbers as a base.
    '''
    global_fin0, global_fin1 = get_global_fin_numbers(events, system_shape)
    global_module0 = events['module0'] + system_shape[3] * global_fin0
    global_module1 = events['module1'] + system_shape[3] * global_fin1
    return global_module0, global_module1

def get_global_apd_numbers(events, system_shape=default_system_shape):
    '''
    Takes eventcoinc_dtype events and returns the left and right global
    apd number for each event given system_shape.  Uses
    get_global_module_numbers as a base.
    '''
    global_module0, global_module1 = \
            get_global_module_numbers(events, system_shape)
    global_apd0 = events['apd0'] + system_shape[4] * global_module0
    global_apd1 = events['apd1'] + system_shape[4] * global_module1
    return global_apd0, global_apd1

def get_global_crystal_numbers(events, system_shape=default_system_shape):
    '''
    Takes eventcoinc_dtype events and returns the left and right global
    crystal number for each event given system_shape.  Uses
    get_global_apd_numbers as a base.
    '''
    global_apd0, global_apd1 = get_global_apd_numbers(events, system_shape)
    global_crystal0 = events['crystal0'] + system_shape[5] * global_apd0
    global_crystal1 = events['crystal1'] + system_shape[5] * global_apd1
    return global_crystal0, global_crystal1

def get_global_lor_number(events, system_shape=default_system_shape):
    '''
    Takes eventcoinc_dtype events and returns the global lor number for each
    event given system_shape.  Uses get_global_crystal_numbers as a base.
    Global lor calculated as:
        (global_crystal0 * no_crystals_per_panel) +
        (global_crystal1 - no_crystals_per_panel)
    '''
    global_crystal0, global_crystal1 = \
            get_global_crystal_numbers(events, system_shape)
    no_crystals_per_panel = np.prod(system_shape[1:])

    return (global_crystal0 * no_crystals_per_panel) + \
           (global_crystal1 - no_crystals_per_panel)

def get_crystals_from_lor(lors, system_shape=default_system_shape):
    '''
    Takes an array of lor indices and returns the left and right global crystal
    number based on the given system shape.
    '''
    crystal0 = lors // np.prod(system_shape[1:])
    crystal1 = lors % np.prod(system_shape[1:]) + np.prod(system_shape[1:])
    return crystal0, crystal1

def get_apds_from_lor(lors, system_shape=default_system_shape):
    '''
    Takes an array of lor indices and returns the left and right global apd
    number based on the given system shape.  Uses get_crystals_from_lor as a
    base for calculation.
    '''
    crystal0, crystal1 = get_crystals_from_lor(lors, system_shape)
    apd0 = crystal0 // system_shape[5]
    apd1 = crystal1 // system_shape[5]
    return apd0, apd1

def get_modules_from_lor(lors, system_shape=default_system_shape):
    '''
    Takes an array of lor indices and returns the left and right global module
    number based on the given system shape.  Uses get_apds_from_lor as a base
    for calculation.
    '''
    apd0, apd1 = get_apds_from_lor(lors, system_shape)
    module0 = apd0 // system_shape[4]
    module1 = apd1 // system_shape[4]
    return module0, module1

def tcal_coinc_events(
        events, tcal,
        system_shape = default_system_shape,
        uv_period_ns = 1024.41,
        breast_daq_json_config = None):
    '''
    Takes an array of eventcoinc_dtype events and applies a time calibration,
    making sure the dtf are then wrapped to the uv period in nanoseconds.  The
    default uv period represent 980kHz in nanoseconds.

    If tcal is a string, load_time_calibration will be called to load the time
    calibration.

    Returned array is a copy of the original array.
    '''

    # If tcal is a string, load in that time calibration first.
    if type(tcal) is str:
        tcal = load_time_calibration(filename)

    idx0, idx1 = get_global_crystal_numbers(events, system_shape)
    ft0_offset = tcal['offset'][idx0] + \
                 tcal['edep_offset'][idx0] * (events['E0'] - 511)
    ft1_offset = tcal['offset'][idx1] + \
                 tcal['edep_offset'][idx1] * (events['E1'] - 511)
    cal_events = events.copy()
    cal_events['dtf'] -= ft0_offset
    cal_events['dtf'] += ft1_offset

    # Load in the uv_period_ns from a json file if specified.
    if breast_daq_json_config is not None:
        uv_period_ns = load_uv_period(breast_daq_json_config) * 1e9

    # wrap all of the events to the specified uv period.
    while np.any(cal_events['dtf'] > uv_period_ns):
        cal_events['dtf'][cal_events['dtf'] > uv_period_ns] -= uv_period_ns
    while np.any(cal_events['dtf'] < -uv_period_ns):
        cal_events['dtf'][cal_events['dtf'] < -uv_period_ns] += uv_period_ns

    return cal_events

def force_array(x, dtype=None):
    '''
    Helper function to force a value to be represented as a numpy array whether
    or not it is a scalar, list, or numpy array type.
    '''
    if np.isscalar(x):
        x = (x,)
    return np.asarray(x, dtype=dtype)

def get_position_pcfmax(
        panel, cartridge, fin, module, apd, crystal,
        system_shape = default_system_shape,
        panel_sep = default_panel_sep,
        x_crystal_pitch = default_x_crystal_pitch,
        y_crystal_pitch = default_y_crystal_pitch,
        x_module_pitch = default_x_module_pitch,
        y_apd_pitch = default_y_apd_pitch,
        y_apd_offset = default_y_apd_offset,
        z_pitch = default_z_pitch):
    '''
    Calculates the position of a crystal based upon it's PCFMAX number.
    '''

    panel = force_array(panel, dtype = float)
    cartridge = force_array(cartridge, dtype = float)
    fin = force_array(fin, dtype = float)
    module = force_array(module, dtype = float)
    apd = force_array(apd, dtype = float)
    crystal = force_array(crystal, dtype = float)

    if np.any(panel >= system_shape[0]):
        raise ValueError('panel value out of range')
    if np.any(cartridge >= system_shape[1]):
        raise ValueError('cartridge value out of range')
    if np.any(fin >= system_shape[2]):
        raise ValueError('fin value out of range')
    if np.any(module >= system_shape[3]):
        raise ValueError('module value out of range')
    if np.any(apd >= system_shape[4]):
        raise ValueError('apd value out of range')
    if np.any(crystal >= system_shape[5]):
        raise ValueError('crystal value out of range')

    positions = np.zeros((len(panel), 3), dtype=float)

    positions[:, 0] = (x_module_pitch - 8 * x_crystal_pitch) / 2 + \
        (module - 8) * x_module_pitch

    positions[panel == 0, 0] += x_crystal_pitch * \
            ((crystal[panel == 0] // 8) + 0.5)

    positions[panel == 1, 0] += x_crystal_pitch * \
            (7 - (crystal[panel == 1] // 8) +
             0.5)

    positions[:, 1] = panel_sep / 2.0 + y_apd_offset + \
        apd * y_apd_pitch + \
        (7 - crystal % 8 + 0.5) * y_crystal_pitch

    positions[:, 2] = z_pitch * (0.5 + fin +
            system_shape[2] * (cartridge - system_shape[1] / 2.0))

    positions[panel == 0, 1] *= -1

    return positions

def get_position_global_crystal(
        global_crystal_ids,
        system_shape = default_system_shape,
        panel_sep = default_panel_sep,
        x_crystal_pitch = default_x_crystal_pitch,
        y_crystal_pitch = default_y_crystal_pitch,
        x_module_pitch = default_x_module_pitch,
        y_apd_pitch = default_y_apd_pitch,
        y_apd_offset = default_y_apd_offset,
        z_pitch = default_z_pitch):
    if np.isscalar(global_crystal_ids):
        global_crystal_ids = (global_crystal_ids,)
    global_crystal_ids = np.asarray(global_crystal_ids, dtype=float)
    '''
    Get the crystal position based on the global crystal id number.  Does this
    by calling get_position_pcfmax.

    Parameters
    ----------
    global_crystal_ids : scalar or (n,) shaped ndarray
        Scalar or array of globabl crystal ids

    Returns
    -------
    p : (n,3) array
        x, y, z positions of the crystals.
    '''
    if np.any(global_crystal_ids >= np.prod(system_shape)):
        raise ValueError('One or more crystal ids are out of range')
    elif np.any(global_crystal_ids < 0):
        raise ValueError('One or more crystal ids are out of range')

    panel = global_crystal_ids // np.prod(system_shape[1:])
    cartridge = global_crystal_ids // np.prod(system_shape[2:]) % system_shape[1]
    fin = global_crystal_ids // np.prod(system_shape[3:]) % system_shape[2]
    module = global_crystal_ids // np.prod(system_shape[4:]) % system_shape[3]
    apd = global_crystal_ids // np.prod(system_shape[5:]) % system_shape[4]
    crystal = global_crystal_ids % system_shape[5]

    return get_position_pcfmax(panel, cartridge, fin, module, apd, crystal,
            system_shape, panel_sep, x_crystal_pitch, y_crystal_pitch,
            x_module_pitch, y_apd_pitch, y_apd_offset, z_pitch)

def get_positions_cal(
        events,
        system_shape = default_system_shape,
        panel_sep = default_panel_sep,
        x_crystal_pitch = default_x_crystal_pitch,
        y_crystal_pitch = default_y_crystal_pitch,
        x_module_pitch = default_x_module_pitch,
        y_apd_pitch = default_y_apd_pitch,
        y_apd_offset = default_y_apd_offset,
        z_pitch = default_z_pitch):
    '''
    Get the crystal position based on the eventcal_dtype event.  Does this
    by calling get_position_pcfmax.

    Parameters
    ----------
    events : (n,) shaped ndarray of eventcal_dtype
        Scalar or array of calibrated events

    Returns
    -------
    p : (n,3) array
        x, y, z positions of the crystals.
    '''
    pos = get_position_pcfmax(
            events['panel'], events['cartridge'], events['fin'],
            events['module'], events['apd'], events['crystal'],
            system_shape, panel_sep, x_crystal_pitch, y_crystal_pitch,
            x_module_pitch, y_apd_pitch, y_apd_offset, z_pitch)
    return pos

def get_crystal_pos(
        events,
        system_shape = default_system_shape,
        panel_sep = default_panel_sep,
        x_crystal_pitch = default_x_crystal_pitch,
        y_crystal_pitch = default_y_crystal_pitch,
        x_module_pitch = default_x_module_pitch,
        y_apd_pitch = default_y_apd_pitch,
        y_apd_offset = default_y_apd_offset,
        z_pitch = default_z_pitch):
    '''
    Get the left and right crystal position based on the eventcoinc_dtype event.
    Does this by calling get_position_pcfmax.

    Parameters
    ----------
    events : (n,) shaped ndarray of eventcoinc_dtype
        Scalar or array of coincidence events

    Returns
    -------
    p0 : (n,3) array
        x, y, z positions of the left crystals.
    p1 : (n,3) array
        x, y, z positions of the right crystals.
    '''
    pos0 = get_position_pcfmax(
            np.zeros(events.shape), events['cartridge0'], events['fin0'],
            events['module0'], events['apd0'], events['crystal0'],
            system_shape, panel_sep, x_crystal_pitch, y_crystal_pitch,
            x_module_pitch, y_apd_pitch, y_apd_offset, z_pitch)

    pos1 = get_position_pcfmax(
            np.ones(events.shape), events['cartridge1'], events['fin1'],
            events['module1'], events['apd1'], events['crystal1'],
            system_shape, panel_sep, x_crystal_pitch, y_crystal_pitch,
            x_module_pitch, y_apd_pitch, y_apd_offset, z_pitch)

    return pos0, pos1

def get_lor_positions(
        lors,
        system_shape = default_system_shape,
        panel_sep = default_panel_sep,
        x_crystal_pitch = default_x_crystal_pitch,
        y_crystal_pitch = default_y_crystal_pitch,
        x_module_pitch = default_x_module_pitch,
        y_apd_pitch = default_y_apd_pitch,
        y_apd_offset = default_y_apd_offset,
        z_pitch = default_z_pitch):
    '''
    Get the left and right crystal position based on the lor index.
    Does this by calling get_position_pcfmax.

    Parameters
    ----------
    lors : (n,) shaped ndarray
        Scalar or array of lor indices

    Returns
    -------
    p0 : (n,3) array
        x, y, z positions of the left crystals.
    p1 : (n,3) array
        x, y, z positions of the right crystals.
    '''
    crystal0, crystal1 = get_crystals_from_lor(lors, system_shape)
    line_start = get_position_global_crystal(
            crystal0, system_shape, panel_sep, x_crystal_pitch, y_crystal_pitch,
            x_module_pitch, y_apd_pitch, y_apd_offset, z_pitch)
    line_end = get_position_global_crystal(
            crystal1, system_shape, panel_sep, x_crystal_pitch, y_crystal_pitch,
            x_module_pitch, y_apd_pitch, y_apd_offset, z_pitch)
    return line_start, line_end

def create_listmode_data(
        events,
        system_shape = default_system_shape,
        panel_sep = default_panel_sep,
        list_type = 0):
    '''
    Creates an array of list mode data in either cudarecon_type0_vec_dtype or
    cudarecon_type1_vec_dtype from eventcoinc data by calling get_crystal_pos.

    Parameters
    ----------
    events : (n,) shaped ndarray of eventcoinc_dtype
        Scalar or array of coincidence events

    Returns
    -------
    events : (n,) shaped ndarray of cudarecon_type[0,1]_vec_dtype
        Scalar or array of list mode data for cudarecon.
    '''
    dtype = cudarecon_type0_vec_dtype
    if list_type == 1:
        dtype = cudarecon_type1_vec_dtype

    lm_data = np.zeros(events.shape, dtype=dtype)
    lm_data['pos0'], lm_data['pos1'] = get_crystal_pos(
            events, system_shape=system_shape, panel_sep=panel_sep)
    return lm_data

def create_listmode_from_vec(
        vec, panel_sep = default_panel_sep,
        system_shape = default_system_shape):
    '''
    Creates an array of list mode data in cudarecon_type1_vec_dtype from
    a sparse column vector representing the counts on each lor.

    Parameters
    ----------
    vec : (n,1) csc_matrix
        sparse column matrix of lor counts

    Returns
    -------
    events : (n,) shaped ndarray of cudarecon_type1_vec_dtype
        Scalar or array of list mode data for cudarecon.
    '''
    lm_data = np.zeros((vec.nnz,), dtype=cudarecon_type1_vec_dtype)
    lm_data['pos0'], lm_data['pos1'] = get_lor_positions(vec.indices,
                                                         system_shape,
                                                         panel_sep)
    lm_data['weight'] = vec.data.copy()
    return lm_data

def create_listmode_from_lors(
        lors,
        panel_sep = default_panel_sep,
        system_shape = default_system_shape,
        list_type = 0):
    '''
    Creates an array of list mode data in cudarecon_type[0,1]_vec_dtype from
    an array of lor indices.  Calls get_lor_positions.

    Parameters
    ----------
    vec : (n,1) csc_matrix
        sparse column matrix of lor counts
    list_type : scalar
        indicates cudarecon_type.  values 0 and 1 indicate
        cudarecon_type0_vec_dtype and cudarecon_type1_vec_dtype respectively.

    Returns
    -------
    events : (n,) shaped ndarray of cudarecon_type[0,1]_vec_dtype
        Scalar or array of list mode data for cudarecon.
    '''
    dtype = cudarecon_type0_vec_dtype
    if list_type == 1:
        dtype = cudarecon_type1_vec_dtype

    lm_data = np.zeros(lors.shape, dtype=dtype)
    lm_data['pos0'], lm_data['pos1'] = get_lor_positions(
            lors, system_shape, panel_sep)
    return lm_data

def correct_resets(data, threshold=1.0e3):
    '''
    Corrects any accidental resets in the coarse timestamp during a run of the
    system.

    Parameters
    ----------
    data : (n,) ndarray of eventraw_dtype or eventcal_dtype
        Array of raw or calibrated event data
    threshold : scalar
        The negative jump that constitues a reset in the coarse timestamp.
        Should be large enough as to not include events that might be out of
        order.

    Returns
    -------
    data : (n,) shaped ndarray of cudarecon_type[0,1]_vec_dtype
        data array with 'ct' values corrected for resets
    '''
    data['ct'][1:] = np.diff(data['ct'])
    # Assume that any negative should just be the next coarse timestampe tick,
    # so we set it to one, so that the ct is incremented in the cumsum step
    data['ct'][data['ct'] < -threshold] = 1
    data['ct'] = np.cumsum(data['ct'])
    return data

def save_sparse_csc(filename, array):
    '''
    Saves a csc_matrix to a file by writing out the individual 'data',
    'indices', and 'indptr' arrays to a numpy zip file.

    Parameters
    ----------
    filename : str
        String of the file name to which to write the npz file.
    array : csc_matrix
        Any scipy csc_matrix
    '''
    np.savez(filename, data = array.data, indices=array.indices,
             indptr = array.indptr, shape=array.shape)

def load_sparse_csc(filename):
    '''
    Loads a csc_matrix saved by save_sparse_csc.  Assumes filename is a npz file
    containing 'data', 'indices', and 'indptr' arrays.

    Parameters
    ----------
    filename : str
        String of the npz file containing the csc_matrix's arrays.

    Returns
    -------
    data : csc_matrix
        A csc_matrix created from the 'data', 'indices', and 'indptr' arrays.
    '''
    loader = np.load(filename)
    return csc_matrix((loader['data'], loader['indices'], loader['indptr']),
                      shape = loader['shape'])

def create_sparse_column_vector(data, size=None):
    '''
    Creates a sparse column vector using a scipy.csc_matrix.  Takes data to be
    a count for that particular index.  Same indices will be summed in the
    column vector.  The vector will be (1,size) in shape.  If size is None,
    then the maximum index in data determines the size of the vector.  Used
    primarily for lor indices.

    Parameters
    ----------
    data : array
        A list of indices.

    Returns
    -------
    vec : (1, size) shaped csc_matrix
        A csc_matrix created from the indices in data.
    '''
    shape = None
    if size is not None:
        shape=(int(size), 1)
    data.sort()
    return csc_matrix((np.ones((len(data),), dtype=float),
                       (data.astype(int), np.zeros((len(data),), dtype=int))
                      ), shape=shape)

def gauss_function(x, a, mu, sigma):
    '''
    Evaluate a gaussian of mean mu, std of sigma, and amplitude a at a point x.
    Primarily used for fitting histograms.

    Parameters
    ----------
    x : ndarray or scalar
        Points to be evaluated
    a : scalar
        Amplitude of the gaussian
    mu : scalar
        Mean of the gaussian
    sigma : scalar
        Standard deviation of the gaussian

    Returns
    -------
    val : array or scalar
        gauss(x) = a * exp(-(x - mu)**2 / (2 * sigma**2))
    '''
    return a * np.exp(-(x - mu)**2.0 / (2 * sigma**2))

def fit_hist_gauss(n, edges):
    '''
    Takes the output of a histogram and fits a gaussian function to it.
    Scipy curve_fit is uses in combination with gauss_function to do a
    non-linear fit.  The fit is initialize to the mean and variance of the given
    data.

    Parameters
    ----------
    n : ndarray
        Array of bin values from histogramming a set of data.
    edges : ndarray
        Array of bin edges from histogramming a set of data.

    Returns
    -------
    popt : array
        Array with 3 elements [a, mu, sigma] of the optimal fit.

    Examples
    --------
    >>> data = 1.0 + 2 * np.random.randn(1000)
    >>> n, edges, patches = plt.hist(data)
    >>> popt = miil.fit_hist_gauss(n, edges)
    '''
    # find the centers of the bins
    centers = (edges[1:] + edges[:-1])/2.0
    # Then do a weighted average of the centers to initialize the estimate of the mean
    mean = np.average(centers, weights=n)
    # Then do a weighted average to initialize the estimate of the variance
    sigma = np.average((centers-mean)**2, weights=n)
    p0 = [np.max(n), mean, sigma]
    popt, pcov = curve_fit(gauss_function, centers, n, p0=p0)
    return popt

def eval_gauss_over_range(popt, n=100, range=None, edges=None):
    '''
    Evaluates a gaussian function over a range, traditionally from the optput of
    fit_hist_gauss.

    Parameters
    ----------
    popt : array
        Array with 3 elements [a, mu, sigma] of the optimal fit.
    n : scalar
        number of points at which to evaluate the fit
    range : array shape=(2,)
        The min and max of the range over which to evaluate the fit.
    edges : ndarray
        Array of bin edges from histogramming a set of data.  Sets the range
        over which the fit is calculated. Equivalent to
        range=(edges.min(), edges.max()).

    Returns
    -------
    x : array
        points where the fit was evaluated
    y : array
        value of the fit where the it was evaluated

    Examples
    --------
    >>> data = 1.0 + 2 * np.random.randn(1000)
    >>> n, edges, patches = plt.hist(data)
    >>> popt = miil.fit_hist_gauss(n, edges)
    >>> x_fit, y_fit = miil.eval_gauss_over_range(popt, 200, edges=edges)
    >>> plt.plot(x_fit, y_fit)
    >>> plt.show()
    '''
    if edges is not None:
        range_min = edges.min()
        range_max = edges.max()
    if range is not None:
        range_min = range[0]
        range_max = range[1]
    x = np.linspace(range_min, range_max, n)
    y = gauss_function(x, *popt)
    return x, y

def main():
    return

if __name__ == '__main__':
    main()
