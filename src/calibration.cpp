#include <miil/calibration.h>
#include <math.h>
#include <iostream>

/*! \brief Calculates temperature of thermistor connected to a maxim chip
 *
 * \param HighNumber The High Number returned from the Maxim thermistor chip
 * \param LowNumber The Low Number returned from the Maxim thermistor chip
 * \param param A structure containing the thermistor calibration parameters
 *
 * \return: The calculated temperature of the thermistor.
 *          Returns -1 if the temperature is NaN
 */
float CalculateThermistorTemp(
        int HighNumber,
        int LowNumber,
        struct TempCalibParams param)
{
    float thermistorR = ((1 / (((float) HighNumber /
                                (float) LowNumber) + 0.0002)) - 1)
                        * param.ExtRes;
    float temp = (param.RefTemp*param.Offset) /
            (param.Offset-param.RefTemp * log(param.RefRes / thermistorR))
            - 273.15;

    if (isnan(temp)) {
        return(-1);
    } else {
        return(temp);
    }
}

/*! \brief Calculates leakage current given inputs and calibration parameters
 *
 * Calculated the theoretical current based on the circuit parameters then,
 * using a linear fit, it is calibrated to the physically measured parameters.
 *
 * \param Number The ADC value returned from the SCMicro
 * \param param A structure with the leakage current calibration parameters
 *
 * \return: the calculated leakage current from the Module
 */
float CalculateLC(int Number, struct LCCalibParams param) {
    float current = ((Number*param.VRef / 1024 / param.OpAmpGain / param.Res)
                      -param.Offset) / param.Slope;
	return(current);
}

/*! \brief Calculate the temperature measured from a Sensirion SHT1x chip.
 *
 * \param so_rh The output value from the chip
 * \param param The calibration parameters to convert the output value into temp
 *
 * \return The measured temperature
 *
 * \see Datasheet Pages 8 and 9
 */
float CalculateChipTemp(float so_t, const struct TempRHCalibParams & param) {
    return(param.d1 + param.d2 * so_t);
}

/*! \brief Calculate Sensirion Relative Humidity
 *
 * Returns a linear relative humidity measured from a Sensirion SHT1x chip. If
 * a temperature is given, the temperature compensated value is returned.
 *
 * \param so_rh The output value from the chip
 * \param param The calibration parameters to convert the output value into RH
 * \param temp optional temperature parameter to compensate measurement
 *
 * \return The measured relative humidity
 *
 * \see Datasheet Pages 8 and 9
 */
float CalculateChipRH(
        float so_rh,
        const struct TempRHCalibParams & param,
        float temp)
{
    float rh_linear(param.c1 + param.c2 * so_rh  + param.c3 * so_rh * so_rh);
    if (temp == FLT_MAX) {
        return(rh_linear);
    } else {
        return((temp - 25) * (param.t1 + param.t2 * so_rh) + rh_linear);
    }
}
