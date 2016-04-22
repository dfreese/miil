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

def backproject_line(
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

    return np.squeeze(weight) * line_weight

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
    crystal1 = lor % np.prod(system_shape[1:]) + np.prod(system_shape[1:])
    return atten_corr_crystal_pair(crystal0, crystal1, line_weight,
            fov_center, fov_size, atten_coef)

def atten_corr_sparse_lor_col_vec(
        vec, fov_center, fov_size,
        atten_coef = None, system_shape = [2, 3, 8, 16, 2, 64]):
    vec.data = atten_corr_lor(vec.indices, vec.data, fov_size,
            fov_center, atten_coef, system_shape)
    return vec

def uniform_activity_lor_predicted(
        lors, fov_center, fov_size,
        lyso_center0, lyso_size0, lyso_center1, lyso_size1,
        atten_coef = 0.0096, lyso_atten_coef =  0.087,
        system_shape = [2, 3, 8, 16, 2, 64]):
    """
    Create an vector of the predicted number of counts (relatively) for a
    uniform phantom with uniform attenuation.  Only lors given will be
    considered.  Duplicate lor numbers will be summed.  Predicted activity is
    calculated by:

        p = fov_length * exp(- mu_fov * fov_length - mu_lyso * 2 * length_lyso)

    The length of lyso is calculated by intersecting with the size of the
    detector given as a voxel on each side of the lor.  This assumes that the
    entire volume of the detector is LYSO, which is not correct.  Could
    theoretically swap out the linear attenuation coefficient for a mixture of
    the materials present in a way that would accurately represent the alumna.

    mu_lyso is taken from "Investigating the temporal resolution limits of
    scintillation detection from pixellated elements: comparison between
    experiment and simulation," Spanoudaki and Levin, Physics in Medicine and
    Biology, 2010.  Units = mm^-1.

    mu_fov is that of water is taken from, "PET attenuation coefficients from
    CT images: experimental evaluation of the transformation of CT into PET
    511-keV attenuation coefficients," in the European Journal of Nuclear
    Medicine, 2002.  Units = mm^-1.
    """
    vec = miil.create_sparse_column_vector(lors)
    line_start, line_end = miil.get_lor_positions(vec.indices, system_shape)
    fov_length = voxel_intersection_length(
            line_start, line_end, fov_center, fov_size)
    lyso_length = voxel_intersection_length(
            line_start, line_end, lyso_center0, lyso_size0) + \
            voxel_intersection_length(
            line_start, line_end, lyso_center1, lyso_size1)
    vec.data = vec.data * np.squeeze(fov_length * np.exp(
            -(atten_coef * fov_length + lyso_atten_coef * lyso_length)))
    # vec.data = vec.data * np.squeeze(fov_length * np.exp(
            # -(atten_coef * fov_length)))
    return vec

def get_crystal_distribution(
        vec, system_shape=miil.default_system_shape, weights = None):
    """
    Takes a sparse lor vector (csc_matrix) and converts into a distribution of
    counts per crystal
    """
    no_crystals_per_system = np.prod(miil.default_system_shape)
    crystal0, crystal1 = miil.get_crystals_from_lor(
            vec.indices, miil.default_system_shape)

    counts = vec.data.copy()
    if weights is not None:
        counts *= weights[crystal0] * weights[crystal1]

    crystal_dist = \
            np.bincount(
                crystal0, weights=counts, minlength=no_crystals_per_system) +\
            np.bincount(
                crystal1, weights=counts, minlength=no_crystals_per_system)
    return crystal_dist

def get_apd_distribution(
        vec, system_shape=miil.default_system_shape, weights = None):
    """
    Takes a sparse lor vector (csc_matrix) and converts into a distribution of
    counts per apd
    """
    no_apds_per_system = np.prod(miil.default_system_shape[:-1])
    apd0, apd1 = miil.get_apds_from_lor(
            vec.indices, miil.default_system_shape)

    counts = vec.data.copy()
    if weights is not None:
        counts *= weights[apd0] * weights[apd1]

    apd_dist = \
            np.bincount(
                apd0, weights=counts, minlength=no_apds_per_system) +\
            np.bincount(
                apd1, weights=counts, minlength=no_apds_per_system)
    return apd_dist

def get_module_distribution(
        vec, system_shape=miil.default_system_shape, weights = None):
    """
    Takes a sparse lor vector (csc_matrix) and converts into a distribution of
    counts per module
    """
    no_modules_per_system = np.prod(miil.default_system_shape[:-2])
    module0, module1 = miil.get_modules_from_lor(
            vec.indices, miil.default_system_shape)

    counts = vec.data.copy()
    if weights is not None:
        counts *= weights[module0] * weights[module1]

    module_dist = \
            np.bincount(
                module0, weights=counts, minlength=no_modules_per_system) +\
            np.bincount(
                module1, weights=counts, minlength=no_modules_per_system)
    return module_dist

def normalize_to_reference(
        vec, est_vec, step=0.3, max_iter=100, epsilon=1e-5, verbose=False,
        system_shape=miil.default_system_shape,
        est_module=True, est_apd=True, est_crystal=True):
    no_crystals_per_system = np.prod(system_shape)
    no_apds_per_system = np.prod(system_shape[0:5])
    no_modules_per_system = np.prod(system_shape[0:4])

    module_weights = np.ones((no_modules_per_system,))
    if est_module:
        if verbose:
            print 'Estimating module weights:'
        module0, module1 = miil.get_modules_from_lor(vec.indices, system_shape)
        est_dist = get_module_distribution(est_vec)
        est_dist /= np.mean(est_dist)
        for ii in range(max_iter):
            dist = get_module_distribution(vec, weights=module_weights)
            with np.errstate(divide='ignore', invalid='ignore'):
                factor = np.divide(est_dist, dist / np.mean(dist))
            factor[dist == 0] = 1
            module_weights *= (1 + (factor - 1) * step)

            rel_error = np.linalg.norm(factor - 1)
            if verbose:
                print '%02d: %0.3e' % (ii, rel_error)
            if rel_error < epsilon:
                break

    apd_weights = np.repeat(module_weights, system_shape[4])
    if est_apd:
        if verbose:
            print 'Estimating apd weights:'
        apd0, apd1 = miil.get_apds_from_lor(vec.indices, system_shape)
        est_dist = get_apd_distribution(est_vec)
        est_dist /= np.mean(est_dist)
        for ii in range(max_iter):
            dist = get_apd_distribution(vec, weights=apd_weights)
            with np.errstate(divide='ignore', invalid='ignore'):
                factor = np.divide(est_dist, dist / np.mean(dist))
            factor[dist == 0] = 1
            apd_weights *= (1 + (factor - 1) * step)

            rel_error = np.linalg.norm(factor - 1)
            if verbose:
                print '%02d: %0.3e' % (ii, rel_error)
            if rel_error < epsilon:
                break

    crystal_weights = np.repeat(apd_weights, system_shape[5])
    if est_crystal:
        if verbose:
            print 'Estimating crystal weights:'
        crystal0, crystal1 = miil.get_crystals_from_lor(
                vec.indices, system_shape)
        est_dist = get_crystal_distribution(est_vec)
        est_dist /= np.mean(est_dist)
        for ii in range(max_iter):
            dist = get_crystal_distribution(vec, weights=crystal_weights)
            with np.errstate(divide='ignore', invalid='ignore'):
                factor = np.divide(est_dist, dist / np.mean(dist))
            factor[dist == 0] = 1
            crystal_weights *= (1 + (factor - 1) * step)

            rel_error = np.linalg.norm(factor - 1)
            if verbose:
                print '%02d: %0.3e' % (ii, rel_error)
            if rel_error < epsilon:
                break

    return crystal_weights

def normalize_to_self(
        vec, step=0.3, max_iter=20, epsilon=1e-4, verbose=False,
        system_shape=miil.default_system_shape,
        est_module=True, est_apd=True, est_crystal=True):
    no_crystals_per_system = np.prod(system_shape)
    no_apds_per_system = np.prod(system_shape[0:5])
    no_modules_per_system = np.prod(system_shape[0:4])

    module_weights = np.ones((no_modules_per_system,))
    if est_module:
        if verbose:
            print 'Estimating module weights:'
        module0, module1 = miil.get_modules_from_lor(vec.indices, system_shape)
        for ii in range(max_iter):
            dist = get_module_distribution(vec, weights=module_weights)
            with np.errstate(divide='ignore', invalid='ignore'):
                factor = np.divide(np.mean(dist), dist)
            factor[dist == 0] = 1
            module_weights *= (1 + (factor - 1) * step)

            rel_error = (np.linalg.norm(factor - 1) / no_modules_per_system)
            if verbose:
                print '%02d: %0.3e' % (ii, rel_error)
            if rel_error < epsilon:
                break

    apd_weights = np.repeat(module_weights, system_shape[4])
    if est_apd:
        if verbose:
            print 'Estimating apd weights:'
        apd0, apd1 = miil.get_apds_from_lor(vec.indices, system_shape)
        for ii in range(max_iter):
            dist = get_apd_distribution(vec, weights=apd_weights)
            with np.errstate(divide='ignore', invalid='ignore'):
                factor = np.divide(np.mean(dist), dist)
            factor[dist == 0] = 1
            apd_weights *= (1 + (factor - 1) * step)

            rel_error = np.linalg.norm(factor - 1) / no_apds_per_system
            if verbose:
                print '%02d: %0.3e' % (ii, rel_error)
            if rel_error < epsilon:
                break

    crystal_weights = np.repeat(apd_weights, system_shape[5])
    if est_crystal:
        if verbose:
            print 'Estimating crystal weights:'
        crystal0, crystal1 = miil.get_crystals_from_lor(
                vec.indices, system_shape)
        for ii in range(max_iter):
            dist = get_crystal_distribution(vec, weights=crystal_weights)
            with np.errstate(divide='ignore', invalid='ignore'):
                factor = np.divide(np.mean(dist), dist)
            factor[dist == 0] = 1
            crystal_weights *= (1 + (factor - 1) * step)

            rel_error = np.linalg.norm(factor - 1) / no_crystals_per_system
            if verbose:
                print '%02d: %0.3e' % (ii, rel_error)
            if rel_error < epsilon:
                break

    return crystal_weights

def correct_uniform_phantom_lors(
        lors, fov_center, fov_size,
        lyso_center0, lyso_size0, lyso_center1, lyso_size1,
        atten_coef = 0.0096, lyso_atten_coef =  0.087,
        system_shape = miil.default_system_shape):
    vec = miil.create_sparse_column_vector(lors)
    line_start, line_end = miil.get_lor_positions(vec.indices, system_shape)

    fov_length = voxel_intersection_length(
            line_start, line_end, fov_center, fov_size)
    vec.data *= np.squeeze(np.exp(atten_coef * fov_length) / fov_length)


    lyso_length = voxel_intersection_length(
            line_start, line_end, lyso_center0, lyso_size0) + \
            voxel_intersection_length(
            line_start, line_end, lyso_center1, lyso_size1)
    vec.data *= np.squeeze(np.exp(lyso_atten_coef * lyso_length))

    return vec

def correct_lors(
        lors, fov_center, fov_size,
        lyso_center0, lyso_size0, lyso_center1, lyso_size1, weights = None,
        atten_coef = 0.0096, lyso_atten_coef =  0.087,
        system_shape = miil.default_system_shape,
        correct_lyso_atten=True, correct_uniform_atten=True):
    vec = miil.create_sparse_column_vector(lors)
    if correct_uniform_atten or correct_lyso_atten:
        line_start, line_end = miil.get_lor_positions(vec.indices, system_shape)
        if correct_uniform_atten:
            fov_length = voxel_intersection_length(
                    line_start, line_end, fov_center, fov_size)
            vec.data *= np.squeeze(np.exp(atten_coef * fov_length))
        if correct_lyso_atten:
            lyso_length = voxel_intersection_length(
                    line_start, line_end, lyso_center0, lyso_size0) + \
                    voxel_intersection_length(
                    line_start, line_end, lyso_center1, lyso_size1)
            vec.data *= np.squeeze(np.exp(lyso_atten_coef * lyso_length))
    if weights is not None:
        crystal0, crystal1 = miil.get_crystals_from_lor(
                vec.indices, miil.default_system_shape)
        vec.data *= weights[crystal0]
        vec.data *= weights[crystal1]
    return vec

def main():
    return

if __name__ == '__main__':
    main()
