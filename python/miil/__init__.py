import numpy as np
import json

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

def load_decoded(filename, count = -1):
    fid = open(filename, 'rb')
    data = np.fromfile(fid, dtype=eventraw_dtype, count=count)
    fid.close()
    return data

def load_calibrated(filename, count = -1):
    fid = open(filename, 'rb')
    data = np.fromfile(fid, dtype=eventcal_dtype, count=count)
    fid.close()
    return data

def load_coincidence(filename, count = -1):
    fid = open(filename, 'rb')
    data = np.fromfile(fid, dtype=eventcoinc_dtype, count=count)
    fid.close()
    return data

def load_decoded_filelist(filename, count = -1):
    fid = open(filename, 'r')
    files = fid.read().splitlines()
    fid.close()
    data = np.hstack([load_decoded(f, count) for f in files])
    return data

def load_calib_filelist(filename, count = -1):
    fid = open(filename, 'r')
    files = fid.read().splitlines()
    fid.close()
    data = np.hstack([load_calibrated(f, count) for f in files])
    return data

def load_coinc_filelist(filename, count = -1):
    fid = open(filename, 'r')
    files = fid.read().splitlines()
    fid.close()
    data = np.hstack([load_coincidence(f, count) for f in files])
    return data

def load_pedestals(filename):
    return np.loadtxt(filename, dtype=ped_dtype)

def load_locations(filename):
    return np.loadtxt(filename, dtype=loc_dtype)

def write_locations(cal, filename):
    return np.savetxt(filename, cal, '%d %0.6f %0.6f')

def load_calibration(filename):
    return np.loadtxt(filename, dtype=cal_dtype)

def write_calibration(cal, filename):
    return np.savetxt(filename, cal, '%d %0.6f %0.6f %0.0f %0.0f %0.4f %0.4f')

def load_time_calibration(filename):
    return np.loadtxt(filename, dtype=tcal_dtype)

def load_system_shape_pcfmax(filename):
    with open(filename, 'r') as f:
        config = json.load(f)
    system_config = config['system_config']
    system_shape = [system_config['NUM_PANEL_PER_DEVICE'],
                    system_config['NUM_CART_PER_PANEL'],
                    system_config['NUM_FIN_PER_CARTRIDGE'],
                    system_config['NUM_MODULE_PER_FIN'],
                    2, 64,]
    return system_shape

# For Calibrated Events
def get_global_cartridge_number(events, system_shape):
    global_cartridge = events['cartridge'].astype(int) + \
                       system_shape[1] * events['panel'].astype(int)
    return global_cartridge

def get_global_fin_number(events, system_shape):
    global_cartridge = get_global_cartridge_number(events, system_shape)
    global_fin = events['fin'] + system_shape[2] * global_cartridge
    return global_fin

def get_global_module_number(events, system_shape):
    global_fin = get_global_fin_number(events, system_shape)
    global_module = events['module'] + system_shape[3] * global_fin
    return global_module

def get_global_apd_number(events, system_shape):
    global_module = get_global_module_number(events, system_shape)
    global_apd = events['apd'] + system_shape[4] * global_module
    return global_apd

def get_global_crystal_number(events, system_shape):
    global_apd = get_global_apd_number(events, system_shape)
    global_crystal = events['crystal'] + system_shape[5] * global_apd
    return global_crystal

# For Coincidence Events
def get_global_cartridge_numbers(events, system_shape):
    global_cartridge0 = events['cartridge0'].astype(int)
    global_cartridge1 = events['cartridge1'].astype(int) + system_shape[1]
    return global_cartridge0, global_cartridge1

def get_global_fin_numbers(events, system_shape):
    global_cartridge0, global_cartridge1 = \
            get_global_cartridge_numbers(events, system_shape)
    global_fin0 = events['fin0'] + system_shape[2] * global_cartridge0
    global_fin1 = events['fin1'] + system_shape[2] * global_cartridge1
    return global_fin0, global_fin1

def get_global_module_numbers(events, system_shape):
    global_fin0, global_fin1 = get_global_fin_numbers(events, system_shape)
    global_module0 = events['module0'] + system_shape[3] * global_fin0
    global_module1 = events['module1'] + system_shape[3] * global_fin1
    return global_module0, global_module1

def get_global_apd_numbers(events, system_shape):
    global_module0, global_module1 = \
            get_global_module_numbers(events, system_shape)
    global_apd0 = events['apd0'] + system_shape[4] * global_module0
    global_apd1 = events['apd1'] + system_shape[4] * global_module1
    return global_apd0, global_apd1

def get_global_crystal_numbers(events, system_shape):
    global_apd0, global_apd1 = get_global_apd_numbers(events, system_shape)
    global_crystal0 = events['crystal0'] + system_shape[5] * global_apd0
    global_crystal1 = events['crystal1'] + system_shape[5] * global_apd1
    return global_crystal0, global_crystal1

def get_global_lor_number(events, system_shape):
    global_crystal0, global_crystal1 = \
            get_global_crystal_numbers(events, system_shape)
    no_crystals_per_panel = np.prod(system_shape[1:])

    return (global_crystal0 * no_crystals_per_panel) + \
           (global_crystal1 - no_crystals_per_panel)

def tcal_coinc_events(events, tcal, system_shape = [2, 3, 8, 16, 2, 64]):
    idx0, idx1 = get_global_crystal_numbers(events, system_shape)
    ft0_offset = tcal['offset'][idx0] + \
                 tcal['edep_offset'][idx0] * (events['E0'] - 511)
    ft1_offset = tcal['offset'][idx1] + \
                 tcal['edep_offset'][idx1] * (events['E1'] - 511)
    cal_events = events.copy()
    # Not correcting ft0 right now, because we'd need to wrap it to the uv
    # period, and I'm being lazy as to not load that in
    # cal_events['ft0'] -= ft0_offset
    cal_events['dtf'] -= ft0_offset
    cal_events['dtf'] += ft1_offset
    return cal_events

def force_array(x, dtype=None):
    if np.isscalar(x):
        x = (x,)
    return np.asarray(x, dtype=dtype)

def get_position_pcfmax(
        panel, cartridge, fin, module, apd, crystal,
        system_shape = [2, 3, 8, 16, 2, 64],
        panel_sep = 64.262,
        x_crystal_pitch = 1.0,
        y_crystal_pitch = 1.0,
        x_module_pitch = 0.405 * 25.4,
        y_apd_pitch = (0.32 + 0.079) * 25.4,
        y_apd_offset = 1.51,
        z_pitch = 0.0565 * 25.4):

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
        system_shape = [2, 3, 8, 16, 2, 64],
        panel_sep = 64.262,
        x_crystal_pitch = 1.0,
        y_crystal_pitch = 1.0,
        x_module_pitch = 0.405 * 25.4,
        y_apd_pitch = (0.32 + 0.079) * 25.4,
        y_apd_offset = 1.51,
        z_pitch = 0.0565 * 25.4):
    if np.isscalar(global_crystal_ids):
        global_crystal_ids = (global_crystal_ids,)
    global_crystal_ids = np.asarray(global_crystal_ids, dtype=float)

    if np.any(global_crystal_ids >= np.prod(system_shape)):
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
        system_shape = [2, 3, 8, 16, 2, 64],
        panel_sep = 64.262,
        x_crystal_pitch = 1.0,
        y_crystal_pitch = 1.0,
        x_module_pitch = 0.405 * 25.4,
        y_apd_pitch = (0.32 + 0.079) * 25.4,
        y_apd_offset = 1.51,
        z_pitch = 0.0565 * 25.4):


    x = (x_module_pitch - 8 * x_crystal_pitch) / 2 + \
        (events['module'].astype(float) - 8) * x_module_pitch

    x[events['panel'] == 0] += x_crystal_pitch * \
            ((events['crystal'][events['panel'] == 0].astype(float) // 8) + 0.5)

    x[events['panel'] == 1] += x_crystal_pitch * \
            (7 - (events['crystal'][events['panel'] == 1].astype(float) // 8) +
             0.5)

    y = panel_sep / 2.0 + y_apd_offset + \
        events['apd'].astype(float) * y_apd_pitch + \
        (7 - events['crystal'].astype(float) % 8 + 0.5) * y_crystal_pitch

    z = z_pitch * (0.5 + events['fin'].astype(float) +
                    system_shape[2] * (events['cartridge'].astype(float) -
                                       system_shape[1] / 2.0))

    y[events['panel'] == 0] *= -1

    return x, y, z

def get_crystal_pos(
        events,
        system_shape = [2, 3, 8, 16, 2, 64],
        panel_sep = 64.262,
        x_crystal_pitch = 1.0,
        y_crystal_pitch = 1.0,
        x_module_pitch = 0.405 * 25.4,
        y_apd_pitch = (0.32 + 0.079) * 25.4,
        y_apd_offset = 1.51,
        z_pitch = 0.0565 * 25.4):

    x0 = (x_module_pitch - 8 * x_crystal_pitch) / 2 + \
         (events['module0'].astype(float) - 8) * x_module_pitch + \
         ((events['crystal0'].astype(float) // 8) + 0.5) * x_crystal_pitch

    x1 = (x_module_pitch - 8 * x_crystal_pitch) / 2 + \
         (events['module1'].astype(float) - 8) * x_module_pitch + \
         (7 - (events['crystal1'].astype(float) // 8) + 0.5) * x_crystal_pitch

    y0 = - panel_sep / 2.0 - y_apd_offset - \
         events['apd0'].astype(float) * y_apd_pitch - \
         (7 - events['crystal0'].astype(float) % 8 + 0.5) * y_crystal_pitch

    y1 = panel_sep / 2.0 + y_apd_offset + \
         events['apd1'].astype(float) * y_apd_pitch + \
         (7 - events['crystal1'].astype(float) % 8 + 0.5) * y_crystal_pitch

    z0 = z_pitch * (0.5 + events['fin0'].astype(float) +
                    system_shape[2] * (events['cartridge0'].astype(float) -
                                       system_shape[1] / 2.0))

    z1 = z_pitch * (0.5 + events['fin1'].astype(float) +
                    system_shape[2] * (events['cartridge1'].astype(float) -
                                       system_shape[1] / 2.0))
    return x0, x1, y0, y1, z0, z1

def create_listmode_data(
        events,
        system_shape = None,
        panel_sep = None):
    lm_data = np.zeros(events.shape, dtype=cudarecon_type0_dtype)
    if system_shape and panel_sep:
        lm_data['x0'], lm_data['x1'], \
                lm_data['y0'], lm_data['y1'],\
                lm_data['z0'], lm_data['z1'] = \
                get_crystal_pos(
                        events,
                        system_shape=system_shape,
                        panel_sep=panel_sep)
    elif panel_sep:
        lm_data['x0'], lm_data['x1'], \
                lm_data['y0'], lm_data['y1'],\
                lm_data['z0'], lm_data['z1'] = \
                get_crystal_pos(events, panel_sep=panel_sep)
    elif system_shape:
        lm_data['x0'], lm_data['x1'], \
                lm_data['y0'], lm_data['y1'],\
                lm_data['z0'], lm_data['z1'] = \
                get_crystal_pos(events, system_shape=system_shape)
    else:
        lm_data['x0'], lm_data['x1'], \
                lm_data['y0'], lm_data['y1'],\
                lm_data['z0'], lm_data['z1'] = \
                get_crystal_pos(events)
    return lm_data

def save_binary(data, filename):
    fid = open(filename, 'wb')
    data.tofile(fid)
    fid.close()
    return

def correct_resets(data, threshold=1.0e3):
    data['ct'][1:] = np.diff(data['ct'])
    # Assume that any negative should just be the next coarse timestampe tick,
    # so we set it to one, so that the ct is incremented in the cumsum step
    data['ct'][data['ct'] < -threshold] = 1
    data['ct'] = np.cumsum(data['ct'])
    return data

def save_sparse_csc(filename,array):
    np.savez(filename,data = array.data ,indices=array.indices,
             indptr =array.indptr, shape=array.shape )

def load_sparse_csc(filename):
    loader = np.load(filename)
    return csc_matrix((loader['data'], loader['indices'], loader['indptr']),
                      shape = loader['shape'])

def main():
    return

if __name__ == '__main__':
    main()
