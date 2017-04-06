#include <Winsock2.h>
#include <iphlpapi.h>
#include <Windows.h>
#include <assert.h>
#include <stdio.h>
#include "resource.h"

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

BYTE /* num is 6 **/ *GetFirstMac (void){

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


INT_PTR CALLBACK keygen_proc (HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{

  HWND edit_output = NULL;
  CHAR tempbuf[13];
  BYTE *pmac = GetFirstMac ();
  BYTE mac[8];
  DWORD ii = 0;

  switch (message)
  {
  case WM_INITDIALOG:
    

    edit_output = GetDlgItem (window, IDC_EDITOUT);

    // calc 
    * (DWORD *) & mac[0] = (* (DWORD *) & pmac[0] & 0xFFFFFF) - 0x788921;
    * (DWORD *) & mac[3] = ( (* (DWORD *) & pmac[3] & 0xFFFFFF)- 0x144567) ^ 0xFFFFFF;

    for (; ii != 12; ii+= 2) {
      tempbuf[ii] = num_to_ascii (mac[ii>>1] & 0x0F);
      tempbuf[ii+1] = num_to_ascii ( (mac[ii>>1]>> 4));
    }



tempbuf[12] = 0;

    SetWindowTextA (edit_output, tempbuf);

    return (INT_PTR)TRUE;

  case WM_COMMAND:
    if (LOWORD (wparam) == IDCANCEL)
    {
      EndDialog (window, 0);
      return (INT_PTR)TRUE;
    }
    
  default:
    break;
  }
  return (INT_PTR)FALSE;
}


int WINAPI WinMain (HINSTANCE instance, HINSTANCE prev_instance,
                     PSTR cmdline, int show)
{
  DialogBox (instance, MAKEINTRESOURCE(IDD_DIALOG_KEYGEN), NULL, keygen_proc);
  return 0;
}

