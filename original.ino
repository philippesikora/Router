/* cal_CTx_v_meter.ino
*/

#include <Arduino.h>
#include <TimerOne.h>

#define ADC_TIMER_PERIOD 125 // uS (determines the sampling rate / amount of idle time)

// Change this value to suit the local mains frequency
#define CYCLES_PER_SECOND 50

// Change this value to suit the electricity meter's Joules-per-flash rate (3600J = 1Wh).
#define ENERGY_BUCKET_CAPACITY_IN_JOULES 3600

// Modification for 3-phase PCB (Fred)
// ***************************
#define NO_OF_PHASES 3      // nb of phases
#define CURRENT_CAL_PHASE 0 // current phase/CT to be calibrated (L1-->0, L2-->1,L3-->2)

// definition of enumerated types
enum polarities
{
  NEGATIVE,
  POSITIVE
};
enum LED_states
{
  LED_ON,
  LED_OFF
}; // active low for use at the "trigger" port which is active low

// allocation of digital pins
// **************************
const byte outputForLED = 4; // <-- the "trigger" port is active-low

// allocation of analogue pins
// ***************************
const byte voltageSensor[NO_OF_PHASES] = {0, 2, 4};      // Voltage Sensors for 3-phase PCB
const byte currentSensor_grid[NO_OF_PHASES] = {1, 3, 5}; // Current Sensors for 3-phase PCB

const byte delayBeforeSerialStarts = 1; // in seconds, to allow Serial window to be opened
const byte startUpPeriod = 3;           // in seconds, to allow LP filter to settle
const int DCoffset_I = 512;             // nominal mid-point value of ADC @ x1 scale

long cycleCount = 0; // used to time LED events, rather than calling millis()
int samplesDuringThisMainsCycle = 0;

// General global variables that are used in multiple blocks so cannot be static.
// For integer maths, many variables need to be 'long'
long sumP_grid = 0;                 // for per-cycle summation of 'real power'
boolean beyondStartUpPhase = false; // start-up delay, allows things to settle
long energyInBucket_long;           // in Integer Energy Units
long capacityOfEnergyBucket_long;   // depends on powerCal, frequency & the 'sweetzone' size.
long DCoffset_V_long;               // <--- for LPF
long DCoffset_V_min;                // <--- for LPF
long DCoffset_V_max;                // <--- for LPF

// for interaction between the main processor and the ISRs
volatile boolean dataReady = false;
volatile int sampleI_grid;
volatile int sampleV;

// For an enhanced polarity detection mechanism, which includes a persistence check
#define PERSISTENCE_FOR_POLARITY_CHANGE 1 // sample sets
enum polarities polarityOfMostRecentVsample;
enum polarities polarityConfirmed;
enum polarities polarityConfirmedOfLastSampleV;

// For a mechanism to check the continuity of the sampling sequence
#define CONTINUITY_CHECK_MAXCOUNT 250 // mains cycles
int sampleCount_forContinuityChecker;
int sampleSetsDuringThisMainsCycle;
int lowestNoOfSampleSetsPerMainsCycle;

// for control of the LED at the "trigger" port (port D4)
enum LED_states LED_state;
boolean LED_pulseInProgress = false;
unsigned long LED_onAt;

const float powerCal_grid[NO_OF_PHASES] = {1.0, 1.0, 1.0};
const float phaseCal[NO_OF_PHASES] = {1.0, 1.0, 1.0}; 
int phaseCal_int[NO_OF_PHASES];                       

void setup()
{
  pinMode(outputForLED, OUTPUT);
  delay(100);
  LED_state = LED_ON;                    // to mimic the behaviour of an electricity
  digitalWrite(outputForLED, LED_state); // meter which starts up in 'sleep' mode

  delay(delayBeforeSerialStarts * 1000); // allow time to open Serial monitor

  Serial.begin(9600);
 
  // When using integer maths, the SIZE of the ENERGY BUCKET is altered to match the
  // scaling of the energy detection mechanism that is in use. This avoids the need
  // to re-scale every energy contribution, thus saving processing time. This process
  // is described in more detail in the function, allGeneralProcessing(), just before
  // the energy bucket is updated at the start of each new cycle of the mains.
  //
  // For the flow of energy at the 'grid' connection point (CTx)
  capacityOfEnergyBucket_long =
    (long)ENERGY_BUCKET_CAPACITY_IN_JOULES * CYCLES_PER_SECOND * (1 / powerCal_grid[CURRENT_CAL_PHASE]);
  energyInBucket_long = 0;

  // When using integer maths, calibration values that have supplied in floating point
  // form need to be rescaled.
  //
  phaseCal_int[CURRENT_CAL_PHASE] = phaseCal[CURRENT_CAL_PHASE] * 256; // for integer maths

  // Define operating limits for the LP filter which identifies DC offset in the voltage
  // sample stream. By limiting the output range, the filter always should start up
  // correctly.
  DCoffset_V_long = 512L * 256;              // nominal mid-point value of ADC @ x256 scale
  DCoffset_V_min = (long)(512L - 100) * 256; // mid-point of ADC minus a working margin
  DCoffset_V_max = (long)(512L + 100) * 256; // mid-point of ADC plus a working margin

  // Set up the ADC to be triggered by a hardware timer of fixed duration
  ADCSRA = (1 << ADPS0) + (1 << ADPS1) + (1 << ADPS2); // Set the ADC's clock to system clock / 128
  ADCSRA |= (1 << ADEN);                               // Enable ADC

  Timer1.initialize(ADC_TIMER_PERIOD); // set Timer1 interval
  Timer1.attachInterrupt(timerIsr);    // declare timerIsr() as interrupt service routine
}

void timerIsr(void)
{
  static byte sample_index = 0;
  static int sampleI_grid_raw;

  switch (sample_index)
  {
    case 0:
      sampleV = ADC;                                   		// store the ADC value (this one is for Voltage)
      ADMUX = 0x40 + currentSensor_grid[CURRENT_CAL_PHASE]; // set up the next conversion, which is for Grid Current
      ADCSRA |= (1 << ADSC);                                // start the ADC
      ++sample_index;                                       // increment the control flag
      sampleI_grid = sampleI_grid_raw;
      dataReady = true; // all three ADC values can now be processed
      break;
    case 1:
      sampleI_grid_raw = ADC;                          // store the ADC value (this one is for Grid Current)
      ADMUX = 0x40 + voltageSensor[CURRENT_CAL_PHASE]; // set up the next conversion, which is for Voltage
      ADCSRA |= (1 << ADSC);                           // start the ADC
      sample_index = 0;                                // reset the control flag
      break;
    default:
      sample_index = 0; // to prevent lockup (should never get here)
  }
}

void loop()
{
  if (dataReady) // flag is set after every set of ADC conversions
  {
    dataReady = false;      // reset the flag
    allGeneralProcessing(); // executed once for each set of V&I samples
  }
} // end of loop()

// This routine is called to process each set of V & I samples. The main processor and
// the ADC work autonomously, their operation being only linked via the dataReady flag.
// As soon as a new set of data is made available by the ADC, the main processor can
// start to work on it immediately.
//
void allGeneralProcessing()
{
  static long sumVdeltasThisCycle_long; // for the LPF which determines DC offset (voltage)
  static long lastSampleVminusDC_long;  // for the phaseCal algorithm

  // remove DC offset from the raw voltage sample by subtracting the accurate value
  // as determined by a LP filter.
  long sampleVminusDC_long = ((long)sampleV << 8) - DCoffset_V_long;

  // determine the polarity of the latest voltage sample
  polarityOfMostRecentVsample = (sampleVminusDC_long > 0) ? POSITIVE : NEGATIVE;

  confirmPolarity();

  if (POSITIVE == polarityConfirmed)
  {
    if (POSITIVE != polarityConfirmedOfLastSampleV)
    {
      // This is the start of a new +ve half cycle (just after the zero-crossing point)
      ++cycleCount;
      processPlusHalfCycle();
    }  // end of processing that is specific to the first Vsample in each +ve half cycle
  }    // end of processing that is specific to samples where the voltage is positive
  else // the polarity of this sample is negative
  {
    if (NEGATIVE != polarityConfirmedOfLastSampleV)
    {
      // This is the start of a new -ve half cycle (just after the zero-crossing point)
      // which is a convenient point to update the Low Pass Filter for DC-offset removal
      //  The portion which is fed back into the integrator is approximately one percent
      // of the average offset of all the Vsamples in the previous mains cycle.
      //
      DCoffset_V_long += (sumVdeltasThisCycle_long >> 12);
      sumVdeltasThisCycle_long = 0;

      // To ensure that the LPF will always start up correctly when 240V AC is available, its
      // output value needs to be prevented from drifting beyond the likely range of the
      // voltage signal. This avoids the need to use a HPF as was done for initial Mk2 builds.
      //
      if (DCoffset_V_long < DCoffset_V_min)
        DCoffset_V_long = DCoffset_V_min;
      else if (DCoffset_V_long > DCoffset_V_max)
        DCoffset_V_long = DCoffset_V_max;

    } // end of processing that is specific to the first Vsample in each -ve half cycle
  }   // end of processing that is specific to samples where the voltage is negative

  // processing for EVERY set of samples
  //
  // First, deal with the power at the grid connection point (as measured via CT1)
  // remove most of the DC offset from the current sample (the precise value does not matter)
  long sampleIminusDC_grid = ((long)(sampleI_grid - DCoffset_I)) << 8;

  // phase-shift the voltage waveform so that it aligns with the grid current waveform
  long phaseShiftedSampleVminusDC_grid = lastSampleVminusDC_long +
                                         (((sampleVminusDC_long - lastSampleVminusDC_long) * phaseCal_int[CURRENT_CAL_PHASE]) >> 8);
  // long  phaseShiftedSampleVminusDC_grid = sampleVminusDC_long; // <- simple version for when
  // phaseCal is not in use

  // calculate the "real power" in this sample pair and add to the accumulated sum
  long filtV_div4 = phaseShiftedSampleVminusDC_grid >> 2; // reduce to 16-bits (now x64, or 2^6)
  long filtI_div4 = sampleIminusDC_grid >> 2;             // reduce to 16-bits (now x64, or 2^6)
  long instP = filtV_div4 * filtI_div4;                   // 32-bits (now x4096, or 2^12)
  instP >>= 12;                                           // scaling is now x1, as for Mk2 (V_ADC x I_ADC)
  sumP_grid += instP;                                     // cumulative power, scaling as for Mk2 (V_ADC x I_ADC)

  ++sampleSetsDuringThisMainsCycle;

  // store items for use during next loop
  sumVdeltasThisCycle_long += sampleVminusDC_long;    // for use with LP filter
  lastSampleVminusDC_long = sampleVminusDC_long;      // required for phaseCal algorithm
  polarityConfirmedOfLastSampleV = polarityConfirmed; // for identification of half cycle boundaries
}
//  ----- end of main Mk2i code -----

void processPlusHalfCycle()
{
  if (!beyondStartUpPhase)
  {
    // wait until the DC-blocking filters have had time to settle
    if (millis() > (delayBeforeSerialStarts + startUpPeriod) * 1000)
    {
      beyondStartUpPhase = true;
      sumP_grid = 0;
      sampleSetsDuringThisMainsCycle = 0;   // not yet dealt with for this cycle
      sampleCount_forContinuityChecker = 1; // opportunity has been missed for this cycle
      lowestNoOfSampleSetsPerMainsCycle = 999;
    }

    return;
  }

  // a simple routine for checking the performance of this new ISR structure
  if (sampleSetsDuringThisMainsCycle < lowestNoOfSampleSetsPerMainsCycle)
    lowestNoOfSampleSetsPerMainsCycle = sampleSetsDuringThisMainsCycle;

  // Calculate the real power and energy during the last whole mains cycle.
  //
  // sumP contains the sum of many individual calculations of instantaneous power. In
  // order to obtain the average power during the relevant period, sumP must first be
  // divided by the number of samples that have contributed to its value.
  //
  // The next stage would normally be to apply a calibration factor so that real power
  // can be expressed in Watts. That's fine for floating point maths, but it's not such
  // a good idea when integer maths is being used. To keep the numbers large, and also
  // to save time, calibration of power is omitted at this stage. Real Power (stored as
  // a 'long') is therefore (1/powerCal) times larger than the actual power in Watts.
  //
  long realPower_grid = sumP_grid / sampleSetsDuringThisMainsCycle; // proportional to Watts

  // Next, the energy content of this power rating needs to be determined. Energy is
  // power multiplied by time, so the next step is normally to multiply the measured
  // value of power by the time over which it was measured.
  //   Instantaneous power is calculated once every mains cycle. When integer maths is
  // being used, a repetitive power-to-energy conversion seems an unnecessary workload.
  // As all sampling periods are of similar duration, it is more efficient simply to
  // add all of the power samples together, and note that their sum is actually
  // CYCLES_PER_SECOND greater than it would otherwise be.
  //   Although the numerical value itself does not change, I thought that a new name
  // may be helpful so as to minimise confusion.
  //   The 'energy' variable below is CYCLES_PER_SECOND * (1/powerCal) times larger than
  // the actual energy in Joules.
  //
  long realEnergy_grid = realPower_grid;

  // Energy contributions from the grid connection point (CT1) are summed in an
  // accumulator which is known as the energy bucket. The purpose of the energy bucket
  // is to mimic the operation of the supply meter. Most meters generate a visible pulse
  // when a certain amount of forward energy flow has been recorded, often 3600 Joules.
  // For this calibration sketch, the capacity of the energy bucket is set to this same
  // value within setup().
  //
  // The latest contribution can now be added to this energy bucket
  energyInBucket_long += realEnergy_grid;

  // when operating as a cal program
  if (energyInBucket_long > capacityOfEnergyBucket_long)
  {
    energyInBucket_long -= capacityOfEnergyBucket_long;
    registerConsumedPower(realPower_grid);
  }

  if (energyInBucket_long < 0)
  {
    digitalWrite(outputForLED, LED_ON); // to mimic the nehaviour of an electricity meter
    energyInBucket_long = 0;
  }

  // continuity checker
  ++sampleCount_forContinuityChecker;
  if (sampleCount_forContinuityChecker >= CONTINUITY_CHECK_MAXCOUNT)
  {
    sampleCount_forContinuityChecker = 0;
    lowestNoOfSampleSetsPerMainsCycle = 999;
  }

  // clear the per-cycle accumulators for use in this new mains cycle.
  sampleSetsDuringThisMainsCycle = 0;
  sumP_grid = 0;
  check_LED_status();
}

void confirmPolarity()
{
  /* This routine prevents a zero-crossing point from being declared until
     a certain number of consecutive samples in the 'other' half of the
     waveform have been encountered.
  */
  static byte count = 0;
  if (polarityOfMostRecentVsample != polarityConfirmedOfLastSampleV)
    ++count;
  else
    count = 0;

  if (count > PERSISTENCE_FOR_POLARITY_CHANGE)
  {
    count = 0;
    polarityConfirmed = polarityOfMostRecentVsample;
  }
}

int freeRam()
{
  extern int __heap_start, *__brkval;
  int v;
  return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

void registerConsumedPower(const long powerRaw)
{
  LED_onAt = cycleCount;
  LED_state = LED_ON;
  digitalWrite(outputForLED, LED_state);
  LED_pulseInProgress = true;
  
  Serial.print(R"({"L1":)");
  Serial.print((long)(powerRaw * powerCal_grid[CURRENT_CAL_PHASE]));

  Serial.print(R"(,"L2":)");
  Serial.print(0);

  Serial.print(R"(,"L3":)");
  Serial.print(0);
   
  Serial.print(R"(,"LOAD_0":)");
  Serial.print(0);
   
  Serial.print(R"(,"LOAD_1":)");
  Serial.print(0);
   
  Serial.print(R"(,"LOAD_2":)");
  Serial.print(0);
   
  Serial.println("}");
}

void check_LED_status()
{
  if (LED_pulseInProgress == false)
    return;

  if (cycleCount > (LED_onAt + 2)) // normal pulse duration
  {
    LED_state = LED_OFF;
    digitalWrite(outputForLED, LED_state);
    LED_pulseInProgress = false;
  }
}
