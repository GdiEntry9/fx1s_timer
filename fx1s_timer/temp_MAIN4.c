#include <Windows.h>
#include <windowsx.h> // combo control included.
#include <Commctrl.h> // listview/richedit control included.
#include <intrin.h>
#include <assert.h>
#include <process.h>
#include <stdio.h>
#include "resource.h"
#include "codecb.h"
#include "comm_enum.h"
#include "plcdef_section.h"

#if defined (_UNICODE) || defined (UNICODE)
#define autoi _wtoi
#define aucmp wcscmp
#define auncmp wcsncmp
#define aulwr wcslwr
#define auupr wcsupr
#define aulen wcslen
#define aucat wcscat
#define auprintf wprintf /* only unicode ??? **/
#define secominfo secominfo_unicode
#define secominfo_sec secominfo_section_unicode
#define alloc_secominfos alloc_secominfo_unicode
#define dealloc_secominfos dealloc_secominfo_unicode
#else
#define autoi atoi
#define aucmp strcmp
#define auncmp strncmp
#define aulwr strlwr
#define auupr strupr
#define aulen strlen
#define aucat strcat
#define auprintf sprintf
#define secominfo secominfo_ansi
#define secominfo_sec secominfo_section_ansi
#define alloc_secominfos alloc_secominfo_ansi
#define dealloc_secominfos dealloc_secominfo_ansi
#endif
/* unicode/ansi switch. **/
#define AU_C TEXT
#define AU TCHAR
/* column indicate. **/
#define ISR_COLUMN_LINE 0
#define ISR_COLUMN_C_SETTINGS (ISR_COLUMN_LINE + 1)
#define ISR_COLUMN_C_CURRENT  (ISR_COLUMN_C_SETTINGS + 1)
#define ISR_COLUMN_OUTPUT_COIL_L  (ISR_COLUMN_C_CURRENT + 1)
#define ISR_COLUMN_OUTPUT_CURRENT_L (ISR_COLUMN_OUTPUT_COIL_L + 1)
#define ISR_COLUMN_OUTPUT_DELAY_L (ISR_COLUMN_OUTPUT_CURRENT_L + 1)
#define ISR_COLUMN_OUTPUT_COIL_R  (ISR_COLUMN_OUTPUT_DELAY_L + 1)
#define ISR_COLUMN_OUTPUT_CURRENT_R (ISR_COLUMN_OUTPUT_COIL_R + 1)
#define ISR_COLUMN_OUTPUT_DELAY_R  (ISR_COLUMN_OUTPUT_CURRENT_R + 1)
#define ISR_COLUMN_COMPLETE_IND  (ISR_COLUMN_OUTPUT_DELAY_R + 1)
/* ctl array nums **/
#define CTL_ARRAY_NUMS 36
/* ind mask **/
#define CTL_END_MASK (1 << 0)
#define CTL_FORCE_STOP_MASK (1 << 1)
#define CTL_NORMAL_END_MASK (1 << 2)
#define CTL_FORCE_STOP2_MASK (1 << 3)
#define CTL_FORCE_RESUME_MASK (1 << 4)
#define CTL_FORCE2_RESUME_MASK (1 << 5)
#define CTL_FORCE_TERM_MASK (1 << 6)

/** window/ctrl window **/
static HWND window_main_dialog = NULL;
static HWND window_main_bkbrush = NULL;
static HWND window_main_listhook = NULL;
static HWND window_main_editattach = NULL;
static HWND window_main_editinsert = NULL;
static HWND window_main_editdelete = NULL;
static HWND window_main_editreset = NULL;
static HWND window_main_editstatus = NULL;
static HWND window_main_butstartop = NULL;
static HWND window_main_butfstop = NULL;
static HWND window_main_butfstop2 = NULL;
static HWND window_main_butfend = NULL;
static HWND window_main_richlevel = NULL;
static HWND window_scmb_commidx = NULL;
static HWND window_scmb_parity = NULL;
static HWND window_scmb_stopbit = NULL;
static HWND window_scmb_buad = NULL;
static HWND window_scmb_plcver = NULL;
static HWND window_scmb_hookC = NULL;
/** subclass control. **/
static void *callback_hook_listhook = NULL;
static void *callback_hook_editattach = NULL;
static void *callback_hook_main_dialog = NULL;
static void *callback_hook_editinsert = NULL;
static void *callback_hook_editdelete = NULL;
static void *callback_hook_edithookC = NULL;
static void *callback_hook_editreset = NULL;
/** Misc. **/
static AU _g_chbuf[1024*1024*2]; /* GLOBAL ch buffer **/
/** Look-up table **/
static const AU *parity_tab[] = {
#define PARITY_TAB_NONE         0
  AU_C           ("none"),
#define PARITY_TAB_EVEN         (PARITY_TAB_NONE+1)
  AU_C           ("even"),
#define PARITY_TAB_ODD          (PARITY_TAB_EVEN+1)
  AU_C           ("odd") 
#define PARITY_TAB_STD_IDX      (PARITY_TAB_ODD+0)
#define PARITY_TAB_LST_IDX      (PARITY_TAB_ODD+1)
};
static const AU *buad_tab[] = {
#define BUAD_TAB_9600BPS        0
  AU_C         ("9.6Kbps"),
#define BUAD_TAB_19200BPS       (BUAD_TAB_9600BPS+1)
  AU_C         ("19.2Kbps"),
#define BUAD_TAB_38400BPS       (BUAD_TAB_19200BPS+1)
  AU_C         ("38.4Kbps"),
#define BUAD_TAB_57600BPS       (BUAD_TAB_38400BPS+1)
  AU_C         ("57.6Kbps"),
#define BUAD_TAB_115200BPS      (BUAD_TAB_57600BPS+1)
  AU_C         ("115.2Kbps")
#define BUAD_TAB_STD_IDX        (BUAD_TAB_115200BPS+0)
#define BUAD_TAB_LST_IDX        (BUAD_TAB_115200BPS+1)
};
static const AU *stopbit_tab[] = {
#define STOPBIT_TAB_ONE         0
  AU_C            ("1 stop bit"),
#define STOPBIT_TAB_ONEH        (STOPBIT_TAB_ONE+1)
  AU_C            ("1.5 stop bit"),
#define STOPBIT_TAB_TWO         (STOPBIT_TAB_ONEH+1)
  AU_C            ("2 stop bit")
#define STOPBIT_TAB_STD_IDX     (STOPBIT_TAB_TWO+0)
#define STOPBIT_TAB_LST_IDX     (STOPBIT_TAB_TWO+1)
};
static const AU *vailedBit_tab[] = {
#define VAILEDBIT_TAB_4         0
  AU_C            ("4"),
#define VAILEDBIT_TAB_5         (VAILEDBIT_TAB_4+1)
  AU_C            ("5"),
#define VAILEDBIT_TAB_6         (VAILEDBIT_TAB_5+1)
  AU_C            ("6"),
#define VAILEDBIT_TAB_7         (VAILEDBIT_TAB_6+1)
  AU_C            ("7")
#define VAILEDBIT_TAB_8         (VAILEDBIT_TAB_7+1)
  AU_C            ("8")
#define VAILEDBIT_TAB_STD_IDX   (VAILEDBIT_TAB_8+0)
#define VAILEDBIT_TAB_LST_IDX   (VAILEDBIT_TAB_8+1)
};
static const AU *version_tab[] = {
#define VERSION_TAB_10MR        0
  AU_C            ("10MR"),
#define VERSION_TAB_14MR        (VERSION_TAB_10MR+1)
  AU_C            ("14MR"),
#define VERSION_TAB_20MR        (VERSION_TAB_14MR+1)
  AU_C            ("20MR"),
#define VERSION_TAB_30MR        (VERSION_TAB_20MR+1)
  AU_C            ("30MR")
#define VERSION_TAB_STD_IDX     (VERSION_TAB_30MR+0)
#define VERSION_TAB_LST_IDX     (VERSION_TAB_30MR+1)
};
/* control/misc private data... **/
static struct comset {
  DWORD buad_rate;
  BYTE stop_bits;
  BYTE parity;
  BYTE comm_idx;
  BYTE hookC; /* address **/
  BYTE version; /* 0:10MR 1:14MR 2:20MR 3:30MR **/
} sg_comset[2] = {
  { CBR_9600, ONESTOPBIT, EVENPARITY, -1, 200, 0 },
  { CBR_9600, ONESTOPBIT, EVENPARITY, -1, 200, 0 }
};
static struct comidx_list {
  struct comidx_list *level;
  int combidx;
  int comidx;
};
static struct comseti {
  int parity[PARITY_TAB_LST_IDX];
  int stopbit[STOPBIT_TAB_LST_IDX];
  int buad[BUAD_TAB_LST_IDX];
  int version[VERSION_TAB_LST_IDX];
  int comm_num;  /** dynamic**/
  struct comidx_list *clist; /** dynamic**/
} sg_comseti;
static struct listset {
  int cur_sub;
  int cur_item;
  int cur_line;
  int res_line;
} sg_listset = {
  -1,
  -1,
   1,
  -1
};
static struct ctl_delay_chain_t {
  int8_t out_y;   /* out IO[dec mode] */
  int8_t out_ey;  /* out IO[eight] */
  int64_t pivot;  /* base timing start !*/
  int64_t offset; /* std delay, 100ms/per */
  int64_t offset2;/* real delay for NT's High precision counter **/
};
static struct ctl_delay_chain {
  struct ctl_delay_chain *level;
  struct ctl_delay_chain_t left;
  struct ctl_delay_chain_t right;
  struct ctl_delay_chain_t *switch0;
  int item;
} ctl_delay_array[CTL_ARRAY_NUMS];
/* Small memory optimization for ctl_delay_chain **/
unsigned int ctlheap_delay_locked[CTL_ARRAY_NUMS];
unsigned int ctl_delay_init = FALSE;

struct ctl_delay_chain *ctl_delay_malloc (void) {
  if (ctl_delay_init == FALSE) {
    ctl_delay_init = TRUE;
    ZeroMemory (& ctlheap_delay_locked[0], sizeof (ctlheap_delay_locked));
  }
  {
    int ii = 0;
    for (; ii != CTL_ARRAY_NUMS; ii++) {
      if (ctlheap_delay_locked[ii] == 0) {
        ctlheap_delay_locked[ii] = !0;
        return & ctl_delay_array[ii];
      }
    }
    MessageBox (NULL, AU_C ("没有可用的延迟堆!"), AU_C ("错误"), MB_ICONERROR);
  }
}

void ctl_delay_free (struct ctl_delay_chain *ptr) {
  if (ctl_delay_init == FALSE) {
    return;
  } else {
    int ii = 0;
    for (; ii != CTL_ARRAY_NUMS; ii++) {
      if (ptr == & ctl_delay_array[ii]) {
        ctlheap_delay_locked[ii] = 0;
        return ;
      }
    }
  }
}

static struct ctl_s {
  HANDLE comport;
  HANDLE timingth;
  BOOL list_edis;
  struct plcdef_chunk *pd_chunk;
  struct plcdef_chunk *pd_poll;
  struct plcdef_chunk *xe_poll;
  int32_t pd_len;
  struct ctl_delay_chain *level;
  int cur_runline;
  int unlink;
  int in_fstop;
} sg_ctl = {
  (void *)-1,
  (void *)-1,
   FALSE,
   NULL,
   NULL,
   NULL,
  -1,
   NULL,
   1,
   TRUE,
   FALSE
};

__declspec(align(32)) intptr_t volatile sg_thp_mask = 0;
__declspec(align(32)) intptr_t volatile sg_spinlock = 0;
__declspec(align(32)) uint64_t ticks_overflow = 0;
__declspec(align(32)) uint64_t ticks_Freq = 0;
__declspec(align(32)) uint64_t ticks_Qf = 0;
__declspec(align(32)) uint64_t ticks_St = 0;

int timing_init60 (void) {

#if 1 
  SetThreadAffinityMask (GetCurrentThread (), 1);
#endif 
  QueryPerformanceFrequency ((LARGE_INTEGER *) & ticks_Freq);
  ticks_St = ticks_Freq * 2;
  return 0;
}

int timing_start60 (void) {

  QueryPerformanceCounter ((LARGE_INTEGER *) & ticks_Qf);
  ticks_Qf -= ticks_overflow; /* XXX:maybe neg. **/
  return 0;
}

int timing_terming60 (void) {

  int64_t difference = 0;
  uint64_t time_start = ticks_Qf;
  uint64_t time_current = 0;
  uint64_t time_requir = ticks_St;
  uint64_t time_elapse = 0; /* XXX:win32 timing is LONGLONG not uint64_t */

  do 
  {
    QueryPerformanceCounter ((LARGE_INTEGER *) & time_current);
    time_elapse = time_current - time_start;
    difference = time_elapse -  time_requir;

    /* negative phase ? */
    if (difference >= 0)
      {
        /* burning out .**/
        // tim_v->ticks_Qf += time_elapse; // 脑内观测. 120-2 mid 80[60] diff = 40[60.20] . sec-
        ticks_overflow = difference;
        break;
      }
  } while (1);

  return 0;
}
  /** e.g. 00020 -> 20 **/
AU *bzero_filter (AU *chbuf, int numb) {

  int ii = 0;

  for (; ii != numb && chbuf[0] != 0; ii++) {
    if (chbuf[ii] != AU_C ('0'))
      return & chbuf[ii];
    else if (ii == numb - 1)
      return & chbuf[ii];
  }
  return NULL;
}

static
uint16_t vailed8 (uint16_t nums) {
 
  /* e.g.
   * 1234 vailed.
   * 9000 invailed.
   * 1007 vailed.
   * 1811 invailed.
   * 0 ~ 65535
   */
   uint16_t d0 = nums % 10 >> 0;
   uint16_t d1 = nums % 100 / 10;
   uint16_t d2 = nums % 1000 / 100;
   uint16_t d3 = nums % 10000 / 1000;
  
   if ( d0 > 7 || d1 > 7)
     return -1;
   if ( d2 > 7)
     return -1;
   return d0 + d1 * 8 + d2 * 8 * 8 + d3 * 8 * 8 * 8;
}
/* Convert characters to specific number - noexport */
static
char ascii_to_num (char ch) {
  /* e.g.
   *  source '9' -> target 9
   *  source 'A' -> target 10
   *  source '1' -> target 1
   *  source 'a' -> (nondone, Don't use lowercase letters in fx1s-14mr-001).
   */
  if (ch >= '0' && ch <= '9')
    return (ch - '0');
  if (ch >= 'A' && ch <= 'F')
    return (ch - ('A' - 10));
  else
    assert (0);

  return ch;
}

/* Convert number to specific characters - noexport */
static
char num_to_ascii (char ch) {
  /* e.g.
   *  source 9 -> target '9'
   *  source A -> target '0'
   *  source 1 -> target '1'
   *  source a -> (nondone, Don't use lowercase letters in fx1s-14mr-001).
   */
  if (ch >= 0x00 && ch <= 0x09)
    return (ch + '0');
  if (ch >= 0x0A && ch <= 0x0F)
    return (ch + ('A' - 10));
  else
    assert (0);

  return ch;
}
static 
void append_text (HWND window, AU *chbuf) {

  /* append chbuf */
  SendMessage (window, EM_SETSEL, -2, -1);
  SendMessage (window, EM_REPLACESEL, TRUE, (LPARAM)chbuf);
  /* set scroll tail */
  SendMessage (window, WM_VSCROLL, SB_BOTTOM, 0);
}

static
void __cdecl append_text2 (HWND window, AU *format, ...) {
 
  va_list arg_list; 
  va_start (arg_list, format); 
#if defined (UNICODE) || defined (_UNICODE)
  wvsprintf (& _g_chbuf[0], format, arg_list); 
#else
  vsprintf (& _g_chbuf[0], format, arg_list); 
#endif
  va_end(arg_list); 
  append_text (window, & _g_chbuf[0]);
}


static BOOL wait_inputbuf (int32_t nums, char *buf)
{
  BOOL io_success_;
  DWORD error_;              
  DWORD nums_;
  COMSTAT comm_stat;           
  DWORD try_ = 0;
  BOOL return_ = TRUE;
  assert (nums != 0);
repeat:
  io_success_ = ClearCommError(sg_ctl.comport, & error_, & comm_stat);
  assert (io_success_ != FALSE);
  if (++ try_ > 5000) {
    return_ = FALSE;
    goto end;
  }
  if (nums != comm_stat.cbInQue)          
    goto repeat;       
  io_success_ = ReadFile (sg_ctl.comport, buf, comm_stat.cbInQue, & nums_, NULL);
  assert (io_success_ != FALSE);
  assert (comm_stat.cbInQue == nums_);
end:
  PurgeComm (sg_ctl.comport, PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR | PURGE_TXABORT);
  Sleep (1);
  return return_;
}

static int8_t readOYB (enum FX1S_VERSION fver, int16_t e_addr) {
 
   struct read_section2 rdsec;
   char varsbuf[128];
   char asc_hi;
   char asc_lo;
   int isr = 0;
   int off_numbs = 0;
   int tm_numbs = 0;
   uint16_t rv_numbs = 0;
   uint32_t iw_size = 0;

   BOOL io_success_;
   while (InterlockedExchangePointer (& sg_spinlock, 1) != 0) ;
repeat:
   /* prepare Output-register read_section. */
   isr = fx1s_makersecb ( & rdsec, FX1S_REGISTER_FIELD_Y_OUT,
           & rv_numbs, fver, e_addr);
   assert (isr == FX1S_OK);
  
   /* write Output-register read-code. */

   PurgeComm (sg_ctl.comport, PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR | PURGE_TXABORT);
   io_success_ = WriteFile (sg_ctl.comport, & rdsec,
       sizeof(struct read_section), & tm_numbs, NULL);
   assert (io_success_ != FALSE);
   assert (tm_numbs == sizeof(struct read_section));
   if (wait_inputbuf (rv_numbs, & varsbuf[0]) == FALSE)
     goto repeat;
   assert (varsbuf[0] == SECTION_LINK_STX);
   asc_hi = ascii_to_num ( varsbuf[1]) << 4;
   asc_lo = ascii_to_num ( varsbuf[2]) >> 0;
   Sleep(10);
   InterlockedExchangePointer (& sg_spinlock, 0);
   return asc_hi | asc_lo;
}

static void setOYB (enum FX1S_VERSION fver, int16_t e_addr, int8_t val) {

   char varsbuf[128];
   int isr = 0;
   int tm_numbs = 0;
   uint16_t rv_numbs = 0;
   uint32_t iw_size = 0;

   BOOL io_success_;
   while (InterlockedExchangePointer (& sg_spinlock, 1) != 0) ;
repeat:
   /* prepare Output-register write_section. */
   isr = fx1s_makewsecb (& varsbuf[0], & val, FX1S_REGISTER_FIELD_Y_OUT,
                    & rv_numbs, fver, e_addr);
   assert (isr == FX1S_OK);
   /* write Output-register write-code. */
   PurgeComm (sg_ctl.comport, PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR | PURGE_TXABORT);
   io_success_ = WriteFile (sg_ctl.comport, & varsbuf[0],
       rv_numbs, & tm_numbs, NULL);
   assert (io_success_ != FALSE);
   if (wait_inputbuf (1, & varsbuf[0]) == FALSE)
     goto repeat;
   assert (varsbuf[0] == SECTION_LINK_ACK);
   Sleep(10);
   InterlockedExchangePointer (& sg_spinlock, 0);
   return ;
}

static void writePULSE (int32_t val) {
 
   int isr = 0;
   int tm_numbs = 0;
   char varsbuf[128];
   uint16_t rv_numbs = 0;
   uint32_t iw_size = 0;

   BOOL io_success_;
   while (InterlockedExchangePointer (& sg_spinlock, 1) != 0) ;
repeat:
   /* prepare C-register write_section. */
   isr = fx1s_makewsecb (& varsbuf[0], & val, FX1S_REGISTER_FIELD_C32,
                    & rv_numbs, FX1S_VERSION_14MR, sg_comset[0].hookC);
   assert (isr == FX1S_OK);
   /* write Output-register write-code. */
   PurgeComm (sg_ctl.comport, PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR | PURGE_TXABORT);
   io_success_ = WriteFile (sg_ctl.comport, & varsbuf[0],
       rv_numbs, & tm_numbs, NULL);
   assert (io_success_ != FALSE);
   if (wait_inputbuf (1, & varsbuf[0]) == FALSE)
     goto repeat;
   assert (varsbuf[0] == SECTION_LINK_ACK);
   Sleep(10);
   InterlockedExchangePointer (& sg_spinlock, 0);
   return ;
}

static int8_t readRELAY_M (int16_t addr) {
 
   struct read_section2 rdsec;
   char varsbuf[128];
   char asc_hi;
   char asc_lo;
   int isr = 0;
   int off_numbs = 0;
   int tm_numbs = 0;
   uint16_t rv_numbs = 0;
   uint32_t iw_size = 0;

   BOOL io_success_;
   while (InterlockedExchangePointer (& sg_spinlock, 1) != 0) ;
repeat:
   /* prepare Output-register read_section. */
   isr = fx1s_makersecb ( & rdsec, FX1S_REGISTER_FIELD_M,
           & rv_numbs, FX1S_VERSION_14MR, addr);
   assert (isr == FX1S_OK);
  
   /* write Output-register read-code. */
   PurgeComm (sg_ctl.comport, PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR | PURGE_TXABORT);
   io_success_ = WriteFile (sg_ctl.comport, & rdsec,
       sizeof(struct read_section), & tm_numbs, NULL);
   assert (io_success_ != FALSE);
   if (wait_inputbuf (rv_numbs, & varsbuf[0]) == FALSE)
     goto repeat;
   assert (varsbuf[0] == SECTION_LINK_STX);
   asc_hi = ascii_to_num ( varsbuf[1]) << 4;
   asc_lo = ascii_to_num ( varsbuf[2]) >> 0;
   Sleep(10);
   InterlockedExchangePointer (& sg_spinlock, 0);
   return asc_hi | asc_lo;
}
static void writeRELAY_M (uint16_t addr, int8_t val) {
 
   char varsbuf[128];
   int isr = 0;
   int tm_numbs = 0;
   uint16_t rv_numbs = 0;
   uint32_t iw_size = 0;

   BOOL io_success_;
   while (InterlockedExchangePointer (& sg_spinlock, 1) != 0) ;
repeat:
   /* prepare Output-register write_section. */
   isr = fx1s_makewsecb (& varsbuf[0], & val, FX1S_REGISTER_FIELD_M,
                    & rv_numbs, FX1S_VERSION_14MR, addr);
   assert (isr == FX1S_OK);
   /* write Output-register write-code. */
   PurgeComm (sg_ctl.comport, PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR | PURGE_TXABORT);
   io_success_ = WriteFile (sg_ctl.comport, & varsbuf[0],
       rv_numbs, & tm_numbs, NULL);
   assert (io_success_ != FALSE);
   if (wait_inputbuf (1, & varsbuf[0]) == FALSE)
     goto repeat;
   assert (varsbuf[0] == SECTION_LINK_ACK);
   Sleep(10);
   InterlockedExchangePointer (& sg_spinlock, 0);
   return ;
}

static int32_t readPULSE (void) {
 
   struct read_section2 rdsec;
   char varsbuf[128];
   char asc_hi;
   char asc_lo;
   int isr = 0;
   int off_numbs = 0;
   int tm_numbs = 0;
   uint16_t rv_numbs = 0;
   uint32_t iw_size = 0;
   union {
     char bgroup[4];
     int32_t inter;
   } cc_timing;

   BOOL io_success_;
   while (InterlockedExchangePointer (& sg_spinlock, 1) != 0) ;
   /* prepare C-register read_section. */
repeat:
   isr = fx1s_makersecb ( & rdsec, FX1S_REGISTER_FIELD_C32,
           & rv_numbs, FX1S_VERSION_14MR, sg_comset[0].hookC);
   assert (isr == FX1S_OK);
  
   /* write C-register read-code. */
   PurgeComm (sg_ctl.comport, PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR | PURGE_TXABORT);
   io_success_ =  WriteFile (sg_ctl.comport, & rdsec,
       sizeof(struct read_section), & tm_numbs, NULL);
   assert (io_success_ != FALSE);
   if (wait_inputbuf (rv_numbs, & varsbuf[0]) == FALSE)
     goto repeat;
   assert (varsbuf[0] == SECTION_LINK_STX);
   Sleep(1);
   asc_hi = ascii_to_num ( varsbuf[1]) << 4;
   asc_lo = ascii_to_num ( varsbuf[2]) >> 0;
   cc_timing.bgroup[0] = asc_hi | asc_lo;
   asc_hi = ascii_to_num ( varsbuf[3]) << 4;
   asc_lo = ascii_to_num ( varsbuf[4]) >> 0;
   cc_timing.bgroup[1] = asc_hi | asc_lo;
   asc_hi = ascii_to_num ( varsbuf[5]) << 4;
   asc_lo = ascii_to_num ( varsbuf[6]) >> 0;
   cc_timing.bgroup[2] = asc_hi | asc_lo;
   asc_hi = ascii_to_num ( varsbuf[7]) << 4;
   asc_lo = ascii_to_num ( varsbuf[8]) >> 0;
   cc_timing.bgroup[3] = asc_hi | asc_lo;

   InterlockedExchangePointer (& sg_spinlock, 0);
   return cc_timing.inter;
}


int (__stdcall *s_call)  (void *param) = NULL;

static
int __stdcall thp_monitor (DWORD current_ticks) {
 
   struct ctl_delay_chain *pv_delay;
   struct ctl_delay_chain *cu_delay;
   struct ctl_delay_chain *iq_delay; 
   struct plcdef_chunk *poll_ed = & sg_ctl.pd_chunk[sg_ctl.pd_len];
   enum FX1S_VERSION fver;
   char obt;
   int ii = 0;
   int lines = 0;
   int64_t iq_total = 0;
   int64_t iq_curr;
   int64_t eq_std;
   int64_t eq_ms;
   int64_t eq_hms;
   int64_t eq_s;
   int32_t cc_timing = readPULSE ();
   AU buf[64];
  
   printf ("%d\n", current_ticks);

   SetThreadAffinityMask (sg_ctl.timingth, 1);
   QueryPerformanceCounter ((LARGE_INTEGER *)& iq_total);
   QueryPerformanceFrequency ((LARGE_INTEGER *)& eq_s);
   sg_ctl.pd_poll =  & sg_ctl.pd_chunk[0];
   sg_listset.res_line = -1;

   eq_std = eq_s/68;
   eq_ms = eq_s/1000;
   eq_hms= eq_s/10;
   switch (sg_comset[0].version) {
   case 0: fver = FX1S_VERSION_10MR; break;
   case 1: fver = FX1S_VERSION_14MR; break;
   case 2: fver = FX1S_VERSION_20MR; break;
   case 3: fver = FX1S_VERSION_30MR; break;
   }
   do {
     int64_t elsp;
     if (sg_thp_mask & CTL_FORCE_STOP_MASK) {
       sg_thp_mask &= ~CTL_FORCE_STOP_MASK;
       /* release all ctl_delay_chain, close IO **/
       ZeroMemory (& ctl_delay_array[0], sizeof (ctl_delay_array));
       ZeroMemory (& ctlheap_delay_locked[0], sizeof (ctlheap_delay_locked));
  	   setOYB (fver, 10, 0);
	     setOYB (fver,  0, 0);
       sg_ctl.level = NULL;
       /* set caption **/
       SetWindowText (window_main_butfstop, AU_C ("恢复"));
       /* enable **/
       EnableWindow (window_main_butfstop, TRUE);
       EnableWindow (window_main_butstartop, FALSE);
	     /* force close PLC [by M8037]  **/
	     obt = readRELAY_M (8037);
	     writeRELAY_M (8037, obt | 0x20);
	     continue;
     } else if (sg_thp_mask & CTL_FORCE_RESUME_MASK) {
       sg_thp_mask &= ~CTL_FORCE_RESUME_MASK;
       if (sg_listset.res_line != -1) {
         /* in vailed range **/
         struct plcdef_chunk *pivot = & sg_ctl.pd_chunk[sg_listset.res_line - 1];
         lines = sg_listset.res_line - 1;
         sg_ctl.pd_poll = pivot;
         /* reloc pos **/
         writePULSE (sg_listset.res_line == 1 ? 0 : sg_ctl.pd_chunk[sg_listset.res_line - 2].setv);
         /* reset ListView's infos **/
         for (ii = sg_listset.res_line - 1; ii != sg_listset.cur_line; ii++) {
           ListView_SetItemText (window_main_listhook, ii, ISR_COLUMN_C_CURRENT, AU_C (""));
           ListView_SetItemText (window_main_listhook, ii, ISR_COLUMN_OUTPUT_CURRENT_L, AU_C (""));  
           ListView_SetItemText (window_main_listhook, ii, ISR_COLUMN_OUTPUT_CURRENT_R, AU_C (""));
           ListView_SetItemText (window_main_listhook, ii, ISR_COLUMN_COMPLETE_IND, AU_C ("未完成"));
         }
         sg_listset.res_line = -1;
         SetWindowText (window_main_butstartop, AU_C ("运行中"));
       }
       /* force open PLC [by M8035/M8036/M8037]  **/
       obt = readRELAY_M (8037);
       obt &= ~(1 << (8037 & 7));
       writeRELAY_M (8037, obt);
       obt = readRELAY_M (8035);
       obt |= (11 << (8035 & 7));
       writeRELAY_M (8035, obt);
       /* set caption **/
       SetWindowText (window_main_editreset, AU_C (""));
       SetWindowText (window_main_butstartop, AU_C ("运行中"));
       SetWindowText (window_main_butfstop, AU_C ("PLC强停"));
       GetWindowText (window_main_butstartop, & buf[0], sizeof (buf));
       if (aucmp (& buf[0], AU_C ("结束趟处理")) == 0)
         EnableWindow (window_main_butstartop, TRUE);
       /* enable/disable **/
       EnableWindow (window_main_butfstop, TRUE);
       EnableWindow (window_main_editreset, FALSE);
       EnableWindow (window_main_butfstop2, TRUE);
     } else if (sg_thp_mask & (CTL_FORCE_TERM_MASK | CTL_NORMAL_END_MASK)) {
       EnableWindow (window_main_butfstop, FALSE);
       EnableWindow (window_main_editreset, FALSE);
       EnableWindow (window_main_editdelete, TRUE);
       EnableWindow (window_main_editinsert, TRUE);
       EnableWindow (window_main_butfstop2, FALSE);
       EnableWindow (window_main_butfend, FALSE);
       EnableWindow (window_main_butstartop, TRUE);
       EnableWindow (window_main_editattach, TRUE);
       EnableWindow (window_main_listhook, TRUE);
       SetWindowText (window_main_butfstop, AU_C ("PLC强停"));
       SetWindowText (window_main_editinsert, AU_C (""));
       SetWindowText (window_main_editdelete, AU_C (""));
       SetWindowText (window_main_editreset, AU_C (""));
       SetWindowText (window_main_butfstop2, AU_C ("PLC强停2"));
       SetWindowText (window_main_butfend, AU_C ("强制结束"));
       SetWindowText (window_main_butstartop, AU_C ("开始"));
       CloseHandle (sg_ctl.timingth);
       CloseHandle (sg_ctl.comport);
       sg_ctl.timingth = (void *)-1;
       sg_ctl.list_edis = 0;
       sg_ctl.level = NULL;
       ZeroMemory (& ctl_delay_array[0], sizeof (ctl_delay_array));
       sg_thp_mask &= ~(CTL_FORCE_TERM_MASK | CTL_NORMAL_END_MASK);
       append_text (window_main_editstatus, AU_C ("结束当前测试线程\n"));
       return 0;
     }
     QueryPerformanceCounter ((LARGE_INTEGER *)& iq_curr);
     elsp = iq_total + eq_std;
     if (1) {
       /* update listView's infos **/
       iq_total += (iq_curr - iq_total);
       if (sg_ctl.pd_poll != sg_ctl.xe_poll) {
         auprintf (& buf[0], AU_C ("%d"), cc_timing);
         ListView_SetItemText (window_main_listhook, lines, ISR_COLUMN_C_CURRENT, & buf[0]);
         //Sleep (40);
         	   setOYB (fver, 0, 0);
	     setOYB (fver,  0, 0x0F);
       }
       /* deal delay-chain in refresh cycle **/
       cu_delay = sg_ctl.level;
       for (; cu_delay != NULL; cu_delay = cu_delay->level) {
         int64_t elsp = iq_curr - cu_delay->switch0->pivot;
         int32_t elsp2 = (int32_t) (elsp/eq_hms);
         /* set current IO ouput .**/
         auprintf (& buf[0], AU_C ("%d"), elsp2);
         ListView_SetItemText (window_main_listhook, cu_delay->item,
                cu_delay->switch0 == & cu_delay->left ? ISR_COLUMN_OUTPUT_CURRENT_L
                     : ISR_COLUMN_OUTPUT_CURRENT_R, & buf[0]);
       }
     } else {   
       /* deal delay-chain in usual cycle**/
       if (sg_ctl.level != NULL) {
         pv_delay = (void *)& sg_ctl.level; /** XXX:offset ZERO **/
         cu_delay = sg_ctl.level;
         for (; cu_delay != NULL; cu_delay = iq_delay) {
           /* poll timing-list, test, close output **/
           int64_t elsp = iq_curr - cu_delay->switch0->pivot;
           if (elsp >= cu_delay->switch0->offset2) {
             /* cancel IO ouput .**/
             if (cu_delay->switch0 == & cu_delay->left)
               auprintf (& buf[0], AU_C ("冲压完成"));
             else
               auprintf (& buf[0], AU_C ("动作完成"));

             ListView_SetItemText (window_main_listhook, cu_delay->item, ISR_COLUMN_COMPLETE_IND, & buf[0]);
             auprintf (& buf[0], AU_C ("%d"), cu_delay->switch0->offset);
             ListView_SetItemText (window_main_listhook, cu_delay->item, cu_delay->switch0 == & cu_delay->left
                     ? ISR_COLUMN_OUTPUT_CURRENT_L
                     : ISR_COLUMN_OUTPUT_CURRENT_R,
                     & buf[0]);

 
             if (cu_delay->switch0 == & cu_delay->left) {
               cu_delay->switch0 = & cu_delay->right;
               cu_delay->right.pivot = iq_curr;
               /* set output signal[right] **/
               obt = readOYB (fver, cu_delay->right.out_ey);
               obt |= (1 << (cu_delay->right.out_y & 7));
               setOYB (fver, cu_delay->right.out_ey, obt);
               /* cancel output signal[left] **/
               obt = readOYB (fver, cu_delay->left.out_ey);
               obt &= ~(1 << (cu_delay->left.out_y & 7));
               setOYB (fver, cu_delay->left.out_ey, obt); 
               /* deal delay chain **/
               iq_delay = cu_delay->level;
               pv_delay = cu_delay;
             } else if (cu_delay->switch0 == & cu_delay->right) {
               /* cancel output signal[right] **/
               obt = readOYB (fver, cu_delay->right.out_ey);
               obt &= ~(1 << (cu_delay->right.out_y & 7));
               setOYB (fver, cu_delay->right.out_ey, obt); 
               /* deal delay chain **/
               iq_delay = cu_delay->level;
               pv_delay->level = cu_delay->level;
               ctl_delay_free (cu_delay);
               if (sg_ctl.level == NULL && sg_ctl.pd_poll == sg_ctl.xe_poll) {
                 append_text (window_main_editstatus, AU_C ("当前工控作业完成\n"));
                 SetWindowText (window_main_butstartop, AU_C ("结束趟处理"));
                 EnableWindow (window_main_butstartop, TRUE);
               }
             }
           } else {
             iq_delay = cu_delay->level;
             pv_delay = cu_delay;
           }
         }
       }

       if (sg_ctl.pd_poll == sg_ctl.xe_poll)
         continue;
       else if (sg_ctl.pd_poll->setv <= cc_timing) { /* current pulse-count >= setting ?? */
         /* Trigger work **/
         auprintf (& buf[0], AU_C ("%d"), sg_ctl.pd_poll->setv);
         ListView_SetItemText (window_main_listhook, lines, ISR_COLUMN_C_CURRENT, & buf[0]);
         /* insert timing-list **/
         iq_delay = ctl_delay_malloc ();
         iq_delay->left.out_ey = sg_ctl.pd_poll->writeIO2e[0];
         iq_delay->right.out_ey = sg_ctl.pd_poll->writeIO2e[1];
         iq_delay->left.out_y = sg_ctl.pd_poll->writeIO[0];
         iq_delay->right.out_y = sg_ctl.pd_poll->writeIO[1];
         iq_delay->item = lines;
         iq_delay->left.pivot = iq_curr;
         iq_delay->left.offset = sg_ctl.pd_poll->delay[0];
         iq_delay->right.offset = sg_ctl.pd_poll->delay[1];
         iq_delay->switch0 = & iq_delay->left;
         iq_delay->left.offset2 = sg_ctl.pd_poll->delay[0] * eq_hms;
         iq_delay->right.offset2 = sg_ctl.pd_poll->delay[1] * eq_hms;
         iq_delay->level = sg_ctl.level;
         sg_ctl.level = iq_delay;
         /* set output signal[left] **/
         obt = readOYB (fver, iq_delay->left.out_ey);
         obt |= (1 << (iq_delay->left.out_y & 7));
         setOYB (fver, iq_delay->left.out_ey, obt);
         /* pointer to next, check **/
         sg_ctl.pd_poll = & sg_ctl.pd_poll[1];
         if (sg_ctl.pd_poll != sg_ctl.xe_poll)
           lines++;
       }
     }
   } while (0);

  return 0;
}

int CALLBACK hook_meditidline (HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
#if 1
  UCHAR key = (UCHAR)LOWORD(wparam);

  if (message == WM_CHAR) {
    /* allow ch 0~9 and BACKSPACE and ENTER key **/
    if ((! (key >= '0' && key <= '9') && key != VK_BACK && key != VK_RETURN))
      return 0;
    if (key == VK_RETURN) {
      int input = 0;
      int ii = 0;
      AU chbuf2[32];
      AU *chbuf = NULL;
      LVITEM LVI;
      GetWindowText (window, & _g_chbuf[0], sizeof (_g_chbuf));

      chbuf = bzero_filter (& _g_chbuf[0], aulen (& _g_chbuf[0]));
      if (_g_chbuf[0] == 0 || chbuf == NULL)
        return 0;
      input = autoi (& chbuf[0]);

      if (input == 0) {
        append_text (window_main_editstatus, AU_C ("输入0并没有X用\n"));
        return 0;
      }

      if (window == window_main_editinsert) {
        if (input > sg_listset.cur_line) {
          append_text (window_main_editstatus, AU_C ("插入行数大于当前行数\n"));
          return 0;
        }
        auprintf (chbuf2, AU_C ("%d"), input+1);
        /* insert phase **/
        LVI.mask = LVIF_TEXT;
        LVI.pszText = chbuf2; 
        LVI.iItem = input;
        LVI.iSubItem = 0;

        sg_listset.cur_line++;
        ListView_InsertItem (window_main_listhook, & LVI);
        ListView_SetItemText (window_main_listhook, input, ISR_COLUMN_COMPLETE_IND, AU_C ("未完成"));
        /* adjust line text **/

        for (ii = input; ii != sg_listset.cur_line; ii++) {
          auprintf (chbuf2, AU_C ("%d"), ii+1);
          ListView_SetItemText (window_main_listhook, ii, 0, chbuf2);
        }
        return 0;
      } else if (window == window_main_editdelete) {
        if (input > sg_listset.cur_line) {
          append_text (window_main_editstatus, AU_C ("删除行数大于当前行数\n"));
          return 0;
        } if (sg_listset.cur_line == 1) {
          append_text (window_main_editstatus, AU_C ("没有可供删除的item\n"));
          return 0;
        }

        ListView_DeleteItem (window_main_listhook, input - 1);
        /* adjust line text **/

        for (ii = input - 1; ii != sg_listset.cur_line; ii++) {
          auprintf (chbuf2, AU_C ("%d"), ii + 1);
          ListView_SetItemText (window_main_listhook, ii, 0, chbuf2);
        }
        sg_listset.cur_line--;
        return 0;
      }
    }
  }

#endif
  return
  CallWindowProc (window == window_main_editinsert
             ? callback_hook_editinsert
             : callback_hook_editdelete, window, message, wparam, lparam);
}

int CALLBACK hook_meditreset (HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
#if 1
  UCHAR key = (UCHAR)LOWORD(wparam);

  if (message == WM_CHAR) {
    /* allow ch 0~9 and BACKSPACE and ENTER key **/
    if ((! (key >= '0' && key <= '9') && key != VK_BACK && key != VK_RETURN))
      return 0;
    if (key == VK_RETURN) {
      int input = 0;
      int ii = 0;
      AU *chbuf = NULL;
      LVITEM LVI;
      GetWindowText (window, & _g_chbuf[0], sizeof (_g_chbuf));

      chbuf = bzero_filter (& _g_chbuf[0], aulen (& _g_chbuf[0]));
      if (_g_chbuf[0] == 0 || chbuf == NULL)
        return 0;
      input = autoi (& chbuf[0]);

      if (input == 0) {
        append_text (window_main_editstatus, AU_C ("输入0并没有X用\n"));
        return 0;
      } else if (input > sg_listset.cur_line) {
        append_text (window_main_editstatus, AU_C ("范围超过了\n"));
        return 0;
      }
      // set resline.
      sg_listset.res_line = input;

      append_text (window_main_editstatus, AU_C ("重设成功\n"));
      return 0;
    }
  }

#endif
  return
  CallWindowProc (window == window_main_editinsert
             ? callback_hook_editinsert
             : callback_hook_editdelete, window, message, wparam, lparam);
}

int CALLBACK hook_meditattach (HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
#if 1
  UCHAR key = (UCHAR)LOWORD(wparam);
  AU *chbuf = NULL;

  if (message == WM_KILLFOCUS) {
    GetWindowText (window, & _g_chbuf[0], sizeof (_g_chbuf));
    chbuf = bzero_filter (& _g_chbuf[0], aulen (& _g_chbuf[0]));
    ListView_SetItemText (window_main_listhook, sg_listset.cur_item, sg_listset.cur_sub, & chbuf[0]);
    SetWindowPos (window, 0, 0, 0, 1, 1, SWP_HIDEWINDOW);
  }
  if (message == WM_CHAR && (! (key >= '0' && key <= '9') && key != VK_BACK))
    return 0;
#endif
  return CallWindowProc (callback_hook_editattach, window, message, wparam, lparam);
}


int CALLBACK hook_seditHookC (HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
#if 1
  UCHAR key = (UCHAR)LOWORD(wparam);
  AU *chbuf = NULL;

  if (message == WM_CHAR && (! (key >= '0' && key <= '9') && key != VK_BACK))
    return 0;
#endif
  return CallWindowProc (callback_hook_edithookC, window, message, wparam, lparam);
}

INT_PTR CALLBACK list_view_proc (HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
  return CallWindowProc (callback_hook_listhook, window, message, wparam, lparam);
}

LRESULT CALLBACK hook_maindialog (HWND window, UINT message, WPARAM wparam, LPARAM lparam) {

  switch (message)
  {
  case WM_NOTIFY:

    /*
     * In this message, we mainly deal with two issues,
     * #1 double click the ListView item to display the hidden Edit controls and put the corresponding location
     * #2 cancel the default selection effect
     */
    {
      union {
        void *comp; /* Compatible pointer **/
        LPARAM comp2;
        NMHDR *base;
        NMLISTVIEW *list;
        NMLVCUSTOMDRAW *draw;
      } nmset;

      nmset.comp2 = lparam;

      if (nmset.base->idFrom == IDCM_LIST_MONITOR && nmset.base->hwndFrom == window_main_listhook) {
        if (nmset.base->code == NM_DBLCLK) {
          /*
           * double click
           */
          RECT nsrt;
          sg_listset.cur_item = nmset.list->iItem;
          sg_listset.cur_sub = nmset.list->iSubItem;

          printf("item:%d sub:%d\n", nmset.list->iItem, nmset.list->iSubItem);
          if (nmset.list->iItem != -1 && nmset.list->iSubItem != ISR_COLUMN_LINE
                               && sg_ctl.list_edis == FALSE
                               && nmset.list->iSubItem != ISR_COLUMN_C_CURRENT
                               && nmset.list->iSubItem != ISR_COLUMN_OUTPUT_CURRENT_L
                               && nmset.list->iSubItem != ISR_COLUMN_OUTPUT_CURRENT_R
                               && nmset.list->iSubItem != ISR_COLUMN_COMPLETE_IND) {

            ListView_GetSubItemRect (nmset.list->hdr.hwndFrom, nmset.list->iItem, nmset.list->iSubItem,
              LVIR_LABEL, & nsrt);
            ListView_GetItemText (nmset.list->hdr.hwndFrom, nmset.list->iItem, nmset.list->iSubItem
               , _g_chbuf, sizeof(_g_chbuf));
            SetWindowText (window_main_editattach, _g_chbuf);
            SendMessage (window_main_editattach, EM_SETSEL, 0, -1);
            SetWindowPos (window_main_editattach, HWND_BOTTOM, nsrt.left + 1, nsrt.top + 0
                , nsrt.right - nsrt.left - 1, nsrt.bottom - nsrt.top - 1, SWP_DRAWFRAME | SWP_SHOWWINDOW);
            SetFocus (window_main_editattach);
            return 0;
          }
        } else if (nmset.base->code == NM_CUSTOMDRAW) {
            /*
             * paint phase
             */
            if (nmset.draw->nmcd.dwDrawStage == CDDS_PREPAINT)
              return CDRF_NOTIFYITEMDRAW;
            else if (nmset.draw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
              ListView_SetItemState (nmset.draw->nmcd.hdr.hwndFrom, -1, 0, LVIS_DROPHILITED | LVIS_SELECTED);
            return CDRF_DODEFAULT;
        }
      }
    }
  default:
    break;
  }
  return CallWindowProc (callback_hook_main_dialog, window, message, wparam, lparam);
}

int write_plcdef (AU *file_name) {

  int ii = 0;
  FILE *fp = fopen (file_name, "w+");
  struct plcdef_header phead_;
  assert (fp != NULL);
  phead_.magic[0] = 0x38;
  phead_.magic[1] = 0x25;
  phead_.magic[2] = 0x74;
  phead_.magic[3] = 0x98;
  phead_.comIdx = 0xFF;
  phead_.buad = sg_comset[0].buad_rate;
  phead_.line = sg_listset.cur_line;
  phead_.parity = sg_comset[0].parity;
  phead_.pulse_addr = sg_comset[0].hookC;
  phead_.stopbit = sg_comset[0].stop_bits;
  phead_.version = sg_comset[0].version;
  phead_.comIdx = sg_comset[0].comm_idx;
  fwrite (& phead_, sizeof (phead_), 1, fp);

  /* write format chbuf **/
  for (; ii != sg_listset.cur_line; ii++) {

    struct plcdef_chunk p_chunk;

    AU settings_chbuf[32];
    AU outputL_chbuf[32]; 
    AU outdelayL_chbuf[32]; 
    AU outputR_chbuf[32]; 
    AU outdelayR_chbuf[32]; 

    ListView_GetItemText (window_main_listhook, ii, ISR_COLUMN_C_SETTINGS, settings_chbuf, sizeof (settings_chbuf));
    ListView_GetItemText (window_main_listhook, ii, ISR_COLUMN_OUTPUT_COIL_L, outputL_chbuf, sizeof (outputL_chbuf));
    ListView_GetItemText (window_main_listhook, ii, ISR_COLUMN_OUTPUT_DELAY_L, outdelayL_chbuf, sizeof (outdelayL_chbuf));
    ListView_GetItemText (window_main_listhook, ii, ISR_COLUMN_OUTPUT_COIL_R, outputR_chbuf, sizeof (outputR_chbuf));
    ListView_GetItemText (window_main_listhook, ii, ISR_COLUMN_OUTPUT_DELAY_R, outdelayR_chbuf, sizeof (outdelayR_chbuf));

    p_chunk.setv = autoi (settings_chbuf);
    p_chunk.writeIO2e[0] = autoi (outputL_chbuf);
    p_chunk.writeIO[0] = vailed8 (p_chunk.writeIO2e[0]);
    p_chunk.delay[0] = autoi (outdelayL_chbuf);
    p_chunk.writeIO2e[1] = autoi (outputR_chbuf);
    p_chunk.writeIO[1] = vailed8 (p_chunk.writeIO2e[1]);
    p_chunk.delay[1] = autoi (outdelayR_chbuf);

    fwrite (& p_chunk, sizeof (p_chunk), 1, fp);
  }
  fclose (fp);
  return 0;
};

int read_plcdef (AU *file_name) {

  int ii = 0;
  FILE *fp = fopen (file_name, "r");
  struct plcdef_header phead_;
  assert (fp != NULL);
  fread (& phead_, sizeof (phead_), 1, fp);
  // assert (ii == sizeof (phead_));

  if ( phead_.magic[0] != 0x38 || phead_.magic[1] != 0x25
  || phead_.magic[2] != 0x74
  || phead_.magic[3] != 0xFFFFFF98)
  {
    append_text (window_main_editstatus, AU_C ("不是有效的plcdef文件, 不要轻易尝试自定义plcdef文件\n"));
    return 1;
  }

  sg_comset[0].buad_rate = phead_.buad;
  sg_listset.cur_line = phead_.line;
  sg_comset[0].parity = phead_.parity;
  sg_comset[0].hookC = phead_.pulse_addr;
  sg_comset[0].stop_bits = phead_.stopbit;
  sg_comset[0].version = phead_.version;
  sg_comset[0].comm_idx = phead_.comIdx;

  ListView_DeleteAllItems (window_main_listhook);

  /* write format chbuf **/
  for (; ii != sg_listset.cur_line; ii++) {

    struct plcdef_chunk p_chunk;
    LVITEM LVI;

    AU buf [20];
    AU settings_chbuf[32];
    AU outputL_chbuf[32]; 
    AU outdelayL_chbuf[32]; 
    AU outputR_chbuf[32]; 
    AU outdelayR_chbuf[32];

    fread (& p_chunk, sizeof (p_chunk), 1, fp);
    auprintf ( & buf[0], AU_C ("%d"), ii + 1);

    LVI.mask = LVIF_TEXT;
    LVI.pszText = & buf[0]; 
    LVI.iItem = ii;
    LVI.iSubItem = 0;
    ListView_InsertItem (window_main_listhook, & LVI);

    auprintf ( & settings_chbuf[0], AU_C ("%d"), p_chunk.setv);
    auprintf ( & outputL_chbuf[0], AU_C ("%d"), p_chunk.writeIO2e[0]);
    auprintf ( & outdelayL_chbuf[0], AU_C ("%d"), p_chunk.delay[0]);
    auprintf ( & outputR_chbuf[0], AU_C ("%d"), p_chunk.writeIO2e[1]);
    auprintf ( & outdelayR_chbuf[0], AU_C ("%d"), p_chunk.delay[1]);

    ListView_SetItemText (window_main_listhook, ii, ISR_COLUMN_C_SETTINGS, settings_chbuf);
    ListView_SetItemText (window_main_listhook, ii, ISR_COLUMN_OUTPUT_COIL_L, outputL_chbuf);
    ListView_SetItemText (window_main_listhook, ii, ISR_COLUMN_OUTPUT_DELAY_L, outdelayL_chbuf);
    ListView_SetItemText (window_main_listhook, ii, ISR_COLUMN_OUTPUT_COIL_R, outputR_chbuf);
    ListView_SetItemText (window_main_listhook, ii, ISR_COLUMN_OUTPUT_DELAY_R, outdelayR_chbuf);
    ListView_SetItemText (window_main_listhook, ii, ISR_COLUMN_COMPLETE_IND, AU_C ("未完成"));
  }
  fclose (fp);
  return 0;
};

INT_PTR CALLBACK dialog_set_proc (HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
  int ii;
  AU buf_0[32];

  switch (message)
  {
  case WM_INITDIALOG:
    window_scmb_commidx = GetDlgItem (window, IDCS_COMBO_COMMIDX);
    window_scmb_parity = GetDlgItem (window, IDCS_COMBO_COMMPARITY);
    window_scmb_stopbit = GetDlgItem (window, IDCS_COMBO_COMMSTOP);
    window_scmb_buad = GetDlgItem (window, IDCS_COMBO_COMMBUAD);
    window_scmb_plcver = GetDlgItem (window, IDCS_COMBO_PLCVER);
    window_scmb_hookC = GetDlgItem (window, IDCS_EDIT_LOOKC);
    callback_hook_edithookC = (void *)SetWindowLong (window_scmb_hookC, GWL_WNDPROC, (LONG)hook_seditHookC);  

   if (sg_comset[0].comm_idx != 0xFF)
      {
        auprintf (& buf_0[0], AU_C ("COM%d"), sg_comset[0].comm_idx);
        ii = ComboBox_AddItemData (window_scmb_commidx, & buf_0[0]);
        ComboBox_SetCurSel (window_scmb_commidx, ii);
      }
   
    for (ii = 0; ii != BUAD_TAB_LST_IDX; ii++)
      sg_comseti.buad[ii] = ComboBox_AddItemData (window_scmb_buad, buad_tab[ii]);
    for (ii = 0; ii != PARITY_TAB_LST_IDX; ii++)
      sg_comseti.parity[ii] = ComboBox_AddItemData (window_scmb_parity, parity_tab[ii]);
    for (ii = 0; ii != STOPBIT_TAB_LST_IDX; ii++)
      sg_comseti.stopbit[ii] = ComboBox_AddItemData (window_scmb_stopbit, stopbit_tab[ii]);
    for (ii = 0; ii != VERSION_TAB_LST_IDX; ii++)
      sg_comseti.version[ii] = ComboBox_AddItemData (window_scmb_plcver, version_tab[ii]);

    switch (sg_comset[0].buad_rate) {
    case 9600: ComboBox_SetCurSel (window_scmb_buad, BUAD_TAB_9600BPS); break;
    case 19200: ComboBox_SetCurSel (window_scmb_buad, BUAD_TAB_19200BPS); break;
    case 38400:  ComboBox_SetCurSel (window_scmb_buad, BUAD_TAB_38400BPS); break;
    case 57600: ComboBox_SetCurSel (window_scmb_buad, BUAD_TAB_57600BPS); break;
    case 115200:  ComboBox_SetCurSel (window_scmb_buad, BUAD_TAB_115200BPS); break;
    default: assert (0);
    }
    switch (sg_comset[0].parity) {
    case NOPARITY: ComboBox_SetCurSel (window_scmb_parity, PARITY_TAB_NONE); break;
    case EVENPARITY: ComboBox_SetCurSel (window_scmb_parity, PARITY_TAB_EVEN); break;
    case ODDPARITY:  ComboBox_SetCurSel (window_scmb_parity, PARITY_TAB_ODD); break;
    default: assert (0);
    }
    switch (sg_comset[0].stop_bits) {
    case ONESTOPBIT: ComboBox_SetCurSel (window_scmb_stopbit, STOPBIT_TAB_ONE); break;
    case ONE5STOPBITS: ComboBox_SetCurSel (window_scmb_stopbit, STOPBIT_TAB_ONEH); break;
    case TWOSTOPBITS:  ComboBox_SetCurSel (window_scmb_stopbit, STOPBIT_TAB_TWO); break;
    default: assert (0);
    }
    switch (sg_comset[0].version) {
    case 0: ComboBox_SetCurSel (window_scmb_plcver, VERSION_TAB_10MR); break;
    case 1: ComboBox_SetCurSel (window_scmb_plcver, VERSION_TAB_14MR); break;
    case 2: ComboBox_SetCurSel (window_scmb_plcver, VERSION_TAB_20MR); break;
    case 3: ComboBox_SetCurSel (window_scmb_plcver, VERSION_TAB_30MR); break;
    default: assert (0);
    }
    auprintf (& buf_0[0], AU_C ("%d"), sg_comset[0].hookC);
    SetWindowText (window_scmb_hookC, & buf_0[0]);
    return (INT_PTR)TRUE;

  case WM_COMMAND:
    if (LOWORD(wparam) == IDCANCEL || LOWORD(wparam) == IDCS_BUTTON_COMMCANCEL)
    {
      EndDialog(window, LOWORD(wparam));
      return (INT_PTR)TRUE;
    }
    if (LOWORD(wparam) == IDCS_BUTTON_COMMOK)
    {
      int ii;
      AU comm_buf[8];
      AU stop_buf[16];
      AU buad_buf[16];
      AU pver_buf[16];
      AU hookC_buf[16];
      AU parity_buf[16];
      GetWindowText (window_scmb_commidx, & comm_buf[0], sizeof (comm_buf));
      GetWindowText (window_scmb_parity, & parity_buf[0], sizeof (parity_buf));
      GetWindowText (window_scmb_stopbit, & stop_buf[0], sizeof (stop_buf));
      GetWindowText (window_scmb_buad, & buad_buf[0], sizeof (buad_buf));
      GetWindowText (window_scmb_plcver, & pver_buf[0], sizeof (pver_buf));
      GetWindowText (window_scmb_hookC, & hookC_buf[0], sizeof (hookC_buf));

      if (parity_buf[0] == 0 || hookC_buf[0] == 0
         || buad_buf[0] == 0 || pver_buf[0] == 0 || stop_buf[0] == 0)
         return (INT_PTR)TRUE;

      for (ii = 0; ii != PARITY_TAB_LST_IDX; ii++) {
        if (aucmp (parity_tab[ii], & parity_buf[0]) == 0) {
          switch (ii) {
          case PARITY_TAB_NONE: sg_comset[0].parity = NOPARITY; break;
          case PARITY_TAB_EVEN: sg_comset[0].parity = EVENPARITY; break;
          case PARITY_TAB_ODD: sg_comset[0].parity = ODDPARITY; break;
          default: assert (0);
          }
          break;
        }
      }
      for (ii = 0; ii != STOPBIT_TAB_LST_IDX; ii++) {
        if (aucmp (stopbit_tab[ii], & stop_buf[0]) == 0) {
          switch (ii) {
          case STOPBIT_TAB_ONE: sg_comset[0].stop_bits = ONESTOPBIT; break;
          case STOPBIT_TAB_ONEH: sg_comset[0].stop_bits = ONE5STOPBITS; break;
          case STOPBIT_TAB_TWO:  sg_comset[0].stop_bits = TWOSTOPBITS; break;
          default: assert (0);
          }
          break;
        }
      }
      for (ii = 0; ii != BUAD_TAB_LST_IDX; ii++) {
        if (aucmp (buad_tab[ii], & buad_buf[0]) == 0) {
          switch (ii) {
          case BUAD_TAB_9600BPS: sg_comset[0].buad_rate = 9600; break;
          case BUAD_TAB_19200BPS: sg_comset[0].buad_rate = 19200; break;
          case BUAD_TAB_38400BPS:  sg_comset[0].buad_rate = 38400; break;
          case BUAD_TAB_57600BPS: sg_comset[0].buad_rate = 57600; break;
          case BUAD_TAB_115200BPS:  sg_comset[0].buad_rate = 115200; break;
          default: assert (0);
          }
          break;
        }
      }
      for (ii = 0; ii != VERSION_TAB_LST_IDX; ii++) {
        if (aucmp (version_tab[ii], & pver_buf[0]) == 0) {
          switch (ii) {
          case VERSION_TAB_10MR: sg_comset[0].version = 0; break;
          case VERSION_TAB_14MR: sg_comset[0].version = 1; break;
          case VERSION_TAB_20MR: sg_comset[0].version = 2; break;
          case VERSION_TAB_30MR: sg_comset[0].version = 3; break;
          default: assert (0);
          }
          break;
        }
      }
      sg_comset[0].hookC = autoi (& hookC_buf[0]);
      /* comm **/
      if (comm_buf[0] != 0) {
        sg_comset[0].comm_idx = autoi (& comm_buf[3]);
      }
      EndDialog(window, LOWORD(wparam));
      return (INT_PTR)TRUE;
    }
    if (LOWORD(wparam) == IDCS_COMBO_COMMIDX)
    {
      if (HIWORD(wparam) == CBN_DROPDOWN)
      {
        struct secominfo_sec *sec;
        struct comidx_list *pchain = sg_comseti.clist;
        struct secominfo *scpoll;
        AU tchbuf[512];
        void *temp;

        alloc_secominfos (& sec, NULL);
        ComboBox_ResetContent (window_scmb_commidx);
       
        if (sec == NULL)
          return (INT_PTR)TRUE;
        if (sec->num == 0) {
          dealloc_secominfos (sec);
          return (INT_PTR)TRUE;
        }
         
        for (; pchain; free (pchain), pchain = temp)
          temp = pchain->level;
          
        sg_comseti.comm_num = sec->num;
        sg_comseti.clist = NULL;
        scpoll = sec->level;

        for (ii = 0; ii != sec->num; ii++) {
          struct comidx_list *pcom = malloc (sizeof(struct comidx_list));

          assert (pcom != NULL);
          pcom->level = NULL;
          pcom->comidx = scpoll->comm_pos;
          auprintf (& tchbuf[0], AU_C ("COM%d"), pcom->comidx);
          pcom->combidx = ComboBox_AddItemData (window_scmb_commidx, & tchbuf[0]);
          if (scpoll->comm_pos == sg_comset[0].comm_idx)
            ComboBox_SetCurSel (window_scmb_commidx, pcom->combidx);
          scpoll = scpoll->level;
          pcom->level = sg_comseti.clist;
          sg_comseti.clist = pcom;
        }

        dealloc_secominfos (sec);
        return (INT_PTR)TRUE;
      }
  default:
    break;
  }
  }
  return (INT_PTR)FALSE;
}

// 0:success, 1:fail
int list_check (void) {

  int ii = 0;

  /* phase 1:collect comm infos.**/
  append_text (window_main_editstatus, AU_C ("行列以及基本检查\n"));

  if (sg_comset[0].hookC < 200) {
    append_text (window_main_editstatus, AU_C ("错误:C脉冲计数器正确范围[200~255]\n"));
    return 1;
  }
      
  /* phase 2:precheck, listview's line infos.**/
  for (; ii != sg_listset.cur_line; ii++) {
    int settingsP;
    int settingsC;
    int outputL; 
    int outdelayL; 
    int outputR; 
    int outdelayR;
    AU settingsP_chbuf[32];
    AU settingsC_chbuf[32];
    AU outputL_chbuf[32]; 
    AU outdelayL_chbuf[32]; 
    AU outputR_chbuf[32]; 
    AU outdelayR_chbuf[32]; 
    int16_t yout0;
    int16_t yout1;

    ListView_GetItemText (window_main_listhook, ii ? ii - 1 : 0, ISR_COLUMN_C_SETTINGS, settingsP_chbuf, sizeof (settingsP_chbuf));
    ListView_GetItemText (window_main_listhook, ii, ISR_COLUMN_C_SETTINGS, settingsC_chbuf, sizeof (settingsC_chbuf));
    ListView_GetItemText (window_main_listhook, ii, ISR_COLUMN_OUTPUT_COIL_L, outputL_chbuf, sizeof (outputL_chbuf));
    ListView_GetItemText (window_main_listhook, ii, ISR_COLUMN_OUTPUT_DELAY_L, outdelayL_chbuf, sizeof (outdelayL_chbuf));
    ListView_GetItemText (window_main_listhook, ii, ISR_COLUMN_OUTPUT_COIL_R, outputR_chbuf, sizeof (outputR_chbuf));
    ListView_GetItemText (window_main_listhook, ii, ISR_COLUMN_OUTPUT_DELAY_R, outdelayR_chbuf, sizeof (outdelayR_chbuf));
        
    if (settingsC_chbuf[0] == 0)
      append_text2 (window_main_editstatus, AU_C ("错误:请填脉冲C计数器设置值 具体行数:%d\n"), ii+1);
    else if (outputL_chbuf[0] == 0)
      append_text2 (window_main_editstatus, AU_C ("错误:请填冲压输出地址 具体行数:%d\n"), ii+1);
    else if (outdelayL_chbuf[0] == 0)
      append_text2 (window_main_editstatus, AU_C ("错误:请填冲压延迟计数(单位1为100ms) 具体行数:%d\n"), ii+1);
    else if (outputR_chbuf[0] == 0)
      append_text2 (window_main_editstatus, AU_C ("错误:请填回程输出地址 具体行数:%d\n"), ii+1);
    else if (outdelayR_chbuf[0] == 0)
      append_text2 (window_main_editstatus, AU_C ("错误:请填回程延迟计数(单位1为100ms) 具体行数:%d\n"), ii+1);
    else
      {
        settingsP = autoi (settingsP_chbuf);
        settingsC = autoi (settingsC_chbuf);
        outputL = autoi (outputL_chbuf);
        outdelayL = autoi (outdelayL_chbuf);
        outputR = autoi (outputR_chbuf);
        outdelayR = autoi (outdelayR_chbuf);

        if (ii != 0 && settingsC <= settingsP)
          append_text2 (window_main_editstatus, AU_C ("错误:当前行设置值小于之前, 具体行数:%d\n"), ii+1);
        else if (vailed8 (outputL) == 0xFFFF)
          append_text2 (window_main_editstatus, AU_C ("错误:冲压非法八进制数[Y线圈], 具体行数:%d\n"), ii+1);
        else if (vailed8 (outputR) == 0xFFFF)
          append_text2 (window_main_editstatus, AU_C ("错误:回程非法八进制数[Y线圈], 具体行数:%d\n"), ii+1);
        else
          {
            yout0 = vailed8 (outputL);
            yout1 = vailed8 (outputR);

            switch (sg_comset[0].version) {
            case 0: /* range:0~3*/
              if (yout0 > 3 || yout1 > 3)  goto ovg; break;
            case 1: /* range:0~5*/
              if (yout0 > 5 || yout1 > 5)  goto ovg; break;
            case 2: /* range:0~7*/
              if (yout0 > 7 || yout1 > 7)  goto ovg; break;
            case 3: /* range:0~13*/
              if (yout0 > 13 || yout1 > 13) goto ovg; break;
            default:
              append_text (window_main_editstatus, AU_C ("错误:未知PLC版本\n"));
              return 1;
            ovg:
              append_text2 (window_main_editstatus, AU_C ("错误:冲压或回程输出范围错误, 具体行数:%d\n"), ii+1);
              return 1;
            }
            continue;
          }
        return 1;
      }
    return 1;
  }
  append_text (window_main_editstatus, AU_C ("成功:行检查无语发错误\n"));
  return 0;
}

INT_PTR CALLBACK dialog_proc (HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
  LV_COLUMN LVC;
  LVITEM LVI;

  switch (message)
  {
  case WM_INITDIALOG:
    window_main_listhook = GetDlgItem (window, IDCM_LIST_MONITOR);
    window_main_editattach = GetDlgItem (window, IDCM_EDIT_ATTMON);
    window_main_editdelete = GetDlgItem (window, IDCM_EDIT_DELETENC);
    window_main_editinsert = GetDlgItem (window, IDCM_EDIT_INSERTNC);
    window_main_editreset = GetDlgItem (window, IDCM_EDIT_RESETDC);
    window_main_editstatus = GetDlgItem (window, IDCM_EDIT_STATUSOUT);
    window_main_butstartop = GetDlgItem (window, IDCM_BUTTON_START);
    window_main_butfstop = GetDlgItem (window, IDCM_BUTTON_FSTOP);
    window_main_butfstop2 = GetDlgItem (window, IDCM_BUTTON_FSTOP2);
    window_main_butfend = GetDlgItem (window, IDCM_BUTTON_FEND);
    window_main_dialog = window;
    SetParent (window_main_editattach, window_main_listhook);
    EnableWindow (window_main_butfstop, FALSE);
    EnableWindow (window_main_butfstop2, FALSE);
    EnableWindow (window_main_butfend, FALSE);
    EnableWindow (window_main_editreset, FALSE);
    timing_init60 ();
    // SetBkColor (window_main_dialog, 0x00FFFFFF);

    ListView_SetExtendedListViewStyle (window_main_listhook, ListView_GetExtendedListViewStyle (window_main_listhook)
                  | LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES); 
    callback_hook_editattach = (void *)SetWindowLong (window_main_editattach, GWL_WNDPROC, (LONG)hook_meditattach);   
    callback_hook_main_dialog = (void *)SetWindowLong (window_main_dialog, GWL_WNDPROC, (LONG)hook_maindialog);
    callback_hook_editinsert = (void *)SetWindowLong (window_main_editinsert, GWL_WNDPROC, (LONG)hook_meditidline);
    callback_hook_editdelete = (void *)SetWindowLong (window_main_editdelete, GWL_WNDPROC, (LONG)hook_meditidline);
    callback_hook_editreset = (void *)SetWindowLong (window_main_editreset, GWL_WNDPROC, (LONG)hook_meditreset);

    LVC.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT; 
    LVC.pszText = AU_C ("行号");        // 0 - only read
    LVC.fmt = LVCFMT_CENTER;
    LVC.cx = 45;
    LVC.iSubItem = ISR_COLUMN_LINE; 
    ListView_InsertColumn (window_main_listhook, ISR_COLUMN_LINE, & LVC); 
    LVC.pszText = AU_C ("设定脉冲");    // 1 - read/write
    LVC.cx = 60;
    LVC.iSubItem = ISR_COLUMN_C_SETTINGS;
    ListView_InsertColumn (window_main_listhook, ISR_COLUMN_C_SETTINGS, & LVC);
    LVC.pszText = AU_C ("当前脉冲");    // 2 - only read
    LVC.iSubItem = ISR_COLUMN_C_CURRENT;
    ListView_InsertColumn (window_main_listhook, ISR_COLUMN_C_CURRENT, & LVC);
    LVC.pszText = AU_C ("冲压输出");    // 3 - read/write
    LVC.iSubItem = ISR_COLUMN_OUTPUT_COIL_L;
    ListView_InsertColumn (window_main_listhook, ISR_COLUMN_OUTPUT_COIL_L, & LVC); 
    LVC.pszText = AU_C ("冲压当前");    // 4 - only read
    LVC.iSubItem = ISR_COLUMN_OUTPUT_CURRENT_L;
    ListView_InsertColumn (window_main_listhook, ISR_COLUMN_OUTPUT_CURRENT_L, & LVC); 
    LVC.pszText = AU_C ("冲压延迟");    // 5 - read/write
    LVC.iSubItem = ISR_COLUMN_OUTPUT_DELAY_L;
    ListView_InsertColumn (window_main_listhook, ISR_COLUMN_OUTPUT_DELAY_L, & LVC);
    LVC.pszText = AU_C ("回程输出");    // 6 - read/write
    LVC.iSubItem = ISR_COLUMN_OUTPUT_COIL_R;
    ListView_InsertColumn (window_main_listhook, ISR_COLUMN_OUTPUT_COIL_R, & LVC); 
    LVC.pszText = AU_C ("回程当前");    // 7 - only read
    LVC.iSubItem = ISR_COLUMN_OUTPUT_CURRENT_R;
    ListView_InsertColumn (window_main_listhook, ISR_COLUMN_OUTPUT_CURRENT_R, & LVC); 
    LVC.pszText = AU_C ("回程延迟");    // 8 - read/write
    LVC.iSubItem = ISR_COLUMN_OUTPUT_DELAY_R;
    ListView_InsertColumn (window_main_listhook, ISR_COLUMN_OUTPUT_DELAY_R, & LVC);  
    LVC.pszText = AU_C ("状态");        // 9 - only read
    LVC.iSubItem = ISR_COLUMN_COMPLETE_IND;
    ListView_InsertColumn (window_main_listhook, ISR_COLUMN_COMPLETE_IND, & LVC); 
    LVI.mask = LVIF_TEXT;
    LVI.pszText = AU_C ("1"); 
    LVI.iItem = 0;
    LVI.iSubItem = 0;
    ListView_InsertItem (window_main_listhook, & LVI);
    ListView_SetItemText (window_main_listhook, 0, ISR_COLUMN_COMPLETE_IND, AU_C ("未完成"));
    return (INT_PTR)TRUE;

  case WM_COMMAND:
    if (LOWORD(wparam) == IDEM_COMSET) {
      DialogBox (GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG_COMMSETTINGS), NULL, dialog_set_proc);
      return (INT_PTR)TRUE;
    } else if (LOWORD(wparam) == IDEM_FLCDEF_SAVEFILE) {
      OPENFILENAME ofn; 
      static AU file_name[MAX_PATH];
      ZeroMemory (& ofn, sizeof (ofn));
      ZeroMemory (& file_name[0], sizeof (file_name));

      ofn.lStructSize = sizeof(OPENFILENAME);
      ofn.hwndOwner = NULL;
      ofn.lpstrFilter = AU_C ("plcdef文件 (.plcdef)\0*.plcdef\0");
      ofn.nFilterIndex = 1;
      ofn.lpstrFile = & file_name[0];
      ofn.nMaxFile = sizeof (file_name);
      ofn.lpstrInitialDir = NULL;
      ofn.lpstrTitle = AU_C ("保存plcdef文件到");
      ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

      if (GetSaveFileName (& ofn) != FALSE) { 
        if (list_check () == 0) {
          aucat (& file_name[0], AU_C (".plcdef"));
          write_plcdef (& file_name[0]);
        }
      } else { 
        MessageBox (NULL, AU_C ("请选择一个文件"), NULL, MB_ICONERROR); 
      } 
      return (INT_PTR)TRUE;
    } else if (LOWORD(wparam) == IDEM_FLCDEF_LOADFILE) {
      OPENFILENAME ofn; 
      static AU file_name[MAX_PATH];
      ZeroMemory (& ofn, sizeof (ofn));
      ZeroMemory (& file_name[0], sizeof (file_name));

      ofn.lStructSize = sizeof(OPENFILENAME);
      ofn.hwndOwner = NULL;
      ofn.lpstrFilter = AU_C ("plcdef文件 (.plcdef)\0*.plcdef\0");
      ofn.nFilterIndex = 1;
      ofn.lpstrFile = & file_name[0];
      ofn.nMaxFile = sizeof (file_name);
      ofn.lpstrInitialDir = NULL;
      ofn.lpstrTitle = AU_C ("请选择plcdef文件");
      ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

      if (GetSaveFileName (& ofn) != FALSE) { 
        read_plcdef (& file_name[0]);
      } else { 
        MessageBox(NULL, AU_C ("请选择一个文件"), NULL, MB_ICONERROR); 
      } 
      return (INT_PTR)TRUE;
    } else if (LOWORD(wparam) == IDCM_BUTTON_FSTOP) {
      AU sbuf[128];
      while (InterlockedExchangePointer (& sg_spinlock, 1) != 0) ;

      GetWindowText (window_main_butfstop, & sbuf[0], sizeof (sbuf));
      if (aucmp (AU_C ("恢复"), & sbuf[0]) == 0) {
        sg_thp_mask = CTL_FORCE_RESUME_MASK;
      } else if (aucmp (AU_C ("PLC强停"), & sbuf[0]) == 0) {
		sg_thp_mask = CTL_FORCE_STOP_MASK;

		EnableWindow (window_main_editreset, TRUE);
        EnableWindow (window_main_butfstop2, FALSE);
        EnableWindow (window_main_butfstop, FALSE);
	  }
      InterlockedExchangePointer (& sg_spinlock, 0);
      return (INT_PTR)TRUE;
    } else if (LOWORD(wparam) == IDCM_BUTTON_FEND) {
      sg_thp_mask = CTL_FORCE_TERM_MASK;
      return (INT_PTR)TRUE;
    } else if (LOWORD(wparam) == IDCANCEL) {
      /*
       * destroy window, We are primarily concerned with the thread problem
       */
	  TerminateThread (sg_ctl.timingth, 0);
      CloseHandle (sg_ctl.timingth);
	  CloseHandle (sg_ctl.comport);
	  EndDialog (window, 0);
      break;
	} else if (LOWORD(wparam) == IDCM_BUTTON_START) {

      /* startup monitor.
       * collect line/comm infos, If there is a mistake, you have to give back the error message
       * write plcdef chunk.
       * startup monitor, control listview and some buttons to run information interaction
       */
       int ii = 0;
	   BYTE obt;
       DCB dcbs;
       BOOL res;
       AU comm_buf0[64]; 

       GetWindowText (window_main_butstartop, & comm_buf0[0], sizeof (comm_buf0));

       if (aucmp (& comm_buf0[0], AU_C ("结束趟处理")) == 0) {
         sg_thp_mask = CTL_NORMAL_END_MASK;
         return (INT_PTR)TRUE;
       }

       /* phase 1:collect comm infos.**/
       append_text (window_main_editstatus, AU_C ("正在收集串口信息\n"));

       if (sg_comset[0].comm_idx == 0xFF) {
         append_text (window_main_editstatus, AU_C ("错误:还没设置串口号\n"));
         return (INT_PTR)TRUE;
       } else if (sg_comset[0].hookC < 200) {
         append_text (window_main_editstatus, AU_C ("错误:C脉冲计数器正确范围[200~255]\n"));
         return (INT_PTR)TRUE;
       }
      
       /* phase 2:precheck, listview's line infos.**/
       if (list_check () != 0)
         return (INT_PTR)TRUE;

       /* line check compelte, we will try open comport **/
       auprintf (& comm_buf0[0], AU_C ("//./COM%i"), sg_comset[0].comm_idx);
       append_text2 (window_main_editstatus, AU_C ("行信息收集完毕, 开始尝试打开串口.. 串口号%d\n"), sg_comset[0].comm_idx);
      
       sg_ctl.comport = CreateFile ( & comm_buf0[0], GENERIC_READ | GENERIC_WRITE, 0,
		      NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
       if (INVALID_HANDLE_VALUE  == sg_ctl.comport) {
		 CloseHandle (sg_ctl.comport);
         append_text (window_main_editstatus, AU_C ("错误:串口打开错误,请检查串口链接情况\n"));
         return (INT_PTR)TRUE;
       } else if (GetLastError() == ERROR_ALREADY_EXISTS) {
		 CloseHandle (sg_ctl.comport);
         append_text (window_main_editstatus, AU_C ("错误:串口已经被其他设备打开了\n"));
         return (INT_PTR)TRUE;
       }
      
       res = GetCommState (sg_ctl.comport, & dcbs);
       assert (res != FALSE);

       dcbs.BaudRate = sg_comset[0].buad_rate;
       dcbs.fParity  = sg_comset[0].parity == NOPARITY ? FALSE : TRUE;
       dcbs.Parity   = sg_comset[0].parity;
       dcbs.StopBits = sg_comset[0].stop_bits;
	   dcbs.ByteSize = 7;

	   SetupComm (sg_ctl.comport, 2048, 2048);
       SetCommState (sg_ctl.comport, & dcbs);
       SetCommMask (sg_ctl.comport, EV_RXCHAR | EV_ERR);
       PurgeComm (sg_ctl.comport, PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR | PURGE_TXABORT);
       {
         COMMTIMEOUTS ct;  
         ct.ReadIntervalTimeout = MAXDWORD;
         ct.ReadTotalTimeoutConstant = 0;  
         ct.ReadTotalTimeoutMultiplier = 0;
  
         ct.WriteTotalTimeoutMultiplier = 500;  
         ct.WriteTotalTimeoutConstant = 5000;  
  
         SetCommTimeouts(sg_ctl.comport, &ct);  
       }

       /* alloc plcdef chunk **/
       sg_ctl.pd_len = sg_listset.cur_line;
       sg_ctl.pd_chunk = malloc (sg_ctl.pd_len * sizeof (struct plcdef_chunk));
       assert (sg_ctl.pd_chunk != NULL);

       sg_ctl.pd_poll = & sg_ctl.pd_chunk[0];
       sg_ctl.xe_poll = & sg_ctl.pd_chunk[sg_ctl.pd_len];

       for (ii = 0; ii != sg_listset.cur_line; ii++) {

         AU settings_chbuf[32];
         AU outputL_chbuf[32]; 
         AU outdelayL_chbuf[32]; 
         AU outputR_chbuf[32]; 
         AU outdelayR_chbuf[32]; 
         ListView_SetItemText (window_main_listhook, ii, ISR_COLUMN_C_CURRENT, AU_C (""));
         ListView_SetItemText (window_main_listhook, ii, ISR_COLUMN_OUTPUT_CURRENT_L, AU_C (""));
         ListView_SetItemText (window_main_listhook, ii, ISR_COLUMN_OUTPUT_CURRENT_R, AU_C (""));
         ListView_SetItemText (window_main_listhook, ii, ISR_COLUMN_COMPLETE_IND, AU_C ("未完成"));
         ListView_GetItemText (window_main_listhook, ii, ISR_COLUMN_C_SETTINGS, settings_chbuf, sizeof (settings_chbuf));
         ListView_GetItemText (window_main_listhook, ii, ISR_COLUMN_OUTPUT_COIL_L, outputL_chbuf, sizeof (outputL_chbuf));
         ListView_GetItemText (window_main_listhook, ii, ISR_COLUMN_OUTPUT_DELAY_L, outdelayL_chbuf, sizeof (outdelayL_chbuf));
         ListView_GetItemText (window_main_listhook, ii, ISR_COLUMN_OUTPUT_COIL_R, outputR_chbuf, sizeof (outputR_chbuf));
         ListView_GetItemText (window_main_listhook, ii, ISR_COLUMN_OUTPUT_DELAY_R, outdelayR_chbuf, sizeof (outdelayR_chbuf));

         sg_ctl.pd_chunk[ii].setv = autoi (settings_chbuf);
         sg_ctl.pd_chunk[ii].writeIO2e[0] = autoi (outputL_chbuf);
         sg_ctl.pd_chunk[ii].writeIO[0] = vailed8 (sg_ctl.pd_chunk[ii].writeIO2e[0]);
         sg_ctl.pd_chunk[ii].delay[0] = autoi (outdelayL_chbuf);
         sg_ctl.pd_chunk[ii].writeIO2e[1] = autoi (outputR_chbuf);
         sg_ctl.pd_chunk[ii].writeIO[1] = vailed8 (sg_ctl.pd_chunk[ii].writeIO2e[1]);
         sg_ctl.pd_chunk[ii].delay[1] = autoi (outdelayR_chbuf);
       }

       /* disable/enable some control **/
       sg_ctl.list_edis = TRUE;
       EnableWindow (window_main_editdelete, FALSE);
       EnableWindow (window_main_editinsert, FALSE);
       EnableWindow (window_main_editattach, FALSE);
       EnableWindow (window_main_butfstop, TRUE);
       EnableWindow (window_main_butfstop2, TRUE);
       EnableWindow (window_main_butstartop, FALSE);
       EnableWindow (window_main_butfend, TRUE);
       SetWindowText (window_main_butstartop, AU_C ("运行中"));
       writePULSE (0);
       /* force open PLC [by M8035/M8036/M8037]  **/
       obt = readRELAY_M (8037);
       obt &= ~(1 << (8037 & 7));
       writeRELAY_M (8037, obt);
       obt = readRELAY_M (8035);
       obt |= (11 << (8035 & 7));
       writeRELAY_M (8035, obt);
       Sleep (500);
       s_call = thp_monitor;
       return (INT_PTR)TRUE;
    }
  default:
    break;
  }
  return (INT_PTR)FALSE;
}

int WINAPI WinMain (HINSTANCE instance, HINSTANCE prev_instance, PSTR command_buffer, int command_count)
{
  MSG msg;
  DWORD tts = 0;
  HWND window = CreateDialogParam(instance, MAKEINTRESOURCE(IDD_DIALOG_MAIN), NULL, dialog_proc, NULL);
  window_main_dialog  = window;
  ShowWindow (window, SW_SHOW); 


  while (TRUE) 
  { 
    if (PeekMessage (& msg, NULL, 0, 0, PM_REMOVE)) 
    { 
      if (msg.message == WM_QUIT) 
        break ; 
      else if (IsDialogMessage(window, & msg)) 
        continue;
      TranslateMessage (& msg) ; 
      DispatchMessage (& msg) ; 
    } 
    else 
    {
      timing_start60();

      if (s_call != NULL) {
        // updata routine.
        s_call ( tts);
      }

      timing_terming60();

      ++ tts;
    } 
  } 

  DestroyWindow (window);
  return msg.wParam;
}

int main (void)  {

  HANDLE module = GetModuleHandle(NULL);
  WinMain (module, NULL, NULL, 0);
  return 0;
}
