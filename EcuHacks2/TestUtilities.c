#include "EcuHacks.h"

void TestFailed(char *message) __attribute__ ((section ("Misc")));
void TestFailed(char *message)
{
	// This just provides a place to set a breakpoint.
	asm("nop");
}

void AssertTrue(int condition, char *message)
{
	if (condition == 0)
	{
		TestFailed(message);
	}
}

void AssertEqualInts(int actual, int expected, char *message)
{
	if (actual != expected)
	{
		TestFailed(message);
	}
}

// This will tolerate a small amount of error, because it's hard to
// provide constant values that will match computed values with enough
// precision to use a simple == comparison.
void AssertEqualFloats(float actual, float expected, char *message)
{
	if (actual > 0 && expected > 0)
	{
		if ((actual * 1.001) < expected) 
		{
			TestFailed(message);
			return;
		}
	
		if ((actual * 0.999) > expected)
		{
			TestFailed(message);
			return;
		}	
	}
	else if (actual < 0 && expected < 0)
	{
		if ((actual * 1.001) > expected) 
		{
			TestFailed(message);
			return;
		}
	
		if ((actual * 0.999) < expected)
		{
			TestFailed(message);
			return;
		}	
	}
	else if (actual == expected) // if both are zero
	{
		return;
	}
	else
	{
		TestFailed(message);
	}
}

void DemonstrateAssertionFailure() __attribute__ ((section ("Misc")));
void DemonstrateAssertionFailure()
{
	AssertTrue(0, "Just to prove that assertions can fail.");
}


// It's hard to express a floating point number with enough precision to use
// the == operator.  Or impossible: http://en.wikipedia.org/wiki/Machine_epsilon
// So we'll just test wither the actual an expected values are within +/- 0.001.
int AreCloseEnough(float actual, float expected)
{
	if ((actual * 1.001) < expected) 
	{
		return 0;
	}
	
	if ((actual * 0.999) > expected)
	{
		return 0;
	}	
	
	return 1;
}