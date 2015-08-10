#ifndef PID_H
#define PID_H

#include <vector>

struct PID_Params {
	float P_Gain;
	float I_Gain;
	float D_Gain;
	float Input_Target;
    float Min_Input_Dev;
	float Max_Output;
	float Min_Output;
	float Delta_t;
    float output_offset;
    float integral;
    float prev_err;
    bool use_derivative;

	PID_Params() :
        P_Gain(0),
        I_Gain(0),
        D_Gain(0),
        Input_Target(0),
        Min_Input_Dev(0),
        Max_Output(0),
        Min_Output(0),
        Delta_t(1),
        output_offset(0),
        integral(0),
        prev_err(0),
        use_derivative(false)
    {}

	PID_Params(
            float p,
            float i,
            float d,
            float _input_target,
            float _min_input_dev,
            float _max_output,
            float _min_output,
            float _dt,
            float _output_offset) :
        P_Gain(p),
        I_Gain(i),
        D_Gain(d),
        Input_Target(_input_target),
        Min_Input_Dev(_min_input_dev),
        Max_Output(_max_output),
        Min_Output(_min_output),
        Delta_t(_dt),
        output_offset(_output_offset),
        integral(0),
        prev_err(0),
        use_derivative(false)
    {}
};

float CalculateOutput(const float & Input, PID_Params & Params);
void ResetLoop(PID_Params & Params, const float offset = 0);

#endif /* PID_H */
