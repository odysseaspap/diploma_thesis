///
/// @file
/// @copyright All code copyright Movidius Ltd 2012, all rights reserved
///            For License Warranty see: common/license.txt
///
/// @defgroup Board182ApiDefines Board 182 API Defines
/// @ingroup Board182
/// @{
/// @brief Definitions and types needed by Application Board Interface Module
///
/// This file contains all the definitions of constants, typedefs,
/// structures, enums and exported variables for the Board 182 component
///

#ifndef _BOARD_DEF_H
#define _BOARD_DEF_H

#include "mv_types.h"
#include "DrvI2cMasterDefines.h"
#include "DrvMipiDefines.h"
#include "brdMv0182.h"

// 1: Defines
// ----------------------------------------------------------------------------

// 2: Typedefs (types, enums, structs)
// ----------------------------------------------------------------------------

/// @brief List all of the I2C device handles
typedef struct
{
    I2CM_Device * i2c0Handle;
    I2CM_Device * i2c1Handle;
    I2CM_Device * i2c2Handle;
} tyAppDeviceHandles;

/// @brief Status for board initialization
typedef enum
{
	/// @brief Board initialized successfully
	B_SUCCESS				=  0,
	/// @brief Error caused by i2c
	B_I2C_ERROR				= -1,
	/// @brief Error occurred when external PLL was configured
	B_EXTERNAL_PLL_ERROR	= -2,
	/// @brief Error occurred when PCB detection was performed
	B_UNDETECTED_REVISION	= -3,
	/// @brief This MDK SW package does not support Revision 0 (R0) nor Revision 1 (R1) MDK boards.
	B_UNSUPPORTED_REVISION  = -4,
}brdStatus;

// 3: Local const declarations     NB: ONLY const declarations go here
// ----------------------------------------------------------------------------

/// @}
#endif // BOARD_DEF_H

