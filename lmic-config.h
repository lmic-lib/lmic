// Setting LMIC_CMDLINE_CONFIG allows disabling this file from the
// compiler commandline, to instead configure everything there.
#if !defined(_lmic_config) && !defined(LMIC_CMDLINE_CONFIG)
#define _lmic_config

// Define exactly one of these
#define LMIC_EU686 1
//#define LMIC_US915 1

// SX1272 enables support for the SX1272/SX1273 radio, which is also
// used on the HopeRF RFM92 boards.
// SX1276 enables support for the SX1276/SX1277/SX1278/SX1279 radio,
// which is also used on the HopeRF RFM95 boards.
// Define exactly one of these
//#define LMIC_SX1272 1
#define LMIC_SX1276 1

// Set this to 1 to enable some basic debug output (using printf) about
// RF settings used during transmission and reception. Set to 2 to
// enable more verbose output. Make sure that printf is actually
// configured (e.g. on AVR it is not by default), otherwise using it can
// cause crashing. On the Arduino target, printf automatically gets
// enabled when LMIC_DEBUG_LEVEL is > 0, see its target-config.h for
// details.
#define LMIC_DEBUG_LEVEL 0

// Uncomment this to disable all code related to joining
//#define LMIC_DISABLE_JOIN
// Uncomment this to disable all code related to ping
#define LMIC_DISABLE_PING
// Uncomment this to disable all code related to beacon tracking.
// Requires ping to be disabled too
#define LMIC_DISABLE_BEACONS

// Uncomment these to disable the corresponding MAC commands.
// Class A
//#define LMIC_DISABLE_MCMD_DCAP_REQ // duty cycle cap
//#define LMIC_DISABLE_MCMD_DN2P_SET // 2nd DN window param
//#define LMIC_DISABLE_MCMD_SNCH_REQ // set new channel
// Class B
//#define LMIC_DISABLE_MCMD_PING_SET // set ping freq, automatically disabled by LMIC_DISABLE_PING
//#define LMIC_DISABLE_MCMD_BCNI_ANS // next beacon start, automatical disabled by LMIC_DISABLE_BEACON

// In LoRaWAN, a gateway applies I/Q inversion on TX, and nodes do the
// same on RX. This ensures that gateways can talk to nodes and vice
// versa, but gateways will not hear other gateways and nodes will not
// hear other nodes. By uncommenting this macro, this inversion is
// disabled and this node can hear other nodes. If two nodes both have
// this macro set, they can talk to each other (but they can no longer
// hear gateways). This should probably only be used when debugging
// and/or when talking to the radio directly (e.g. like in the "raw"
// example). Enabling this breaks normal LoRaWAN connectivity.
//#define LMIC_DISABLE_INVERT_IQ_ON_RX


// This allows choosing between multiple included AES implementations.
// Make sure exactly one of these is defined.
//
// This selects the original AES implementation included LMIC. This
// implementation is optimized for speed on 32-bit processors using
// fairly big lookup tables, but it takes up big amounts of flash on the
// AVR architecture.
// #define LMIC_USE_ORIGINAL_AES
//
// This selects the AES implementation written by Ideetroon for their
// own LoRaWAN library. It also uses lookup tables, but smaller
// byte-oriented ones, making it use a lot less flash space (but it is
// also about twice as slow as the original).
#define LMIC_USE_IDEETRON_AES

#endif
