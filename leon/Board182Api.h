///
/// @file
/// @copyright All code copyright Movidius Ltd 2012, all rights reserved
///            For License Warranty see: common/license.txt
///
/// @defgroup Board182 Board 182
/// @defgroup Board182Api Board Setup Functions
/// @ingroup  Board182
/// @{
/// @brief     Board Setup Functions API.
///
/// This is the API to board setup library implementation.
///

#ifndef _BOARD_API_H_
#define _BOARD_API_H_

// 1: Includes
// ----------------------------------------------------------------------------
#include "Board182ApiDefines.h"

#ifdef __cplusplus
extern "C" {
#endif

// 2:  Exported Global Data (generally better to avoid)
// ----------------------------------------------------------------------------
extern tyAppDeviceHandles gAppDevHndls;
// 3:  Exported Functions (non-inline)
// ----------------------------------------------------------------------------

/// @brief This function performs the initialization of basic functions of MV0182 board: I2C buses, 
/// external clock generator and sets up all GPIOS
/// @param[in] clockConfiguration - External PLL clock configuration
/// @return initialization status
///
s32 BoardInitialise(u32 clockConfiguration);
/// @}
#ifdef __cplusplus
}
#endif


#endif // _BOARD_API_H_
