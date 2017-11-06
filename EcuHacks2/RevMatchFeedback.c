#include "EcuHacks.h"

extern TwoDimensionalTable RevMatchTable;
extern float RevMatchProportionalGain, RevMatchIntegralGain;


void RevMatchResetFeedback(void) __attribute__((section("RomHole_RevMatchCode")));
void RevMatchResetFeedback()
{
	pRamVariables->RevMatchProportionalFeedback = 0;
	pRamVariables->RevMatchIntegralFeedback = 0;
}

float RevMatchGetThrottle(float targetRpm) __attribute__((section("RomHole_RevMatchCode")));
float RevMatchGetThrottle(float targetRpm)
{
	float base = Pull2d(&RevMatchTable, targetRpm);
	
	if (!pRamVariables->RevMatchFeedbackEnabled)
	{
		return base;
	}
	
	// Error is how much change we want. 
	// It's a positive value when we want revs to increase.
	float error =  targetRpm - *pRPM;
	
	float proportionalFeedback = error * RevMatchProportionalGain;
	pRamVariables->RevMatchProportionalFeedback = proportionalFeedback;
	
	float integralFeedback = error * RevMatchIntegralGain;
	pRamVariables->RevMatchIntegralFeedback += integralFeedback;
	
	float totalFeedback = proportionalFeedback + pRamVariables->RevMatchIntegralFeedback;
	
	const float Limit = 5.0f;
	if (totalFeedback > Limit)	
	{
		totalFeedback = Limit;
	}
	
	if (totalFeedback < -Limit)
	{
		totalFeedback = -Limit;
	}
	
	float result = base + totalFeedback;
	if (result < 0)
	{
		result = 0;
	}
	
	if (result > 25)
	{
		result = 25;
	}
	
	return result;
}
