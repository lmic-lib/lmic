/*
 * Copyright (c) 2014-2016 IBM Corporation.
 * All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of the <organization> nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//! @file
//! @brief LMIC API

#ifndef _lmic_h_
#define _lmic_h_

#include "oslmic.h"
#include "lorabase.h"

#ifdef __cplusplus
extern "C"{
#endif

// LMIC version
#define LMIC_VERSION_MAJOR 1
#define LMIC_VERSION_MINOR 6
#define LMIC_VERSION_BUILD 1468577746

//! Only For Antenna Tuning Tests !
//#define CFG_TxContinuousMode 1

enum { MAX_FRAME_LEN      =  64 };   //!< Library cap on max frame length
enum { TXCONF_ATTEMPTS    =   8 };   //!< Transmit attempts for confirmed frames
enum { MAX_MISSED_BCNS    =  20 };   // threshold for triggering rejoin requests
enum { MAX_RXSYMS         = 100 };   // stop tracking beacon beyond this

enum { LINK_CHECK_CONT    =  12 ,    // continue with this after reported dead link
       LINK_CHECK_DEAD    =  24 ,    // after this UP frames and no response from NWK assume link is dead
       LINK_CHECK_INIT    = -12 ,    // UP frame count until we inc datarate
       LINK_CHECK_OFF     =-128 };   // link check disabled

enum { TIME_RESYNC        = 6*128 }; // secs
enum { TXRX_GUARD_ms      =  6000 };  // msecs - don't start TX-RX transaction before beacon
enum { JOIN_GUARD_ms      =  9000 };  // msecs - don't start Join Req/Acc transaction before beacon
enum { TXRX_BCNEXT_secs   =     2 };  // secs - earliest start after beacon time
enum { RETRY_PERIOD_secs  =     3 };  // secs - random period for retrying a confirmed send

#if defined(LMIC_EU686) // EU868 spectrum ====================================================

enum { MAX_CHANNELS = 16 };      //!< Max supported channels
enum { MAX_BANDS    =  4 };

enum { LIMIT_CHANNELS = (1<<4) };   // EU868 will never have more channels
//! \internal
struct band_t {
    uint16_t     txcap;     // duty cycle limitation: 1/txcap
    int8_t     txpow;     // maximum TX power
    uint8_t     lastchnl;  // last used channel
    ostime_t avail;     // channel is blocked until this time
};

#elif defined(LMIC_US915)  // US915 spectrum =================================================

enum { MAX_XCHANNELS = 2 };      // extra channels in RAM, channels 0-71 are immutable
enum { MAX_TXPOW_125kHz = 30 };

#endif // ==========================================================================

// Keep in sync with evdefs.hpp::drChange
enum { DRCHG_SET, DRCHG_NOJACC, DRCHG_NOACK, DRCHG_NOADRACK, DRCHG_NWKCMD };
enum { KEEP_TXPOW = -128 };


#if !defined(LMIC_DISABLE_PING)
//! \internal
struct rxsched_t {
    uint8_t     dr;
    uint8_t     intvExp;   // 0..7
    uint8_t     slot;      // runs from 0 to 128
    uint8_t     rxsyms;
    ostime_t rxbase;
    ostime_t rxtime;    // start of next spot
    uint32_t     freq;
};
#endif // !LMIC_DISABLE_PING


#if !defined(LMIC_DISABLE_BEACONS)
//! Parsing and tracking states of beacons.
enum { BCN_NONE    = 0x00,   //!< No beacon received
       BCN_PARTIAL = 0x01,   //!< Only first (common) part could be decoded (info,lat,lon invalid/previous)
       BCN_FULL    = 0x02,   //!< Full beacon decoded
       BCN_NODRIFT = 0x04,   //!< No drift value measured yet
       BCN_NODDIFF = 0x08 }; //!< No differential drift measured yet
//! Information about the last and previous beacons.
struct bcninfo_t {
    ostime_t txtime;  //!< Time when the beacon was sent
    int8_t     rssi;    //!< Adjusted RSSI value of last received beacon
    int8_t     snr;     //!< Scaled SNR value of last received beacon
    uint8_t     flags;   //!< Last beacon reception and tracking states. See BCN_* values.
    uint32_t     time;    //!< GPS time in seconds of last beacon (received or surrogate)
    //
    uint8_t     info;    //!< Info field of last beacon (valid only if BCN_FULL set)
    int32_t     lat;     //!< Lat field of last beacon (valid only if BCN_FULL set)
    int32_t     lon;     //!< Lon field of last beacon (valid only if BCN_FULL set)
};
#endif // !LMIC_DISABLE_BEACONS

// purpose of receive window - lmic_t.rxState
enum { RADIO_RST=0, RADIO_TX=1, RADIO_RX=2, RADIO_RXON=3 };
// Netid values /  lmic_t.netid
enum { NETID_NONE=(int)~0U, NETID_MASK=(int)0xFFFFFF };
// MAC operation modes (lmic_t.opmode).
enum { OP_NONE     = 0x0000,
       OP_SCAN     = 0x0001, // radio scan to find a beacon
       OP_TRACK    = 0x0002, // track my networks beacon (netid)
       OP_JOINING  = 0x0004, // device joining in progress (blocks other activities)
       OP_TXDATA   = 0x0008, // TX user data (buffered in pendTxData)
       OP_POLL     = 0x0010, // send empty UP frame to ACK confirmed DN/fetch more DN data
       OP_REJOIN   = 0x0020, // occasionally send JOIN REQUEST
       OP_SHUTDOWN = 0x0040, // prevent MAC from doing anything
       OP_TXRXPEND = 0x0080, // TX/RX transaction pending
       OP_RNDTX    = 0x0100, // prevent TX lining up after a beacon
       OP_PINGINI  = 0x0200, // pingable is initialized and scheduling active
       OP_PINGABLE = 0x0400, // we're pingable
       OP_NEXTCHNL = 0x0800, // find a new channel
       OP_LINKDEAD = 0x1000, // link was reported as dead
       OP_TESTMODE = 0x2000, // developer test mode
};
// TX-RX transaction flags - report back to user
enum { TXRX_ACK    = 0x80,   // confirmed UP frame was acked
       TXRX_NACK   = 0x40,   // confirmed UP frame was not acked
       TXRX_NOPORT = 0x20,   // set if a frame with a port was RXed, clr if no frame/no port
       TXRX_PORT   = 0x10,   // set if a frame with a port was RXed, LMIC.frame[LMIC.dataBeg-1] => port
       TXRX_DNW1   = 0x01,   // received in 1st DN slot
       TXRX_DNW2   = 0x02,   // received in 2dn DN slot
       TXRX_PING   = 0x04 }; // received in a scheduled RX slot
// Event types for event callback
enum _ev_t { EV_SCAN_TIMEOUT=1, EV_BEACON_FOUND,
             EV_BEACON_MISSED, EV_BEACON_TRACKED, EV_JOINING,
             EV_JOINED, EV_RFU1, EV_JOIN_FAILED, EV_REJOIN_FAILED,
             EV_TXCOMPLETE, EV_LOST_TSYNC, EV_RESET,
             EV_RXCOMPLETE, EV_LINK_DEAD, EV_LINK_ALIVE, EV_SCAN_FOUND,
             EV_TXSTART };
typedef enum _ev_t ev_t;

enum {
        // This value represents 100% error in LMIC.clockError
        MAX_CLOCK_ERROR = 65536,
};

struct lmic_t {
    // Radio settings TX/RX (also accessed by HAL)
    ostime_t    txend;
    ostime_t    rxtime;
    uint32_t        freq;
    int8_t        rssi;
    int8_t        snr;
    rps_t       rps;
    uint8_t        rxsyms;
    uint8_t        dndr;
    int8_t        txpow;     // dBm

    osjob_t     osjob;

    // Channel scheduling
#if defined(LMIC_EU686)
    band_t      bands[MAX_BANDS];
    uint32_t        channelFreq[MAX_CHANNELS];
    uint16_t        channelDrMap[MAX_CHANNELS];
    uint16_t        channelMap;
#elif defined(LMIC_US915)
    uint32_t        xchFreq[MAX_XCHANNELS];    // extra channel frequencies (if device is behind a repeater)
    uint16_t        xchDrMap[MAX_XCHANNELS];   // extra channel datarate ranges  ---XXX: ditto
    uint16_t        channelMap[(72+MAX_XCHANNELS+15)/16];  // enabled bits
    uint16_t        chRnd;        // channel randomizer
#endif
    uint8_t        txChnl;          // channel for next TX
    uint8_t        globalDutyRate;  // max rate: 1/2^k
    ostime_t    globalDutyAvail; // time device can send again

    uint32_t        netid;        // current network id (~0 - none)
    uint16_t        opmode;
    uint8_t        upRepeat;     // configured up repeat
    int8_t        adrTxPow;     // ADR adjusted TX power
    uint8_t        datarate;     // current data rate
    uint8_t        errcr;        // error coding rate (used for TX only)
    uint8_t        rejoinCnt;    // adjustment for rejoin datarate
#if !defined(LMIC_DISABLE_BEACONS)
    int16_t        drift;        // last measured drift
    int16_t        lastDriftDiff;
    int16_t        maxDriftDiff;
#endif

    uint16_t        clockError; // Inaccuracy in the clock. CLOCK_ERROR_MAX
                            // represents +/-100% error

    uint8_t        pendTxPort;
    uint8_t        pendTxConf;   // confirmed data
    uint8_t        pendTxLen;    // +0x80 = confirmed
    uint8_t        pendTxData[MAX_LEN_PAYLOAD];

    uint16_t        devNonce;     // last generated nonce
    uint8_t        nwkKey[16];   // network session key
    uint8_t        artKey[16];   // application router session key
    devaddr_t   devaddr;
    uint32_t        seqnoDn;      // device level down stream seqno
    uint32_t        seqnoUp;

    uint8_t        dnConf;       // dn frame confirm pending: LORA::FCT_ACK or 0
    int8_t        adrAckReq;    // counter until we reset data rate (0=off)
    uint8_t        adrChanged;

    uint8_t        rxDelay;      // Rx delay after TX
    
    uint8_t        margin;
    bool       ladrAns;      // link adr adapt answer pending
    bool       devsAns;      // device status answer pending
    uint8_t        adrEnabled;
    uint8_t        moreData;     // NWK has more data pending
#if !defined(LMIC_DISABLE_MCMD_DCAP_REQ)
    bool       dutyCapAns;   // have to ACK duty cycle settings
#endif
#if !defined(LMIC_DISABLE_MCMD_SNCH_REQ)
    uint8_t        snchAns;      // answer set new channel
#endif
    // 2nd RX window (after up stream)
    uint8_t        dn2Dr;
    uint32_t        dn2Freq;
#if !defined(LMIC_DISABLE_MCMD_DN2P_SET)
    uint8_t        dn2Ans;       // 0=no answer pend, 0x80+ACKs
#endif

    // Class B state
#if !defined(LMIC_DISABLE_BEACONS)
    uint8_t        missedBcns;   // unable to track last N beacons
    uint8_t        bcninfoTries; // how often to try (scan mode only)
#endif
#if !defined(LMIC_DISABLE_MCMD_PING_SET) && !defined(LMIC_DISABLE_PING)
    uint8_t        pingSetAns;   // answer set cmd and ACK bits
#endif
#if !defined(LMIC_DISABLE_PING)
    rxsched_t   ping;         // pingable setup
#endif

    // Public part of MAC state
    uint8_t        txCnt;
    uint8_t        txrxFlags;  // transaction flags (TX-RX combo)
    uint8_t        dataBeg;    // 0 or start of data (dataBeg-1 is port)
    uint8_t        dataLen;    // 0 no data or zero length data, >0 byte count of data
    uint8_t        frame[MAX_LEN_FRAME];

#if !defined(LMIC_DISABLE_BEACONS)
    uint8_t        bcnChnl;
    uint8_t        bcnRxsyms;    //
    ostime_t    bcnRxtime;
    bcninfo_t   bcninfo;      // Last received beacon info
#endif

    uint8_t        noRXIQinversion;
};
//! \var struct lmic_t LMIC
//! The state of LMIC MAC layer is encapsulated in this variable.
DECLARE_LMIC; //!< \internal

//! Construct a bit map of allowed datarates from drlo to drhi (both included).
#define DR_RANGE_MAP(drlo,drhi) (((uint16_t)0xFFFF<<(drlo)) & ((uint16_t)0xFFFF>>(15-(drhi))))
#if defined(LMIC_EU686)
enum { BAND_MILLI=0, BAND_CENTI=1, BAND_DECI=2, BAND_AUX=3 };
bool LMIC_setupBand (uint8_t bandidx, int8_t txpow, uint16_t txcap);
#endif
bool LMIC_setupChannel (uint8_t channel, uint32_t freq, uint16_t drmap, int8_t band);
void  LMIC_disableChannel (uint8_t channel);
#if defined(LMIC_US915)
void  LMIC_enableChannel (uint8_t channel);
void  LMIC_enableSubBand (uint8_t band);
void  LMIC_disableSubBand (uint8_t band);
void  LMIC_selectSubBand (uint8_t band);
#endif

void  LMIC_setDrTxpow   (dr_t dr, int8_t txpow);  // set default/start DR/txpow
void  LMIC_setAdrMode   (bool enabled);        // set ADR mode (if mobile turn off)
#if !defined(LMIC_DISABLE_JOIN)
bool LMIC_startJoining (void);
#endif

void  LMIC_shutdown     (void);
void  LMIC_init         (void);
void  LMIC_reset        (void);
void  LMIC_clrTxData    (void);
void  LMIC_setTxData    (void);
int   LMIC_setTxData2   (uint8_t port, uint8_t *data, uint8_t dlen, uint8_t confirmed);
void  LMIC_sendAlive    (void);

#if !defined(LMIC_DISABLE_BEACONS)
bool LMIC_enableTracking  (uint8_t tryBcnInfo);
void  LMIC_disableTracking (void);
#endif

#if !defined(LMIC_DISABLE_PING)
void  LMIC_stopPingable  (void);
void  LMIC_setPingable   (uint8_t intvExp);
#endif
#if !defined(LMIC_DISABLE_JOIN)
void  LMIC_tryRejoin     (void);
#endif

void LMIC_setSession (uint32_t netid, devaddr_t devaddr, uint8_t *nwkKey, uint8_t *artKey);
void LMIC_setLinkCheckMode (bool enabled);
void LMIC_setClockError(uint16_t error);

// Declare onEvent() function, to make sure any definition will have the
// C conventions, even when in a C++ file.
DECL_ON_LMIC_EVENT;



// Special APIs - for development or testing
// !!!See implementation for caveats!!!

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _lmic_h_
