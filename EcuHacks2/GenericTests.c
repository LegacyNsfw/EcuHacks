#include "EcuHacks.h"

extern void* StartOfRevLimiterTables;
extern void* EndOfRevLimiterCode;
extern void* StartOfSpeedDensityTables;
extern void* EndOfSpeedDensityCode;

void GenericTests() __attribute__ ((section ("Misc")));
void GenericTests() 
{
	AssertTrue(&StartOfRevLimiterTables < &EndOfRevLimiterCode, "rev limiter patch section start/end");
	AssertTrue(&StartOfSpeedDensityTables < &EndOfSpeedDensityCode, "speed density patch section start/end");
	AssertTrue(&EndOfRevLimiterCode < &StartOfSpeedDensityTables, "rev limiter / speed density patch overlap");
}

