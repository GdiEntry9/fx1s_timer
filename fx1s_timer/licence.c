#include <Winsock2.h>
#include <iphlpapi.h>
#include <Windows.h>
#include <assert.h>
#include <stdio.h>
#include "resource.h"

static HWND window_edit_input = FALSE;
static void *callback_hook_edit = NULL;
static BOOL pass_licence = FALSE;
static BYTE password[13] =  { 0 }; // XXX: init maybe invailed. 

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

static BYTE * /* num is 6, not mine. from internet. **/  
GetFirstMac (void){

  ULONG outBufLen = sizeof(IP_ADAPTER_ADDRESSES); 
  static IP_ADAPTER_ADDRESSES ip_adp_address[256];
  static PIP_ADAPTER_ADDRESSES pCurrAddresses = & ip_adp_address[0];
  DWORD dwRetVal;
  GetAdaptersAddresses(0, 0, NULL, NULL, & outBufLen);
  dwRetVal = GetAdaptersAddresses(0, 0, NULL, pCurrAddresses, &outBufLen);
  assert (dwRetVal == NO_ERROR);
  assert (pCurrAddresses != NULL);

  return & pCurrAddresses->PhysicalAddress[0];
}

static 
void decode_encipher_mac (BYTE *chrbuf)
{
  if ( strlen (chrbuf) != 12 )
    pass_licence = FALSE;

  {
    DWORD ii = 0;
    BYTE mac[8];
    BYTE *mac2 = GetFirstMac ();
    BYTE encode6[7];

    * (DWORD *) & mac[0] = (* (DWORD *) & mac2[0] & 0xFFFFFF) - 0x788921;
    * (DWORD *) & mac[3] = ( (* (DWORD *) & mac2[3] & 0xFFFFFF)- 0x144567) ^ 0xFFFFFF;

    for (; ii != 12; ii+= 2) {
      BYTE obi = ascii_to_num (chrbuf[ii+1]);
      BYTE obt = obi << 4;
           obt|= ascii_to_num (chrbuf[ii]);
      encode6[ii>>1]= obt;
    }

    if (strncmp (& encode6[0], & mac[0], 6) == 0)
      pass_licence = TRUE;
    else 
      pass_licence = FALSE;
  }
}

static 
int CALLBACK hook_editnumb (HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
  UCHAR key = (UCHAR)LOWORD(wparam);

  if (message == WM_CHAR)
  {
    if ((key >= '0' && key <= '9') )
      goto __final;
    if ((key >= 'A' && key <= 'F') || key == VK_BACK)
      goto __final;
    return 0;
  } 
  __final:
  return CallWindowProc (callback_hook_edit, window, message, wparam, lparam);
}

static 
int CALLBACK licence_proc (HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
  switch (message)
  {
  case WM_INITDIALOG:

    window_edit_input = GetDlgItem (window, IDCL_EDITINPUT);
    callback_hook_edit = (void *)SetWindowLong (window_edit_input, GWL_WNDPROC, (LONG)hook_editnumb);

    return (INT_PTR)TRUE;

  case WM_COMMAND:
    if (LOWORD(wparam) == IDCANCEL) {
      pass_licence = FALSE;
      EndDialog (window, 0);
      return (INT_PTR) TRUE;
    } else if (LOWORD(wparam) == IDOK) {
      CHAR chbuf[256];
      GetWindowTextA (window_edit_input, chbuf, sizeof (chbuf));
      /* decode user's licence **/
      decode_encipher_mac (& chbuf[0]);

      EndDialog (window, 0);
      return (INT_PTR) TRUE;
    }

  default:
    break;
  }
  return (INT_PTR)FALSE;
}

BOOL licence_check (void) 
{
  LSTATUS status = 0;
  HKEY key = NULL;
  DWORD ii = 0;
  BYTE mac[8];
  BYTE *mac2 = GetFirstMac ();

  * (DWORD *) & mac[0] = (* (DWORD *) & mac2[0] & 0xFFFFFF) - 0x788921;
  * (DWORD *) & mac[3] = ( (* (DWORD *) & mac2[3] & 0xFFFFFF)- 0x144567) ^ 0xFFFFFF;

  for (; ii != 12; ii+= 2) {
    password[ii+0] = num_to_ascii (mac[ii>>1] & 0x0F);
    password[ii+1] = num_to_ascii (mac[ii>>1]>> 4);
    password[12] = 0;
  }

  status = RegOpenKeyExA (HKEY_CURRENT_USER, & password[0], 0, KEY_ALL_ACCESS, & key);
  if (status != 0) {
    /* first init... ***/ 
    DialogBox (GetModuleHandle (NULL), MAKEINTRESOURCE(IDD_DIALOG_VAILECHECK), NULL, licence_proc);

    if (pass_licence != FALSE) {
      LSTATUS status = 0;
      DWORD rw;

      RegCreateKeyExA (HKEY_CURRENT_USER, & password[0], 0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, & key, & rw);
      RegCloseKey (key);
    }
    return pass_licence;
  } else {
    RegCloseKey (key);
    return TRUE;
  }
}
