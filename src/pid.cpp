#include <miil/pid.h>

/*!
 * \brief Performs a standard PID loop with adjustable limits
 *
 * This peforms a standard PID loop that is augmented with limits for swings in
 * the output as well as the output itself.  Takes an input structure
 * describing the loops gains and limits, calculates the difference between the
 * target and the input and then applies the gains, contained within the
 * structure to calculate the change in the output that is required.  The
 * limits that are programable are the minimum required swing in the input that
 * should be recognized, the maximum swing in the output that should be allowed
 * and the limits that are allowed for the output, typically to reflect a
 * hardware limit.
 *
 * \param Input The measured value for the PID loop.  This is the value that is
 *              being controlled to reach the target.
 * \param Params The structure containing settings relevant to the operation of
 *               the PID loop, including gain parameters and limits.
 *
 * \return The output of the function represents the level the output should be
 *         placed at as the next time step for the discrete PID loop.
 */
float CalculateOutput(const float & Input, PID_Params & Params)
{

    // Calculate the difference between the target value for input and actual
	float Req_Input_Delta = Params.Input_Target - Input;
    // If the deviation is too small, based on a specified value, then zero it
	if ( (Req_Input_Delta < Params.Min_Input_Dev) &&
         (Req_Input_Delta > (0-Params.Min_Input_Dev)) )
    {
		Req_Input_Delta = 0;
    }

    // Calculate the relevant values for each portion of the PID loop
    Params.integral += Req_Input_Delta * Params.Delta_t;
	float derivative = (Req_Input_Delta - Params.prev_err) / Params.Delta_t;
    // If an error hasn't been calculated previously in the loop, don't
    // use the derivative yet.  Wait until the next calculation.
    if (!Params.use_derivative) {
        derivative = 0;
        Params.use_derivative = true;
    }

    // Apply the gains to the differences
    float Req_Output_Delta = Params.P_Gain * Req_Input_Delta
                             + Params.I_Gain * Params.integral
                             + Params.D_Gain * derivative;
    if (!Params.use_derivative) {
        Req_Output_Delta = Params.P_Gain * Req_Input_Delta
                                 + Params.I_Gain * Params.integral;
        Params.use_derivative = true;
    }

	float Capped_Req_Output_Delta = Req_Output_Delta;

    // The output can also be capped by the user, so calculate the requested
    // output from the PID loop and then set it to the limit if the limits are
    // exceeded
	float Req_Output = Params.output_offset + Capped_Req_Output_Delta;
	if ( Req_Output > Params.Max_Output) {
        Req_Output = Params.Max_Output;
    } else if ( Req_Output < Params.Min_Output) {
        Req_Output = Params.Min_Output;
    }

    // Update the previous error to the current one for the next usage
    Params.prev_err = Req_Input_Delta;

	return(Req_Output);
}

/*!
 * \brief Resets the memory of the PID loop
 *
 * Resets the integral and derivative memory of the PID loop.  It optionally
 * changes the offset to a specific value so the loop can take over fresh from
 * the current operating conditions.
 *
 * \param Params The parameters of the PID loop to be reset
 * \param offset The new output offset that the loop should use.
 */
void ResetLoop(PID_Params & Params, const float offset) {
    Params.prev_err = 0;
    Params.integral = 0;
    Params.output_offset = offset;
    Params.use_derivative = false;
}
