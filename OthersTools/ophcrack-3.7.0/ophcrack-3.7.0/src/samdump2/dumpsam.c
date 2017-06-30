/** dumpsam.c
    Dumpsam file (only for Windows) for samdump2 3.x
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

    This program is released under the GPL with the additional exemption 
    that compiling, linking, and/or using OpenSSL is allowed.
    
    Copyright (C) 2009 Cedric Tissieres <http://www.objectif-securite.ch>
*/

#ifdef WIN32

#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <stdint.h>
#include <tchar.h>
#ifndef WIN32
#include "ntdll.h"
#else
#include <winternl.h>
#include <subauth.h>
#endif


/***********************************************************/
/* code from http://www.ivanlef0u.tuxfamily.org/?p=77 */
ULONG EnablePrivilege(char *Privilege)
{
  HANDLE hToken;
  ULONG Ret=1;
  TOKEN_PRIVILEGES TP;
	
  if(!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
    {
      printf("Error with OpenProcessToken: %d\n", (int)GetLastError());
      Ret=0;
      goto bye;	
    } 
	
  if(!LookupPrivilegeValue(NULL, Privilege, &TP.Privileges[0].Luid))
    {
      printf("Error with LookupPrivilegeValue: %d\n", (int)GetLastError());
      Ret=0;
      goto bye;	
		
    }
	
  TP.PrivilegeCount=1;
  TP.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
	
  if(!AdjustTokenPrivileges(hToken,
			    0,
			    &TP,
			    0,
			    NULL,
			    NULL))
    {
      printf("Error with AdjustTokenPrivileges: %d\n", (int)GetLastError());
      Ret=0;
      goto bye;	
		
    }
	
 bye:
  if(hToken)
    CloseHandle(hToken);
	
  return Ret;	
}


/***********************************************************/
/* code from http://www.ivanlef0u.tuxfamily.org/?p=77 */
void hexdump(unsigned char *data, unsigned int amount) {
  unsigned int      dp, p;
  const char        trans[] =
    "................................ !\"#$%&'()*+,-./0123456789"
    ":;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklm"
    "nopqrstuvwxyz{|}~...................................."
    "....................................................."
    "........................................";

  for (dp = 1; dp <= amount; dp++)  {
    printf ("%02x ", data[dp-1]);
    if ((dp % 8) == 0)
      printf (" ");
    if ((dp % 16) == 0) {
      printf ("| ");
      p = dp;
      for (dp -= 16; dp < p; dp++)
	printf ("%c", trans[data[dp]]);
      printf ("\n");
    }
  }
  if ((amount % 16) != 0) {
    p = dp = 16 - (amount % 16);
    for (dp = p; dp > 0; dp--) {
      printf ("   ");
      if (((dp % 8) == 0) && (p != 8))
	printf (" ");
    }
    printf (" | ");
    for (dp = (amount - (16 - p)); dp < amount; dp++)
      printf ("%c", trans[data[dp]]);
  }
  printf ("\n");
  return ;
}

/***********************************************************/
/* code from http://www.ivanlef0u.tuxfamily.org/?p=77 */
int GetFileLCN(HANDLE hFile, int64_t *location, char *error)
{
  ULONG Status;
  IO_STATUS_BLOCK IoBlock;
  unsigned char *temp;
	
  STARTING_VCN_INPUT_BUFFER VCN;
  RETRIEVAL_POINTERS_BUFFER Buff;

  memset(&VCN, 0, sizeof(STARTING_VCN_INPUT_BUFFER));

  temp = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 0x10000);
  Status=NtFsControlFile(hFile,
			 NULL,
			 NULL,
			 NULL,
			 &IoBlock,
			 FSCTL_GET_RETRIEVAL_POINTERS,
			 &VCN,
			 sizeof(STARTING_VCN_INPUT_BUFFER),
			 &Buff,
			 sizeof(RETRIEVAL_POINTERS_BUFFER));
  if(!NT_SUCCESS(Status))
    {
      sprintf(error,"Error with NtFsControlFile : 0x%x\n", (int)Status);
      return -1;
    }
	
  //  printf("ExtentCount : %d\n", Buff.ExtentCount);
  *location = Buff.Extents[0].Lcn.QuadPart;

/*   for(i=0; i<Buff.ExtentCount; i++) */
/*     { */
		
/*       printf("LCN : 0x%x\n", Buff.Extents[i].Lcn); */
/*       printf("VCN : 0x%x\n", Buff.Extents[i].NextVcn); */
		
/*     } */

  HeapFree(GetProcessHeap(), 0, temp);
  return 0;
}
/***********************************************************/
int get_handle(unsigned char *path, PHANDLE outHandle, char *error, int debug) {

  OBJECT_ATTRIBUTES ObjectAttributes;
  IO_STATUS_BLOCK status_block;
  UNICODE_STRING unicode_string;
  int status;
  unsigned char *fullpath;
  wchar_t *fullpathunicode;

  fullpath = (unsigned char *) malloc(5+strlen(path));
  memset(fullpath, 0, 5+strlen(path));
  fullpathunicode = (wchar_t *) malloc((5+strlen(path))*sizeof(wchar_t));
  strncpy(fullpath, "\\??\\", 4);
  strncpy(fullpath+4, path, strlen(path));
  if (debug)
    printf("Full path: %s\n", fullpath);
  mbstowcs(fullpathunicode, fullpath, strlen(fullpath));
  fullpathunicode[strlen(fullpath)] = 0;

  RtlInitUnicodeString(&unicode_string, fullpathunicode);

  InitializeObjectAttributes(&ObjectAttributes, &unicode_string, 0, NULL, NULL);

  status = NtOpenFile(outHandle, FILE_READ_ATTRIBUTES|SYNCHRONIZE, &ObjectAttributes, &status_block, 3, FILE_SYNCHRONOUS_IO_NONALERT);

  if(status!=STATUS_SUCCESS)
    {
      sprintf(error, "Error with NtOpenFile : 0x%x : %d \n", status, (int)RtlNtStatusToDosError(status));
      return -1;	
    }

  if (debug)
    printf("Got handle : %d\n", (int)*outHandle);

  return 0;
}

/***********************************************************/

int get_live_syskey(unsigned char *pkey, char *error, int debug) {
  int i;
  HKEY hk;
  HKEY hkt;
  char *kn[] = {"JD", "Skew1", "GBG", "Data"};
  char kv[9];
  unsigned long kvsize;
  unsigned char key[16];
  unsigned char pkeystring[16*2+1] = {0};

#if BYTE_ORDER == LITTLE_ENDIAN
  int p[] = { 0xb, 0x6, 0x7, 0x1, 0x8, 0xa, 0xe, 0x0, 0x3, 0x5, 0x2, 0xf, 0xd, 0x9, 0xc, 0x4 };
#elif BYTE_ORDER == BIG_ENDIAN
  int p[] = { 0x8, 0x5, 0x4, 0x2, 0xb, 0x9, 0xd, 0x3, 0x0, 0x6, 0x1, 0xc, 0xe, 0xa, 0xf, 0x7 };
#endif
  
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Control\\Lsa", 0, KEY_READ, &hk)==0) {
   
    for (i=0; i<4; i++) {
      if (RegOpenKeyEx(hk, kn[i], 0, KEY_READ, &hkt)) {
	sprintf(error, "Error reading %s key\n", kn[i]);
	return -1;
      }
      
      kvsize = sizeof(kv);
	  
      RegQueryInfoKey(hkt, kv, &kvsize, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
      RegCloseKey(hkt);
      
      sscanf(kv, "%x", (int*) (&key[i*4]));
    }
    RegCloseKey(hk);
    
    for(i=0; i<16; i++) {
      pkey[i] = key[p[i]];
      if (debug) 
	sprintf(pkeystring+i*2, "%.2x", pkey[i]);
    }
  } else {
    sprintf(error, "Error reading LSA key\n");
    return -1;
  }
  if (debug)
    printf("key = %s\n", pkeystring);
  return 0;
} 



/***********************************************************/
int get_sam(unsigned char **buff_sam, int *buff_sam_size, char *error, int debug) {

  uint32_t bytesread, filesize, sectorsize, clustersize;
  int64_t location;
  HANDLE hDevice, hSamFile;
  LARGE_INTEGER tmp;
  char *windir;
  char drive[] = "C:\\";
  char fs[16] ={0};
  char filename[MAX_PATH] ={0}; 	
  char devicename[] = "\\\\.\\C:";
  char boot_sector[512]={0};
  char ntfs = 0;

  if(!EnablePrivilege("SeDebugPrivilege")) {
    sprintf(error, "Unable to set Debug Privilege. You must be admin to run this program.\n");
    return -1;
  }

  windir = getenv("windir");
  strncpy(drive, windir, 1);
  strncpy(devicename+4, windir, 1);
  strncpy(filename, windir, strlen(windir));
  strncat(filename,"\\SYSTEM32\\CONFIG\\SAM", 20);
  if (debug)
    printf("Filename: %s\n", filename);

  if (!GetVolumeInformation(drive, NULL, 0, NULL, NULL, NULL, fs, sizeof(fs)))
    {
      sprintf(error, "Error with GetVolumeInformationByHandleW : %d \n", (int)GetLastError());
      return -1;
    }
    
  if (debug)
    printf("Partition type: %s\n", fs); //FAT32 //NTFS

  if (strncmp(fs, "NTFS", 4))
    ntfs = 1;
  else if (strncmp(fs, "FAT32", 5)) 
    ntfs = 0;
  else {
    sprintf(error, "This tool works only on NTFS or FAT32 volumes\n");
    return -1;
  }

  // Creating a handle to disk drive using CreateFile () function ..
  hDevice = CreateFile(devicename,
		       GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		       NULL, OPEN_EXISTING, 0, NULL);

  if (hDevice == INVALID_HANDLE_VALUE) {
    sprintf(error, "Error with CreateFile: %d\n", (int)GetLastError());
    return -1;
  }
  
  SetFilePointer (hDevice, 0, NULL, FILE_BEGIN);
  
  if (!ReadFile (hDevice, boot_sector, 0x200, (PDWORD)&bytesread, NULL) ) {
    sprintf(error, "Error with ReadFile: %d\n", (int)GetLastError());
    CloseHandle(hDevice);
    return -1;
  }

  sectorsize = *(uint16_t*)(boot_sector+0xb);
  clustersize = boot_sector[0xd];
  if (debug)
    printf("Sector size: %d, Number of sectors by cluster: %d\n", sectorsize, clustersize);

  //go on from here in fat32

  
  if (get_handle(filename, &hSamFile, error, debug)) {
    CloseHandle(hDevice);
    return -1;
  }
  if (debug)
    printf("%d\n", (int)hSamFile);
  
  if (GetFileLCN(hSamFile, &location, error)) {
    CloseHandle(hDevice);
    return -1;
  }


  location *=clustersize;
  filesize = GetFileSize(hSamFile, NULL);
  if (filesize % sectorsize != 0) 
    filesize = (filesize/sectorsize+1)*sectorsize;

  *buff_sam = (char *)malloc(filesize);

  *buff_sam_size = filesize;
  if (debug)
    printf("File size: %d, Location = %lld\n", filesize, location);

  tmp.QuadPart = location*sectorsize;
  SetFilePointerEx(hDevice, tmp, NULL, FILE_BEGIN);

  if (!ReadFile (hDevice, *buff_sam, filesize, (PDWORD)&bytesread, NULL) ) {
    sprintf(error, "Error with ReadFile: %d\n", (int)GetLastError());
    CloseHandle(hDevice);
    return -1;
  }
  
  if (debug)
    hexdump(*buff_sam, 0x200);

  CloseHandle(hDevice);
  CloseHandle(hSamFile);
		
  return 0;	
}
#else
typedef int to_avoid_empty_translation_unit_compiler_warning;
#endif
