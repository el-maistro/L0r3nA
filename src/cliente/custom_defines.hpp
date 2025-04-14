#ifndef __CUSTOM_DEFINES
#define __CUSTOM_DEFINES 1

//Advapi23.dll  winnt.h
typedef void* PVOID;
typedef PVOID PSID;

//typedef enum _SID_NAME_USE {
//    SidTypeUser = 1,
//    SidTypeGroup,
//    SidTypeDomain,
//    SidTypeAlias,
//    SidTypeWellKnownGroup,
//    SidTypeDeletedAccount,
//    SidTypeInvalid,
//    SidTypeUnknown,
//    SidTypeComputer,
//    SidTypeLabel,
//    SidTypeLogonSession
//} SID_NAME_USE, * PSID_NAME_USE;
//
////Kernel32  (GlobalMemoryStatusEx)
//typedef struct _MEMORYSTATUSEX {
//    DWORD     dwLength;
//    DWORD     dwMemoryLoad;
//    DWORDLONG ullTotalPhys;
//    DWORDLONG ullAvailPhys;
//    DWORDLONG ullTotalPageFile;
//    DWORDLONG ullAvailPageFile;
//    DWORDLONG ullTotalVirtual;
//    DWORDLONG ullAvailVirtual;
//    DWORDLONG ullAvailExtendedVirtual;
//} MEMORYSTATUSEX, * LPMEMORYSTATUSEX;

//Wtsapi32.dll
#define WTS_CURRENT_SERVER_HANDLE  ((HANDLE)NULL)



#endif
