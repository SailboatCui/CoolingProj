//#############################################################################
//
// FILE:   epwm_ex2_updown_aq.c
//
// TITLE:  ePWM Action Qualifier Module - Using up/down count.
//
//! \addtogroup driver_example_list
//! <h1> ePWM Up Down Count Action Qualifier</h1>
//!
//! This example configures ePWM1, ePWM2, ePWM3 to produce a waveform with
//! independent modulation on ePWMxA and ePWMxB.
//!
//! The compare values CMPA and CMPB are modified within the ePWM's ISR.
//!
//! The TB counter is in up/down count mode for this example.
//!
//! View the ePWM1A/B(GPIO0 & GPIO1), ePWM2A/B(GPIO2 &GPIO3)
//! and ePWM3A/B(GPIO4 & GPIO5) waveforms on oscilloscope.
//
//#############################################################################
// $TI Release: F2837xD Support Library v3.10.00.00 $
// $Release Date: Tue May 26 17:13:46 IST 2020 $
// $Copyright:
// Copyright (C) 2013-2020 Texas Instruments Incorporated - http://www.ti.com/
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions 
// are met:
// 
//   Redistributions of source code must retain the above copyright 
//   notice, this list of conditions and the following disclaimer.
// 
//   Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the 
//   documentation and/or other materials provided with the   
//   distribution.
// 
//   Neither the name of Texas Instruments Incorporated nor the names of
//   its contributors may be used to endorse or promote products derived
//   from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// $
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
// Globals to hold the ePWM information used in this example
//
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
void InitEPWMInfox(epwmInformation *epwmInfo, float Frequency, float DutyRatio, float Offset);
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
    //
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

    //
    // Assign the interrupt service routines to ePWM interrupts
    //
//    Interrupt_register(INT_EPWM1, &epwm1ISR);
//    Interrupt_register(INT_EPWM2, &epwm2ISR);
//    Interrupt_register(INT_EPWM3, &epwm3ISR);

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

    //
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
    //
    // Update the CMPA and CMPB values
    //
    //updateCompare(&epwm1Info);

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

//
// initEPWM1 - Configure ePWM1
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

    //
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

    //
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
}

void InitEPWMInfox(epwmInformation *epwmInfo, float Frequency, float DutyRatio, float Offset)
{
    epwmInfo->epwmCounterPrdx = (5000/Frequency);
    epwmInfo->epwmPhasex = (5000/Frequency)*(1-Offset);
    epwmInfo->epwmCompActx = (5000/Frequency)*DutyRatio;
    epwmInfo->epwmCompBctx = (5000/Frequency)*DutyRatio;
}

