///
/// @file
/// @copyright All code copyright Movidius Ltd 2012, all rights reserved.
///            For License Warranty see: common/license.txt
///
/// @brief     Board setup.
///
/// This is the implementation of all board setup handling events.
///
///

// 1: Includes
// ----------------------------------------------------------------------------
#include "DrvGpioDefines.h"
#include "DrvGpio.h"
#include "DrvI2cMaster.h"
#include "Board182Api.h"
#include "DrvCDCEL.h"
#include "brdGpioCfgs/brdMv0182R2GpioDefaults.h"
#include "brdGpioCfgs/brdMv0182R3GpioDefaults.h"
#include "brdGpioCfgs/brdMv0182R4GpioDefaults.h"
#include "brdGpioCfgs/brdMv0182R5GpioDefaults.h"
#include <stdio.h>
#include <stdlib.h>
// 2:  Source Specific #defines and types  (typedef,enum,struct)
// ----------------------------------------------------------------------------
#define IRQ_SRC_0   0
#define IRQ_SRC_1   1
#define IRQ_SRC_2   2
#define IRQ_SRC_3   3
#define PLL_STATUS_OK   0
#define I2C_STATUS_OK   0
#define GPIO_57_SDCARD_VOLTAGE  57
// #define DRV_BRDM_DEBUG  //Debug Messages
#ifdef DRV_BRDM_DEBUG
#define DPRINTF(...) printf(__VA_ARGS__)
#else
#define DPRINTF(...)
#endif

// 3: Global Data (Only if absolutely necessary)
// ----------------------------------------------------------------------------
tyAppDeviceHandles gAppDevHndls;

// 4: Static Local Data
// ----------------------------------------------------------------------------

// 5: Static Function Prototypes
// ----------------------------------------------------------------------------

// 6: Functions Implementation
// ----------------------------------------------------------------------------

s32 BoardInitialise(u32 clockConfiguration)
{
    s32 status=0;
    tyMv0182PcbRevision revisionCode;

    DrvGpioIrqSrcDisable(IRQ_SRC_0);
    DrvGpioIrqSrcDisable(IRQ_SRC_1);
    DrvGpioIrqSrcDisable(IRQ_SRC_2);
    DrvGpioIrqSrcDisable(IRQ_SRC_3);

    status = brd182GetPcbRevison(&revisionCode);
    if (status != (int)B_SUCCESS)
        return B_UNDETECTED_REVISION;

    switch(revisionCode)
    {
    case MV0182_R0R1:
        {
        DPRINTF("\nBoardInitialize() fail: revision R0-R1 is not supported\n");
        return B_UNSUPPORTED_REVISION;
        }
        break;

    case MV0182_R2:
        {
        DrvGpioInitialiseRange(brdMV0182R2GpioCfgDefault);
        }

        break;

    case MV0182_R3:
        {
        DrvGpioInitialiseRange(brdMV0182R3GpioCfgDefault);
        }
        break;

    case MV0182_R4:
        {
        DrvGpioInitialiseRange(brdMV0182R4GpioCfgDefault);
        }
        break;

    case MV0182_R5:
        {
        DrvGpioInitialiseRange(brdMV0182R5GpioCfgDefault);
        //GPIO 57 is used to enable control of the SDCard voltage
        DrvGpioSetMode(GPIO_57_SDCARD_VOLTAGE, D_GPIO_DIR_OUT | D_GPIO_MODE_7);
        DrvGpioSetPinHi(GPIO_57_SDCARD_VOLTAGE);
        }
        break;

    default:
        {
        //never should enter on this branch:
        //if status of brd182GetPcbRevison(&revisionCode) is B_SUCCESS,
        //the revisionCode should be among the cases above
        return MYR_DRV_ERROR;
        }
        break;
    }
    status = brd182InitialiseI2C(NULL,NULL,NULL,                 // Use Default I2C configuration
            &gAppDevHndls.i2c0Handle,
            &gAppDevHndls.i2c1Handle,
            &gAppDevHndls.i2c2Handle);

    if(status != I2C_STATUS_OK)
    {
        status = ((int)B_I2C_ERROR);
        return status;
    }

    if (clockConfiguration != 0)
    {
        status = brd182ExternalPllConfigure(clockConfiguration);
        if (status != PLL_STATUS_OK)
            status = ((int)B_EXTERNAL_PLL_ERROR);
    }
return status;
}
