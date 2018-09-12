///
/// @file
/// @copyright All code copyright Movidius Ltd 2014, all rights reserved
///            For License Warranty see: common/license.txt
///
/// @defgroup Shave
/// @{
/// @brief     Shave low level functionality for RTEMS.
/// --------------------------------------------------------------------------------
///
#ifndef _OS_DRV_SVU_H_
#define _OS_DRV_SVU_H_

// System Includes
// ----------------------------------------------------------------------------

// Application Includes
// ----------------------------------------------------------------------------
#include <rtems.h>
#include "OsDrvSvuDefines.h"
// Common for all OS drivers
#include <OsCommon.h>
// Bare-metal layer include
#include "DrvSvu.h"

#ifdef __cplusplus
extern "C" {
#endif

// Source Specific #defines and types  (typedef,enum,struct)
// ----------------------------------------------------------------------------

// Global function prototypes
// ----------------------------------------------------------------------------
/// Initializes the shave driver
/// @return
///     OS_MYR_DRV_SUCCESS - the operation finished successfully
///     OS_MYR_DRV_ERROR - an error has occurred during initialization
///     OS_MYR_DRV_TIMEOUT - could not access the resource in the given time
///     OS_MYR_DRV_ALREADY_INITIALIZED - the driver was already initialized
///
int OsDrvSvuInit(void);

/// Tries to get access to use the shave given as parameter
/// @param[in] handler - shave handler
/// @param[in] shaveNumber - shave number
/// @param[in] protection - the protection type used for the shave
/// @return
///     OS_MYR_DRV_SUCCESS - the access to the shave was granted
///     OS_MYR_DRV_TIMEOUT - could not access the resource in the given time
///     OS_MYR_DRV_ALREADY_OPENED - the shave was already opened by a previous call to OsDrvSvuOpen()
///     OS_MYR_DRV_RESOURCE_BUSY - the shave is currently used by another thread
///     OS_MYR_DRV_ERROR - an error has occurred
///
int OsDrvSvuOpenShave(osDrvSvuHandler_t* handler, u32 shaveNumber, OS_MYRIAD_DRIVER_PROTECTION protection);

/// Returns the window registers for the given shave
/// @param[in] handler - shave handler
/// @param[out] windowA - register value for window A
/// @param[out] windowB - register value for window B
/// @param[out] windowC - register value for window C
/// @param[out] windowD - register value for window D
/// @return
/// return
///     OS_MYR_DRV_SUCCESS - the operation completed successfully
///     OS_MYR_DRV_RESOURCE_BUSY - the shave cannot be accessed because it is used by another thread
///
int OsDrvSvuGetShaveWindowRegs(osDrvSvuHandler_t* handler, u32* windows);

/// Set the default values for the widows register of the given shave
/// @param[in] handler - shave handler
/// @return
/// return
///     OS_MYR_DRV_SUCCESS - the operation completed successfully
///     OS_MYR_DRV_RESOURCE_BUSY - the shave cannot be accessed because it is used by another thread
///
int OsDrvSvuSetShaveWindowsToDefault(osDrvSvuHandler_t* handler);

/// Set the default values for the widows register of the given shave
/// @param[in] handler - shave handler
/// @param[in] windowA - register number for window A
/// @param[in] windowB - register number for window B
/// @param[in] windowC - register number for window C
/// @param[in] windowD - register number for window D
/// @return
///     OS_MYR_DRV_SUCCESS - the operation completed successfully
///     OS_MYR_DRV_RESOURCE_BUSY - the shave cannot be accessed because it is used by another thread
///
int OsDrvSvuSetShaveWindows(osDrvSvuHandler_t* handler, u32 windowA, u32 windowB, u32 windowC, u32 windowD);

/// Sets a default value for stack
/// !WARNING: only use this if you are using the default ldscript or really
/// know what you're doing!
/// @param[in] handler - shave handler
/// @return
///     OS_MYR_DRV_SUCCESS - the operation completed successfully
///     OS_MYR_DRV_RESOURCE_BUSY - the shave cannot be accessed because it is used by another thread
///
int OsDrvSvuSetWindowedDefaultStack(osDrvSvuHandler_t* handler);

/// Sets a default value for stack with absolute address
/// !WARNING: only use this if you are using the default ldscript or really
/// know what you're doing!
/// @param[in] handler - shave handler
/// @return
///     OS_MYR_DRV_SUCCESS - the operation completed successfully
///     OS_MYR_DRV_RESOURCE_BUSY - the shave cannot be accessed because it is used by another thread
///
int OsDrvSvuSetAbsoluteDefaultStack(osDrvSvuHandler_t* handler);

/// Starts the shave execution and waits for it to finish.
/// @param[in] handler - shave handler
/// @param[in] entryPoint memory address to load in the shave instruction pointer before starting
/// @return
///     OS_MYR_DRV_SUCCESS - the operation completed successfully
///     OS_MYR_DRV_RESOURCE_BUSY - the shave cannot be accessed because it is used by another thread
///
int OsDrvSvuRunShave(osDrvSvuHandler_t* handler, u32 entryPoint);

/// Starts non blocking execution of a shave
/// @param[in] handler - shave handler
/// @param[in] entryPoint memory address to load in the shave instruction pointer before starting
/// @return
///     OS_MYR_DRV_SUCCESS - the operation completed successfully
///     OS_MYR_DRV_RESOURCE_BUSY - the shave cannot be accessed because it is used by another thread
///
int OsDrvSvuStartShave(osDrvSvuHandler_t* handler, u32 entryPoint);

/// Write the value to a IRF/VRF Registers from a specific Shave and starts the shave
/// @param[in] handler - shave handler
/// @param[in] pc - function called from the program counter pc
/// @param[in] *fmt - string containing i or v according to irf or vrf ex. "iiv"
/// @param[in] ... - variable number of parameters according to fmt
/// @return
///     OS_MYR_DRV_SUCCESS - the operation completed successfully
///     OS_MYR_DRV_RESOURCE_BUSY - the shave cannot be accessed because it is used by another thread
///
int OsDrvSvuStartShaveCC(osDrvSvuHandler_t* handler, u32 pc, const char *fmt, ...);

/// Write the value to a IRF/VRF Registers from a specific Shave
/// @param[in] handler - shave handler
/// @param[in] *fmt - string containing i or v according to irf or vrf ex. "iiv"
/// @param[in] ... - variable number of params according to fmt
/// @return
///     OS_MYR_DRV_SUCCESS - the operation completed successfully
///     OS_MYR_DRV_RESOURCE_BUSY - the shave cannot be accessed because it is used by another thread
///
int OsDrvSvuSetupShaveCC(osDrvSvuHandler_t* handler, const char *fmt, ...);

/// Resets the given shave
/// @param[in] handler - shave handler
///
/// @return
///     OS_MYR_DRV_SUCCESS - the operation completed successfully
///     OS_MYR_DRV_RESOURCE_BUSY - the shave cannot be accessed because it is used by another thread
///
int OsDrvSvuResetShave(osDrvSvuHandler_t* handler);

/// Waits for the shaves in the given list to finish execution
/// 
/// This function can only be used to wait for shaves that were started in the same thread. If this is
/// not the case OS_MYR_RESOURCE_NOT_OWNED will be returned.
/// @param[in]  noOfShaves the number of shaves in the list
/// @param[in]  shaveList a list of shave handlers
/// @param[in]  timeout the amount of time in system ticks to wait until checking for the shave
///     status
/// @param[in]  timeout the amount of time in system ticks to wait until checking for the shave
/// @param[out] runningShaves a bit field with the shaves that did not finish their execution
///
/// @return
///     OS_MYR_DRV_SUCCESS - the operation completed successfully
///     OS_MYR_DRV_RESOURCE_BUSY - the shave cannot be accessed because it is used by another thread
///     OS_MYR_RESOURCE_NOT_OWNED - the shave for which is waited was started by other thread
///
///     When this code is returned, the runningShaves parameter contains the first shave that generated
///     the error
///
int OsDrvSvuWaitShaves(u32 noOfShaves, osDrvSvuHandler_t* shaveList, u32 timeout, u32 *runningShaves);

/// Sets the rounding bits for a particular shave
/// @param[in]  handler shave handler
/// @param[in]  roundingBits rounding bits
///
/// @return
///     OS_MYR_DRV_SUCCESS - the operation completed successfully
///     OS_MYR_DRV_RESOURCE_BUSY - the shave cannot be accessed because it is used by another thread
///     OS_MYR_DRV_NOTOPEND - the shave was not opened
int OsDrvSvuSetRounding(osDrvSvuHandler_t* handler, u32 roundingBits);

/// Converts a shave relative address to a System solved address based on the target CMX slice
/// and current window it relates to
/// @param[in]  inAddr shave relative address (can be code/data/absolute type of address)
/// @param[in]  shaveNumber shave number
/// @param[in]  outAddr the resolved address
///
/// @return
///     OS_MYR_DRV_SUCCESS - the operation completed successfully
int OsDrvSvuSolveShaveRelAddr(u32 inAddr, u32 shaveNumber, u32* outAddr);

/// Performs an L1 data cache operation on the given shave
/// @param[in]  handler shave handler
/// @param[in] action - the type of the action that needs to be performed
///
/// It can have one of the following values:
/// SVUL1DATACACHE_ENABLE - enables shave L1 data cache
/// SVUL1DATACACHE_DISABLE - disables shave L1 data cache
/// SVUL1DATACACHE_STOP - stops shave L1 data cache
/// SVUL1DATACACHE_INVALIDATE_ALL - invalidates all cache
/// SVUL1DATACACHE_FLUSH_ALL - flushes all cache
/// SVUL1DATACACHE_FLUSH_INVALIDATE_ALL - flushes and invalidates all cache
///
/// @return - MYR_DRV_SUCCESS the operations finished successfully
///         - MYR_DRV_ERROR the operation provided as parameter is invalid
///
int OsDrvSvuL1DataCacheCtrl(osDrvSvuHandler_t* handler, u32 action);

/// Performs an L1 instruction cache operation on the given shave
/// @param[in] handler shave handler
/// @param[in] action the type of the action that needs to be performed
///
/// It can have one of the following values:
/// SVUL1INSTRCACHE_DEFAULT enables shave L1 instruction cache
/// SVUL1INSTRCACHE_BYPASS bypasses shave L1 instruction cache
/// SVUL1INSTRCACHE_LOCK locks shave L1 instruction cache
/// SVUL1INSTRCACHE_INVALIDATE invalidates L1 instruction cache
///
/// @return - MYR_DRV_SUCCESS the operations finished successfully
///         - MYR_DRV_ERROR the operation provided as parameter is invalid
///
int OsDrvSvuL1InstrCacheCtrl(osDrvSvuHandler_t* handler, u32 action);

/// Stops the execution of the given shave
/// By default the Shave L1 ICache is enabled unless this function
/// is called
/// @param[in] handler - shave handler
/// @return
///     OS_MYR_DRV_SUCCESS - the operation completed successfully
///     OS_MYR_DRV_RESOURCE_BUSY - the shave cannot be accessed because it is used by another thread
///
int OsDrvSvuStop(osDrvSvuHandler_t* handler);

/// Releases the resources allocated for the given shave
/// @param[in] handler - shave handler
///
/// @return
///     OS_MYR_DRV_SUCCESS - the operation finished successfully
///     OS_MYR_DRV_ERROR - an error has occurred
///     OS_MYR_DRV_TIMEOUT - the resource couldn't be accessed in the given amount of time
///     OS_MYR_DRV_NOTOPENED - cannot close the driver because it was not opened
///     OS_MYR_DRV_RESOURCE_BUSY - the shave cannot be closed because other thread is using it
///
int OsDrvSvuCloseShave(osDrvSvuHandler_t* handler);

/// Releases the driver resources if no shave is running
///
/// @return
///     OS_MYR_DRV_SUCCESS - the operation finished successfully, the driver is closed
///     OS_MYR_DRV_ERROR - an error has occurred
///     OS_MYR_DRV_TIMEOUT - could not access the resource in the given time
///     OS_MYR_DRV_RESOURCE_BUSY - all the shaves owned but the current thread were closed, but
///     there are another thread working with shaves, so the resources cannot be released
///     OS_MYR_DRV_NOTOPENED - trying to close an uninitialized driver
///
int OsDrvSvuClose(void);

/// Dynamically load shvdlib file - These are elf object files stripped of any symbols
/// @param[in] startAddr - starting address where to load the shvdlib file
/// @param[in] handler   - shave handler
/// @return 
///
int OsDrvSvuLoadShvdlib(u8 *startAddr, osDrvSvuHandler_t* handler);
/// }@
#ifdef __cplusplus
}
#endif


#endif
