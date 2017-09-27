/*
 * @brief LPCSPIFILIB example
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2013
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "board.h"
#include <string.h>
#include "spifilib_api.h"

#include "test_suite.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
#define TICKRATE_HZ1 (1000)	/* 1000 ticks per second I.e 1 mSec / tick */

#ifndef SPIFLASH_BASE_ADDRESS
#define SPIFLASH_BASE_ADDRESS (0x14000000)
#endif

STATIC const PINMUX_GRP_T spifipinmuxing[] = {
	{0x3, 3,  (SCU_PINIO_FAST | SCU_MODE_FUNC3)},	/* SPIFI CLK */
	{0x3, 4,  (SCU_PINIO_FAST | SCU_MODE_FUNC3)},	/* SPIFI D3 */
	{0x3, 5,  (SCU_PINIO_FAST | SCU_MODE_FUNC3)},	/* SPIFI D2 */
	{0x3, 6,  (SCU_PINIO_FAST | SCU_MODE_FUNC3)},	/* SPIFI D1 */
	{0x3, 7,  (SCU_PINIO_FAST | SCU_MODE_FUNC3)},	/* SPIFI D0 */
	{0x3, 8,  (SCU_PINIO_FAST | SCU_MODE_FUNC3)}	/* SPIFI CS/SSEL */
};

/* Local memory, 32-bit aligned that will be used for driver context (handle) */
static uint32_t lmem[21];

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/
static SPIFI_HANDLE_T *initializeSpifi(void)
{
	int idx;
	int devIdx;
	uint32_t memSize;
	SPIFI_HANDLE_T *pReturnVal;

	/* Initialize LPCSPIFILIB library, reset the interface */
	spifiInit(LPC_SPIFI_BASE, true);

	/* register support for the family(s) we may want to work with
	     (only 1 is required) */
	spifiRegisterFamily(SPIFI_REG_FAMILY_SpansionS25FLP);
	spifiRegisterFamily(SPIFI_REG_FAMILY_SpansionS25FL1);
	spifiRegisterFamily(SPIFI_REG_FAMILY_MacronixMX25L);
	
	/* Return the number of families that are registered */
	idx = spifiGetSuppFamilyCount();

	/* Show all families that are registered */
	for (devIdx = 0; devIdx < idx; ++devIdx) {
		DEBUGOUT("FAMILY: %s\r\n", spifiGetSuppFamilyName(devIdx));
	}

	/* Get required memory for detected device, this may vary per device family */
	memSize = spifiGetHandleMemSize(LPC_SPIFI_BASE);
	if (memSize == 0) {
		/* No device detected, error */
		test_suiteError("spifiGetHandleMemSize", SPIFI_ERR_GEN);
	}

	/* Initialize and detect a device and get device context */
	pReturnVal = spifiInitDevice(&lmem, sizeof(lmem), LPC_SPIFI_BASE, SPIFLASH_BASE_ADDRESS);
	if (pReturnVal == NULL) {
		test_suiteError("spifiInitDevice", SPIFI_ERR_GEN);
	}
	return pReturnVal;
}

static void RunExample(void)
{
	uint32_t spifiBaseClockRate;
	uint32_t maxSpifiClock;
	uint16_t libVersion;
	SPIFI_HANDLE_T *pSpifi;

	/* Report the library version to start with */
	libVersion = spifiGetLibVersion();
	DEBUGOUT("\r\n\r\nSPIFI Lib Version %02d%02d\r\n", ((libVersion >> 8) & 0xff), (libVersion & 0xff));
	
	/* set the blink rate to 1/2 second while testing */
	test_suiteSetErrorBlinkRate(500);
	
	/* Setup SPIFI FLASH pin muxing (QUAD) */
	Chip_SCU_SetPinMuxing(spifipinmuxing, sizeof(spifipinmuxing) / sizeof(PINMUX_GRP_T));

	/* SPIFI base clock will be based on the main PLL rate and a divider */
	spifiBaseClockRate = Chip_Clock_GetClockInputHz(CLKIN_MAINPLL);

	/* Setup SPIFI clock to run around 1Mhz. Use divider E for this, as it allows
	   higher divider values up to 256 maximum) */
	Chip_Clock_SetDivider(CLK_IDIV_E, CLKIN_MAINPLL, ((spifiBaseClockRate / 1000000) + 1));
	Chip_Clock_SetBaseClock(CLK_BASE_SPIFI, CLKIN_IDIVE, true, false);
	DEBUGOUT("SPIFI clock rate %d\r\n", Chip_Clock_GetClockInputHz(CLKIN_IDIVE));

	/* initialize and get a handle to the spifi lib */
	pSpifi = initializeSpifi();

	/* Get some info needed for the application */
	maxSpifiClock = spifiDevGetInfo(pSpifi, SPIFI_INFO_MAXCLOCK);

	/* Get info */
	DEBUGOUT("Device family       = %s\r\n", spifiDevGetFamilyName(pSpifi));
	DEBUGOUT("Capabilities        = 0x%x\r\n", spifiDevGetInfo(pSpifi, SPIFI_INFO_CAPS));
	DEBUGOUT("Device size         = %d\r\n", spifiDevGetInfo(pSpifi, SPIFI_INFO_DEVSIZE));
	DEBUGOUT("Max Clock Rate      = %d\r\n", maxSpifiClock);
	DEBUGOUT("Erase blocks        = %d\r\n", spifiDevGetInfo(pSpifi, SPIFI_INFO_ERASE_BLOCKS));
	DEBUGOUT("Erase block size    = %d\r\n", spifiDevGetInfo(pSpifi, SPIFI_INFO_ERASE_BLOCKSIZE));
	DEBUGOUT("Erase sub-blocks    = %d\r\n", spifiDevGetInfo(pSpifi, SPIFI_INFO_ERASE_SUBBLOCKS));
	DEBUGOUT("Erase sub-blocksize = %d\r\n", spifiDevGetInfo(pSpifi, SPIFI_INFO_ERASE_SUBBLOCKSIZE));
	DEBUGOUT("Write page size     = %d\r\n", spifiDevGetInfo(pSpifi, SPIFI_INFO_PAGESIZE));
	DEBUGOUT("Max single readsize = %d\r\n", spifiDevGetInfo(pSpifi, SPIFI_INFO_MAXREADSIZE));
	DEBUGOUT("Current dev status  = 0x%x\r\n", spifiDevGetInfo(pSpifi, SPIFI_INFO_STATUS));
	DEBUGOUT("Current options     = %d\r\n", spifiDevGetInfo(pSpifi, SPIFI_INFO_OPTIONS));

	/* Setup SPIFI clock to at the maximum interface rate the detected device
	   can use. This should be done after device init. */
	Chip_Clock_SetDivider(CLK_IDIV_E, CLKIN_MAINPLL, ((spifiBaseClockRate / maxSpifiClock) + 1));

	DEBUGOUT("SPIFI final Rate    = %d\r\n", Chip_Clock_GetClockInputHz(CLKIN_IDIVE));
	DEBUGOUT("\r\n");

	/* Test the helper functions first */
	test_suiteLibHelperBattery(pSpifi);

	/* Now test the erase functions first in block mode, then in address mode */
	test_suiteLibEraseBattery(pSpifi, false);
	test_suiteLibEraseBattery(pSpifi, true);

	/* test data integrity */
	test_suiteDataBattery(pSpifi, false, false);
	test_suiteDataBattery(pSpifi, true, false);
	test_suiteDataBattery(pSpifi, true, true);

	/* Done, de-init will enter memory mode */
	spifiDevDeInit(pSpifi);

	/* Indicate success */
	DEBUGOUT("Complete.\r\n");
	test_suiteSetErrorBlinkRate(0);
	
	while (1) {
		__WFI();
	}
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/
void SysTick_Handler(void)
{
	static int x = 0;
	
	/* If no blink rate just leave the light on */
	if (!test_suiteGetErrorBlinkRate()) {
		Board_LED_Set(0, true);
		return;
	}
	
	if (x++ > test_suiteGetErrorBlinkRate()) {
		Board_LED_Toggle(0);
		x = 0;
	}
}

/**
 * @brief	Main entry point
 * @return	Nothing
 */
int main(void)
{
		/* Initialize the board & LEDs for error indication */
	SystemCoreClockUpdate();
	Board_Init();
	
	/* Enable and setup SysTick Timer at a periodic rate */
	SysTick_Config(SystemCoreClock / TICKRATE_HZ1);
	
	/* Run the example code */
	RunExample();
	
	return 0;
}
