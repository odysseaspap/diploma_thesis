///
/// @file
/// @copyright All code copyright Movidius Ltd 2014, all rights reserved
///            For License Warranty see: common/license.txt
///
/// @defgroup Timer API for RTEMS
/// @{
/// @brief     Timer functions API.
/// --------------------------------------------------------------------------------

#ifndef _OS_DRV_TIMER_H_
#define _OS_DRV_TIMER_H_

// System Includes
// ----------------------------------------------------------------------------

// Application Includes
// ----------------------------------------------------------------------------
#include <rtems.h>
#include "OsDrvTimerDefines.h"
// Common for all OS drivers
#include <OsCommon.h>
// Bare-metal layer include
#include "DrvTimer.h"

#ifdef __cplusplus
extern "C" {
#endif

// Source Specific #defines and types  (typedef,enum,struct)
// ----------------------------------------------------------------------------

// Global function prototypes
// ----------------------------------------------------------------------------

/// Initializes the Timer driver API
///
/// This function must be called first in order to be able to call any other
/// function from the driver
///
/// @return
///     OS_MYR_DRV_SUCCESS - the operation finished successfully
///     OS_MYR_DRV_ERROR - an error has occurred during initialization
///     OS_MYR_DRV_TIMEOUT - could not access the resource in the given time
///     OS_MYR_DRV_ALREADY_INITIALIZED - the driver was already initialized
///
int OsDrvTimerInit(void);

/// Tries to get access to use the shave given as parameter
/// @param[in] handler - timer handler
/// @param[in] maxWaitMicroSeconds - the maximum delay in microseconds the timer will be used to wait for
/// This number is used to decide whether one or two timer should be used
/// @param[in] protection - the protection type used for the timer
///
/// @return
///     OS_MYR_DRV_SUCCESS - the access to the timer was granted
///     OS_MYR_DRV_TIMEOUT - could not access the resource in the given time
///     OS_MYR_DRV_ALREADY_OPENED - the timer was already opened by a previous call to OsDrvTimerOpen()
///     OS_MYR_DRV_RESOURCE_BUSY - the timer is currently used
///     OS_MYR_DRV_ERROR - an error has occurred
///
int OsDrvTimerOpen(osDrvTimerHandler_t* handler, u32 maxWaitMicroSeconds, OS_MYRIAD_DRIVER_PROTECTION protection);

/// Adjust a currently running timer to a new time value
///
/// This function may be used in the callback routine of an infinite count timer
/// @param[in] handler timer handle returned from timer open function
/// @param[in] numMicroSeconds set adjusted number of micro-seconds until the next timer callback
///
/// @return
///     OS_MYR_DRV_SUCCESS - the access to the timer was granted
///     OS_MYR_DRV_TIMEOUT - could not access the resource in the given time
///     OS_MYR_DRV_RESOURCE_BUSY - the timer is currently used
///     OS_MYR_DRV_NOTOPENED - the timer was not opened
///     OS_MYR_DRV_ERROR - an error has occurred
///
int OsDrvTimerSetTimeout(osDrvTimerHandler_t* handler, u32 numMicroSeconds);

/// Request a callback after numMicroSeconds
///
/// Optionally have the callback for repeatCount times
/// @param[in] handler timer handler
/// @param[in] numMicroSeconds number of microseconds before the callback is called
/// @param[in] callback pointer to the callback function
/// @param[in] repeatCount number of times to execute the callback (0-> forever until DrvTimerDisable), 1 once, x -> x times)
/// @param[in] priority level of the IRQ
///
/// @return
///     OS_MYR_DRV_SUCCESS - the access to the timer was granted
///     OS_MYR_DRV_TIMEOUT - could not access the resource in the given time
///     OS_MYR_DRV_RESOURCE_BUSY - the timer is currently used
///     OS_MYR_DRV_NOTOPENED - the timer was not opened
///     OS_MYR_DRV_ERROR - an error has occurred
///
int OsDrvTimerStartTimer(osDrvTimerHandler_t* handler, u32 numMicroSeconds,
        tyTimerCallback callback, int repeatCount, int priority);

/// Pause the timer keeping it's state
///
/// The timer can be resumed by calling OsDrvTimerResumeTimer
///
/// @param[in] handler timer handler
///
/// @return
///     OS_MYR_DRV_SUCCESS - the access to the timer was granted
///     OS_MYR_DRV_TIMEOUT - could not access the resource in the given time
///     OS_MYR_DRV_RESOURCE_BUSY - the timer is currently used
///     OS_MYR_DRV_NOTOPENED - the timer was not opened
///     OS_MYR_DRV_ERROR - an error has occurred
///
int OsDrvTimerPauseTimer(osDrvTimerHandler_t* handler);

/// Resumes a timer paused by a call to DrvTimerPause
///
/// @param[in] handler timer handler
///
/// @return
///     OS_MYR_DRV_SUCCESS - the access to the timer was granted
///     OS_MYR_DRV_TIMEOUT - could not access the resource in the given time
///     OS_MYR_DRV_RESOURCE_BUSY - the timer is currently used
///     OS_MYR_DRV_NOTOPENED - the timer was not opened
///     OS_MYR_DRV_ERROR - an error has occurred
///
int OsDrvTimerResumeTimer(osDrvTimerHandler_t* handler);

/// Stops a running timer.
///
/// @param[in] handler timer handler
///
/// @return
///     OS_MYR_DRV_SUCCESS - the access to the timer was granted
///     OS_MYR_DRV_TIMEOUT - could not access the resource in the given time
///     OS_MYR_DRV_RESOURCE_BUSY - the timer is currently used
///     OS_MYR_DRV_NOTOPENED - the timer was not opened
///     OS_MYR_DRV_ERROR - an error has occurred
///
int OsDrvTimerStopTimer(osDrvTimerHandler_t* handler);

/// Returns the number of clock cycles since DrvTimerInit() called
///
/// This is a full 64 bit free running counter so it only overflows
/// every 3249 years @ 180Mhz
/// @param[out] ticks number of clock cycles since DrvCprInit() [64 bit value]
///
/// @return
///     MYR_DRV_SUCCESS the operation completed successfully
///     MYR_DRV_NOT_INITIALIZED the driver was not initialized
///
int OsDrvTimerGetSystemTicks64(u64* ticks);

/// Initialize a timeout
///
/// Creates a timestamp which is used to check if a timeout has occurred
/// Uses storage passed by the user to make the function re-entrant
/// @param[in] pTimeStamp pointer to storage for calculated timestamp
/// @param[in] intervalUs interval for the timestamp in microseconds
///
/// @return
///     MYR_DRV_SUCCESS the operation completed successfully
///     MYR_DRV_NOT_INITIALIZED the driver was not initialized
///
int OsDrvTimerInitTimeStamp(tyTimeStamp * pTimeStamp, u32 intervalUs);

/// Reports if a timeout has occurred based on previously initialized timestamp
///
/// @param[in]  pTimeStamp pointer to timestamp which after which the function will return TRUE
/// @param[out] timeoutReached  FALSE if current time is < timestamp; TRUE if timeout has occurred
///
/// @return
///     MYR_DRV_SUCCESS the operation completed successfully
///     MYR_DRV_NOT_INITIALIZED the driver was not initialized
///
int OsDrvTimerCheckTimeStamp(tyTimeStamp * pTimeStamp, u32* timeoutReached);

/// Update pTimestamp with the current tickcount
///
/// This function is part of a pair of functions used for event timing
/// Call this function with a pointer to a tyTimeStamp which is used to
/// record the current tick count.Later this timestamp will be used to
/// report the elapsed ticks since the original timestamp
/// @param[in] pTimeStamp pointer to storage for calculated timestamp
///
/// @return
///     MYR_DRV_SUCCESS the operation completed successfully
///     MYR_DRV_NOT_INITIALIZED the driver was not initialized
///
int OsDrvTimerStartTicksCount(tyTimeStamp * pTimeStamp);

/// Report the ticks elapsed since DrvTimerStart was called on this timestamp
///
/// @param[in] pTimeStamp pointer to the previously initialized timeStamp (using DrvTimerStart)
/// @param[out] elapsedTicks 64 bit tick count of elapsed time since start
///
/// @return
///     MYR_DRV_SUCCESS the operation completed successfully
///     MYR_DRV_NOT_INITIALIZED the driver was not initialized
///
int OsDrvTimerGetElapsedTicks(tyTimeStamp * pTimeStamp, u64* elapsedTicks);

/// Release the handler and makes the timer free to use
///
/// @param[in] handler timer handler
///
/// @return
///     OS_MYR_DRV_SUCCESS - the access to the timer was granted
///     OS_MYR_DRV_TIMEOUT - could not access the resource in the given time
///     OS_MYR_DRV_RESOURCE_BUSY - the timer is currently used
///     OS_MYR_DRV_NOTOPENED - the timer was not opened
///     OS_MYR_DRV_ERROR - an error has occurred
///
int OsDrvTimerCloseTimer(osDrvTimerHandler_t* handler);

/// Closes the timer driver.
///
/// @param[in] handler timer handler
///
/// @return
///     OS_MYR_DRV_SUCCESS - the access to the timer was granted
///     OS_MYR_DRV_TIMEOUT - could not access the resource in the given time
///     OS_MYR_DRV_RESOURCE_BUSY - the timer is currently used
///     OS_MYR_DRV_ERROR - an error has occurred
///
int OsDrvTimerClose(void);

/// }@
#ifdef __cplusplus
}
#endif


#endif
