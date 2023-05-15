#include "stdafx.h"

#include "processctl.h"
#include "memtools.h"
#include "debugger.h"
#include "log.h"

// оффсет считается от адреса последнего байта шаблона

static const ProcessCtl::PatchData disableProtector
{
    // .text:0045F7E2 FA4 89 55 D8                                mov     [ebp+ProcessInformation.hProcess], edx
    // .text:0045F7E5 FA4 89 55 DC                                mov     [ebp+ProcessInformation.hThread], edx
    // .text:0045F7E8 FA4 89 55 E0                                mov     [ebp+ProcessInformation.dwProcessId], edx
    // .text:0045F7EB FA4 89 55 E4                                mov     [ebp+ProcessInformation.dwThreadId], edx
    // .text:0045F7EE FA4 8D 45 D8                                lea     eax, [ebp+ProcessInformation]
    // .text:0045F7F1 FA4 50                                      push    eax             ; lpProcessInformation
    // .text:0045F7F2 FA8 8D 4D 88                                lea     ecx, [ebp+StartupInfo]
    // .text:0045F7F5 FA8 51                                      push    ecx             ; lpStartupInfo
    // .text:0045F7F6 FAC 52                                      push    edx             ; lpCurrentDirectory
    // .text:0045F7F7 FB0 52                                      push    edx             ; lpEnvironment
    // .text:0045F7F8 FB4 52                                      push    edx             ; dwCreationFlags
    // .text:0045F7F9 FB8 52                                      push    edx             ; bInheritHandles
    // .text:0045F7FA FBC 52                                      push    edx             ; lpThreadAttributes
    // .text:0045F7FB FC0 52                                      push    edx             ; lpProcessAttributes
    // .text:0045F7FC FC4 52                                      push    edx             ; lpCommandLine
    // .text:0045F7FD FC8 68 10 14 CB 00                          push    offset aReportbugsPwprotector_ ; "reportbugs\\pwprotector.exe"
    // .text:0045F802 FCC FF 15 EC DE D2 00                       call    o_CreateProcessW
    0x050000,
    0x050000,
    {
        0x89, 0x55, 0x1D8, 0x89, 0x55, 0x1DC, 0x89, 0x55, 0x1E0, 0x89, 0x55, 0x1E4, 0x8D, 0x45, 0x1D8, 0x50,
        0x8D, 0x4D, 0x188, 0x51, 0x52, 0x52, 0x52, 0x52, 0x52, 0x52, 0x52, 0x68, 0x110, 0x114, 0x1CB, 0x00,
        0xFF
    },
    0,
    { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 }
};

static const ProcessCtl::PatchData disableProtector2
{
    // FIXME patch is actually wrong
    // .text:004186CA F78 89 45 BC                                mov     [ebp+0EA0h+var_EE4], eax
    // .text:004186CD F78 89 45 C0                                mov     [ebp+0EA0h+var_EE0], eax
    // .text:004186D0 F78 89 45 C4                                mov     [ebp+0EA0h+var_EDC], eax
    // .text:004186D3 F78 89 45 C8                                mov     [ebp+0EA0h+var_ED8], eax
    // .text:004186D6 F78 8D 4D BC                                lea     ecx, [ebp+0EA0h+var_EE4]
    // .text:004186D9 F78 51                                      push    ecx
    // .text:004186DA F7C 8D 95 3C FF FF FF                       lea     edx, [ebp+0EA0h+var_F64]
    // .text:004186E0 F7C 52                                      push    edx
    // .text:004186E1 F80 53                                      push    ebx
    // .text:004186E2 F84 53                                      push    ebx
    // .text:004186E3 F88 53                                      push    ebx
    // .text:004186E4 F8C 53                                      push    ebx
    // .text:004186E5 F90 53                                      push    ebx
    // .text:004186E6 F94 53                                      push    ebx
    // .text:004186E7 F98 53                                      push    ebx
    // .text:004186E8 F9C 68 E0 51 C8 00                          push    offset aReportbugsPwpr ; "reportbugs\\pwprotector.exe"
    // .text:004186ED FA0 FF 15 94 B9 E4 00                       call    off_E4B994
    0x010000,
    0x050000,
    {
        0x89, 0x45, 0x1D8, 0x89, 0x45, 0x1DC, 0x89, 0x45, 0x1E0, 0x89, 0x45, 0x1E4, 0x8D, 0x4D, 0x1D8, 0x51,
        0x8D, 0x95, 0x188, 0xFF, 0xFF, 0xFF, 0x52, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x53, 0x68, 0x110,
        0x114, 0x1CB, 0x00, 0xFF
    },
    0,
    { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 }
};

static const ProcessCtl::PatchData disableProtector3
{
    // .text:00418737 F98 8D 45 C0                                      lea     eax, [ebp+0EC0h+var_F00]
    // .text:0041873A F98 50                                            push    eax
    // .text:0041873B F9C 8D 8D 3C FF FF FF                             lea     ecx, [ebp+0EC0h+var_F84]
    // .text:00418741 F9C 51                                            push    ecx
    // .text:00418742 FA0 6A 00                                         push    0
    // .text:00418744 FA4 6A 00                                         push    0
    // .text:00418746 FA8 6A 00                                         push    0
    // .text:00418748 FAC 6A 00                                         push    0
    // .text:0041874A FB0 6A 00                                         push    0
    // .text:0041874C FB4 6A 00                                         push    0
    // .text:0041874E FB8 6A 00                                         push    0
    // .text:00418750 FBC 68 78 14 CA 00                                push    offset aReportbugsPwpr ; "reportbugs\\pwprotector.exe"
    // .text:00418755 FC0 FF 15 88 D2 E7 00                             call    off_E7D288
    0x010000,
    0x050000,
    {
        0x50,
        0x8D, 0x8D, 0x13C, 0xFF, 0xFF, 0xFF,
        0x51,
        0x6A, 0x00,
        0x6A, 0x00,
        0x6A, 0x00,
        0x6A, 0x00,
        0x6A, 0x00,
        0x6A, 0x00,
        0x6A, 0x00,
        0x68, 0x110, 0x114, 0x1CB, 0x00,
        0xFF, 0x15
    },
    0x00418737 - 0x00418756,
    { 0xEB, 0x22 }
};

static const ProcessCtl::PatchData disableProtector4
{
    // .text:0044A141 F4C 33 FF                                   xor     edi, edi
    // .text:0044A143 F4C 33 C0                                   xor     eax, eax
    // .text:0044A145 F4C 89 45 D0                                mov     [ebp+0EC0h+ProcessInformation.hProcess], eax
    // .text:0044A148 F4C 89 45 D4                                mov     [ebp+0EC0h+ProcessInformation.hThread], eax
    // .text:0044A14B F4C 89 45 D8                                mov     [ebp+0EC0h+ProcessInformation.dwProcessId], eax
    // .text:0044A14E F4C 89 45 DC                                mov     [ebp+0EC0h+ProcessInformation.dwThreadId], eax
    // .text:0044A151 F4C 8D 45 D0                                lea     eax, [ebp+0EC0h+ProcessInformation]
    // .text:0044A154 F4C 50                                      push    eax             ; lpProcessInformation
    // .text:0044A155 F50 8D 4D 88                                lea     ecx, [ebp+0EC0h+StartupInfo]
    // .text:0044A158 F50 51                                      push    ecx             ; lpStartupInfo
    // .text:0044A159 F54 57                                      push    edi             ; lpCurrentDirectory
    // .text:0044A15A F58 57                                      push    edi             ; lpEnvironment
    // .text:0044A15B F5C 57                                      push    edi             ; dwCreationFlags
    // .text:0044A15C F60 57                                      push    edi             ; bInheritHandles
    // .text:0044A15D F64 57                                      push    edi             ; lpThreadAttributes
    // .text:0044A15E F68 57                                      push    edi             ; lpProcessAttributes
    // .text:0044A15F F6C 57                                      push    edi             ; lpCommandLine
    // .text:0044A160 F70 68 D0 97 00 01                          push    offset aReportbugsPwpr ; lpApplicationName
    // .text:0044A165 F74 FF 15 A8 23 00 01                       call    ds:CreateProcessW
    0x040000,
    0x050000,
    {
        0x51, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x57, 0x68, 0x110, 0x114, 0x1CB, 0x100,
        0xFF
    },
    0,
    { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 }
};

static const ProcessCtl::PatchData disableProtector5
{
    // .text:00BA28F4 F54 0F 11 85 04 F1 FF FF                          movups  xmmword ptr [ebp+ProcessInformation.hProcess], xmm0
    // .text:00BA28FB F54 8D 85 04 F1 FF FF                             lea     eax, [ebp+ProcessInformation]
    // .text:00BA2901 F54 50                                            push    eax             ; lpProcessInformation
    // .text:00BA2902 F58 8D 85 C0 F0 FF FF                             lea     eax, [ebp+StartupInfo]
    // .text:00BA2908 F58 50                                            push    eax             ; lpStartupInfo
    // .text:00BA2909 F5C 6A 00                                         push    0               ; lpCurrentDirectory
    // .text:00BA290B F60 6A 00                                         push    0               ; lpEnvironment
    // .text:00BA290D F64 6A 00                                         push    0               ; dwCreationFlags
    // .text:00BA290F F68 6A 00                                         push    0               ; bInheritHandles
    // .text:00BA2911 F6C 6A 00                                         push    0               ; lpThreadAttributes
    // .text:00BA2913 F70 6A 00                                         push    0               ; lpProcessAttributes
    // .text:00BA2915 F74 6A 00                                         push    0               ; lpCommandLine
    // .text:00BA2917 F78 68 20 1E 09 01                                push    offset aReportbugsPwprotect ; lpApplicationName
    // .text:00BA291C F7C FF 15 08 F3 FF 00                             call    ds:CreateProcessW
    0x0750000,
    0x0200000,
    {
        0x50,
        0x6A, 0x00,
        0x6A, 0x00,
        0x6A, 0x00,
        0x6A, 0x00,
        0x6A, 0x00,
        0x6A, 0x00,
        0x6A, 0x00,
        0x68, 0x110, 0x114, 0x1CB, 0x100,
        0xFF, 0x15
    },
    0x00BA2901 - 0x00BA291D,
    { 0xEB, 0x1F }
};

static const ProcessCtl::PatchData disableServerSelPatch
{
    //       006F92E6 EB 4С
    // 
    // .text:006F92CD 018 E8 AE 93 00 00                                call    pCECServerList
    // .text:006F92D2 018 8B 48 10                                      mov     ecx, [eax+10h]
    // .text:006F92D5 018 5F                                            pop     edi
    // .text:006F92D6 014 85 C9                                         test    ecx, ecx
    // .text:006F92D8 014 7D 5A                                         jge     short loc_6F9334
    // .text:006F92DA 014 E8 A1 93 00 00                                call    pCECServerList
    // .text:006F92DF 014 8B C8                                         mov     ecx, eax
    // .text:006F92E1 014 E8 DA B2 00 00                                call    readCurrentServer
    //*.text:006F92E6 014 8B 16                                         mov     edx, [esi]
    // .text:006F92E8 014 6A 00                                         push    0
    // .text:006F92EA 018 68 20 25 C4 00                                push    offset ??_R0?AVCDlgLoginServerList@@@8 ; CDlgLoginServerList `RTTI Type Descriptor'
    // .text:006F92EF 01C 68 18 52 C2 00                                push    offset ??_R0?AVAUIDialog@@@8 ; AUIDialog `RTTI Type Descriptor'
    // .text:006F92F4 020 6A 00                                         push    0
    // .text:006F92F6 024 68 18 21 C4 00                                push    offset aWin_loginser_0 ; "Win_LoginServerList"
    // .text:006F92FB 028 8B CE                                         mov     ecx, esi
    // .text:006F92FD 028 FF 52 34                                      call    dword ptr [edx+34h]
    // .text:006F9300 028 50                                            push    eax
    // .text:006F9301 02C E8 B0 E4 3E 00                                call    __RTDynamicCast
    // .text:006F9306 02C 8B 10                                         mov     edx, [eax]
    // .text:006F9308 02C 83 C4 14                                      add     esp, 14h
    // .text:006F930B 018 8B C8                                         mov     ecx, eax
    // .text:006F930D 018 6A 01                                         push    1
    // .text:006F930F 01C 6A 00                                         push    0
    // .text:006F9311 020 6A 01                                         push    1
    // .text:006F9313 024 FF 52 1C                                      call    dword ptr [edx+1Ch]
    // .text:006F9316 014 8B 0D AC DF C9 00                             mov     ecx, g_CECGame
    // .text:006F931C 014 8B 49 1C                                      mov     ecx, [ecx+1Ch]
    // .text:006F931F 014 E8 EC 96 D4 FF                                call    sub_442A10
    // .text:006F9324 014 5E                                            pop     esi
    // .text:006F9325 010 8B 4C 24 04                                   mov     ecx, [esp+10h+var_C]
    // .text:006F9329 010 64 89 0D 00 00 00 00                          mov     large fs:0, ecx
    // .text:006F9330 010 83 C4 10                                      add     esp, 10h
    // .text:006F9333 000 C3                                            retn
    0x280000,
    0x200000,
    {
        0xE8, 0x13E, 0x191, 0x100, 0x00, 0x8B, 0x48, 0x10,  0x5F, 0x85, 0xC9, 0x7D, 0x5A, 0xE8, 0x131, 0x191,
        0x100, 0x00, 0x8B, 0xC8, 0xE8, 0x16A, 0x1B0, 0x100,  0x00, 0x8B, 0x16, 0x6A, 0x00, 0x68
    },
    -4,
    { 0xEB, 0x4C }
};

static const ProcessCtl::PatchData disableServerSelPatch2
{
    // .text:00824E06 014 E8 25 D3 00 00                          call    sub_832130
    // .text:00824E0B 014 8B 48 10                                mov     ecx, [eax+10h]
    // .text:00824E0E 014 85 C9                                   test    ecx, ecx
    // .text:00824E10 014 7D 5A                                   jge     short loc_824E6C
    // .text:00824E12 014 E8 19 D3 00 00                          call    sub_832130
    // .text:00824E17 014 8B C8                                   mov     ecx, eax
    // .text:00824E19 014 E8 C2 F4 00 00                          call    sub_8342E0
    //*.text:00824E1E 014 8B 16                                   mov     edx, [esi]
    // .text:00824E20 014 6A 00                                   push    0
    //-.text:00824E22 018 68 30 26 E9 00                          push    offset ??_R0?AVCDlgLoginServerList@@@8 ; CDlgLoginServerList `RTTI Type Descriptor'
    // .text:00824E27 01C 68 C8 9C E6 00                          push    offset ??_R0?AVAUIDialog@@@8 ; AUIDialog `RTTI Type Descriptor'
    // .text:00824E2C 020 6A 00                                   push    0
    // .text:00824E2E 024 68 50 22 E9 00                          push    offset aWin_loginser_0 ; "Win_LoginServerList"
    // .text:00824E33 028 8B CE                                   mov     ecx, esi
    // .text:00824E35 028 FF 52 38                                call    dword ptr [edx+38h]
    // .text:00824E38 028 50                                      push    eax
    // .text:00824E39 02C E8 AE 23 49 00                          call    __RTDynamicCast
    // .text:00824E3E 02C 8B 10                                   mov     edx, [eax]
    // .text:00824E40 02C 83 C4 14                                add     esp, 14h
    // .text:00824E43 018 8B C8                                   mov     ecx, eax
    // .text:00824E45 018 6A 01                                   push    1
    // .text:00824E47 01C 6A 00                                   push    0
    // .text:00824E49 020 6A 01                                   push    1
    // .text:00824E4B 024 FF 52 1C                                call    dword ptr [edx+1Ch]
    // .text:00824E4E 014 8B 0D 44 83 EF 00                       mov     ecx, dword_EF8344
    // .text:00824E54 014 8B 49 1C                                mov     ecx, [ecx+1Ch]
    // .text:00824E57 014 E8 94 E3 C2 FF                          call    sub_4531F0
    // .text:00824E5C 014 5E                                      pop     esi
    // .text:00824E5D 010 8B 4C 24 04                             mov     ecx, [esp+10h+var_C]
    // .text:00824E61 010 64 89 0D 00 00 00 00                    mov     large fs:0, ecx
    // .text:00824E68 010 83 C4 10                                add     esp, 10h
    // .text:00824E6B 000 C3                                      retn
    0x280000,
    0x200000,
    {
        0xE8, 0x13E, 0x191, 0x100, 0x00,
        0x8B, 0x48, 0x10,
        0x85, 0xC9,
        0x7D, 0x5A,
        0xE8, 0x131, 0x191, 0x100, 0x00,
        0x8B, 0xC8,
        0xE8, 0x16A, 0x1B0, 0x100, 0x00,
        0x8B, 0x16,
        0x6A, 0x00, 0x68
    },
    -4,
    { 0xEB, 0x4C }
};

static const ProcessCtl::PatchData disableServerSelPatch3
{
    // .text:005CB027 020 7D 61                                   jge     short loc_5CB08A
    // .text:005CB029 020 E8 12 11 01 00                          call    pCECServerList
    // .text:005CB02E 020 8B C8                                   mov     ecx, eax
    // .text:005CB030 020 E8 7B 12 01 00                          call    readCurrentServer
    // .text:005CB035 020 8B 16                                   mov     edx, [esi]
    // .text:005CB037 020 8B 42 38                                mov     eax, [edx+38h]
    //-.text:005CB03A 020 6A 00                                   push    0
    // .text:005CB03C 024 68 14 14 E0 00                          push    offset ??_R0?AVCDlgLoginServerList@@@8 ; CDlgLoginServerList `RTTI Type Descriptor'
    // .text:005CB041 028 68 A0 60 DF 00                          push    offset ??_R0?AVAUIDialog@@@8 ; AUIDialog `RTTI Type Descriptor'
    // .text:005CB046 02C 6A 00                                   push    0
    // .text:005CB048 030 68 C8 CD CA 00                          push    offset aWin_loginser_0 ; "Win_LoginServerList"
    // .text:005CB04D 034 8B CE                                   mov     ecx, esi
    //*.text:005CB04F 034 FF D0                                   call    eax
    // .text:005CB051 034 50                                      push    eax
    // .text:005CB052 038 E8 99 71 5B 00                          call    __RTDynamicCast
    // .text:005CB057 038 8B 10                                   mov     edx, [eax]
    // .text:005CB059 038 83 C4 14                                add     esp, 14h
    // .text:005CB05C 024 6A 01                                   push    1
    // .text:005CB05E 028 6A 00                                   push    0
    // .text:005CB060 02C 8B C8                                   mov     ecx, eax
    // .text:005CB062 02C 8B 42 1C                                mov     eax, [edx+1Ch]
    // .text:005CB065 02C 6A 01                                   push    1
    // .text:005CB067 030 FF D0                                   call    eax
    // .text:005CB069 020 8B 0D B0 BA E4 00                       mov     ecx, dword_E4BAB0
    // .text:005CB06F 020 8B 49 1C                                mov     ecx, [ecx+1Ch]
    // .text:005CB072 020 E8 19 C3 E3 FF                          call    sub_407390
    // .text:005CB077 020 8B 4C 24 14                             mov     ecx, [esp+20h+var_C]
    // .text:005CB07B 020 64 89 0D 00 00 00 00                    mov     large fs:0, ecx
    // .text:005CB082 020 59                                      pop     ecx
    // .text:005CB083 01C 5F                                      pop     edi
    // .text:005CB084 018 5E                                      pop     esi
    // .text:005CB085 014 5B                                      pop     ebx
    // .text:005CB086 010 83 C4 10                                add     esp, 10h
    // .text:005CB089 000 C3                                      retn
    //>.text:005CB08A 020 8B 8E 04 0D 00 00                       mov     ecx, [esi+0D04h]
    0x180000,
    0x200000,
    {
        0xE8, 0x112, 0x111, 0x101, 0x100,
        0x8B, 0xC8,
        0xE8, 0x17B, 0x112, 0x101, 0x100,
        0x8B, 0x116,
        0x8B, 0x42, 0x38,
        0x6A, 0x00,
        0x68, 0x114, 0x114, 0x1E0, 0x100,
        0x68, 0x1A0, 0x160, 0x1DF, 0x100,
        0x6A, 0x00,
        0x68, 0x1C8, 0x1CD, 0x1CA, 0x100,
        0x8B, 0x1CE,
        0xFF, 0xD0
    },
    -22,
#if CLIENT_VERSION < 1590
    { 0xEB, 0x4E }
#else
    { 0xEB, 0x2F }
#endif
};

static const ProcessCtl::PatchData disableServerSelPatch4
{
    // .text:00A6951A 024 E8 A1 22 0F 00                                call    pCECServerList
    // .text:00A6951F 024 8B C8                                         mov     ecx, eax
    // .text:00A69521 024 E8 3A 2E 0F 00                                call    sub_B5C360
    // .text:00A69526 024 E8 95 22 0F 00                                call    pCECServerList
    // .text:00A6952B 024 83 78 10 00                                   cmp     dword ptr [eax+10h], 0
    // .text:00A6952F 024 7D 3E                                         jge     short loc_A6956F
    // .text:00A69531 024 E8 8A 22 0F 00                                call    pCECServerList
    // .text:00A69536 024 8B C8                                         mov     ecx, eax
    // .text:00A69538 024 E8 03 27 0F 00                                call    readCurrentServer
    // .text:00A6953D 024 8B 06                                         mov     eax, [esi]
    // .text:00A6953F 024 8B CE                                         mov     ecx, esi
    //-.text:00A69541 024 6A 00                                         push    0
    // .text:00A69543 028 68 E8 37 26 01                                push    offset ??_R0?AVCDlgLoginServerList@@@8 ; CDlgLoginServerList `RTTI Type Descriptor'
    // .text:00A69548 02C 68 50 8D 25 01                                push    offset ??_R0?AVAUIDialog@@@8 ; AUIDialog `RTTI Type Descriptor'
    // .text:00A6954D 030 6A 00                                         push    0
    // .text:00A6954F 034 68 38 02 02 01                                push    offset aWin_loginserve ; "Win_LoginServerList"
    // .text:00A69554 038 FF 50 38                                      call    dword ptr [eax+38h]
    // .text:00A69557 038 50                                            push    eax
    // .text:00A69558 03C E8 65 3B 4E 00                                call    __RTDynamicCast
    // .text:00A6955D 03C 83 C4 14                                      add     esp, 14h
    // .text:00A69560 028 8B C8                                         mov     ecx, eax
    // .text:00A69562 028 8B 10                                         mov     edx, [eax]
    // .text:00A69564 028 6A 01                                         push    1
    // .text:00A69566 02C 6A 00                                         push    0
    // .text:00A69568 030 6A 01                                         push    1
    // .text:00A6956A 034 FF 52 24                                      call    dword ptr [edx+24h]
    // .text:00A6956D 024 EB 18                                         jmp     short loc_A69587
    // .text:00A6956F                                   loc_A6956F:                             ; CODE XREF: sub_A693F0+13Fj
    // .text:00A6956F 024 8B 8E B0 12 10 00                             mov     ecx, [esi+1012B0h]
    // e8 ? ? ? ? 8b c8 e8 ? ? ? ? 8b 06 8b ce 6a 00 68 ? ? ? ? 68 ? ? ? ? 6a 00 68 ? ? ? ? ff 50
    0x600000,
    0x200000,
    {
        0xE8, 0x112, 0x111, 0x101, 0x100,
        0x8B, 0xC8,
        0xE8, 0x17B, 0x112, 0x101, 0x100,
        0x8B, 0x116,
        0x8B, 0xCE,
        0x6A, 0x00,
        0x68, 0x114, 0x114, 0x1E0, 0x100,
        0x68, 0x1A0, 0x160, 0x1DF, 0x100,
        0x6A, 0x00,
        0x68, 0x1C8, 0x1CD, 0x1CA, 0x100,
        0xFF, 0x50, 0x38,
        0x50,
        0xE8, 0x165, 0x13B, 0x14E, 0x00,
        0x83, 0xC4, 0x14,
        0x8B, 0xC8,
        0x8B, 0x10,
    },
    -0x22,
    { 0xEB, 0x2C }
};

static const ProcessCtl::PatchData passOpenMarketPatch
{
    // .text:00521A36 03C 85 FF                                   test    edi, edi
    // .text:00521A38 03C 77 32                                   ja      short loc_521A6C
    // .text:00521A3A 03C 85 ED                                   test    ebp, ebp
    // .text:00521A3C 03C 7F 2E                                   jg      short loc_521A6C
    // .text:00521A3E 03C 7C 04                                   jl      short loc_521A44
    // .text:00521A40 03C 85 C0                                   test    eax, eax
    // .text:00521A42 03C 77 28                                   ja      short loc_521A6C
    // .text:00521A44 03C 68 08 0C C6 00                          push    offset aDefault_txt_na ; "DEFAULT_Txt_Name"
    // .text:00521A49 040 8B CE                                   mov     ecx, esi        ; this
    // .text:00521A4B 040 E8 D0 E0 41 00                          call    sub_93FB20
    // .text:00521A50 03C 8B C8                                   mov     ecx, eax
    // .text:00521A52 03C E8 99 48 41 00                          call    sub_9362F0
    // .text:00521A57 03C 84 C0                                   test    al, al
    // .text:00521A59 03C 75 11                                   jnz     short loc_521A6C
    0x100000,
    0x200000,
    {
        0x85, 0xFF,
        0x77, 0x132,
        0x85, 0xED,
        0x7F, 0x12E,
        0x7C, 0x104,
        0x85, 0xC0,
        0x77, 0x128,
        0x68, 0x108, 0x10C, 0x1C6, 0x00,
        0x8B, 0xCE,
        0xE8, 0x1D0, 0x1E0, 0x141, 0x00,
        0x8B, 0xC8,
        0xE8, 0x199, 0x148, 0x141, 0x00,
        0x84, 0xC0,
        0x75
    },
    0,
    {0xEB}
};

static const ProcessCtl::PatchData passOpenMarketPatch2
{
    // .text:00678503 040 85 FF                                         test    edi, edi
    // .text:00678505 040 77 2D                                         ja      short loc_678534
    // .text:00678507 040 85 ED                                         test    ebp, ebp
    // .text:00678509 040 7F 29                                         jg      short loc_678534
    // .text:0067850B 040 7C 04                                         jl      short loc_678511
    // .text:0067850D 040 85 C0                                         test    eax, eax
    // .text:0067850F 040 77 23                                         ja      short loc_678534
    // .text:00678511 040 68 28 48 DE 00                                push    offset aDefaultTxtName ; "DEFAULT_Txt_Name"
    // .text:00678516 044 8B CE                                         mov     ecx, esi
    // .text:00678518 044 E8 F3 6C 47 00                                call    sub_AEF210
    // .text:0067851D 040 80 78 7E 00                                   cmp     byte ptr [eax+7Eh], 0
    // .text:00678521 040 75 11                                         jnz     short loc_678534
    0x100000,
    0x200000,
    {
        0x85, 0xFF,
        0x77, 0x132,
        0x85, 0xED,
        0x7F, 0x12E,
        0x7C, 0x104,
        0x85, 0xC0,
        0x77, 0x128,
        0x68, 0x108, 0x10C, 0x1C6, 0x100,
        0x8B, 0xCE,
        0xE8, 0x1D0, 0x1E0, 0x141, 0x00,
        0x80, 0x78, 0x17E, 0x00,
        0x75
    },
    0,
    {0xEB}
};

static const ProcessCtl::PatchData passOpenMarketPatch3
{
    // .text:005B7A1C 040 85 FF                                         test    edi, edi
    // .text:005B7A1E 040 7F 34                                         jg      short loc_5B7A54
    // .text:005B7A20 040 7C 04                                         jl      short loc_5B7A26
    // .text:005B7A22 040 85 DB                                         test    ebx, ebx
    // .text:005B7A24 040 75 2E                                         jnz     short loc_5B7A54
    // .text:005B7A26 040 85 C9                                         test    ecx, ecx
    // .text:005B7A28 040 7F 2A                                         jg      short loc_5B7A54
    // .text:005B7A2A 040 7C 04                                         jl      short loc_5B7A30
    // .text:005B7A2C 040 85 C0                                         test    eax, eax
    // .text:005B7A2E 040 75 24                                         jnz     short loc_5B7A54
    // .text:005B7A30 040 68 84 F0 00 01                                push    offset aDefault_txt_na ; "DEFAULT_Txt_Name"
    // .text:005B7A35 044 8B CE                                         mov     ecx, esi
    // .text:005B7A37 044 E8 F4 17 86 00                                call    sub_E19230
    // .text:005B7A3C 040 8A 40 7E                                      mov     al, [eax+7Eh]
    // .text:005B7A3F 040 84 C0                                         test    al, al
    //*.text:005B7A41 040 75 11                                         jnz     short loc_5B7A54
    // .text:005B7A43 040 68 0C A9 00 01                                push    offset aIdcancel ; "IDCANCEL"
    // 85 c9 7f ? 7c ? 85 c0 75 ?
    0x100000,
    0x200000,
    {
        0x85, 0xC9,
        0x7F, 0x12E,
        0x7C, 0x104,
        0x85, 0xC0,
        0x75, 0x128,
        0x68, 0x108, 0x10C, 0x1C6, 0x100,
        0x8B, 0xCE,
        0xE8, 0x1D0, 0x1E0, 0x141, 0x00,
        0x8A, 0x40, 0x17E,
        0x84, 0xC0,
        0x75
    },
    0,
    {0xEB}
};


static const ProcessCtl::BreakpointData replaceServerBp
{
    // int __stdcall GNET_init_thread(CECGameSession *arg_)
    // CECGameSession::DoOvertimeCheck, up fn
    //
    // .text:0074E8D0 51                                      push    ecx
    // .text:0074E8D1 A1 AC EF CC 00                          mov     eax, g_CECGame
    // .text:0074E8D6 55                                      push    ebp
    // .text:0074E8D7 56                                      push    esi
    // .text:0074E8D8 57                                      push    edi
    // .text:0074E8D9 8A 88 C4 02 00 00                       mov     cl, [eax+2C4h]
    // .text:0074E8DF 8D A8 C4 02 00 00                       lea     ebp, [eax+2C4h]   // CECGame.serverHost]
    // .text:0074E8E5 84 C9                                   test    cl, cl
    // .text:0074E8E7 74 49                                   jz      short loc_74E932
    // .text:0074E8E9 8B 88 C4 03 00 00                       mov     ecx, [eax+3C4h]   // serverPort
    // .text:0074E8EF 85 C9                                   test    ecx, ecx
    // .text:0074E8F1 74 3F                                   jz      short loc_74E932
    // .text:0074E8F3 8B 7C 24 14                             mov     edi, [esp+10h+arg_0]
    //
    0x300000,
    0x300000,
    {
        0x51,
        0xA1, 0x1AC, 0x1EF, 0x1CC, 0x100,
        0x55,
        0x56,
        0x57,
        0x8A, 0x88, 0x1C4, 0x02, 0x00, 0x00,
        0x8D, 0xA8, 0x1C4, 0x02, 0x00, 0x00,
        0x84, 0xC9,
        0x74, 0x149,
        0x8B, 0x88, 0x1C4, 0x03, 0x00, 0x00,
        0x85, 0xC9,
        0x74, 0x13F,
        0x8B,
    },
    0
};

static const ProcessCtl::BreakpointData replaceServerBp2
{
    // .text:007ECB10 000 51                                      push    ecx
    // .text:007ECB11 004 8B 0D B0 BA E4 00                       mov     ecx, g_CECGame
    // .text:007ECB17 004 80 B9 FC 02 00 00 00                    cmp     byte ptr [ecx+2FCh], 0
    // .text:007ECB1E 004 8D 81 FC 02 00 00                       lea     eax, [ecx+2FCh]
    // .text:007ECB24 004 56                                      push    esi
    // .text:007ECB25 008 57                                      push    edi
    // .text:007ECB26 00C 74 23                                   jz      short loc_7ECB4B
    // .text:007ECB28 00C 83 B9 FC 03 00 00 00                    cmp     dword ptr [ecx+3FCh], 0
    // .text:007ECB2F 00C 74 1A                                   jz      short loc_7ECB4B
    // .text:007ECB31 00C 0F B7 89 FC 03 00 00                    movzx   ecx, word ptr [ecx+3FCh]
    0x300000,
    0x300000,
    {
        0x51,
        0x8B, 0x0D, 0x1B0, 0x1BA, 0x1E4, 0x100,
        0x80, 0xB9, 0x1FC, 0x102, 0x00, 0x00, 0x00,
        0x8D, 0x81, 0x1FC, 0x102, 0x00, 0x00,
        0x56,
        0x57,
        0x74, 0x123,
        0x83, 0xB9, 0x1FC, 0x103, 0x00, 0x00, 0x00,
        0x74, 0x11A,
        0x0F,
    },
    0
};

static const ProcessCtl::BreakpointData replaceServerBp3
{
    // .text:00BC2A34 008 A1 00 D6 2A 01                                mov     eax, pCECGame
    // .text:00BC2A39 008 80 B8 B4 02 00 00 00                          cmp     byte ptr [eax+2B4h], 0 ; host
    // .text:00BC2A40 008 8D 88 B4 02 00 00                             lea     ecx, [eax+2B4h]
    // .text:00BC2A46 008 56                                            push    esi
    // .text:00BC2A47 00C 57                                            push    edi
    // .text:00BC2A48 010 74 28                                         jz      short loc_BC2A72
    // .text:00BC2A4A 010 83 B8 B4 03 00 00 00                          cmp     dword ptr [eax+3B4h], 0
    // .text:00BC2A51 010 74 1F                                         jz      short loc_BC2A72
    // .text:00BC2A53 010 FF B0 C0 04 00 00                             push    dword ptr [eax+4C0h] ; int
    // .text:00BC2A59 014 0F B7 80 B4 03 00 00                          movzx   eax, word ptr [eax+3B4h] ; port
    // .text:00BC2A60 014 8B 75 08                                      mov     esi, [ebp+lpThreadParameter]
    // .text:00BC2A63 014 50                                            push    eax             ; hostshort
    // .text:00BC2A64 018 51                                            push    ecx             ; name
    // .text:00BC2A65 01C 8B 8E AC 00 00 00                             mov     ecx, [esi+0ACh] ; int
    // .text:00BC2A6B 01C E8 70 FF FF FF                                call    sub_BC29E0
    // 51 a1 ? ? ? ? 80 b8 ? ? ? ? 00 8d 88 ? ? ? ?
    0x750000,
    0x200000,
    {
        0x51,
        0xA1, 0x1B0, 0x1BA, 0x1E4, 0x100,
        0x80, 0xB8, 0x1FC, 0x102, 0x00, 0x00, 0x00,
        0x8D, 0x88,
    },
    5
};

#if CLIENT_VERSION < 1520
static const ProcessCtl::BreakpointData replaceRoleBp {
    // .text:007099D4 8B F1                                   mov     esi, ecx
    // .text:007099D6 0F 84 3B 01 00 00                       jz      loc_709B17
    // .text:007099DC 8D 44 24 08                             lea     eax, [esp+0BCh+role]
    // .text:007099E0 C6 05 94 E2 C7 00 00                    mov     byte_C7E294, 0
    // .text:007099E7 50                                      push    eax
    // .text:007099E8 E8 F3 37 D0 FF                          call    get_cmg_value_role
    // .text:007099ED 83 C4 04                                add     esp, 4
    // .text:007099F0 8B 4C 24 08                             mov     ecx, [esp+0BCh+role]
    // .text:007099F4 A1 30 FC C9 00                          mov     eax, static_null_str
    // .text:007099F9 33 DB                                   xor     ebx, ebx
    // .text:007099FB 3B C8                                   cmp     ecx, eax
    // .text:007099FD 89 9C 24 B8 00 00 00                    mov     [esp+0BCh+var_4], ebx
    // .text:00709A04 75 2B                                  *jnz     short loc_709A31
    0x300000,
    0x100000,
    {
        0x8B, 0xF1, 0x0F, 0x84, 0x13B, 0x101, 0x00, 0x00,  0x8D, 0x44, 0x124, 0x108, 0xC6, 0x05, 0x194, 0x1E2,
        0x1C7, 0x00, 0x00, 0x50, 0xE8, 0x1F3, 0x137, 0x1D0,  0xFF, 0x83, 0xC4, 0x04, 0x8B, 0x4C, 0x124, 0x108,
        0xA1, 0x130, 0x1FC, 0x1C9, 0x00, 0x33, 0xDB, 0x3B,  0xC8, 0x89, 0x9C, 0x124, 0x1B8, 0x00, 0x00, 0x00,
        0x75
    },
    0
};
#else
static const ProcessCtl::BreakpointData replaceRoleBp
{
    // .text:0073E22E 8B F1                                   mov     esi, ecx
    // .text:0073E230 74 4E                                   jz      short loc_73E280
    // .text:0073E232 8D 44 24 04                             lea     eax, [esp+14h+var_10]
    // .text:0073E236 C6 05 5C BF CD 00 00                    mov     byte_CDBF5C, 0
    // .text:0073E23D 50                                      push    eax
    // .text:0073E23E E8 DD F0 CC FF                          call    get_cmg_value_role
    // .text:0073E243 83 C4 04                                add     esp, 4
    // .text:0073E246 8B 4C 24 04                             mov     ecx, [esp+14h+var_10]
    // .text:0073E24A A1 E8 ED CF 00                          mov     eax, static_null_str
    // .text:0073E24F 3B C8                                   cmp     ecx, eax
    // .text:0073E251 C7 44 24 10 00 00 00 00                 mov     [esp+14h+var_4], 0
    // .text:0073E259 74 14                                   jz      short loc_73E26F
    0x300000,
    0x250000,
    {
        0x8B, 0xF1, 0x74, 0x14E, 0x8D, 0x44, 0x124, 0x104, 0xC6, 0x05, 0x15C, 0x1BF, 0x1CD, 0x00, 0x00, 0x50,
        0xE8, 0x1DD, 0x1F0, 0x1CC, 0x1FF, 0x83, 0xC4, 0x04, 0x8B, 0x4C, 0x124, 0x104,
        0xA1, 0x1E8, 0x1ED, 0x1CF, 0x00,
        0x3B, 0xC8,
        0xC7, 0x44, 0x24, 0x110, 0x00, 0x00, 0x00, 0x00,
        0x74,
    },
    0
};
static const ProcessCtl::BreakpointData replaceRoleBp2
{
    // .text:005CA962 018 8B F1                                   mov     esi, ecx
    // .text:005CA964 018 80 3D F4 11 E0 00 00                    cmp     byte_E011F4, 0
    // .text:005CA96B 018 74 4D                                   jz      short loc_5CA9BA
    // .text:005CA96D 018 8D 44 24 08                             lea     eax, [esp+18h+var_10]
    // .text:005CA971 018 50                                      push    eax
    // .text:005CA972 01C C6 05 F4 11 E0 00 00                    mov     byte_E011F4, 0
    // .text:005CA979 01C E8 02 AB E5 FF                          call    get_cmg_value_role
    // .text:005CA97E 01C 83 C4 04                                add     esp, 4
    // .text:005CA981 018 8B 4C 24 08                             mov     ecx, [esp+18h+var_10]
    // .text:005CA985 018 3B 0D F0 D7 E1 00                       cmp     ecx, off_E1D7F0
    // .text:005CA98B 018 C7 44 24 14 00 00 00 00                 mov     [esp+18h+var_4], 0
    // .text:005CA993 018 74 14                                   jz      short loc_5CA9A9
    0x180000,
    0x200000,
    {
        0x8B, 0xF1,
        0x80, 0x3D, 0x1F4, 0x111, 0x1E0, 0x100, 0x00,
        0x74, 0x14D,
        0x8D, 0x44, 0x24, 0x108,
        0x50,
        0xC6, 0x05, 0x1F4, 0x111, 0x1E0, 0x100, 0x00,
        0xE8, 0x102, 0x1AB, 0x1E5, 0x1FF,
        0x83, 0xC4, 0x04,
        0x8B, 0x4C, 0x24, 0x108,
        0x3B, 0x0D, 0x1F0, 0x1D7, 0x1E1, 0x100,
        0xC7, 0x44, 0x24, 0x114, 0x00, 0x00, 0x00, 0x00,
        0x74,
    },
    0
};

static const ProcessCtl::BreakpointData replaceRoleBp3
{
    // .text:0061D1F2 018 8B F1                                         mov     esi, ecx
    // .text:0061D1F4 018 80 3D A4 F0 F7 00 00                          cmp     byte_F7F0A4, 0
    // .text:0061D1FB 018 74 4D                                         jz      short loc_61D24A
    // .text:0061D1FD 018 8D 44 24 08                                   lea     eax, [esp+18h+var_10]
    // .text:0061D201 018 50                                            push    eax
    // .text:0061D202 01C C6 05 A4 F0 F7 00 00                          mov     byte_F7F0A4, 0
    // .text:0061D209 01C E8 D2 25 E2 FF                                call    get_cmg_value_role
    // .text:0061D20E 01C 83 C4 04                                      add     esp, 4
    // .text:0061D211 018 8B 4C 24 08                                   mov     ecx, [esp+18h+var_10]
    // .text:0061D215 018 C7 44 24 14 00 00 00 00                       mov     [esp+18h+var_4], 0
    // .text:0061D21D 018 3B 0D E0 F1 F9 00                             cmp     ecx, static_null_str
    // .text:0061D223 018 74 14                                         jz      short loc_61D239
    0x180000,
    0x200000,
    {
        0x8B, 0xF1,
        0x80, 0x3D, 0x1F4, 0x111, 0x1E0, 0x100, 0x100,
        0x74, 0x14D,
        0x8D, 0x44, 0x24, 0x108,
        0x50,
        0xC6, 0x05, 0x1F4, 0x111, 0x1E0, 0x100, 0x100,
        0xE8, 0x102, 0x1AB, 0x1E5, 0x1FF,
        0x83, 0xC4, 0x04,
        0x8B, 0x4C, 0x24, 0x108,
        0xC7, 0x44, 0x24, 0x114, 0x00, 0x00, 0x00, 0x00,
        0x3B, 0x0D, 0x1F0, 0x1D7, 0x1E1, 0x100,
        0x74,
    },
    0
};

static const ProcessCtl::BreakpointData replaceRoleBp4
{
    // .text:00A60682 01C C6 05 08 28 1F 01 00                          mov     byte_11F2808, 0
    // .text:00A60689 01C 50                                            push    eax
    // .text:00A6068A 020 E8 E1 2E E4 FF                                call    get_cmg_value_role
    // .text:00A6068F 020 83 C4 04                                      add     esp, 4
    // .text:00A60692 01C 8B 45 F0                                      mov     eax, [ebp+var_10]
    // .text:00A60695 01C C7 45 FC 00 00 00 00                          mov     [ebp+var_4], 0
    // .text:00A6069C 01C 3B 05 44 64 1F 01                             cmp     eax, off_11F6444
    // .text:00A606A2 01C 74 13                                         jz      short loc_A606B7
    0x600000,
    0x200000,
    {
        0xC6, 0x05, 0x1F4, 0x111, 0x1E0, 0x100, 0x00,
        0x50,
        0xE8, 0x102, 0x1AB, 0x1E5, 0x1FF,
        0x83, 0xC4, 0x04,
        0x8B, 0x45, 0xF0,
        0xC7, 0x45, 0x124, 0x114, 0x00, 0x00, 0x00,
        0x3B, 0x05, 0x1F0, 0x1D7, 0x1E1, 0x100,
        0x74,
    },
    0
};

static const ProcessCtl::BreakpointData replaceRoleStepBp
{
    // .text:0073E334 0C0 6A 01                                   push    1
    // .text:0073E336 0C4 50                                      push    eax             ; int
    // .text:0073E337 0C8 51                                      push    ecx             ; Str
    // .text:0073E338 0CC C7 84 24 C8 00 00 00 00+                mov     [esp+0CCh+var_4], 0
    // .text:0073E343 0CC E8 A8 54 25 00                          call    _9937F0_escapeString
    // .text:0073E348 0CC 83 C4 0C                                add     esp, 0Ch
    // .text:0073E34B 0C0 8D 54 24 14                             lea     edx, [esp+0C0h+Str2]
    // .text:0073E34F 0C0 8B CD                                   mov     ecx, ebp
    // .text:0073E351 0C0 52                                      push    edx             ; Str2
    // .text:0073E352 0C4 E8 09 14 2D 00                          call    Str_wcsicmp
    0x300000,
    0x200000,
    {
        0x6A, 0x01, 0x50, 0x51, 0xC7, 0x84, 0x24, 0x1C8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE8,
        0x1A8, 0x154, 0x125, 0x00, 0x83, 0xC4, 0x0C, 0x8D, 0x54, 0x24, 0x114, 0x8B, 0xCD, 0x52, 0xE8,
    },
    0
};
static const ProcessCtl::BreakpointData replaceRoleStepBp2
{
    // .text:005C9A44 0C8 6A 01                                   push    1
    // .text:005C9A46 0CC 8D 44 24 1C                             lea     eax, [esp+0CCh+Str2]
    // .text:005C9A4A 0CC 50                                      push    eax
    // .text:005C9A4B 0D0 51                                      push    ecx
    // .text:005C9A4C 0D4 C7 84 24 D0 00 00 00 00 00 00 00        mov     [esp+0D4h+var_4], 0
    // .text:005C9A57 0D4 E8 B4 A9 42 00                          call    escapeString
    // .text:005C9A5C 0D4 83 C4 0C                                add     esp, 0Ch
    // .text:005C9A5F 0C8 8D 54 24 18                             lea     edx, [esp+0C8h+Str2]
    // .text:005C9A63 0C8 52                                      push    edx             ; Str2
    // .text:005C9A64 0CC 8B CD                                   mov     ecx, ebp
    // .text:005C9A66 0CC E8 85 A0 47 00                          call    Str_wcsicmp
    0x600000,
    0x200000,
    {
        0x6A, 0x01,
        0x8D, 0x44, 0x24, 0x11C,
        0x50,
        0x51,
        0xC7, 0x84, 0x24, 0x1D0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xE8, 0x1B4, 0x1A9, 0x142, 0x00,
        0x83, 0xC4, 0x0C,
        0x8D, 0x54, 0x24, 0x118,
        0x52,
        0x8B, 0xCD,
        0xE8
    },
    0
};
static const ProcessCtl::BreakpointData replaceRoleStepBp3
{
    // .text:00A61736 0D8 E8 45 81 3A 00                                call    escapeString
    // .text:00A6173B 0D8 8B 8D 48 FF FF FF                             mov     ecx, [ebp+var_B8]
    // .text:00A61741 0D8 8D 85 50 FF FF FF                             lea     eax, [ebp+Str2]
    // .text:00A61747 0D8 83 C4 0C                                      add     esp, 0Ch
    // .text:00A6174A 0CC 50                                            push    eax             ; Str2
    // .text:00A6174B 0D0 E8 A0 FD 23 00                                call    Str_wcsicmp
    // .text:00A61750 0CC 8D 8D 4C FF FF FF                             lea     ecx, [ebp+var_B4]
    // e8 ? ? ? ? 8b 8d ? ? ? ? 8d 85 ? ? ? ? 83 c4 0c 50 e8 ? ? ? ? 8d 8d
    0x600000,
    0x200000,
    {
        0xE8, 0x1B4, 0x1A9, 0x142, 0x00,
        0x8B, 0x8D, 0x154, 0x124, 0x118, 0xFF,
        0x8D, 0x85, 0x154, 0x124, 0x118, 0xFF,
        0x83, 0xC4, 0x0C,
        0x50,
        0xE8, 0x100, 0x100, 0x100, 0x100,
        0x8D, 0x8D
    },
    -6
};

#endif

static const ProcessCtl::BreakpointData createWindowBp
{
    // .text:00453130 020 6A 00                                   push    0
    // .text:00453132 024 52                                      push    edx
    // .text:00453133 028 6A 00                                   push    0
    // .text:00453135 02C 6A 00                                   push    0
    // .text:00453137 030 55                                      push    ebp
    // .text:00453138 034 57                                      push    edi
    // .text:00453139 038 6A 00                                   push    0
    // .text:0045313B 03C 6A 00                                   push    0
    // .text:0045313D 040 56                                      push    esi
    // .text:0045313E 044 50                                      push    eax
    // .text:0045313F 048 A1 B8 5A C5 00                          mov     eax, wndClassName
    // .text:00453144 048 50                                      push    eax
    // .text:00453145 04C 53                                      push    ebx
    // .text:00453146 050 FF 15 28 EB CC 00                       call    oCreateWindowExW
    // .text:0045314C 020 33 C9                                   xor     ecx, ecx
    0x050000,
    0x100000,
    {
        0x6A, 0x00, 0x52, 0x6A, 0x00, 0x6A,
        0x00, 0x55, 0x57, 0x6A, 0x00, 0x6A, 0x00, 0x56, 0x50, 0xA1, 0x1B8, 0x15A, 0x1C5, 0x00, 0x50, 0x53,
        0xFF, 0x15, 0x128, 0x1EB, 0x1CC, 0x00, 0x33,
    },
    0
};

static const ProcessCtl::BreakpointData createWindowBp2
{
    // .text:0041560E 024 6A 00                                   push    0
    // .text:00415610 028 53                                      push    ebx
    // .text:00415611 02C 6A 00                                   push    0
    // .text:00415613 030 6A 00                                   push    0
    // .text:00415615 034 55                                      push    ebp
    // .text:00415616 038 57                                      push    edi
    // .text:00415617 03C 6A 00                                   push    0
    // .text:00415619 040 6A 00                                   push    0
    // .text:0041561B 044 56                                      push    esi
    // .text:0041561C 048 52                                      push    edx
    // .text:0041561D 04C 50                                      push    eax
    // .text:0041561E 050 51                                      push    ecx
    // .text:0041561F 054 FF 15 10 BA E4 00                       call    oCreateWindowExW
    // .text:00415625 024 33 C9                                   xor     ecx, ecx
    0x000000,
    0x100000,
    {
        0x6A, 0x00, 0x53, 0x6A, 0x00, 0x6A,
        0x00, 0x55, 0x57, 0x6A, 0x00, 0x6A, 0x00, 0x56, 0x52, 0x50, 0x51,
        0xFF, 0x15, 0x128, 0x1EB, 0x1CC, 0x100,
        0x33,
    },
    0
};

static const ProcessCtl::BreakpointData createWindowBp3
{
    // .text:00BA0EF0 034 6A 00                                         push    0               ; lpParam
    // .text:00BA0EF2 038 50                                            push    eax             ; hInstance
    // .text:00BA0EF3 03C 6A 00                                         push    0               ; hMenu
    // .text:00BA0EF5 040 6A 00                                         push    0               ; hWndParent
    // .text:00BA0EF7 044 52                                            push    edx             ; nHeight
    // .text:00BA0EF8 048 51                                            push    ecx             ; nWidth
    // .text:00BA0EF9 04C 6A 00                                         push    0               ; Y
    // .text:00BA0EFB 050 6A 00                                         push    0               ; X
    // .text:00BA0EFD 054 56                                            push    esi             ; dwStyle
    // .text:00BA0EFE 058 FF 35 FC 38 2B 01                             push    lpWideCharStr   ; lpWindowName
    // .text:00BA0F04 05C FF 35 C4 3E 1F 01                             push    lpClassName     ; lpClassName
    // .text:00BA0F0A 060 57                                            push    edi             ; dwExStyle
    // .text:00BA0F0B 064 FF 15 B8 F7 FF 00                             call    ds:CreateWindowExW
    // .text:00BA0F11 034 85 C0                                         test    eax, eax
    0x7A0000,
    0x200000,
    {
        0x6A, 0x00, 0x50, 0x6A, 0x00, 0x6A, 0x00,
        0x52, 0x51, 0x6A, 0x00, 0x6A, 0x00, 0x56,
        0xFF, 0x35, 0x128, 0x1EB, 0x1CC, 0x100,
        0xFF, 0x35, 0x128, 0x1EB, 0x1CC, 0x100,
        0x57,
        0xFF, 0x15, 0x128, 0x1EB, 0x1CC, 0x100,
        0x85,
    },
    0
};

static const ProcessCtl::PatchData createWindowSkipShowPatch
{
    // .text:004531BE 024 D1 F8                                   sar     eax, 1
    // .text:004531C0 024 57                                      push    edi             ; nHeight
    // .text:004531C1 028 56                                      push    esi             ; nWidth
    // .text:004531C2 02C 50                                      push    eax             ; Y
    // .text:004531C3 030 55                                      push    ebp             ; X
    // .text:004531C4 034 52                                      push    edx             ; hWnd
    // .text:004531C5 038 FF 15 DC B7 B7 00                       call    ds:MoveWindow
    // .text:004531CB 020 8B 44 24 28                             mov     eax, [esp+20h+nCmdShow]
    // .text:004531CF 020 8B 0D 7C 02 CD 00                       mov     ecx, hWnd
    //*.text:004531D5 020 50                                      push    eax             ; nCmdShow
    // .text:004531D6 024 51                                      push    ecx             ; hWnd
    // .text:004531D7 028 FF 15 F4 B7 B7 00                       call    ds:ShowWindow
    // .text:004531DD 020 8B 15 7C 02 CD 00                       mov     edx, hWnd
    // .text:004531E3 020 52                                      push    edx             ; hWnd
    // .text:004531E4 024 FF 15 E0 B7 B7 00                       call    ds:UpdateWindow
    // .text:004531EA 020 A1 7C 02 CD 00                          mov     eax, hWnd
    // .text:004531EF 020 50                                      push    eax             ; hWnd
    // .text:004531F0 024 FF 15 D0 B6 B7 00                       call    ds:SetForegroundWindow
    // .text:004531F6 020 5F                                      pop     edi
    // .text:004531F7 01C 5E                                      pop     esi
    // .text:004531F8 018 5D                                      pop     ebp
    // .text:004531F9 014 B0 01                                   mov     al, 1
    0x050000,
    0x100000,
    {
        0xD1, 0xF8,
        0x57,
        0x56,
        0x50,
        0x55,
        0x52,
        0xFF, 0x15, 0x1DC, 0x1B7, 0x1B7, 0x00,
        0x8B, 0x44, 0x24, 0x28,
        0x8B, 0x0D, 0x17C, 0x102, 0x1CD, 0x100,
        0x50,
        0x51,
        0xFF, 0x15, 0x1F4, 0x1B7, 0x1B7, 0x00,
        0x8B, 0x15, 0x17C, 0x102, 0x1CD, 0x100,
        0x52,
        0xFF, 0x15, 0x1E0, 0x1B7, 0x1B7, 0x00,
        0xA1, 0x17C, 0x102, 0x1CD, 0x100,
        0x50,
        0xFF, 0x15, 0x1D0, 0x1B6, 0x1B7, 0x00,
        0x5F,
        0x5E,
        0x5D,
        0xB0, 0x01
    },
    -0x25,      // 004531D5
    { 0xEB, 0x1F }
};

static const ProcessCtl::PatchData createWindowSkipShowPatch2
{
    // .text:0041567F 028 57                                      push    edi             ; nHeight
    // .text:00415680 02C 2B C2                                   sub     eax, edx
    // .text:00415682 02C 56                                      push    esi             ; nWidth
    // .text:00415683 030 D1 F8                                   sar     eax, 1
    // .text:00415685 030 50                                      push    eax             ; Y
    // .text:00415686 034 A1 18 C4 E4 00                          mov     eax, hWnd
    // .text:0041568B 034 55                                      push    ebp             ; X
    // .text:0041568C 038 50                                      push    eax             ; hWnd
    // .text:0041568D 03C FF 15 A8 F8 C7 00                       call    ds:MoveWindow
    // .text:00415693 024 8B 4C 24 28                             mov     ecx, [esp+24h+nCmdShow]
    // .text:00415697 024 8B 15 18 C4 E4 00                       mov     edx, hWnd
    //*.text:0041569D 024 51                                      push    ecx             ; nCmdShow
    // .text:0041569E 028 52                                      push    edx             ; hWnd
    // .text:0041569F 02C FF 15 C0 F8 C7 00                       call    ds:ShowWindow
    // .text:004156A5 024 A1 18 C4 E4 00                          mov     eax, hWnd
    // .text:004156AA 024 50                                      push    eax             ; hWnd
    // .text:004156AB 028 FF 15 AC F8 C7 00                       call    ds:UpdateWindow
    // .text:004156B1 024 8B 0D 18 C4 E4 00                       mov     ecx, hWnd
    // .text:004156B7 024 51                                      push    ecx             ; hWnd
    // .text:004156B8 028 FF 15 B0 F8 C7 00                       call    ds:SetForegroundWindow
    // .text:004156BE 024 5F                                      pop     edi
    // .text:004156BF 020 5E                                      pop     esi
    // .text:004156C0 01C 5D                                      pop     ebp
    // .text:004156C1 018 B0 01                                   mov     al, 1
    // .text:004156C3 018 5B                                      pop     ebx
    // .text:004156C4 014 83 C4 14                                add     esp, 14h
    // .text:004156C7 000 C3                                      retn
    0x010000,
    0x100000,
    {
        0x57,
        0x2B, 0xC2,
        0x56,
        0xD1, 0xF8,
        0x50,
        0xA1, 0x118, 0x1C4, 0x1E4, 0x100,
        0x55,
        0x50,
        0xFF, 0x15, 0x1A8, 0x1F8, 0x1C7, 0x100,
        0x8B, 0x4C, 0x124, 0x28,
        0x8B, 0x15, 0x118, 0x1C4, 0x1E4, 0x100,
        0x51,
        0x52,
        0xFF, 0x15, 0x1C0, 0x1F8, 0x1C7, 0x100,
        0xA1, 0x118, 0x1C4, 0x1E4, 0x100,
        0x50,
        0xFF, 0x15, 0x1AC, 0x1F8, 0x1C7, 0x100,
        0x8B, 0x0D, 0x118, 0x1C4, 0x1E4, 0x100,
        0x51,
        0xFF, 0x15, 0x1B0, 0x1F8, 0xC17, 0x100,
        0x5F,
        0x5E,
        0x5D,
        0xB0, 0x01,
    },
    0x0041569D - 0x004156C2,      // 0041569D
    { 0xEB, 0x1F }
};

static const ProcessCtl::PatchData createWindowSkipShowPatch3
{
    // .text:00BA0F8B 034 FF 75 0C                                      push    [ebp+nCmdShow]  ; nCmdShow
    // .text:00BA0F8E 038 50                                            push    eax             ; hWnd
    // .text:00BA0F8F 03C FF 15 78 F8 FF 00                             call    ds:ShowWindow
    // .text:00BA0F95 034 FF 35 F8 40 2B 01                             push    hWnd            ; hWnd
    // .text:00BA0F9B 038 FF 15 AC F7 FF 00                             call    ds:UpdateWindow
    // .text:00BA0FA1 034 FF 35 F8 40 2B 01                             push    hWnd            ; hWnd
    // .text:00BA0FA7 038 FF 15 A8 F7 FF 00                             call    ds:SetForegroundWindow
    // .text:00BA0FAD 034 8B 4D FC                                      mov     ecx, [ebp+var_4]
    // .text:00BA0FB0 034 B0 01                                         mov     al, 1
    // 50 ff 15 ? ? ? ? ff 35 ? ? ? ? ff 15 ? ? ? ? ff 35 ? ? ? ? ff 15 ? ? ? ?
    0x780000,
    0x200000,
    {
        0x50,
        0xFF, 0x15, 0x1A8, 0x1F8, 0x1C7, 0x100,
        0xFF, 0x35, 0x118, 0x1C4, 0x1E4, 0x100,
        0xFF, 0x15, 0x1A8, 0x1F8, 0x1C7, 0x100,
        0xFF, 0x35, 0x118, 0x1C4, 0x1E4, 0x100,
        0xFF, 0x15, 0x1A8, 0x1F8, 0x1C7, 0x100,
        0x8B,
    },
    0x00BA0F8B - 0x00BA0FAD,
    { 0xEB, 0x20 }
};

static const ProcessCtl::BreakpointData closeWindowBp
{
    // need to skip creating tray icon
    //
    // .text:0043C8E7 0DC 6A 06                                   push    6               ; nCmdShow
    // .text:0043C8E9 0E0 56                                      push    esi             ; hWnd
    // .text:0043C8EA 0E4 FF D7                                   call    edi ; ShowWindow
    // .text:0043C8EC 0DC 6A 00                                   push    0               ; nCmdShow
    // .text:0043C8EE 0E0 56                                      push    esi             ; hWnd
    // .text:0043C8EF 0E4 FF D7                                   call    edi ; ShowWindow
    // .text:0043C8F1 0DC 81 C5 4C 05 00 00                       add     ebp, 54Ch
    // .text:0043C8F7 0DC 55                                      push    ebp
    // .text:0043C8F8 0E0 6A 00                                   push    0
    // .text:0043C8FA 0E4 FF 15 18 EB CC 00                       call    oShell_NotifyIconW
    // 
    // .text:00905E5F 0DC 8B 35 78 F8 FF 00                             mov     esi, ds:ShowWindow
    // .text:00905E65 0DC 74 05                                         jz      short loc_905E6C
    // .text:00905E67 0DC 6A 06                                         push    6               ; nCmdShow
    // .text:00905E69 0E0 57                                            push    edi             ; hWnd
    // .text:00905E6A 0E4 FF D6                                         call    esi ; ShowWindow
    // .text:00905E6C 0DC 6A 00                                         push    0               ; nCmdShow
    // .text:00905E6E 0E0 57                                            push    edi             ; hWnd
    // .text:00905E6F 0E4 FF D6                                         call    esi ; ShowWindow
    // .text:00905E71 0DC 8D 83 40 05 00 00                             lea     eax, [ebx+540h]
    // .text:00905E77 0DC 50                                            push    eax
    // .text:00905E78 0E0 6A 00                                         push    0
    // .text:00905E7A 0E4 E9 14 03 00 00                                jmp     loc_906193
    // .text:00906193 0E4 FF 15 E8 F5 FF 00                             call    ds:Shell_NotifyIconW

    // 6a 06 ? ff ? 6a 00 ? ff ?
    0x000000,
    0x650000,
    {
        0x6A, 0x06, 0x156, 0xFF, 0x1D7,
        0x6A, 0x00, 0x156, 0xFF,
    },
    9 // 0043C8F8
};

static const ProcessCtl::BreakpointData adjustSettingsBp
{
    // .text:0040E6B6 03C 50                                      push    eax             ; client_id
    // .text:0040E6B7 040 51                                      push    ecx             ; ec
    // .text:0040E6B8 044 8B CF                                   mov     ecx, edi        ; this_
    // .text:0040E6BA 044 89 7C 24 20                             mov     [esp+44h+this__], edi
    // .text:0040E6BE 044 E8 6D 05 00 00                          call    CECConfigs__LoadConfigs
    // .text:0040E6C3 03C 84 C0                                   test    al, al
    // .text:0040E6C5 03C 75 07                                   jnz     short loc_40E6CE
    // .text:0040E6C7 03C 68 A7 00 00 00                          push    0A7h
    // .text:0040E6CC 040 EB 3C                                   jmp     short loc_40E70A
    // .text:0040E6CE 03C 8B 54 24 48                             mov     edx, [esp+3Ch+systemsettings]
    // .text:0040E6D2 03C 8B CF                                   mov     ecx, edi
    // .text:0040E6D4 03C 52                                      push    edx
    // .text:0040E6D5 040 E8 36 0F 00 00                          call    CECConfigs__LoadSystemSettings
    0x000000,
    0x100000,
    {
        0x50, 0x51, 0x8B, 0xCF, 0x89, 0x7C, 0x24, 0x120, 0xE8, 0x16D, 0x105, 0x100, 0x00, 0x84, 0xC0, 0x75,
        0x07, 0x68, 0x1A7, 0x00, 0x00, 0x00, 0xEB, 0x3C, 0x8B, 0x54, 0x24, 0x48, 0x8B, 0xCF, 0x52, 0xE8
    },
    0
};

static const ProcessCtl::BreakpointData adjustSettingsBp2
{
    // .text:0042AA93 038 68 88 6E C8 00                          push    offset aCecconfigsInit ; "CECConfigs::Init"
    // .text:0042AA98 03C 6A 04                                   push    4
    // .text:0042AA9A 040 E8 91 8D FE FF                          call    sub_413830
    // .text:0042AA9F 040 83 C4 0C                                add     esp, 0Ch
    // .text:0042AAA2 034 32 C0                                   xor     al, al
    // .text:0042AAA4 034 E9 43 01 00 00                          jmp     loc_42ABEC
    // .text:0042AAA9 034 8B 54 24 40                             mov     edx, dword ptr [esp+34h+ArgList]
    // .text:0042AAAD 034 52                                      push    edx             ; ArgList
    // .text:0042AAAE 038 8B CB                                   mov     ecx, ebx
    // .text:0042AAB0 038 E8 8B C6 FF FF                          call    CECConfigs__LoadSystemSettings
    0x000000,
    0x100000,
    {
        0x68, 0x188, 0x16E, 0x1C8, 0x100,
        0x6A, 0x04,
        0xE8, 0x191, 0x18D, 0x1FE, 0xFF,
        0x83, 0xC4, 0x0C,
        0x32, 0xC0,
        0xE9, 0x143, 0x101, 0x00, 0x00,
        0x8B, 0x54, 0x24, 0x40,
        0x52,
        0x8B, 0xCB,
        0xE8,
    },
    0
};
static const ProcessCtl::BreakpointData adjustSettingsBp3
{
    // .text:008A53DC 040 59                                            pop     ecx
    // .text:008A53DD 03C 5F                                            pop     edi
    // .text:008A53DE 038 5E                                            pop     esi
    // .text:008A53DF 034 5B                                            pop     ebx
    // .text:008A53E0 030 8B E5                                         mov     esp, ebp
    // .text:008A53E2 004 5D                                            pop     ebp
    // .text:008A53E3 000 C2 0C 00                                      retn    0Ch
    // .text:008A53E6 040 FF 75 10                                      push    [ebp+arg_8]
    // .text:008A53E9 044 8B CE                                         mov     ecx, esi
    // .text:008A53EB 044 E8 A0 0C 00 00                                call    CECConfigs__LoadSystemSettings
    // .text:008A53F0 040 84 C0                                         test    al, al
    // .text:008A53F2 040 75 07                                         jnz     short loc_8A53FB
    // .text:008A53F4 040 68 DE 00 00 00                                push    0DEh
    // .text:008A53F9 044 EB C6                                         jmp     short loc_8A53C1
    // c2 0c 00 ff 75 10 8b ce e8 ? ? ? ?
    0x4A0000,
    0x150000,
    {
        0xC2, 0x0C, 0x00,
        0xff, 0x75, 0x10,
        0x8b, 0xce,
        0xE8,
    },
    0
};

static const ProcessCtl::PatchData noSaveSettingsPatch
{
    // search "systemsettings.ini"
    // .text:0040FC60                             CECConfigs__SaveSettings proc near      ; CODE XREF: sub_609110+101p
    // .text:0040FC60 000 6A FF                                   push    0FFFFFFFFh
    // .text:0040FC62 004 68 F6 39 B2 00                          push    offset SEH_40FC60
    // .text:0040FC67 008 64 A1 00 00 00 00                       mov     eax, large fs:0
    // .text:0040FC6D 008 50                                      push    eax
    // .text:0040FC6E 00C 64 89 25 00 00 00 00                    mov     large fs:0, esp
    // .text:0040FC75 00C 81 EC 54 08 00 00                       sub     esp, 854h
    // .text:0040FC7B 860 56                                      push    esi
    // .text:0040FC7C 864 8B F1                                   mov     esi, ecx
    // .text:0040FC7E 864 57                                      push    edi
    // .text:0040FC7F 868 8D 4C 24 0C                             lea     ecx, [esp+868h+var_85C]
    // .text:0040FC83 868 E8 08 0B 5C 00                          call    sub_9D0790
    // .text:0040FC88 868 68 B0 E6 C4 00                          push    offset aInfo    ; "Info"
    // .text:0040FC8D 86C 8D 4C 24 0C                             lea     ecx, [esp+86Ch+var_860]

    0x000000,
    0x050000,
    {
        0x6A, 0xFF, 0x68, 0x1F6, 0x139, 0x1B2, 0x00, 0x64, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x50, 0x64, 0x89,
        0x25, 0x00, 0x00, 0x00, 0x00, 0x81, 0xEC, 0x154, 0x108, 0x00, 0x00, 0x56, 0x8B, 0xF1, 0x57, 0x8D,
        0x4C, 0x124, 0x10C, 0xE8, 0x108, 0x10B, 0x15C, 0x00, 0x68, 0x1B0, 0x1E6, 0x1C4, 0x00, 0x8D, 0x4C
    },
    0x0040FC60 - 0x0040FC8E,
    { 0xC3 }
};
static const ProcessCtl::PatchData noSaveSettingsPatch2
{
    // search "systemsettings.ini"
    // .text:00426380 000 6A FF                                   push    0FFFFFFFFh
    // .text:00426382 004 68 C6 8F B9 00                          push    offset loc_B98FC6
    // .text:00426387 008 64 A1 00 00 00 00                       mov     eax, large fs:0
    // .text:0042638D 008 50                                      push    eax
    // .text:0042638E 00C 81 EC 58 08 00 00                       sub     esp, 858h
    // .text:00426394 864 A1 48 64 E4 00                          mov     eax, dword_E46448
    // .text:00426399 864 33 C4                                   xor     eax, esp
    // .text:0042639B 864 89 84 24 54 08 00 00                    mov     [esp+864h+var_10], eax
    // .text:004263A2 864 56                                      push    esi
    // .text:004263A3 868 57                                      push    edi
    // .text:004263A4 86C A1 48 64 E4 00                          mov     eax, dword_E46448
    // .text:004263A9 86C 33 C4                                   xor     eax, esp
    // .text:004263AB 86C 50                                      push    eax
    // .text:004263AC 870 8D 84 24 64 08 00 00                    lea     eax, [esp+870h+var_C]
    // .text:004263B3 870 64 A3 00 00 00 00                       mov     large fs:0, eax
    0x000000,
    0x100000,
    {
        0x6A, 0xFF,
        0x68, 0x1C6, 0x18F, 0x1B9, 0x100,
        0x64, 0xA1, 0x00, 0x00, 0x00, 0x00,
        0x50,
        0x81, 0xEC, 0x158, 0x108, 0x00, 0x00,
        0xA1, 0x148, 0x164, 0x1E4, 0x100,
        0x33, 0xC4,
        0x89, 0x84, 0x24, 0x154, 0x108, 0x00, 0x00,
        0x56,
        0x57,
        0xA1, 0x148, 0x164, 0x1E4, 0x100,
        0x33, 0xC4,
        0x50,
        0x8D, 0x84, 0x24, 0x164, 0x108, 0x00, 0x00,
        0x64, 0xA3, 0x00, 0x00, 0x00, 0x00,
    },
    0x00426380 - 0x004263B8,
    { 0xC3 }
};

static const ProcessCtl::PatchData noSaveSettingsPatch3
{
    // .text:008A7930 000 55                                            push    ebp
    // .text:008A7931 004 8B EC                                         mov     ebp, esp
    // .text:008A7933 004 6A FF                                         push    0FFFFFFFFh
    // .text:008A7935 008 68 96 21 FA 00                                push    offset sub_FA2196
    // .text:008A793A 00C 64 A1 00 00 00 00                             mov     eax, large fs:0
    // .text:008A7940 00C 50                                            push    eax
    // .text:008A7941 010 81 EC 78 08 00 00                             sub     esp, 878h
    // .text:008A7947 888 A1 38 81 25 01                                mov     eax, ___security_cookie
    // .text:008A794C 888 33 C5                                         xor     eax, ebp
    // .text:008A794E 888 89 45 F0                                      mov     [ebp+var_10], eax
    // .text:008A7951 888 56                                            push    esi
    // .text:008A7952 88C 50                                            push    eax
    // .text:008A7953 890 8D 45 F4                                      lea     eax, [ebp+var_C]
    // .text:008A7956 890 64 A3 00 00 00 00                             mov     large fs:0, eax
    // .text:008A795C 890 8B F1                                         mov     esi, ecx
    // .text:008A795E 890 8D 8D 7C F7 FF FF                             lea     ecx, [ebp+var_884]
    // .text:008A7964 890 E8 D7 21 41 00                                call    sub_CB9B40
    // .text:008A7969 890 68 84 5B 05 01                                push    offset aInfo    ; "Info"
    // .text:008A796E 894 8D 8D A0 F7 FF FF                             lea     ecx, [ebp+var_860]
    // .text:008A7974 894 C7 45 FC 00 00 00 00                          mov     [ebp+var_4], 0
    // .text:008A797B 894 E8 C0 75 3F 00                                call    sub_C9EF40

    // 55 8b ec 6a ff 68 ? ? ? ? 64 a1 0 0 0 0 50 81 ec ? ? 0 0 a1 ? ? ? ? 33 c5 89 45 F0 56 50 8d 45 F4 64 a3 0 0 0 0 8b f1 8d 8d ? ? ? ? e8 ? ? ? ? 68
    0x450000,
    0x200000,
    {
        0x55,
        0x8b, 0xEC,
        0x6A, 0xFF,
        0x68, 0x1C6, 0x18F, 0x1B9, 0x100,
        0x64, 0xA1, 0x00, 0x00, 0x00, 0x00,
        0x50,
        0x81, 0xEC, 0x158, 0x108, 0x00, 0x00,
        0xA1, 0x148, 0x164, 0x1E4, 0x100,
        0x33, 0xC5,
        0x89, 0x45, 0xf0,
        0x56,
        0x50,
        0x8D, 0x45, 0xF4,
        0x64, 0xA3, 0x00, 0x00, 0x00, 0x00,
        0x8B, 0xF1,
        0x8D, 0x8D, 0x124, 0x164, 0x108, 0x100,
        0xE8, 0x124, 0x164, 0x108, 0x100,
        0x68, 0x124, 0x164, 0x108, 0x100,
        0x8D, 0x8D, 0x124, 0x164, 0x108, 0x100,
        0xC7, 0x45, 0x124, 0x164, 0x108, 0x100, 0x00,
        0xE8,
    },
    0x008A7930 - 0x008A797B,
    { 0xC3 }
};

static const ProcessCtl::PatchData noSaveSettingsPatch4
{
    // .text:0094B2F0 000 55                                            push    ebp
    // .text:0094B2F1 004 8B EC                                         mov     ebp, esp
    // .text:0094B2F3 004 6A FF                                         push    0FFFFFFFFh
    // .text:0094B2F5 008 68 96 9C 06 01                                push    offset SEH_94B2F0
    // .text:0094B2FA 00C 64 A1 00 00 00 00                             mov     eax, large fs:0
    // .text:0094B300 00C 50                                            push    eax
    // .text:0094B301 010 81 EC 78 08 00 00                             sub     esp, 878h
    // .text:0094B307 888 A1 28 04 34 01                                mov     eax, ___security_cookie
    // .text:0094B30C 888 33 C5                                         xor     eax, ebp
    // .text:0094B30E 888 89 45 F0                                      mov     [ebp+var_10], eax
    // .text:0094B311 888 53                                            push    ebx
    // .text:0094B312 88C 56                                            push    esi
    // .text:0094B313 890 57                                            push    edi
    // .text:0094B314 894 50                                            push    eax
    // .text:0094B315 898 8D 45 F4                                      lea     eax, [ebp+var_C]
    // .text:0094B318 898 64 A3 00 00 00 00                             mov     large fs:0, eax
    // .text:0094B31E 898 8B D9                                         mov     ebx, ecx
    // .text:0094B320 898 8D 8D 7C F7 FF FF                             lea     ecx, [ebp+var_884]
    // .text:0094B326 898 E8 05 C5 42 00                                call    sub_D77830
    // .text:0094B32B 898 68 3C 20 12 01                                push    offset aInfo    ; "Info"
    // .text:0094B330 89C 8D 8D A0 F7 FF FF                             lea     ecx, [ebp+var_860]
    // .text:0094B336 89C C7 45 FC 00 00 00 00                          mov     [ebp+var_4], 0
    // .text:0094B33D 89C E8 1E 18 41 00                                call    sub_D5CB60
    // .text:0094B342 898 A1 E8 84 39 01                                mov     eax, pCECGame
    0x450000,
    0x200000,
    {
        0x55,
        0x8b, 0xEC,
        0x6A, 0xFF,
        0x68, 0x1C6, 0x18F, 0x1B9, 0x100,
        0x64, 0xA1, 0x00, 0x00, 0x00, 0x00,
        0x50,
        0x81, 0xEC, 0x158, 0x108, 0x00, 0x00,
        0xA1, 0x148, 0x164, 0x1E4, 0x100,
        0x33, 0xC5,
        0x89, 0x45, 0xf0,
        0x53,
        0x56,
        0x57,
        0x50,
        0x8D, 0x45, 0xF4,
        0x64, 0xA3, 0x00, 0x00, 0x00, 0x00,
        0x8B, 0x1F1,
        0x8D, 0x8D, 0x124, 0x164, 0x108, 0x100,
        0xE8, 0x124, 0x164, 0x108, 0x100,
        0x68, 0x124, 0x164, 0x108, 0x100,
        0x8D, 0x8D, 0x124, 0x164, 0x108, 0x100,
        0xC7, 0x45, 0x124, 0x164, 0x108, 0x100, 0x00,
        0xE8,
    },
    0x0094B2F0 - 0x0094B33D,
    { 0xC3 }
};

static const ProcessCtl::PatchData noMediaPatch
{
    // .text:009CDE20                             AMSoundStream__Play__ proc near         ; CODE XREF: sub_817A00+6Ep
    // .text:009CDE20 000 56                                      push    esi
    // .text:009CDE21 004 57                                      push    edi             ; ArgList
    // .text:009CDE22 008 8B 7C 24 0C                             mov     edi, [esp+8+Str2]
    // .text:009CDE26 008 8B F1                                   mov     esi, ecx
    // .text:009CDE28 008 85 FF                                   test    edi, edi
    // .text:009CDE2A 008 74 44                                   jz      short loc_9CDE70
    // .text:009CDE2C 008 8D 46 3E                                lea     eax, [esi+3Eh]
    // .text:009CDE2F 008 57                                      push    edi             ; Str2
    // .text:009CDE30 00C 50                                      push    eax             ; Str1
    // .text:009CDE31 010 FF 15 60 B4 B7 00                       call    ds:_stricmp
    // .text:009CDE37 010 83 C4 08                                add     esp, 8
    // .text:009CDE3A 008 85 C0                                   test    eax, eax
    // .text:009CDE3C 008 74 32                                   jz      short loc_9CDE70
    0x550000,
    0x300000,
    {
         0x56, 0x57, 0x8B, 0x7C, 0x24, 0x0C,
         0x18B, 0x1F1, 0x185, 0x1FF,
         0x74, 0x44,
         0x8D, 0x46, 0x3E,
         0x57,
         0x50,
         0xFF, 0x15, 0x160, 0x1B4, 0x1B7, 0x100,
         0x83, 0xC4, 0x08, 0x85, 0xC0, 0x74,
    },
    0x009CDE20 - 0x009CDE3C,
    { 0xb0, 0x01, 0xc2, 0x04, 0x00 }
};

static const ProcessCtl::PatchData noMediaPatch2
{
    // .text:00CA3F20
    // .text:00CA3F20 000 55                                            push    ebp
    // .text:00CA3F21 004 8B EC                                         mov     ebp, esp
    // .text:00CA3F23 004 56                                            push    esi
    // .text:00CA3F24 008 57                                            push    edi
    // .text:00CA3F25 00C 8B 7D 08                                      mov     edi, [ebp+Str2]
    // .text:00CA3F28 00C 8B F1                                         mov     esi, ecx
    // .text:00CA3F2A 00C 85 FF                                         test    edi, edi
    // .text:00CA3F2C 00C 74 45                                         jz      short loc_CA3F73
    // .text:00CA3F2E 00C 8D 46 3E                                      lea     eax, [esi+3Eh]
    // .text:00CA3F31 00C 57                                            push    edi             ; Str2
    // .text:00CA3F32 010 50                                            push    eax             ; Str1
    // .text:00CA3F33 014 FF 15 58 FC FF 00                             call    ds:_stricmp
    // 55 8b ec 56 57 8b 7d 08 8b f1 85 ff 74 ? 8d 46
    0x850000,
    0x200000,
    {
        0x55, 0x8b, 0xec, 0x56, 0x57, 0x8b, 0x7d, 0x08, 0x8b, 0xf1, 0x85, 0xff, 0x74, 0x100, 0x8d, 0x46,
    },
    0x00CA3F20 - 0x00CA3F2F,
    { 0xb0, 0x01, 0xc2, 0x04, 0x00} // mov al, 01, ret 04
};

// static const ProcessCtl::PatchData noMediaPatch
// {
//     // .text:009CAF30                             AMSoundBufferMan__LoadSoundFromFile proc near
//     // .text:009CAF30 000 6A FF                                   push    0FFFFFFFFh
//     // .text:009CAF32 004 68 D6 01 B7 00                          push    offset SEH_9CAF30
//     // .text:009CAF37 008 64 A1 00 00 00 00                       mov     eax, large fs:0
//     // .text:009CAF3D 008 50                                      push    eax
//     // .text:009CAF3E 00C 64 89 25 00 00 00 00                    mov     large fs:0, esp
//     // .text:009CAF45 00C 51                                      push    ecx
//     // .text:009CAF46 010 8A 44 24 18                             mov     al, byte ptr [esp+10h+arg_4]
//     // .text:009CAF4A 010 53                                      push    ebx
//     // .text:009CAF4B 014 55                                      push    ebp
//     // .text:009CAF4C 018 56                                      push    esi
//     // .text:009CAF4D 01C 57                                      push    edi             ; ArgList
//     // .text:009CAF4E 020 8B F9                                   mov     edi, ecx
//     // .text:009CAF50 020 84 C0                                   test    al, al
//     // .text:009CAF52 020 89 7C 24 10                             mov     [esp+20h+var_10], edi
//     // .text:009CAF56 020 74 26                                   jz      short loc_9CAF7E
//     0x550000,
//     0x200000,
//     {
//         0x6A, 0xFF, 0x68, 0x1D6, 0x101, 0x1B7, 0x00, 0x64, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x50, 0x64, 0x89,
//         0x25, 0x00, 0x00, 0x00, 0x00, 0x51, 0x8A, 0x44, 0x24, 0x18, 0x53, 0x55, 0x56, 0x57, 0x8B, 0xF9,
//         0x84, 0xC0, 0x89, 0x7C, 0x24, 0x10
//     },
//     0x009CAF30 - 0x009CAF55,
//     { 0x33, 0xC0, 0xC2, 0x08, 0x00 }
// };

static const ProcessCtl::PatchData lowCpuPatch
{
    // CECGame::Run(), break because CECGameRun::Render() return false!

    // .text:0044CBB2 044 B0 01                                   mov     al, 1
    // .text:0044CBB4 044 EB 02                                   jmp     short loc_44CBB8
    // .text:0044CBB6 044 32 C0                                   xor     al, al
    // .text:0044CBB8 044 3A D3                                   cmp     dl, bl
    // .text:0044CBBA 044 75 11                                   jnz     short loc_44CBCD
    // .text:0044CBBC 044 3A C3                                   cmp     al, bl
    // .text:0044CBBE 044 75 0D                                   jnz     short loc_44CBCD
    // .text:0044CBC0 044 6A 0F                                   push    0Fh             ; dwMilliseconds
    // .text:0044CBC2 048 FF 15 48 42 D4 00                       call    ds:Sleep

    0x000000,
    0x100000,
    {
        0xB0, 0x01, 0xEB, 0x02, 0x32, 0xC0, 0x3A, 0xD3, 0x75, 0x111, 0x3A, 0xC3, 0x75, 0x10D, 0x6A, 0x0F,
        0xFF, 0x15, 0x148, 0x142, 0x1D4, 0x00
    },
    0x0044CBC1 - 0x0044CBC7,
    { 0x7F }
};

static const ProcessCtl::PatchData lowCpuPatch2
{
    // CECGame::Run(), break because CECGameRun::Render() return false!

    // .text:00433DAF 050 38 9D F0 04 00 00                             cmp     [ebp+4F0h], bl
    // .text:00433DB5 050 75 10                                         jnz     short loc_433DC7
    // .text:00433DB7 050 38 5C 24 17                                   cmp     [esp+50h+var_39], bl
    // .text:00433DBB 050 75 0A                                         jnz     short loc_433DC7
    // .text:00433DBD 050 6A 0F                                         push    0Fh             ; dwMilliseconds
    // .text:00433DBF 054 FF 15 1C A3 FB 00                             call    ds:Sleep
    // .text:00433DC5 050 EB 77                                         jmp     short loc_433E3E

    0x000000,
    0x100000,
    {
        0x6A, 0x0F,
        0xFF, 0x15, 0x148, 0x142, 0x1D4, 0x100,
        0xEB,
    },
    0x00433DBE - 0x00433DC5,
    { 0x7F }
};
static const ProcessCtl::PatchData lowCpuPatch3
{
    // CECGame::Run(), break because CECGameRun::Render() return false!

    // .text:00433DAF 050 38 9D F0 04 00 00                             cmp     [ebp+4F0h], bl
    // .text:00433DB5 050 75 10                                         jnz     short loc_433DC7
    // .text:00433DB7 050 38 5C 24 17                                   cmp     [esp+50h+var_39], bl
    // .text:00433DBB 050 75 0A                                         jnz     short loc_433DC7
    // .text:00433DBD 050 6A 0F                                         push    0Fh             ; dwMilliseconds
    // .text:00433DBF 054 FF 15 1C A3 FB 00                             call    ds:Sleep
    // .text:00433DC5 038 E9 88 00 00 00                                jmp     loc_433C75

    0x000000,
    0x650000,
    {
        0x6A, 0x0F,
        0xFF, 0x15, 0x148, 0x142, 0x1D4, 0x100,
        0xE9,
    },
    0x00433DBE - 0x00433DC5,
    { 0x7F }
};


//

ProcessCtl::ProcessCtl(QObject * parent)
    : QObject(parent)
    , debugger_(new Debugger)
    , memoryAccess_(0)
{
    connect(debugger_, &Debugger::breakpoint, this, &ProcessCtl::breakpoint);
    connect(debugger_, &Debugger::debuggingStarted, this, &ProcessCtl::started);
    connect(debugger_, &Debugger::debuggingStopped, this, &ProcessCtl::stopped);
    init();
}

ProcessCtl::~ProcessCtl()
{
    stop();
    delete debugger_;
}

void ProcessCtl::init()
{
    isStartup_ = false;
    adjustSettingsBp_ = 0;
    pSettings_ = 0;
    createWindowBp_ = 0;
    windowHandle_ = 0;
    closeWindowBp_ = 0;
    replaceServerBp_ = 0;
    replaceServerBp2_ = 0;
    replaceServerBp3_ = 0;
    replaceRoleBp_ = 0;
    replaceRoleEndBp_ = 0;
    replaceRoleStepBp_ = 0;
    replaceRoleThreadId_ = 0;
    role_.clear();
    memoryAccess_.reset();
}

void ProcessCtl::setServerSubstitution(const QString & address, unsigned port)
{
    serverAddress_ = address;
    serverPort_ = port;
}

bool ProcessCtl::start(const QString & executable, const QString & user, const QString & password)
{
    QFileInfo fi(executable);
#if defined(MAILRU_TOKEN_AUTH)
    if (debugger_->startProcess(QString("\"%1\" startbypatcher user:%2 token2:%3 role:0000")
#else
    if (debugger_->startProcess(QString("\"%1\" startbypatcher user:%2 token:%3 role:0000")
#endif
                                .arg(executable)
                                .arg(user)
                                .arg(password)
                                , fi.absolutePath()))
    {
        isStartup_ = true;
        return true;
    }
    return false;
}

void ProcessCtl::stop()
{
    memoryAccess_.reset();
    debugger_->stopDebugging();
    debugger_->wait();
    init();
}

bool ProcessCtl::selectChar(const QString & charName)
{
    qDebug() << "selectChar:" << charName;

    role_ = charName;

    // если мы прошли старт выбора в клиенте - то продолжаем, иначе продолжим после достижения.
    if (replaceRoleThreadId_ != 0)
    {
        return replaceRoleStart();
    }
    return true;
}

void ProcessCtl::showWindow()
{
    if (windowHandle_ != 0)
    {
        ShowWindowAsync(windowHandle_, SW_SHOWNORMAL);
        PostMessage(windowHandle_, WM_ACTIVATEAPP, 1, 0);
    }
}

void ProcessCtl::hideWindow()
{
    if (windowHandle_ != 0)
    {
        PostMessage(windowHandle_, WM_ACTIVATEAPP, 0, 0);
        PostMessage(windowHandle_, WM_CLOSE, 0, 0);
    }
}

void ProcessCtl::flushMemory()
{
    debugger_->flushMemory();
}

//

void ProcessCtl::breakpoint(quintptr addr, unsigned threadId)
{
    if (!debugger_->isRunning())
    {
        // filter out late events
        return;
    }

    Log("BP %08X", addr);

    bool success = false;
    unsigned opId = 0;

    if (isStartup_)
    {
        // first BP at OEP

        isStartup_ = false;

        memoryAccess_.reset(new MemoryAccess(debugger_->processId()));
        if (memoryAccess_->isGood())
        {
            if ((opId++, (applyPatch(disableServerSelPatch)
                          || applyPatch(disableServerSelPatch2)
                          || applyPatch(disableServerSelPatch3)
                          || applyPatch(disableServerSelPatch4)))
                && ((opId++, serverAddress_.isEmpty()) // if server not empty then set BP
                    || ((replaceServerBp_ = placeBreakpoint(replaceServerBp)) != 0
                        || (replaceServerBp2_ = placeBreakpoint(replaceServerBp2)) != 0
                        || (replaceServerBp3_ = placeBreakpoint(replaceServerBp3)) != 0))       // 2
                && (isSafeMode_
                    || (   (opId++, (applyPatch(disableProtector) || applyPatch(disableProtector2) || applyPatch(disableProtector3) || applyPatch(disableProtector4) || applyPatch(disableProtector5)))
                        && (opId++, (applyPatch(createWindowSkipShowPatch) || applyPatch(createWindowSkipShowPatch2) || applyPatch(createWindowSkipShowPatch3)))  //3
                        && (opId++, ((adjustSettingsBp_ = placeBreakpoint(adjustSettingsBp)) != 0
                                     || (adjustSettingsBp_ = placeBreakpoint(adjustSettingsBp2)) != 0
                                     || (adjustSettingsBp_ = placeBreakpoint(adjustSettingsBp3)) != 0)) // 5
                        && (opId++, ((createWindowBp_ = placeBreakpoint(createWindowBp)) != 0   // 6
                                     || (createWindowBp_ = placeBreakpoint(createWindowBp2)) != 0
                                     || (createWindowBp_ = placeBreakpoint(createWindowBp3)) != 0))
                        && (opId++, closeWindowBp_ = placeBreakpoint(closeWindowBp)) != 0       // 7
                        && (opId++, ((replaceRoleBp_ = placeBreakpoint(replaceRoleBp)) != 0)    // 8
                        #if CLIENT_VERSION >= 1520
                            || (replaceRoleBp_ = placeBreakpoint(replaceRoleBp2)) != 0
                            || (replaceRoleBp_ = placeBreakpoint(replaceRoleBp3)) != 0
                            || (replaceRoleBp_ = placeBreakpoint(replaceRoleBp4)) != 0
                        #endif
                            )
                        && (opId++, (applyPatch(passOpenMarketPatch)
                                     || applyPatch(passOpenMarketPatch2)
                                     || applyPatch(passOpenMarketPatch3)))      // 9
                        && (opId++, (applyPatch(noSaveSettingsPatch)
                                     || applyPatch(noSaveSettingsPatch2)
                                     || applyPatch(noSaveSettingsPatch3)
                                     || applyPatch(noSaveSettingsPatch4)))      // 10
                        && (opId++, applyPatch(noMediaPatch) || applyPatch(noMediaPatch2))
                        && (opId++, applyPatch(lowCpuPatch) || applyPatch(lowCpuPatch2) || applyPatch(lowCpuPatch3))    // 12
                        )
                    )
                )
            {
                opId++;
                success = debugger_->continueDebugging(threadId);
            }
        }
    }
    else
    {
        if (addr == adjustSettingsBp_)
        {
            if (pSettings_ == 0)
            {
                opId = 20;
                success =
                    getSettingsPtr(threadId)
                    && debugger_->removeBreakpoint(adjustSettingsBp_)
                    && debugger_->addBreakpoint(adjustSettingsBp_ + 5)  // jump over call
                    ;
                adjustSettingsBp_ += 5;
            }
            else
            {
                const unsigned wid = 1024;
                const unsigned hei = 768;
                const unsigned z = 0;
            #if CLIENT_VERSION >= 1520 && CLIENT_VERSION < 1760
                const unsigned sm = 4;
            #endif

                success =
                #if CLIENT_VERSION < 1520
                    memoryAccess_->write(((byte *)pSettings_) + 0xAB + 0x13, &wid, 4)    // RenderWid
                    && memoryAccess_->write(((byte *)pSettings_) + 0xAB + 0x17, &hei, 4)    // RenderHei
                    && memoryAccess_->write(((byte *)pSettings_) + 0xAB + 0x23, &z, 1)      // FullScreen
                    && memoryAccess_->write(((byte *)pSettings_) + 0xAB + 0x25, &z, 1)      // VerticalSync
                #elif CLIENT_VERSION < 1530
                    memoryAccess_->write(((byte *)pSettings_) + 0xBD + 0x13, &wid, 4)    // RenderWid
                    && memoryAccess_->write(((byte *)pSettings_) + 0xBD + 0x17, &hei, 4)    // RenderHei
                    && memoryAccess_->write(((byte *)pSettings_) + 0xBD + 0x23, &z, 1)      // FullScreen
                    && memoryAccess_->write(((byte *)pSettings_) + 0xBD + 0x25, &z, 1)      // VerticalSync
                    && memoryAccess_->write(((byte *)pSettings_) + 0x31D, &sm, 4)           // SimplifyMode
                #elif CLIENT_VERSION <= 1550
                    memoryAccess_->write(((byte *)pSettings_) + 0xBD + 0x13, &wid, 4)    // RenderWid
                    && memoryAccess_->write(((byte *)pSettings_) + 0xBD + 0x17, &hei, 4)    // RenderHei
                    && memoryAccess_->write(((byte *)pSettings_) + 0xBD + 0x23, &z, 1)      // FullScreen
                    && memoryAccess_->write(((byte *)pSettings_) + 0xBD + 0x25, &z, 1)      // VerticalSync
                    && memoryAccess_->write(((byte *)pSettings_) + 0x322, &sm, 4)           // SimplifyMode
                #elif CLIENT_VERSION <= 1550
                    memoryAccess_->write(((byte *)pSettings_) + 0xBD + 0x13, &wid, 4)    // RenderWid
                    && memoryAccess_->write(((byte *)pSettings_) + 0xBD + 0x17, &hei, 4)    // RenderHei
                    && memoryAccess_->write(((byte *)pSettings_) + 0xBD + 0x23, &z, 1)      // FullScreen
                    && memoryAccess_->write(((byte *)pSettings_) + 0xBD + 0x25, &z, 1)      // VerticalSync
                    && memoryAccess_->write(((byte *)pSettings_) + 0x323, &sm, 4)           // SimplifyMode
                #elif CLIENT_VERSION < 1560
                    memoryAccess_->write(((byte *)pSettings_) + 0xBD + 0x13, &wid, 4)    // RenderWid
                    && memoryAccess_->write(((byte *)pSettings_) + 0xBD + 0x17, &hei, 4)    // RenderHei
                    && memoryAccess_->write(((byte *)pSettings_) + 0xBD + 0x23, &z, 1)      // FullScreen
                    && memoryAccess_->write(((byte *)pSettings_) + 0xBD + 0x25, &z, 1)      // VerticalSync
                    && memoryAccess_->write(((byte *)pSettings_) + 0x32B, &sm, 4)           // SimplifyMode
                #elif CLIENT_VERSION < 1570
                    memoryAccess_->write(((byte *)pSettings_) + 0xBD + 0x13, &wid, 4)    // RenderWid
                    && memoryAccess_->write(((byte *)pSettings_) + 0xBD + 0x17, &hei, 4)    // RenderHei
                    && memoryAccess_->write(((byte *)pSettings_) + 0xBD + 0x23, &z, 1)      // FullScreen
                    && memoryAccess_->write(((byte *)pSettings_) + 0xBD + 0x25, &z, 1)      // VerticalSync
                    && memoryAccess_->write(((byte *)pSettings_) + 0x339, &sm, 4)           // SimplifyMode
                #elif CLIENT_VERSION < 1590
                    memoryAccess_->write(((byte *)pSettings_) + 0xD7, &wid, 4)           // RenderWid
                    && memoryAccess_->write(((byte *)pSettings_) + 0xDB, &hei, 4)            // RenderHei
                    && memoryAccess_->write(((byte *)pSettings_) + 0xE7, &z, 1)              // FullScreen
                    && memoryAccess_->write(((byte *)pSettings_) + 0xE9, &z, 1)              // VerticalSync
                    && memoryAccess_->write(((byte *)pSettings_) + 0x33B, &sm, 4)            // SimplifyMode
                #elif CLIENT_VERSION < 1620
                    memoryAccess_->write(((byte *)pSettings_) + 0xD7, &wid, 4)               // RenderWid
                    && memoryAccess_->write(((byte *)pSettings_) + 0xDB, &hei, 4)            // RenderHei
                    && memoryAccess_->write(((byte *)pSettings_) + 0xE7, &z, 1)              // FullScreen
                    && memoryAccess_->write(((byte *)pSettings_) + 0xE9, &z, 1)              // VerticalSync
                    && memoryAccess_->write(((byte *)pSettings_) + 0x340, &sm, 4)             // SimplifyMode
                #elif CLIENT_VERSION < 1660
                    memoryAccess_->write(((byte *)pSettings_) + 0xD7, &wid, 4)               // RenderWid
                    && memoryAccess_->write(((byte *)pSettings_) + 0xDB, &hei, 4)            // RenderHei
                    && memoryAccess_->write(((byte *)pSettings_) + 0xE7, &z, 1)              // FullScreen
                    && memoryAccess_->write(((byte *)pSettings_) + 0xE9, &z, 1)              // VerticalSync
                    && memoryAccess_->write(((byte *)pSettings_) + 0x343, &sm, 4)            // SimplifyMode
                #elif CLIENT_VERSION < 1700
                    memoryAccess_->write(((byte *)pSettings_) + 0xD7, &wid, 4)               // RenderWid
                    && memoryAccess_->write(((byte *)pSettings_) + 0xDB, &hei, 4)            // RenderHei
                    && memoryAccess_->write(((byte *)pSettings_) + 0xE7, &z, 1)              // FullScreen
                    && memoryAccess_->write(((byte *)pSettings_) + 0xE9, &z, 1)              // VerticalSync
                    && memoryAccess_->write(((byte *)pSettings_) + 0x349, &sm, 4)            // SimplifyMode
                #elif CLIENT_VERSION < 1710
                    memoryAccess_->write(((byte *)pSettings_) + 0xD7, &wid, 4)               // RenderWid
                    && memoryAccess_->write(((byte *)pSettings_) + 0xDB, &hei, 4)            // RenderHei
                    && memoryAccess_->write(((byte *)pSettings_) + 0xE7, &z, 1)              // FullScreen
                    && memoryAccess_->write(((byte *)pSettings_) + 0xE9, &z, 1)              // VerticalSync
                    && memoryAccess_->write(((byte *)pSettings_) + 0x34A, &sm, 4)            // SimplifyMode
                #elif CLIENT_VERSION < 1760
                    memoryAccess_->write(((byte *)pSettings_) + 0x10B, &wid, 4)              // RenderWid
                    && memoryAccess_->write(((byte *)pSettings_) + 0x10F, &hei, 4)           // RenderHei
                    && memoryAccess_->write(((byte *)pSettings_) + 0x11C, &z, 1)             // FullScreen
                    && memoryAccess_->write(((byte *)pSettings_) + 0x11D, &z, 1)             // VerticalSync
                    && memoryAccess_->write(((byte *)pSettings_) + 0x128, &sm, 1)            // SimplifyMode
                #else
                    memoryAccess_->write(((byte *)pSettings_) + 0x10F, &wid, 4)              // RenderWid
                    && memoryAccess_->write(((byte *)pSettings_) + 0x113, &hei, 4)           // RenderHei
                    && memoryAccess_->write(((byte *)pSettings_) + 0x11F, &z, 1)             // FullScreen
                    && memoryAccess_->write(((byte *)pSettings_) + 0x121, &z, 1)             // VerticalSync
                    && memoryAccess_->write(((byte *)pSettings_) + 0x12C, &z, 1)             // ModeSetting (SimplifyMode)
                #endif
                    && debugger_->removeBreakpoint(adjustSettingsBp_)
                    ;
                adjustSettingsBp_ = 0;
            }
        }
        else if (addr == createWindowBp_)
        {
            opId = 21;
            success = getWindowHandle(threadId);
            debugger_->removeBreakpoint(createWindowBp_);
            //  это возможно, но окно все равно мелькает при создании
            //     hideWindow();

        }
        else if (addr == closeWindowBp_)
        {
            opId = 22;
            success = processCloseWindow(threadId);
        }
        else if (addr == replaceServerBp_)
        {
            opId = 23;
            success = replaceServer(threadId);
            debugger_->removeBreakpoint(replaceServerBp_);
        }
        else if (addr == replaceServerBp2_)
        {
            opId = 23;
            success = replaceServer2(threadId);
            debugger_->removeBreakpoint(replaceServerBp2_);
        }
        else if (addr == replaceServerBp3_)
        {
            opId = 23;
            success = replaceServer3(threadId);
            debugger_->removeBreakpoint(replaceServerBp3_);
        }
        else if (addr == replaceRoleBp_)
        {
            qDebug() << "replaceRoleBp";

            replaceRoleThreadId_ = threadId;

            if (role_.isNull())
            {
                // wait for user to perform selection
                return; // not continuing current thread
            }
            else
            {
                // ready to select
                opId = 24;
                success = replaceRoleStart();
            }
        }
        else if (addr == replaceRoleStepBp_)
        {
            qDebug() << "replaceRoleStepBp";
            // stepping by comparing char names
            opId = 25;
            success = replaceRoleStep();
        }
        else if (addr == replaceRoleEndBp_)
        {
        #if defined(GAME_USING_CLIENT)
            if (role_.isEmpty())
            {
                // allow user to create character
                success = true;
            }
            else
            {
                opId = 26;
            }
        #else
            opId = 26;
            // if we're here then we failed to select character and have to be terminated
        #endif
        }
    }

    if (!success)
    {
        error(tr("Failed to operate with process (%1) ").arg(opId));
        stop();
    }
    else
    {
        debugger_->continueDebugging(threadId);
    }
}

bool ProcessCtl::applyPatch(const PatchData & patchData)
{
    byte * addr = memoryAccess_->findPattern(patchData.hint, patchData.lookupLength, patchData.pattern.data(), patchData.pattern.size());
    if (addr != 0)
    {
        qDebug() << "patch @" << addr + patchData.offset << ' ' << patchData.patch;
        return memoryAccess_->write(addr + patchData.offset, patchData.patch.data(), patchData.patch.size());
    }
    return false;
}

quintptr ProcessCtl::placeBreakpoint(const BreakpointData & breakpoint)
{
    quintptr addr = (quintptr)memoryAccess_->findPattern(breakpoint.hint, breakpoint.lookupLength, breakpoint.pattern.data(), breakpoint.pattern.size());
    if (addr == 0)
    {
        return 0;
    }

    addr += breakpoint.offset;
    if (!debugger_->addBreakpoint(addr))
    {
        return 0;
    }

    return addr;
}

bool ProcessCtl::getSettingsPtr(unsigned threadId)
{
    CONTEXT ctx;
    if (!debugger_->getContext(threadId, &ctx))
    {
        qWarning() << "Failed to operate with process," << debugger_->lastError();
        return false;
    }

    pSettings_ = ctx.Ecx;
    return true;
}

bool ProcessCtl::getWindowHandle(unsigned threadId)
{
    CONTEXT ctx;
    if (!debugger_->getContext(threadId, &ctx))
    {
        qWarning() << "Failed to operate with process," << debugger_->lastError();
        return false;
    }

    windowHandle_ = (HWND)ctx.Eax;
    return true;
}

bool ProcessCtl::processCloseWindow(unsigned threadId)
{
    CONTEXT ctx;
    if (!debugger_->getContext(threadId, &ctx))
    {
        qWarning() << "Failed to operate with process," << debugger_->lastError();
        return false;
    }

    // stepping over tray icon creation: corrupt Shell_NotifyIconW message
    ctx.Eip += 2;
    ctx.Esp -= 4;
    uint32_t msg = 0x100;
    debugger_->writeMemory(ctx.Esp, (char *)&msg, 4);
    if (!debugger_->setContext(threadId, &ctx))
    {
        qWarning() << "Failed to operate with process," << debugger_->lastError();
        return false;
    }

    windowClosed();

    return debugger_->continueDebugging(threadId);
}

bool ProcessCtl::replaceServer(unsigned threadId)
{
    // ebp = указатель на хост
    // указатель на порт = ebp + 0x100
    // ecx - уже прочитанный порт, нужно тоже обновить

    CONTEXT ctx;
    if (debugger_->getContext(threadId, &ctx))
    {
        ctx.Ecx = serverPort_;
        if (debugger_->setContext(threadId, &ctx)
            && debugger_->writeMemory(ctx.Ebp, serverAddress_.toLatin1().data(), serverAddress_.size() + 1)
            && debugger_->writeMemory(ctx.Ebp + 0x100, (const char *)&serverPort_, 4))
        {
            qDebug() << "Replaced server in client";
            return true;
        }
    }

    qWarning() << "Failed to operate with process," << debugger_->lastError();
    return false;
}
bool ProcessCtl::replaceServer2(unsigned threadId)
{
    // eax = указатель на хост ([ecx+2FCh])
    // указатель на порт = eax + 0x100 ([ecx+3FCh])

    CONTEXT ctx;
    if (debugger_->getContext(threadId, &ctx))
    {
        if (debugger_->setContext(threadId, &ctx)
            && debugger_->writeMemory(ctx.Eax, serverAddress_.toLatin1().data(), serverAddress_.size() + 1)
            && debugger_->writeMemory(ctx.Eax + 0x100, (const char *)&serverPort_, 4))
        {
            qDebug() << "Replaced server in client";
            return true;
        }
    }

    qWarning() << "Failed to operate with process," << debugger_->lastError();
    return false;
}
bool ProcessCtl::replaceServer3(unsigned threadId)
{
    // ecx = указатель на хост ([ecx+2FCh])
    // указатель на порт = ecx + 0x100 ([ecx+3FCh])

    CONTEXT ctx;
    if (debugger_->getContext(threadId, &ctx))
    {
        if (debugger_->setContext(threadId, &ctx)
            && debugger_->writeMemory(ctx.Ecx, serverAddress_.toLatin1().data(), serverAddress_.size() + 1)
            && debugger_->writeMemory(ctx.Ecx + 0x100, (const char *)&serverPort_, 4))
        {
            qDebug() << "Replaced server in client";
            return true;
        }
    }

    qWarning() << "Failed to operate with process," << debugger_->lastError();
    return false;
}


bool ProcessCtl::replaceRoleStart()
{
    assert(replaceRoleThreadId_ != 0);

    if (!debugger_->removeBreakpoint(replaceRoleBp_))
    {
        qWarning() << "Failed to remove replaceRoleBp_";
        return false;
    }

    // ставим бряк внутри цикла перебора чаров и на выходе из функции
    // обнуляем ZF якобы role у нас присутствует

#if CLIENT_VERSION < 1520
    // .text:00709B20 0B4 64 89 0D 00 00 00 00                    mov     large fs : 0, ecx
    // .text:00709B27 0B4 81 C4 B4 00 00 00                       add     esp, 0B4h
    //*.text:00709B2D 000 C3                                      retn
    uintptr_t endAddr = replaceRoleBp_ + 0x129;

    // .text:00709A87 E8 C4 F0 2B 00                          call    Str_wcsicmp
    uintptr_t stepAddr = replaceRoleBp_ + 0x83;

#elif CLIENT_VERSION < 1552
    // .text:0073E285 010 64 89 0D 00 00 00 00                    mov     large fs:0, ecx
    // .text:0073E28C 010 83 C4 10                                add     esp, 10h
    // .text:0073E28F 000 C3                                      retn
    quintptr endAddr = replaceRoleBp_ + 0x36;

    // с шагом здесь немного сложнее - цикл находится в другой функции
#else
    // .text:005CA9CA 000 C3                                      retn
    quintptr endAddr = (quintptr)memoryAccess_->findPattern(replaceRoleBp_ - memoryAccess_->processBase(), 0x40, { 0xC3, 0xCC });
#endif

    replaceRoleBp_ = 0; // теперь можно и обнулить

    if (!endAddr)
    {
        qWarning() << "replaceRoleEndBp_ end position not found";
        return false;
    }
    --endAddr;
    {
        const QByteArray data = debugger_->readMemory(endAddr, 1);
        if (data.size() < 1 || data.at(0) != '\xC3')
        {
            qWarning() << "replaceRoleEndBp_ position mismatch";
            return false;
        }
        replaceRoleEndBp_ = debugger_->addBreakpoint(endAddr) ? endAddr : 0;
    }
#if CLIENT_VERSION < 1520
    {
        const QByteArray data = debugger_->readMemory(stepAddr, 1);
        if (data.size() < 1 || data[0] != '\xE8')
        {
            qWarning() << "replaceRoleStepBp_ position mismatch";
            return false;
        }
        replaceRoleStepBp_ = debugger_->addBreakpoint(stepAddr) ? stepAddr : 0;
    }
#else
    replaceRoleStepBp_ = placeBreakpoint(replaceRoleStepBp);
    if (replaceRoleStepBp_ == 0)
    {
        replaceRoleStepBp_ = placeBreakpoint(replaceRoleStepBp2);
        if (replaceRoleStepBp_ == 0)
        {
            replaceRoleStepBp_ = placeBreakpoint(replaceRoleStepBp3);
        }
    }
#endif

    if (replaceRoleStepBp_ != 0
        && replaceRoleEndBp_ != 0)
    {
        // continue thread from role string skipping bass by zero
        CONTEXT ctx;
        if (debugger_->getContext(replaceRoleThreadId_, &ctx))
        {
            ctx.EFlags &= ~0x40;    // reset ZF
            if (debugger_->setContext(replaceRoleThreadId_, &ctx))
            {
                qDebug() << "replaceRole starting";
                return debugger_->continueDebugging(replaceRoleThreadId_);
            }
            else
            {
                qWarning() << "SetContext failed with" << debugger_->lastError();
            }
        }
        else
        {
            qWarning() << "GetContext failed with" << debugger_->lastError();
        }
    }
    else
    {
        qWarning() << "Failed to set BPs:" << replaceRoleStepBp_ << replaceRoleEndBp_;
    }

    qWarning() << "Failed to start replaceRole process";
    return false;
}

bool ProcessCtl::replaceRoleStep()
{
    assert(replaceRoleThreadId_ != 0);

    // .text:00709A6C 50                                      push    eax             ; int
    // .text:00709A6D 51                                      push    ecx             ; Str
    // .text:00709A6E C6 84 24 CC 00 00 00 01                 mov     byte ptr [esp+0D0h+var_4], 1
    // .text:00709A76 E8 95 31 24 00                          call    _94CC10_escapeString
    // .text:00709A7B 83 C4 0C                                add     esp, 0Ch
    // .text:00709A7E 8D 54 24 18                             lea     edx, [esp+0C4h+Str2]
    // .text:00709A82 8D 4C 24 10                             lea     ecx, [esp+0C4h+role]
    // .text:00709A86 52                                      push    edx             ; Str2
    // .text:00709A87 E8 C4 F0 2B 00                          call    Str_wcsicmp

    CONTEXT ctx;
    if (!debugger_->getContext(replaceRoleThreadId_, &ctx))
    {
        qWarning() << "GetContext failed with" << debugger_->lastError();
        return false;
    }

    // pointer to charName is on stack
    auto ptra = debugger_->readMemory(ctx.Esp, 4);
    uint32_t ptr = *(uint32_t *)ptra.data();
    QByteArray name = debugger_->readMemory(ptr, 64);
    for (int i = 0; i < (name.size() - 1); i += 2)
    {
        if (name.at(i) == '\0' && name.at(i + 1) == '\0')
        {
            name.truncate(i);
        }
    }
    if (name.size() < 2)
    {
        qDebug() << "replaceRoleStep: empty name, skipping";
        return true;
    }
    QString sName = QString::fromWCharArray((wchar_t *)name.data(), name.size() / 2);
    if (sName == role_)
    {
        // found char. selecting
        qDebug() << "Found character" << role_;

        // removing BPs
        debugger_->removeBreakpoint(replaceRoleEndBp_);
        debugger_->removeBreakpoint(replaceRoleStepBp_);

        // jump over call
        ctx.Eax = 0;
        ctx.Eip += 5;
        ctx.Esp += 4;   // pop stack arg
        bool result = debugger_->setContext(replaceRoleThreadId_, &ctx);
        if (!result)
        {
            qWarning() << "SetContext failed with" << debugger_->lastError();
        }
        return result;
    }
    // name mismatch, just continue
    qDebug() << "Skipped character" << sName;
    return true;
}
