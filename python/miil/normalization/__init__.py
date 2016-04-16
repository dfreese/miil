import numpy as np
import miil

def check_and_promote_coordinates(coordinate):
    if len(coordinate.shape) == 1:
        coordinate = np.expand_dims(coordinate, axis=0)
    if coordinate.shape[1] != 3:
        raise ValueError('coordinate.shape != (n,3)')
    return coordinate.astype(float)

'''
line_start (1, 3) coordinate representing (x,y,z) start of line
line_end (1, 3) coordinate representing (x,y,z) end of line
voxel_centers (n, 3) coordinates representing (x,y,z) centers of voxel
voxel_size (1, 3) coordinate representing (x,y,z) size of voxels

Based primarily off of this stackoverflow response
http://stackoverflow.com/questions/3106666/intersection-of-line-segment-with-axis-aligned-box-in-c-sharp#3115514
'''
def voxel_intersection_length(line_start, line_end, voxel_centers, voxel_size):
    line_start = check_and_promote_coordinates(line_start)
    line_end = check_and_promote_coordinates(line_end)
    voxel_centers = check_and_promote_coordinates(voxel_centers)
    voxel_size = check_and_promote_coordinates(voxel_size)

    n = voxel_centers.shape[0]
    m = line_start.shape[0]
    if line_start.shape != line_end.shape:
        raise ValueError('line_start and line_end are not the same size')

    if m != 1 and n != 1:
        raise ValueError('Multiple lines with multiple voxels not supported')

    if np.any(voxel_size < 0):
        raise ValueError('Negative voxel_size value is incorrect')

    line_delta = line_end - line_start

    voxel_start = voxel_centers - (voxel_size / 2)
    voxel_end = voxel_centers + (voxel_size / 2)

    line_start_to_vox_start = voxel_start - line_start
    line_start_to_vox_end = voxel_end - line_start

    # This implementation is careful about the IEEE spec for handling divide by
    # 0 cases.  For more details see this paper here:
    # http://people.csail.mit.edu/amy/papers/box-jgt.pdf
    with np.errstate(divide='ignore'):
        inv_delta = np.divide(1.0, line_delta)
    with np.errstate(invalid='ignore'):
        t1 = line_start_to_vox_start * inv_delta
        t2 = line_start_to_vox_end * inv_delta

    swap_mask = (inv_delta < 0)
    if m == 1:
        swap_mask = np.tile(swap_mask, (n,1))

    t1[swap_mask], t2[swap_mask] = t2[swap_mask], t1[swap_mask].copy()

    # t1[:, inv_delta.ravel() < 0], t2[:, inv_delta.ravel() < 0] = \
    #     t2[:, inv_delta.ravel() < 0], t1[:, inv_delta.ravel() < 0].copy()

    # Ignore nans that can happen when 0.0/inf or 0.0/-inf which is IEEE spec
    tnear = np.nanmax(t1, axis=1, keepdims=True)
    tfar = np.nanmin(t2, axis=1, keepdims=True)

    # We then bound tnear and tfar to be [0,1].  This takes care of the
    # following cases:
    #   1) Line if fully outside of the box
    #   2) Line has one vertex in the box
    #   3) Line lies on the face of one box
    remove = tnear > tfar
    tfar[remove] = 0
    tnear[remove] = 0
    tnear[tnear < 0] = 0
    tnear[tnear > 1] = 1
    tfar[tfar < 0] = 0
    tfar[tfar > 1] = 1

    dist = np.linalg.norm(line_delta * (tfar - tnear), axis=1, keepdims=True)
    return dist

def voxel_intersection_frac(line_start, line_end, voxel_centers, voxel_size):
    dist = voxel_intersection_length(
            line_start, line_end, voxel_centers, voxel_size)
    # Normalize corner to corner to 1
    max_voxel_distance = np.linalg.norm(voxel_size)
    # Then correct it for the number of dimensions we are operating in, so that
    # corner to corner on a perfect cubic voxel is sqrt(3), a flat square voxel
    # is sqrt(2), etc...
    equal_voxel_correction = np.sqrt(np.sum(voxel_size != 0))
    return dist / max_voxel_distance * equal_voxel_correction

def forward_project_line(
        line_start, line_end, voxel_centers, voxel_size, voxel_value):
    frac = voxel_intersection_length(
            line_start, line_end, voxel_centers, voxel_size)
    return np.sum(frac * voxel_value)

def attenuation_correction_factor(length, atten_coef = 0.0096):
    """
    Calculates the factor a weight of a line must be multiplied by in order to
    correct for some constant attenuation along that line.  The attenuation
    coefficient default is for water in units of mm^-1, so the length is
    expected to be in units of mm.  Default value was taken from abstract of,
    "PET attenuation coefficients from CT images: experimental evaluation of the
    transformation of CT into PET 511-keV attenuation coefficients," in the
    European Journal of Nuclear Medicine, 2002. Factor is calculated as:
        exp(length * atten_coef)
    """
    return np.exp(length * atten_coef)

def atten_corr_lines(
        line_start, line_end, line_weight,
        fov_center, fov_size,
        atten_coef = None):
    """
    Corrects lines for their attenuation by calculating the length they travel
    through a given rectangular FOV.
    """
    length = voxel_intersection_length(line_start, line_end, fov_center, fov_size)
    if atten_coef is None:
        weight = attenuation_correction_factor(length)
    else:
        weight = attenuation_correction_factor(length, atten_coef)

    return weight * line_weight

def atten_corr_crystal_pair(
        crystal0, crystal1, line_weight, fov_center, fov_size,
        atten_coef = None):
    line_start = miil.get_position_global_crystal(crystal0)
    line_end = miil.get_position_global_crystal(crystal1)
    return atten_corr_lines(line_start, line_end, line_weight,
            fov_center, fov_size, atten_coef)

def atten_corr_lor(
        lor, line_weight, fov_center, fov_size,
        atten_coef = None, system_shape = [2, 3, 8, 16, 2, 64]):
    crystal0 = lor // np.prod(system_shape[1:])
    crystal1 = lor % np.prod(system_shape[1:])
    return atten_corr_lines(crystal0, crystal1, line_weight
            fov_center, fov_size, atten_coef)
