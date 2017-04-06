 /*-
 * Copyright (c) 2017 moecmks
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRCMD, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
 
#if !defined (codec_included_MOECMKS)
#define codec_included_MOECMKS

/* 
 * fx1s read/write codec. 
 * simple, each read a current register size unit memory 
 * more information, please refer to the PLC manual and Internet resources.
 */
#if defined (__cplusplus)  /** __cplusplus */
extern "C" {
#endif  /** __cplusplus */

/* Portable fixed length ***/
#include "stdint.h"

/*
 * errror code
 */
#define FX1S_RANGE 1 
#define FX1S_FAIL 2 
#define FX1S_KFAIL 3 
#define FX1S_UNKN 4 
#define FX1S_LIMIT 5 
#define FX1S_NAK 6 
#define FX1S_ACK 7
#define FX1S_PARA 8
#define FX1S_INCOP 9
#define FX1S_OK 0

/*
 * version control
 */
enum FX1S_VERSION {
  FX1S_VERSION_10MR = 0,
  FX1S_VERSION_14MR, 
  FX1S_VERSION_20MR, 
  FX1S_VERSION_30MR,
  FX1S_VERSION_OVERFLAGS
};

/*
 * register field.
 * Ignore T bit register and set register.
 * Ignore C bit register
 */
enum FX1S_REGISTER_FIELD {
  FX1S_REGISTER_FIELD_S = 0,
  FX1S_REGISTER_FIELD_X,
  FX1S_REGISTER_FIELD_Y_OUT,
  FX1S_REGISTER_FIELD_Y_PLS,
  FX1S_REGISTER_FIELD_D,
  FX1S_REGISTER_FIELD_T,
  FX1S_REGISTER_FIELD_M,
  FX1S_REGISTER_FIELD_C16,
  FX1S_REGISTER_FIELD_C32,
  FX1S_REGISTER_FIELD_CRESET
};

/*
 * section .link code
 */
#define SECTION_LINK_STX 0x02 /* PLC info-section begin flags. */
#define SECTION_LINK_ETX 0x03 /* PLC info-section end flags .***/
#define SECTION_LINK_EOT 0x03 /* .***/
#define SECTION_LINK_ENQ 0x05 /* PLC only test enable */
#define SECTION_LINK_ACK 0x06 /* PLC reply "can do" */
#define SECTION_LINK_LF 0x0A /* .***/
#define SECTION_LINK_CL 0x0C /* .***/
#define SECTION_LINK_CR 0x0D /* .***/
#define SECTION_LINK_NAK 0x15 /* PLC reply "bad things" */ 

/*
 * section cmdion.
 */
#define SECTION_CMD_READ      '0' /* .***/
#define SECTION_CMD_WRITE     '1' /* .***/
#define SECTION_CMD_FORCE_ON  '7' /* .***/
#define SECTION_CMD_FORCE_OFF '8' /* .***/

/*
 * read section 
 */
struct read_section {
  uint8_t stx; /* read_section's stdhead. always SECTIOM_LINK_STX */
  uint8_t cmd; /* read_section's cmd  always SECTIOM_CMD_READ  */
  uint8_t unit_address[4];  /* read_section's address*/
  uint8_t numb[2];          /* read's byte count. simple always one */
  uint8_t etx;    /* read_section's stdend.   always SECTION_LINK_ETX  */
  uint8_t crc[2]; /* correcting code*/
};

/*
 * read section2
 */
struct read_section2 { 
  uint8_t stx; /* read_section's stdhead. always SECTIOM_LINK_STX */
  uint8_t cmd; /* read_section's cmd  always SECTIOM_CMD_READ  */
  uint8_t unit_address[4];  /* read_section's address*/
  uint8_t numb[2];          /* read's byte count. simple always one */
  uint8_t etx;    /* read_section's stdend.   always SECTION_LINK_ETX  */
  uint8_t crc[2]; /* correcting code*/
  uint8_t crce; /* easy to read..**/
  uint16_t opbsize; /******/
  uint8_t opboff; /* for bit register(X, Y, M.) **/
  uint16_t opbaddr; /* easy to read. **/
};

/*
 * write section 
 */
struct write_section {
  uint8_t stx; /* write_section's stdhead.  always SECTIOM_LINK_STX */
  uint8_t cmd; /* write_section's cmd  always SECTIOM_CMD_WRITE   */
  uint8_t unit_address[4]; /* write_section's address*/
  uint8_t numb[2]; /* write's byte count.. must <= 64  */
  uint8_t etx; /* write_section's stdend. */
  uint8_t crc[2]; /* correcting code*/
  uint8_t crce; /* easy to read..**/
  uint8_t obpoff; /* for bit register(X, Y, M.) **/
  uint16_t opbaddr; /* easy to read. **/
};

/*
 * force section 
 */
struct force_section {
  uint8_t stx; /* force_section's stdhead.  always SECTIOM_LINK_STX */
  uint8_t cmd; /* force_section's cmd  always SECTIOM_CMD_FORCE_OFF or SECTIOM_CMD_FORCE_ON   */
  uint8_t unit_address[4]; /* force_section's address*/
  uint8_t etx; /* force_section's stdend. */
  uint8_t crc[2]; /* correcting code*/
};

/*
 * Accept the write section is very simple, 
 * if successful send SECTION_LINK_ACK otherwise SECTION_LINK_NAK
 */
int fx1s_makersecb (struct read_section2 *rsec, /* write to the serial port, use the size of the read_section */
                         enum FX1S_REGISTER_FIELD rf, uint16_t  *rvap_size,
                         enum FX1S_VERSION ver, uint16_t address);
int fx1s_makewsecb (void *wsec, /* Variable size structure, so use void *, please understand **/
                   void *spval, 
                         enum FX1S_REGISTER_FIELD rf, uint16_t  *wsec_size,
                         enum FX1S_VERSION ver, uint16_t address);
int fx1s_makefsecb (struct force_section *fsec, 
                         enum FX1S_REGISTER_FIELD rf, 
                         enum FX1S_VERSION ver, uint16_t address);            

#if defined (__cplusplus)  /** __cplusplus */
}
#endif  /** __cplusplus */

#endif /* codec_included_MOECMKS */