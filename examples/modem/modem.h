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


#include "lmic.h"

// modem version
#define VERSION_MAJOR 1
#define VERSION_MINOR 2
#define VERSION_STR   "VERSION 1.2 ("__DATE__" "__TIME__")"

// LED ids
#define LED_SESSION 1  // (IMST: yellow, LRSC: green)
#define LED_POWER   2  // (IMST: green,  LRSC: red)

// patch patterns
#define PATTERN_JOINCFG_STR "g0CMw49rRbav6HwQN0115g42OpmvTn7q" // (32 bytes)
#define PATTERN_JOINCFG_HEX "6730434d7734397252626176364877514e303131356734324f706d76546e3771"
#define PATTERN_JOINCFG_CRC 0x6B3D
#define PATTERN_SESSCFG_STR "Bmf3quaCJwVKURWWREeGKtm0pqLD0Yhr5cpPkP6s" // (40 bytes)
#define PATTERN_SESSCFG_HEX "426d6633717561434a77564b55525757524565474b746d3070714c4430596872356370506b503673"
#define PATTERN_SESSCFG_CRC 0xC9D5

// layout of join paramters
typedef struct {
    uint8_t deveui[8];
    uint8_t appeui[8];
    uint8_t devkey[16];
} joinparam_t;

// layout of session parameters
typedef struct {
    uint32_t netid;
    devaddr_t devaddr;
    uint8_t nwkkey[16];
    uint8_t artkey[16];
} sessparam_t;

// persistent state
typedef struct {
    uint32_t cfghash;
    uint32_t flags;
    joinparam_t joinpar;
    sessparam_t sesspar;
    uint32_t seqnoDn;
    uint32_t seqnoUp;
    uint32_t eventmask;
} persist_t;

#define FLAGS_JOINPAR 0x01
#define FLAGS_SESSPAR 0x02

#define PERSIST ((persist_t*)EEPROM_BASE)


// frame rx/tx state
#define FRAME_INIT   0x00
#define FRAME_A_A    0xA1
#define FRAME_A_T    0xA2
#define FRAME_A_OK   0xA3
#define FRAME_A_ERR  0xA4
#define FRAME_B_B    0xB1
#define FRAME_B_LEN  0xB2
#define FRAME_B_LRC  0xB3
#define FRAME_B_OK   0xB4
#define FRAME_B_ERR  0xB5

typedef struct {
    uint8_t state;
    uint8_t *buf;
    uint16_t len;
    uint16_t max;
    uint8_t lrc;
 } FRAME;


void modem_init (void);
void modem_rxdone (osjob_t* j);
void modem_txdone (osjob_t* j);

void buffer_init (void);
uint8_t* buffer_alloc (uint16_t len);
void buffer_free (uint8_t* buf, uint16_t len);

void queue_init (void);
void queue_add (uint8_t* buf, uint16_t len);
uint8_t queue_shift (FRAME* f);

void frame_init (FRAME* f, uint8_t* buf, uint16_t max);
uint16_t frame_tx (uint8_t next);
uint8_t frame_rx (uint8_t c);

void usart_init (void);
void usart_starttx (void);
void usart_startrx (void);

void leds_init (void);
void leds_set (uint8_t id, uint8_t state);

uint8_t gethex (uint8_t* dst, const uint8_t* src, uint16_t len);
uint8_t puthex (uint8_t* dst, const uint8_t* src, uint8_t len);
uint8_t int2hex (uint8_t* dst, uint32_t v);
uint8_t hex2int (uint32_t* n, const uint8_t* src, uint8_t len);
uint8_t dec2int (uint32_t* n, const uint8_t* src, uint8_t len);
void reverse (uint8_t* dst, const uint8_t* src, uint8_t len);
uint8_t tolower (uint8_t c);
uint8_t toupper (uint8_t c);

uint8_t cpystr (uint8_t* dst, const char* src);
uint8_t cmpstr (uint8_t* buf, uint8_t len, char* str);
