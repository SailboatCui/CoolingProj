//#############################################################################
//
// FILE:   coolingproj_4ch_epwm.c
//
// TITLE:  based on ePWM Action Qualifier Module - Using up count / 4 modules synchronziation
//
//! \addtogroup driver_example_list
//! <h1> ePWM Up Down Count Action Qualifier</h1>
//!
//! This example configures ePWM1, ePWM2, ePWM3 to produce a waveform with
//! independent modulation on ePWMxA and ePWMxB.
//!
//! The compare values CMPA and CMPB are modified within the ePWM's ISR.
//!
//! The TB counter is in up count mode for this example.
//!
//! View the ePWM1A/B(GPIO0 & GPIO1), ePWM2A/B(GPIO2 &GPIO3)
//! and ePWM3A/B(GPIO4 & GPIO5) waveforms on oscilloscope.
//
//#############################################################################
// $TI Release: F2837xD Support Library v3.10.00.00 $
// $Release Date: 08/27 IST 2020 $
// $Copyright:
// Xiaofan Cui
//#############################################################################

//
// Included Files
//
#include "driverlib.h"
#include "device.h"
#include "command.h"
//
// Defines
//

//
// Globals
//
typedef struct
{
    uint32_t epwmModule;
    uint16_t epwmCompADirection;
    uint16_t epwmCompBDirection;
    uint16_t epwmTimerIntCount;
    uint16_t epwmMaxCompA;
    uint16_t epwmMinCompA;
    uint16_t epwmMaxCompB;
    uint16_t epwmMinCompB;
    uint16_t epwmPhasex;
    uint16_t epwmCompActx;
    uint16_t epwmCompBctx;
    uint16_t epwmCounterPrdx;
}epwmInformation;

//
// Globals
volatile uint16_t sData[2];         // Send data buffer
volatile uint16_t rData[2];         // Receive data buffer
volatile uint16_t rDataPoint = 0;   // To keep track of where we are in the
                                    // data stream to check received data
// Globals to hold the ePWM information used in this example
epwmInformation epwm1Info;
epwmInformation epwm2Info;
epwmInformation epwm3Info;
epwmInformation epwm4Info;
//
// Function Prototypes
//
void initEPWM(uint32_t base, epwmInformation *epwmInfo);
__interrupt void epwm1ISR(void);
__interrupt void epwm2ISR(void);
__interrupt void epwm3ISR(void);
__interrupt void epwm4ISR(void);
void InitEPWMInfox(epwmInformation *epwmInfo, float Frequency, float DutyRatio, float Offset);
void initSPIA(void);
void initSPIB(void);
__interrupt void spiRxFIFOISR(void);
void configGPIOs(void);
//
// Main
//
void main(void)
{
        // Add this for Flash

        //
        // Copy time critical code and Flash setup code to RAM
        // This includes InitFlash(), Flash API functions and any functions that are
        // assigned to ramfuncs section.
        // The  RamfuncsLoadStart, RamfuncsLoadEnd, and RamfuncsRunStart
        // symbols are created by the linker. Refer to the device .cmd file.
        //
           //memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (size_t)&RamfuncsLoadSize);
        //
        // WARNING: Always ensure you call memcpy before running any functions from
        // RAM InitSysCtrl includes a call to a RAM based function and without a
        // call to memcpy first, the processor will go "into the weeds"
        //
        #ifdef _FLASH
            memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (size_t)&RamfuncsLoadSize);
        #endif
        //
    uint16_t i;

    // Initialize device clock and peripherals
    // Note that SYSCLK is set to be 100MHz, not the 200MHz as expected!
    Device_init();

    //
    // Disable pin locks and enable internal pull ups.
    //
    Device_initGPIO();

    //
    // Initialize PIE and clear PIE registers. Disables CPU interrupts.
    //
    Interrupt_initModule();

    //
    // Initialize the PIE vector table with pointers to the shell Interrupt
    // Service Routines (ISR).
    //
    Interrupt_initVectorTable();

    // Interrupts that are used in this example are re-mapped to ISR functions
    // found within this file.
    //
//    Interrupt_register(INT_SPIA_TX, &spiTxFIFOISR);
    Interrupt_register(INT_SPIA_RX, &spiRxFIFOISR);

    //
    // Set up SPI
    //
    initSPIA();  // initializing SPIA for FIFO mode
    initSPIB();  // initializing SPIB for FIFO mode

    // Initialize the data buffers
    //
    for(i = 0; i < 2; i++)
    {
        sData[i] = 1;
        rData[i]= 0;
    }
    //
    // Assign the interrupt service routines to ePWM interrupts
    //
      Interrupt_register(INT_EPWM1, &epwm1ISR);
//    Interrupt_register(INT_EPWM2, &epwm2ISR);
//    Interrupt_register(INT_EPWM3, &epwm3ISR);

    InitEPWMInfox(&epwm1Info, Frequencyx, DutyRatio1x, Offset1x);
    InitEPWMInfox(&epwm2Info, Frequencyx, DutyRatio2x, Offset2x);
    InitEPWMInfox(&epwm3Info, Frequencyx, DutyRatio3x, Offset3x);
    InitEPWMInfox(&epwm4Info, Frequencyx, DutyRatio4x, Offset4x);
    //
    // Disable sync(Freeze clock to PWM as well)
    //
    SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);

    // Initialize PWM1 without phase shift as master
    initEPWM(EPWM1_BASE, &epwm1Info);

    // Initialize PWM2 with phase shift as a slave
    //
    initEPWM(EPWM2_BASE, &epwm2Info);
    EPWM_selectPeriodLoadEvent(EPWM2_BASE, EPWM_SHADOW_LOAD_MODE_SYNC);
    EPWM_setPhaseShift(EPWM2_BASE, epwm2Info.epwmPhasex);
    EPWM_setTimeBaseCounter(EPWM2_BASE, epwm2Info.epwmPhasex);

    //
    // Initialize PWM3 with phase shift as a slave
    //
    initEPWM(EPWM3_BASE, &epwm3Info);
    EPWM_selectPeriodLoadEvent(EPWM3_BASE, EPWM_SHADOW_LOAD_MODE_SYNC);
    EPWM_setPhaseShift(EPWM3_BASE, epwm3Info.epwmPhasex);
    EPWM_setTimeBaseCounter(EPWM3_BASE, epwm3Info.epwmPhasex);

    //
    // Initialize PWM4 with phase shift as a slave
    //
    initEPWM(EPWM4_BASE, &epwm4Info);
    EPWM_selectPeriodLoadEvent(EPWM4_BASE, EPWM_SHADOW_LOAD_MODE_SYNC);
    EPWM_setPhaseShift(EPWM4_BASE, epwm4Info.epwmPhasex);
    EPWM_setTimeBaseCounter(EPWM4_BASE, epwm4Info.epwmPhasex);

    //
    // ePWM1 SYNCO is generated on CTR=0
    //
    EPWM_setSyncOutPulseMode(EPWM1_BASE, EPWM_SYNC_OUT_PULSE_ON_COUNTER_ZERO);
    // ePWM2 uses the ePWM 1 SYNCO as its SYNCIN by default
    // set PULSEMODE so that ePWM2 SYNCO is generated from its SYNCIN, which is ePWM1 SYNCO
    //
    EPWM_setSyncOutPulseMode(EPWM2_BASE, EPWM_SYNC_OUT_PULSE_ON_EPWMxSYNCIN);
    // ePWM3 uses the ePWM 2 SYNCO as its SYNCIN by default
    // ePWM3 SYNCO is generated from its SYNCIN, which is ePWM2/1 SYNCO
    //
    EPWM_setSyncOutPulseMode(EPWM3_BASE, EPWM_SYNC_OUT_PULSE_ON_EPWMxSYNCIN);
    // ePWM4 uses the ePWM3  SYNCO as its SYNCIN by default
    // ePWM4 SYNCO is generated from its SYNCIN, which is ePWM3/2/1 SYNCO
    //
    EPWM_setSyncOutPulseMode(EPWM4_BASE, EPWM_SYNC_OUT_PULSE_ON_EPWMxSYNCIN);
    //SysCtl_setSyncInputConfig(SYSCTL_SYNC_IN_EPWM4, SYSCTL_SYNC_IN_SRC_EPWM1SYNCOUT);   // ePWM4 uses the ePWM 1 SYNCO

    //
    // Enable all phase shifts.
    //
    EPWM_enablePhaseShiftLoad(EPWM2_BASE);
    EPWM_enablePhaseShiftLoad(EPWM3_BASE);
    EPWM_enablePhaseShiftLoad(EPWM4_BASE);
    //
    // Enable sync and clock to PWM
    //
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);

    //
    // Enable ePWM interrupts
    //
//    Interrupt_enable(INT_EPWM1);
//    Interrupt_enable(INT_EPWM2);
//    Interrupt_enable(INT_EPWM3);
//    Interrupt_enable(INT_EPWM4);

    // Enable the SPIA Rx interrupt
//    Interrupt_enable(INT_SPIA_TX);
//   Interrupt_enable(INT_SPIA_RX);


    // Enable Global Interrupt (INTM) and realtime interrupt (DBGM)
    //
    EINT;
    ERTM;

    //
    // IDLE loop. Just sit and loop forever (optional):
    //
    for(;;)
    {
        NOP;
    }
}

//
// epwm1ISR - ePWM 1 ISR
//
__interrupt void epwm1ISR(void)
{
      // SPI B Transmit data in EPWM interrupt
      uint16_t i;
      // Send data
      //
      for(i = 0; i < 2; i++)
      {
         SPI_writeDataNonBlocking(SPIB_BASE, sData[i]);
      }
      // Increment data for next cycle
      //
      for(i = 0; i < 2; i++)
      {
         sData[i] = 1;
      }
    //
    // Clear INT flag for this timer
    //
    EPWM_clearEventTriggerInterruptFlag(EPWM1_BASE);

    //
    // Acknowledge interrupt group
    //
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP3);
}

//
// epwm2ISR - ePWM 2 ISR
//
__interrupt void epwm2ISR(void)
{
    //
    // Update the CMPA and CMPB values
    //
    //updateCompare(&epwm2Info);

    //
    // Clear INT flag for this timer
    //
    EPWM_clearEventTriggerInterruptFlag(EPWM2_BASE);

    //
    // Acknowledge interrupt group
    //
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP3);
}

//
// epwm3ISR - ePWM 3 ISR
//
__interrupt void epwm3ISR(void)
{
    //
    // Update the CMPA and CMPB values
    //
    //updateCompare(&epwm3Info);

    //
    // Clear INT flag for this timer
    //
    EPWM_clearEventTriggerInterruptFlag(EPWM3_BASE);

    //
    // Acknowledge interrupt group
    //
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP3);
}
// epwm4ISR - ePWM 4 ISR
//
__interrupt void epwm4ISR(void)
{
    //
    // Update the CMPA and CMPB values
    //
    //updateCompare(&epwm3Info);

    //
    // Clear INT flag for this timer
    //
    EPWM_clearEventTriggerInterruptFlag(EPWM4_BASE);

    //
    // Acknowledge interrupt group
    //
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP3);
}

// SPI A Receive FIFO ISR
//
 __interrupt void spiRxFIFOISR(void)
{
    uint16_t i;
    //
    // Read data
    //
    for(i = 0; i < 2; i++)
    {
        rData[i] = SPI_readDataNonBlocking(SPIA_BASE);
    }

    //
    // Check received data
    //
    for(i = 0; i < 2; i++)
    {
        if(rData[i] != (rDataPoint + i))
        {
            // Something went wrong. rData doesn't contain expected data.
            Example_Fail = 1;
            ESTOP0;
        }
    }

    rDataPoint++;

    //
    // Clear interrupt flag and issue ACK
    //
    SPI_clearInterruptStatus(SPIA_BASE, SPI_INT_RXFF);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP6);

    Example_PassCount++;
}

// Configure GPIOs for external loopback.
//
void configGPIOs(void)
{
    //
       // Configure GPIO0/1 , GPIO2/3, GPIO4/5, GPIO6/7 and as ePWM1A/1B, ePWM2A/2B,
       // ePWM3A/3B, ePWM4A/4B pins respectively
       //
       GPIO_setPadConfig(0, GPIO_PIN_TYPE_STD);
       GPIO_setPinConfig(GPIO_0_EPWM1A);
       GPIO_setPadConfig(1, GPIO_PIN_TYPE_STD);
       GPIO_setPinConfig(GPIO_1_EPWM1B);

       GPIO_setPadConfig(2, GPIO_PIN_TYPE_STD);
       GPIO_setPinConfig(GPIO_2_EPWM2A);
       GPIO_setPadConfig(3, GPIO_PIN_TYPE_STD);
       GPIO_setPinConfig(GPIO_3_EPWM2B);

       GPIO_setPadConfig(4, GPIO_PIN_TYPE_STD);
       GPIO_setPinConfig(GPIO_4_EPWM3A);
       GPIO_setPadConfig(5, GPIO_PIN_TYPE_STD);
       GPIO_setPinConfig(GPIO_5_EPWM3B);

       GPIO_setPadConfig(6, GPIO_PIN_TYPE_STD);
       GPIO_setPinConfig(GPIO_6_EPWM4A);
       GPIO_setPadConfig(7, GPIO_PIN_TYPE_STD);
       GPIO_setPinConfig(GPIO_7_EPWM4B);
    //
    // This test is designed for an external loopback between SPIA master
    // and SPIB slave
    // External Connections:
    // -GPIO58 -> GPIO63 - SPISIMOA -> SPISIMOB
    // -GPIO59 <- GPIO64 - SPISOMIA <- SPISOMIB
    // -GPIO1 (EPWM1#)-> GPIO66 - CS of SPISTEB, THE CHIP SELECTION OF B
    // -GPIO60 and GPIO65 - SPIACLK -  SPIBCLK
    //
    // GPIO59 is the SPISOMIA.
    //
    GPIO_setMasterCore(59, GPIO_CORE_CPU1);
    GPIO_setPinConfig(GPIO_59_SPISOMIA);
    GPIO_setPadConfig(59, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(59, GPIO_QUAL_ASYNC);

    //
    // GPIO58 is the SPISIMOA clock pin.
    //
    GPIO_setMasterCore(58, GPIO_CORE_CPU1);
    GPIO_setPinConfig(GPIO_58_SPISIMOA);
    GPIO_setPadConfig(58, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(58, GPIO_QUAL_ASYNC);

    //
    // A is master, so we donot need to set SPISTEA.
//    GPIO_setMasterCore(19, GPIO_CORE_CPU1);
//    GPIO_setPinConfig(GPIO_19_SPISTEA);
//    GPIO_setPadConfig(19, GPIO_PIN_TYPE_PULLUP);
//    GPIO_setQualificationMode(19, GPIO_QUAL_ASYNC);

    //
    // GPIO60 is the SPICLKA.
    //
    GPIO_setMasterCore(60, GPIO_CORE_CPU1);
    GPIO_setPinConfig(GPIO_60_SPICLKA);
    GPIO_setPadConfig(60, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(60, GPIO_QUAL_ASYNC);

    //
    // GPIO64 is the SPISOMIB.
    //
    GPIO_setMasterCore(64, GPIO_CORE_CPU1);
    GPIO_setPinConfig(GPIO_64_SPISOMIB);
    GPIO_setPadConfig(64, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(64, GPIO_QUAL_ASYNC);

    //
    // GPIO63 is the SPISIMOB clock pin.
    //
    GPIO_setMasterCore(63, GPIO_CORE_CPU1);
    GPIO_setPinConfig(GPIO_63_SPISIMOB);
    GPIO_setPadConfig(63, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(63, GPIO_QUAL_ASYNC);

    //
    // we the GPIO66 is the SPISTEB.
    //
    GPIO_setMasterCore(66, GPIO_CORE_CPU1);
    GPIO_setPinConfig(GPIO_66_SPISTEB);
    GPIO_setPadConfig(66, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(66, GPIO_QUAL_ASYNC);

    //
    // GPIO65 is the SPICLKB.
    //
    GPIO_setMasterCore(65, GPIO_CORE_CPU1);
    GPIO_setPinConfig(GPIO_65_SPICLKB);
    GPIO_setPadConfig(65, GPIO_PIN_TYPE_PULLUP);
    GPIO_setQualificationMode(65, GPIO_QUAL_ASYNC);
}


//
// initEPWMx - Configure ePWMx
//
void initEPWM(uint32_t base, epwmInformation *epwmInfo)
{
    //
    // Set-up TBCLK
    //
    EPWM_setTimeBasePeriod(base, epwmInfo->epwmCounterPrdx);
 //   EPWM_selectPeriodLoadEvent(EPWM2_BASE, EPWM_SHADOW_LOAD_MODE_SYNC);// disable the phase shift loading when the syncronization event happen,
    //it is a master
//    EPWM_enablePhaseShiftLoad(EPWM2_BASE);
    EPWM_setPhaseShift(base, 0U);
    EPWM_setTimeBaseCounter(base,0U);
    //
    // Set Compare values
    //
    EPWM_setCounterCompareValue(base,
                                EPWM_COUNTER_COMPARE_A,
                                epwmInfo->epwmCompActx);
    EPWM_setCounterCompareValue(base,
                                EPWM_COUNTER_COMPARE_B,
                                epwmInfo->epwmCompBctx);

    // Set up counter mode
    //
    EPWM_setTimeBaseCounterMode(base, EPWM_COUNTER_MODE_UP);
    EPWM_disablePhaseShiftLoad(base);
    EPWM_setClockPrescaler(base,
                           EPWM_CLOCK_DIVIDER_1,
                           EPWM_HSCLOCK_DIVIDER_10);
    //! \b Note: EPWMCLK is a scaled version of SYSCLK. At reset EPWMCLK is half
    //!          SYSCLK. Given SYSCLK = 100MHz, EPWMCLK = 50MHz
    //!   TBCLK = EPWMCLK/(highSpeedPrescaler * pre-scaler) = 50MHz/10 = 5 MHz
    //!
    //
    // Set up shadowing
    //
    EPWM_setCounterCompareShadowLoadMode(base,
                                         EPWM_COUNTER_COMPARE_A,
                                         EPWM_COMP_LOAD_ON_CNTR_ZERO);
    EPWM_setCounterCompareShadowLoadMode(base,
                                         EPWM_COUNTER_COMPARE_B,
                                         EPWM_COMP_LOAD_ON_CNTR_ZERO);

    // Set actions
    // EPWMxA is set 1 when counter == 0
    EPWM_setActionQualifierAction(base,
                                  EPWM_AQ_OUTPUT_A,
                                  EPWM_AQ_OUTPUT_HIGH,
                                  EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);
    // EPWMxA is set 0 when counter > compA
    EPWM_setActionQualifierAction(base,
                                  EPWM_AQ_OUTPUT_A,
                                  EPWM_AQ_OUTPUT_LOW,
                                  EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPA);
    // EPWMxB is set 0 when counter == 0
    EPWM_setActionQualifierAction(base,
                                  EPWM_AQ_OUTPUT_B,
                                  EPWM_AQ_OUTPUT_LOW,
                                  EPWM_AQ_OUTPUT_ON_TIMEBASE_ZERO);
    // EPWMxB is set 1 when counter > compB
    EPWM_setActionQualifierAction(base,
                                  EPWM_AQ_OUTPUT_B,
                                  EPWM_AQ_OUTPUT_HIGH,
                                  EPWM_AQ_OUTPUT_ON_TIMEBASE_UP_CMPB);

    // Interrupts
    // Select INT on Time base counter zero event,
    // Enable INT, generate INT on 1rd event
    //
    EPWM_setInterruptSource(base, EPWM_INT_TBCTR_ZERO);
    EPWM_enableInterrupt(base);
    EPWM_setInterruptEventCount(base, 1U);
}

void InitEPWMInfox(epwmInformation *epwmInfo, float Frequency, float DutyRatio, float Offset)
{
    epwmInfo->epwmCounterPrdx = (5000/Frequency);
    epwmInfo->epwmPhasex = (5000/Frequency)*(1-Offset);
    epwmInfo->epwmCompActx = (5000/Frequency)*DutyRatio;
    epwmInfo->epwmCompBctx = (5000/Frequency)*DutyRatio;
}

// Function to configure SPI A as master with FIFO enabled, SPIA receives the data from the SPIB
//
void initSPIA(void)
{
    //
    // Must put SPI into reset before configuring it
    //
    SPI_disableModule(SPIA_BASE);
    // SPI configuration. Use a 500kHz SPICLK and 16-bit word size.
    //
    SPI_setConfig(SPIA_BASE, DEVICE_LSPCLK_FREQ, SPI_PROT_POL0PHA0,    //! Mode 0. Polarity 0, phase 0. Rising edge without delay.
                  SPI_MODE_MASTER, 1000000, 16);
    SPI_disableLoopback(SPIA_BASE);
    SPI_setEmulationMode(SPIA_BASE, SPI_EMULATION_FREE_RUN);
    //
    // FIFO and interrupt configuration
    SPI_enableFIFO(SPIA_BASE);
    SPI_clearInterruptStatus(SPIA_BASE, SPI_INT_RXFF);
    SPI_setFIFOInterruptLevel(SPIA_BASE, SPI_FIFO_TX2, SPI_FIFO_RX2);   // interrupt happen when the FIFO level =2
    SPI_enableInterrupt(SPIA_BASE, SPI_INT_RXFF);

    //
    // Configuration complete. Enable the module.
    //
    SPI_enableModule(SPIA_BASE);
}

// Function to configure SPI B as slave with FIFO enabled, SPIA receives the data from the SPIB
void initSPIB(void)
{
    //mySPI0 initialization
    SPI_disableModule(SPIB_BASE);
    SPI_setConfig(SPIB_BASE, DEVICE_LSPCLK_FREQ, SPI_PROT_POL0PHA0,
                  SPI_MODE_SLAVE, 1000000, 16);
    SPI_disableLoopback(SPIB_BASE);
    SPI_setEmulationMode(SPIB_BASE, SPI_EMULATION_FREE_RUN);
    SPI_enableModule(SPIB_BASE);
}
