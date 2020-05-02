/**
 * @file      ptimer.h
 * @author    Michele Pes
 * @copyright BSD-3-Clause
 * @date      Wed Nov 28 12:24:18 2018
 * @brief     Header of ptimer library
 */

/**
 * @mainpage PTimer C library
 *
 * @section intro_sec Introduction
 *
 * The PTimer library is a very small, easy and portable timer library\n
 * You can create, use, modify, reuse and destroy single-shot or periodic timers.\n
 * The library has just an header to include and just a single .C source file to compile.
 *
 * Remember to define \b PTIMER_SHARED_BUILD when building a DLL, and\n
 * define PTIMER_SHARED when using the DLL.
 *
 * See the complete @link ptimer.h API reference @endlink of PTimer library
 */

#ifndef PTIMER_H_
#define PTIMER_H_

#ifdef __cplusplus
extern "C" {
#endif


#if defined(_WIN32) && defined(PTIMER_SHARED_BUILD)
#  define PTIMER_API __declspec(dllexport)
#elif defined(_WIN32) && defined(PTIMER_SHARED)
#  define PTIMER_API __declspec(dllimport)
#else
#  define PTIMER_API
#endif


/**
 * @def PTIMER_SECONDS(milliseconds) ((milliseconds) * 1000)
 * Convert milliseconds to seconds, can be used with timeout/sleep functions
 */
#define PTIMER_SECONDS(milliseconds) ((milliseconds) * 1000)

/**
 * @typedef ptimer_ptr_t
 * An opaque pointer to timer struct
 */
typedef void* ptimer_ptr_t;

/**
 * @typedef ptimer_callback_t
 * @details void (*ptimer_callback_t)(void * argument);
 * @brief The timer callback
 * @param[in] argument A callback argument
 */
typedef void (*ptimer_callback_t)(void* argument);

/**
 * @enum ptimer_type
 * The timer can be single shot or periodic
 */
typedef enum {
    PTIMER_SINGLE_SHOT, /**< Single shot, must be started manually every time */
    PTIMER_PERIODIC     /**< Periodic, callback fired at given interval       */
} ptimer_type;

/**
 * @brief Create a new timer
 * @param[in,out] timer_address A pointer to the timer to create
 * @param[in] type The type of timer, see ::ptimer_type
 * @param[in] timeout The timer timeout in \b milliseconds
 * @param[in] callback The callback to call when timeout expires,
 *            see ::ptimer_callback_t
 * @param[in] callback_argument The argument to pass to callback
 * @return Zero on errors, non-zero if ok
 * @see ptimer_destroy()
 */
PTIMER_API int ptimer_create(ptimer_ptr_t* timer_address,
                             ptimer_type type,
                             unsigned int timeout,
                             ptimer_callback_t callback,
                             void* callback_argument);

/**
 * @brief Start given timer
 * @details Use this function to restart a single-shot timer too
 * @param[in] p_timer Pointer to timer
 * @see ptimer_stop()
 */
PTIMER_API void ptimer_start(ptimer_ptr_t p_timer);

/**
 * @brief Stop given timer
 * @param[in] p_timer Pointer to timer
 * @see ptimer_start()
 */
PTIMER_API void ptimer_stop(ptimer_ptr_t p_timer);

/**
 * @brief Destroy a previously created timer
 * @param[in] p_timer Pointer to timer to destroy
 * \see ptimer_create()
 */
PTIMER_API void ptimer_destroy(ptimer_ptr_t p_timer);

/**
 * @brief Summary
 * @param[in] p_timer Pointer to timer
 * @param[in] timeout The new timeout in \b milliseconds
 */
PTIMER_API void ptimer_set_timeout(ptimer_ptr_t p_timer, unsigned int timeout);

/**
 * @brief Update timer callback
 * @param[in] p_timer Pointer to timer
 * @param[in] new_callback The new callback function
 */
PTIMER_API void ptimer_set_callback(ptimer_ptr_t p_timer,
                                    ptimer_callback_t new_callback);

/**
 * @brief Update timer callback parameter
 * @param[in] p_timer Pointer to timer
 * @param[out] callback_arg The new callback argument
 */
PTIMER_API void ptimer_set_callback_argument(ptimer_ptr_t p_timer,
                                             void* callback_arg);

/**
 * @brief Utility functions to make a thread sleep
 * @param[in] interval The sleeping time in \b milliseconds
 */
PTIMER_API void ptimer_sleep(unsigned int interval);

#ifdef __cplusplus
}
#endif

#endif /* PTIMER_H_ */
