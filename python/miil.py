import numpy as np

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
