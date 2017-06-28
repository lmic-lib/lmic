#ifndef _lmic_arduino_hal_config_h_
#define _lmic_arduino_hal_config_h_

// 16 μs per tick
// LMIC requires ticks to be 15.5μs - 100 μs long
#define LMIC_US_PER_OSTICK_EXPONENT 4
#define LMIC_US_PER_OSTICK (1 << LMIC_US_PER_OSTICK_EXPONENT)
#define LMIC_OSTICKS_PER_SEC (1000000 / LMIC_US_PER_OSTICK)

// Enable this to allow using printf() to print to the given serial port
// (or any other Print object). This can be easy for debugging. The
// current implementation only works on AVR, though. Enable this by
// default when a debug level is set, to prevent random crashing.
#if LMIC_DEBUG_LEVEL > 0
#define LMIC_PRINTF_TO Serial
#endif

// Any runtime assertion failures are printed to this serial port (or
// any other Print object). If this is unset, any failures just silently
// halt execution.
#define LMIC_FAILURE_TO Serial

#endif // _lmic_arduino_hal_config_h_
