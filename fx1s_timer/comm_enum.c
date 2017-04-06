/* not mine. thanks Zach Gorman.<gormanjz@hotmail.com>
 * ---------------------------------------------------------------------------------------------------
 * We will not maintain the old version, 
 * in order to be simple. Only on the WDM driver model of the serial device
 * see c++ source code about http://www.codeguru.com/code/legacy/system/SerialEnum.zip - moecmks
 * ---------------------------------------------------------------------------------------------------
 */
 
#include "comm_enum.h"
#include <windows.h>
#include <SetupAPI.h>
#include <wchar.h>
#include <stdio.h>

#if defined (__UNICODE_DEFINED)
#  define comsccmp wcscmp
#  define comscncmp wcsncmp 
#  define comsclwr wcslwr
#  define comscupr wcsupr
#  define comsclen wcslen
#  define comscprintf wsprintf /* only unicode ??? **/
#  define comsc_t wchar_t
#  define secominfo_section secominfo_section_unicode
#  define secominfo secominfo_unicode
#  define alloc_secominfo_itp alloc_secominfo_unicode
#  define dealloc_secominfo_itp dealloc_secominfo_unicode
#  define SP_DEVICE_INTERFACE_DETAIL_DATA_ SP_DEVICE_INTERFACE_DETAIL_DATA_W
#  define SP_CHBUF_BEHIND_DEVICE 512
#  define SetupDiGetDeviceRegistryProperty_ SetupDiGetDeviceRegistryPropertyW
#  define SetupDiGetDeviceInterfaceDetail_ SetupDiGetDeviceInterfaceDetailW
#  define SetupDiGetClassDevs_ SetupDiGetClassDevsW
#  define CAST_CH_(fz_) L##fz_ 
#  define CAST_CH(fz) CAST_CH_(fz)
#else 
#  define comsccmp strcmp
#  define comscncmp strncmp 
#  define comsclwr strlwr
#  define comscupr strupr
#  define comsclen strlen
#  define comscprintf sprintf
#  define comsc_t char
#  define secominfo_section secominfo_section_ansi
#  define secominfo secominfo_ansi
#  define alloc_secominfo_itp alloc_secominfo_ansi
#  define dealloc_secominfo_itp dealloc_secominfo_ansi
#  define SP_DEVICE_INTERFACE_DETAIL_DATA_ SP_DEVICE_INTERFACE_DETAIL_DATA_A
#  define SP_CHBUF_BEHIND_DEVICE 256
#  define SetupDiGetDeviceRegistryProperty_ SetupDiGetDeviceRegistryPropertyA
#  define SetupDiGetDeviceInterfaceDetail_ SetupDiGetDeviceInterfaceDetailA
#  define SetupDiGetClassDevs_ SetupDiGetClassDevsA
#  define CAST_CH(fz) (fz)
#endif 

static comsc_t _g_dstab[] = {
  CAST_CH('0'),
  CAST_CH('1'),
  CAST_CH('2'),
  CAST_CH('3'),
  CAST_CH('4'),
  CAST_CH('5'),
  CAST_CH('6'),
  CAST_CH('7'),
  CAST_CH('8'),
  CAST_CH('9')
};

int alloc_secominfo_itp (struct secominfo_section **sec, bool *complete) {
  
	/* Create a device information set that will be the container for 
	 * the device interfaces.
   */
  GUID *guid_comport = (GUID *) & GUID_CLASS_COMPORT;
  comsc_t desc[256]; 
  comsc_t loc[256];   
  comsc_t fname[256];
  
  HDEVINFO device_infos = INVALID_HANDLE_VALUE;
  SP_DEVICE_INTERFACE_DETAIL_DATA_ *devinfo_details = NULL;
  SP_DEVICE_INTERFACE_DATA devifd;
  DWORD ii = 0;
  DWORD det_dsize = sizeof (SP_DEVICE_INTERFACE_DETAIL_DATA_) + SP_CHBUF_BEHIND_DEVICE;
  struct secominfo_section *S = NULL;
  struct secominfo *CRL;
  int comp = false; /* I hate bool type ): **/
  int res = -1;/* **/
  
  device_infos = SetupDiGetClassDevs_( guid_comport,
                                       NULL,
                                       NULL, /* Create, populate device info list*/
                                       DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

  if (device_infos == INVALID_HANDLE_VALUE) 
    goto __cleanup0;
  
  devinfo_details = malloc (det_dsize);
  if (devinfo_details == NULL)
    goto __cleanup0;
  
  devinfo_details->cbSize = sizeof (SP_DEVICE_INTERFACE_DETAIL_DATA_);
  devifd.cbSize = sizeof (SP_DEVICE_INTERFACE_DATA);
    
  for (; TRUE; ii++)  
    {
      /* enum device interface ..
       * until the completion of the traversal or error occurred.
       */
       if (!! SetupDiEnumDeviceInterfaces 
                   (device_infos, 
                      NULL, 
                      guid_comport, 
                      ii, 
                    & devifd) == true) 
       {
         SP_DEVINFO_DATA devda;
         devda.cbSize = sizeof (SP_DEVINFO_DATA);
         
         if (!! SetupDiGetDeviceInterfaceDetail_
                     (device_infos,
					             & devifd, 
                         devinfo_details, 
                         det_dsize, 
                         NULL, 
                         & devda) == true)
         {
           if ( ! SetupDiGetDeviceRegistryProperty_ 
                           (device_infos, 
                          & devda, 
                            SPDRP_DEVICEDESC, 
                            NULL, (void *)desc, sizeof(desc), NULL)
           ||   ! SetupDiGetDeviceRegistryProperty_ 
                           (device_infos, 
                          & devda, 
                            SPDRP_FRIENDLYNAME, 
                            NULL, (void *)fname, sizeof(fname), NULL)
           ||   ! SetupDiGetDeviceRegistryProperty_ 
                           (device_infos, 
                          & devda, 
                            SPDRP_LOCATION_INFORMATION, 
                            NULL, (void *)loc, sizeof(loc), NULL) )
              continue;
              
           if (S == NULL) {
            /* The first allocation will initialize the secominfo_section */
             S = malloc (sizeof (struct secominfo_section));
             if (S == NULL) 
                goto __cleanup0;
             S->num = 0;
             S->level = NULL;
           }
            /* initialize the secominfo */
            
           CRL = calloc (sizeof (struct secominfo), 1);
           if (CRL != NULL) {
             int ld = comsclen (& desc[0]); 
             int lf = comsclen (& fname[0]); 
             int lp = comsclen ( devinfo_details->DevicePath);  
             int j = lf - 1;
             
             CRL->usb_device = comscncmp (& loc[0], CAST_CH("USB"), 3) == 0 ? true : false;
             CRL->port_desc     = malloc (ld + 2);
             CRL->device_path   = malloc (lp + 2);
             CRL->friendly_name = malloc (lf + 2);
             CRL->level         = NULL;
             CRL->comm_pos      = -1;

             if (CRL->port_desc == NULL || CRL->device_path == NULL 
              || CRL->friendly_name == NULL ) 
              {
                if (CRL->port_desc != NULL)
                  free (CRL->port_desc);
                if (CRL->friendly_name != NULL)
                  free (CRL->friendly_name);
                if (CRL->device_path != NULL)
                  free (CRL->device_path);
                  free (CRL); CRL = NULL;
                goto __cleanup0;       
              }
                
             /* copy some infos... **/
             memcpy (CRL->port_desc,     & desc[0],  ld + 2);
             memcpy (CRL->friendly_name, & fname[0], lf + 2);
             memcpy (CRL->device_path, devinfo_details->DevicePath, lp + 2);
             
             /* find com index. **/
             /* find (COM_NNN) **/
             
             if (CRL->friendly_name[lf-1] 
                          != CAST_CH(')'))
               goto _insert;     
             
             for (--j; j != -1; j--)  
               {
                 int iii = 0;
                 int stage = 1;
             if (CRL->friendly_name[j] 
                          == CAST_CH('('))
               break; 
                 for (; iii != 10; iii++)  
                   {
                     if (_g_dstab[iii] == CRL->friendly_name[j]) {
                       if (CRL->comm_pos == -1)
                         CRL->comm_pos = 0;
                       CRL->comm_pos += stage * iii;
                       stage *= 10;
                       break;
                     }
                   }
               }
               
             /* insert head. ***/
       _insert:CRL->level = S->level;
             S->level = CRL;
             S->num++;
           }
         }
       } else if (GetLastError() == ERROR_NO_MORE_ITEMS) {
           comp = true;
           res = 0;
       } 
__cleanup0:
       if (complete != NULL) 
         *complete = comp;
              *sec = S;
       if (devinfo_details != NULL) 
         free (devinfo_details);
       if (device_infos != INVALID_HANDLE_VALUE)
         SetupDiDestroyDeviceInfoList (device_infos);
       return res;
    } /* ! enum loop **/
  /* !!! nevenr reach heare **/
  return 0;
} 

void dealloc_secominfo_itp (struct secominfo_section *sec) {
  
  struct secominfo_section *G = NULL;
  struct secominfo *Q, *S;
  
  if (G != NULL) 
    {
      Q = G->level;
      free (G);
      
      for (; Q != NULL; Q = S)
        {
          S = Q->level;
          
          free (Q->friendly_name);
          free (Q->device_path);
          free (Q->port_desc);  
          free (Q);  
        }
    }
}
