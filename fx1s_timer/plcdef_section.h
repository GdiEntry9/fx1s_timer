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
 
#if !defined (plcdef_section_included_MOECMKS)
#define plcdef_section_included_MOECMKS

/* 
 * PLCDEF file structure is very simple, 
 * according to the real section of the section to find the first segment, 
 * followed by a continuous array of chunk. 
 */
#if defined (__cplusplus)  /** __cplusplus */
extern "C" {
#endif  /** __cplusplus */

/* Portable fixed length ***/
#include "stdint.h"

struct plcdef_chunk_chain {
  struct plcdef_chunk_chain *level;
  int32_t setv;
  int32_t delay[2];
  int32_t writeIO[2];
  int32_t writeIO2e[2];
};

struct plcdef_chunk {
  int32_t setv;   
  int32_t delay[2];
  int32_t writeIO[2];
  int32_t writeIO2e[2];
};

struct plcdef_header {
  int8_t magic[4]; /* always 0x38, 0x25, 0x74, 0x98 **/
  int32_t line; /* total line nums. **/
  int32_t buad; /* PLC's buad **/
  int32_t parity;/* PLC's check bit **/
  int32_t stopbit;/* PLC's stop bit **/
  int32_t comIdx;   /* default link's comport. **/
  int32_t pulse_addr; /* Pulse count address **/
  int32_t version; /* fx1s's version **/
#if defined (MAY_NEED_BUT_THIS_IS_JUST_A_SIMPLE_LITTLE_PROGRAM)
  int8_t roff;     /* real random offset **/
  int32_t crc32_chunk;/* adjust code. for _plcdef_chunk_chain_s array **/
  int32_t crc32_headNotself;/* adjust code. magic~crc32_chunk **/
  int32_t crc32_headNotself2;/* adjust code level-2. magic~crc32_headNotself ^ roff's crc32- total **/
#endif
};         

#if defined (__cplusplus)  /** __cplusplus */
}
#endif  /** __cplusplus */

#endif /* codec_included_MOECMKS */