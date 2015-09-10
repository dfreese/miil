#ifndef SORTING_H
#define SORTING_H

#include <deque>
#include <vector>

/*! \brief Sort stl container using the insertion sort method
 *
 * Sorts the values within a container using the '<' operator within the
 * insertion sort algorithm.  This is nearly linear for mostly sorted data.
 *
 * \param array The type of array that will be sorted.  Must have a size_type,
 *              as well as a value_type, which all stl containers will.
 *
 * \return 0 on success, less than zero on error.  No error checking is
 *         currently implemented.
 */
template <class T>
int insertion_sort(T & array) {
    for (typename T::size_type ii = 1; ii < array.size(); ii++) {
        for (typename T::size_type kk = ii;
             (kk > 0) && (array[kk] < array[kk-1]);
             kk--)
        {
            typename T::value_type temp = array[kk];
            array[kk] = array[kk - 1];
            array[kk - 1] = temp;
        }
    }
    return(0);
}

 /*! \brief Perform insertion sort on an stl container using a custom comparison
  *
  * Sorts the values within a container using a custom function within the
  * insertion sort algorithm.  This is nearly linear for mostly sorted data.
  * The custom function is useful for sorting classes or structures where one
  * element of the class should be used for sorting.
  *
  * \param array The type of array that will be sorted.  Must have a size_type,
  *              as well as a value_type, which all stl containers will.
  * \param f The custom comparison function that should be used. This is a func
  *          with two arguments.  It evaluates to true if the first argument
  *          should be place first, or false if the second argument should be.
  *
  * \return 0 on success, less than zero on error.  No error checking is
  *         currently implemented.
  */
template <class T>
int insertion_sort(
        T & array,
        bool (*f) (const typename T::value_type &,
                   const typename T::value_type &))
{
    for (typename T::size_type ii = 1; ii < array.size(); ii++) {
        for (typename T::size_type kk = ii;
             (0 < kk) && f(array[kk], array[kk-1]);
             kk--)
        {
            typename T::value_type temp = array[kk];
            array[kk] = array[kk - 1];
            array[kk - 1] = temp;
        }
    }
    return(0);
}

 /*! \brief Perform insertion sort on an stl container using a custom comparison
  *
  * Sorts the values within a container using a custom function within the
  * insertion sort algorithm.  This is nearly linear for mostly sorted data.
  * The custom function is useful for sorting classes or structures where one
  * element of the class should be used for sorting.  This template is for
  * custom functions requiring two arguments
  *
  * \param array The type of array that will be sorted.  Must have a size_type,
  *              as well as a value_type, which all stl containers will.
  * \param f The custom comparison function that should be used. This is a func
  *          with four arguments.  It evaluates to true if the first argument
  *          should be place first, or false if the second argument should be.
  *          The third through fifth arguments are for parameters within the
  *          comparison.
  *
  * \return 0 on success, less than zero on error.  No error checking is
  *         currently implemented.
  */
template <class T, class Arg1, class Arg2, class Arg3>
int insertion_sort(
        T & array,
        bool (*f) (const typename T::value_type &,
                   const typename T::value_type &,
                   Arg1, Arg2, Arg3),
        Arg1 arg1, Arg2 arg2, Arg3 arg3)
{
    for (typename T::size_type ii = 1; ii < array.size(); ii++) {
        for (typename T::size_type kk = ii;
             (0 < kk) && f(array[kk], array[kk-1], arg1, arg2, arg3);
             kk--)
        {
            typename T::value_type temp = array[kk];
            array[kk] = array[kk - 1];
            array[kk - 1] = temp;
        }
    }
    return(0);
}

/*! \brief Sort stl container using the insertion sort method with a saved key
 *
 * Sorts the values within a container using the '<' operator within the
 * insertion sort algorithm.  This is nearly linear for mostly sorted data.
 * This function takes a second argument of key that will allow for the result
 * of the sorting to be indexed later on.  This is useful for figuring out how
 * to sort a non-stl container that cannot be given to the sorting algorithm.
 * The information from the container required to sort the container can be
 * placed into an stl container and the resulting key can be used to access the
 * non-stl container in a sorted fashion.
 *
 * \param array The type of array that will be sorted.  Must have a size_type,
 *              as well as a value_type, which all stl containers will.
 * \param key An stl container of sorted index values from 0 to
 *            (array.size() - 1).  This will be modified to so that
 *            non_stl_container[key[i]] will access the ith sorted element of
 *            non_stl_container.
 *
 * \return 0 on success, less than zero on error.  No error checking is
 *         currently implemented.
 */
template <class T, class K>
int insertion_sort_with_key(T & array, K & key) {
    if (array.size() != key.size()) {
        return(-1);
    }
    for (typename T::size_type ii = 1; ii < array.size(); ii++) {
        for (typename T::size_type kk = ii;
             (kk > 0) && (array[kk] < array[kk-1]);
             kk--)
        {
            typename T::value_type temp = array[kk];
            array[kk] = array[kk - 1];
            array[kk - 1] = temp;
            typename K::value_type temp_key = key[kk];
            key[kk] = key[kk - 1];
            key[kk - 1] = temp_key;
        }
    }
    return(0);
}

#endif /* SORTING_H */
