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

def load_calibration(filename):
    return np.loadtxt(filename, dtype=cal_dtype)

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
def get_global_cartridge_numbers(events, system_shape):
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

def tcal_coinc_events(events, tcal, system_shape):
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
         (7 - (events['module1'].astype(float) // 8) + 0.5) * x_crystal_pitch

    y0 = - panel_sep / 2.0 - \
         events['apd0'].astype(float) * y_apd_pitch - \
         ((7 - (events['crystal0'].astype(float) % 8) * y_crystal_pitch) + 0.5)

    y1 = + panel_sep / 2.0 - \
         events['apd1'].astype(float) * y_apd_pitch + \
         ((7 - (events['crystal1'].astype(float) % 8) * y_crystal_pitch) + 0.5)

    z0 = z_pitch * (0.5 + events['fin0'].astype(float) +
                    system_shape[2] * (events['cartridge0'].astype(float) -
                                       system_shape[1] / 2.0))

    z1 = z_pitch * (0.5 + events['fin1'].astype(float) +
                    system_shape[2] * (events['cartridge1'].astype(float) -
                                       system_shape[1] / 2.0))
    return x0, x1, y0, y1, z0, z1

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

def main():
    return

if __name__ == '__main__':
    main()
