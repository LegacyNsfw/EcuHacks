// These constants prevent the compiler from referencing implicit global
// variables every time you read or write a memory location.  You'd think
// that just using 'const' symbols would work the same way, but it doesn't.

// Existing variables needed for the LC/FFS hack
#define pRPM                      ((float*)0xFFFF51F8)
#define pSpeed                    ((float*)0xFFFF51E8)
#define pFlagsRevLimit_0x80       ((char*) 0xFFFF5A40)
#define pCruiseFlagsA             ((char*) 0xFFFF4D65)
#define pLeftTgvVoltage           ((float*)0xFFFF2D2C)
#define pThrottlePedal            ((float*)0xFFFF5134)

// Existing variables needed for the speed-density hack
#define pAtmosphericPressure      ((float*)0xFFFF2E34)
#define pManifoldAbsolutePressure ((float*)0xFFFF4F40)
#define pIntakeAirTemperature     ((float*)0xFFFF2DB8)
#define pCoolantTemperature       ((float*)0xFFFF2DC8)
#define pMassAirFlow              ((float*)0xFFFF2DF4) 
#define pMafSensorVoltage         ((short*)0xFFFF2D22) // Only used by the test code.

// Existing variables needed for the rev-match hack

// In IDA this is E_Target_Throttle_Plate_PositionExt_E57 
// The stock ECU code computes this from the DBW tables.
#define pTargetThrottlePlatePosition_In  ((float*)0xFFFF5F04) 

// In IDA this is NsfwTargetThrottlePlatePosition_1 
// The stock ECU code just copies the above value to this location.
// Then it applies a low-pass filter to this, stores that elsewhere,
// and uses the filtered value is several places. So we hook into 
// the ECU code by replacing the method that does that copy (it was
// tiny, as it literally did NOTHING else).
#define pTargetThrottlePlatePosition_Out ((float*)0xFFFF5EC4) 

// Both of these values came from the AssignGearCalculatedExt 
// function, which was found by searching for references to 
// the gear position tables.
#define pCurrentGear                     ((char*) 0xFFFF52F9)
#define pGearFactor                      ((float*)0xFFFF52FC)

// I tried clearing in the 0x80 bit, but that wasn't sufficient.
// This must be forced to 16.
#define pOverrunFuelCutFlags_1           ((char*) 0xFFFF5A08) // AKA Flags023
#define OverrunFuelCutFlags_1_Defeat     16

// In addition to the above, these two must be forced to zero.
#define pOverrunFuelCutFlags_2           ((char*) 0xFFFF5555) // AKA Flags056 - zero in cruise, 1 or 15 during ordinary shifts
#define pOverrunFuelCutFlags_3           ((char*) 0xFFFF5A0A) // AKA Flags055 - zero in cruise, 240 during fuel cut

// The original ECU code adds this value to the throttle plate target, 
// which will cause the engine to rev much faster than the desired RPM.
// Forcing this to zero fixes that problem.
#define pThrottleCompensation            ((float*)0xFFFF5F2C)

#define pNeutralAndOtherFlags            ((char*) 0xFFFF51C7)
#define NeutralSwitchBit 0x02

extern void RevMatchResetFeedback();
extern float RevMatchGetThrottle(float targetRpm);

typedef struct
{
	// Base + 00
	char MafMode; // See MafModeValues
	char InjectorMode;
	char RevMatchState; // See RevMatchStates
	char LastCruiseCancelState; // Used to switch rev-match modes
	
	// Base + 0x04
	float VolumetricEfficiency;
	
	// Base + 0x08
	float MafFromSpeedDensity;
	
	// Base + 0x0C
	float MafFromSensor;
	
	// Base + 0x10
	float AtmosphericCompensation;	
	
	// Base + 0x14
	float InjectorScaling; // Intended for a future flex-fuel hack, not yet implemented.
	
	// Base + 0x18
	unsigned int Counter;
	
	// Base + 0x1C
	float UpshiftRpm;
	
	// Base + 0x20
	float DownshiftRpm;
	
	// Base + 0x24
	char RevMatchCalibrationIndex;
	char RevMatchCalibrationIndexChanged;
	char RevMatchCalibrationThrottleChanged;
	char RevMatchFromGear;
	
	// Base + 0x28
	unsigned int RevMatchConditionStart;
	
	// Base + 0x2C
	unsigned int RevMatchTimeoutStart;
	
	// Base + 0x30
	enum RevMatchStates (*RevMatchTransitionEvaluator)();
	
	// Base + 0x34
	float CounterAsFloat;
	
	// Base + 0x38
	float RevMatchCalibrationThrottle;
	
	// Base + 0x3C
	float RevMatchProportionalFeedback;
	
	// Base + 0x40
	float RevMatchIntegralFeedback;
	
	// Base + 0x44
	float RevMatchDerivativeFeedback;
	
	float RevMatchPreviousError;
	
	char RevMatchFeedbackEnabled;
	char RevMatchCalibrationFeedbackEnabled;
	char Unused2;
	char Unused3;
	
} RamVariables;

// For A2WC522N, RAM from 0xFFA000 - 00FFBFCF is apparently unused (8,143 bytes)
// The previous 8k appears unused as well, FF8000 - FF9FFF.
// However the previous-previous 8K (FF6000-FF7FFF) is definitely used by the ECU.
#define pRamVariables             ((RamVariables*) 0xFFFFA000)

#define MafVoltageToInternalUnits 13107.20005368709

// Values used by the SD code and unit tests.
enum MafModeValues
{
	MafModeUndefined = 0,
	MafModeSensor = 1,
	MafModeSpeedDensity = 2,
};

// Values used by the RevMatch code and unit tests
enum RevMatchStates
{
	RevMatchDisabled = 0,
	RevMatchAlmostEnabled = 1,
	RevMatchEnabled = 2,
	RevMatchReadyForAccelerationDownshift = 3,
	RevMatchDecelerationDownshift = 4,
	RevMatchAccelerationDownshift = 5,
	RevMatchCalibration = 6,
	RevMatchExpired = 7,
};

enum CruiseFlagsABits
{
	CruiseFlagsAEnableButton = 1,
	CruiseFlagsAIsEnabled = 2,
	CruiseFlagsASetCoast = 4,
	CruiseFlagsAResumeAccel = 8,
	CruiseFlagsAHardBrake = 16,
	CruiseFlagsALightBrake = 32,
	CruiseFlagsACancel = 64,
	CruiseFlagsAClutch = 128	
};


// Values used by the flex-fuel injector scaling code and unit tests.
// Barely started, never finished.
// enum InjectorModeValues
// {
// 	InjectorModeUndefined = 0,
// 	InjectorModeDefault = 1,
// 	InjectorModeLeftTgv = 2,
// };

// In order to call Pull3d, we need to tell the compiler what arguments to 
// pass, and what return type to expect.  Since we'll be passing a pointer to
// the table structure, we don't actually need to specify the fields in the
// structure, but specifying the fields makes it easier to view the table 
// information in the debugger.
typedef struct 
{
	short columnCount;
	short rowCount;
	float* columnHeaderArray;
	float* rowHeaderArray;
	short* tableCells;
	int tableType;
	float multiplier;
	float offset;
} ThreeDimensionalTable;

// This is derived from "Table_MAF_Sensor_Scaling" in A2WC522N (see 0x00085438)
// That table uses float values for input and output. Tables that use byte or
// int values may be defined differently, or may use flags in the 'unknown' 
// members of this structure. Not sure, haven't investigated.
typedef struct
{
	char unknown1;
	char elementCount;
	char unknown2;
	char unknown3;
	float* inputArray;
	float* outputArray;
} TwoDimensionalTable;

// This is for calling Pull2d, using whatever values happen to be in r4 and 
// fr4.  That's handy when custom code is injected by hijacking a call to 
// Pull2d in the stock ROM.  If the first thing you do in the injected code
// is call this, you get the value that the stock ROM was going to get.
extern float Pull2dHooked();

// This is for calling Pull2d using explicit arguments.
extern float Pull2d(TwoDimensionalTable *table, float value);

// This is for calling Pull3d, with all parameters specified.
extern float Pull3d(ThreeDimensionalTable *table, float columnValue, float rowValue);

// This is the original code to read the accelerator pedal position.
extern void ReadAcceleratorSensors();

// Test utility functions.
void TestFailed(char *message) __attribute__ ((section ("Misc")));
void Assert(int condition, char *message) __attribute__ ((section ("Misc")));
int AreCloseEnough(float actual, float expected) __attribute__ ((section ("Misc")));

// Constants need to be declared as 'extern' with their values defined in the 
// function body or in an external asm file.  This is a convoluted way to 
// declare and define variables, but the standard approach causes the compiler
// to generate references to addresses in another segment, which makes it 
// harder to stitch this code into the ECU code.  
// 
// I suspect that turning on compiler optimizations might might cause the 
// compiler to generate the sort of code we want for constants and RAM 
// variables, however other optimizations also make the code harder to verify
// when stepping through it one instruction at a time.  For now I'd much 
// rather have code that's very easy to verify (and verify with the highest
// possible confidence), but at some point it might be worthwhile to turn on
// optimizations and see what happens.
extern float CelsiusToKelvin;
extern float Displacement;
extern float SpeedDensityConstant;
extern int DefaultMafMode;

extern float RedlineCut, RedlineResume;
extern float LaunchControlCut, LaunchControlResume;
extern float RevMatchFfsFuelCutDelta, RevMatchFfsFuelResumeDelta;
extern float FlatFootShiftSpeedThreshold;

extern float RevMatchMinimumSpeed;

#define MAX_COUNTER 1000000
