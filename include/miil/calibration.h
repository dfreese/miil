#ifndef CALIBRATION_H
#define CALIBRATION_H
#include <float.h>

struct TempRHCalibParams {
    float c1; ///< RH measurement DC offset
    float c2; ///< RH measurement first order term
    float c3; ///< RH measurement quadratic term
    float d1; ///< Temperature measurement DC term
    float d2; ///< Temperature measurement first order term
    float t1; ///< RH temperature compensation DC term
    float t2; ///< RH temperature compensation first order term
    TempRHCalibParams():c1(),c2(),c3(),d1(),d2(),t1(),t2(){}
    TempRHCalibParams(
            float c_1,
            float c_2,
            float c_3,
            float d_1,
            float d_2,
            float t_1,
            float t_2) :
        c1(c_1),c2(c_2),c3(c_3),d1(d_1),d2(d_2),t1(t_1),t2(t_2){}
};

struct TempCalibParams {
	float ExtRes; ///< Resistance external to the Maxim chip
	float RefTemp; ///< Temperature at which RefRes is measured
	float Offset; ///< Offset to the fit used TODO: Describe Fit Used
	float RefRes; ///< Resistance of the thermistor at RefTemp
    TempCalibParams():ExtRes(),RefTemp(),Offset(),RefRes(){}
    TempCalibParams(float er,float rt,float off,float rr) :
        ExtRes(er),RefTemp(rt),Offset(off),RefRes(rr){}
};

/*! \brief: A Structure with parameters for leakage current calculation */
struct LCCalibParams {
	float VRef; ///< Reference voltage of the OpAmp
	float OpAmpGain; ///< The gain of the OpAmp specified by the circuitry
    float Res; ///< The resistance leakage current is measured over
	float Offset; ///< The Y-Offset in the linear fit 
    float Slope; ///< The slope of the linear fit
    LCCalibParams():VRef(),OpAmpGain(),Res(),Offset(),Slope(){}
    LCCalibParams(float vr,float og,float r,float of,float s) :
        VRef(vr),OpAmpGain(og),Res(r),Offset(of),Slope(s){}
};

float CalculateThermistorTemp(
        int HighNumber,
        int LowNumber,
        struct TempCalibParams param);

float CalculateLC(int Number, struct LCCalibParams param);

float CalculateChipTemp(float so_t, const struct TempRHCalibParams & param);

float CalculateChipRH(
        float so_rh,
        const struct TempRHCalibParams & param,
        float temp = FLT_MAX);

#endif /* CALIBRATION_H */
