#ifndef _SIDER_PATTERNS_H
#define _SIDER_PATTERNS_H

// code patterns to search

static BYTE lcpk_pattern_at_read_file[23] =
    "\x48\x8b\x0b"
    "\x4c\x8d\x4c\x24\x60"
    "\x48\x83\x64\x24\x20\x00"
    "\x44\x8b\xc7"
    "\x48\x8b\xd6"
    "\xff\x15";
static int lcpk_offs_at_read_file = 20;

/**
Same code, different instruction encoding:

0000000143689ADC | 48 8B 0B                           | mov rcx,qword ptr ds:[rbx]             |
0000000143689ADF | 48 83 64 24 20 00                  | and qword ptr ss:[rsp+20],0            |
0000000143689AE5 | 4C 8D 4C 24 60                     | lea r9,qword ptr ss:[rsp+60]           |
0000000143689AEA | 41 89 F8                           | mov r8d,edi                            |
0000000143689AED | 48 89 F2                           | mov rdx,rsi                            |
0000000143689AF0 | FF 15 A2 5B 3E 0E                  | call qword ptr ds:[<&ReadFile>]        |
*/
static BYTE lcpk_pattern2_at_read_file[23] =
    "\x48\x8b\x0b"
    "\x48\x83\x64\x24\x20\x00"
    "\x4c\x8d\x4c\x24\x60"
    "\x41\x89\xf8"
    "\x48\x89\xf2"
    "\xff\x15";
static int lcpk_offs2_at_read_file = 20;

/**
Again same code, different instruction encoding:

0000000152A7236C | 48 8B 0B                           | mov rcx,qword ptr ds:[rbx]             |
0000000152A7236F | 4C 8D 4C 24 60                     | lea r9,qword ptr ss:[rsp+60]           |
0000000152A72374 | 48 83 64 24 20 00                  | and qword ptr ss:[rsp+20],0            |
0000000152A7237A | 41 89 F8                           | mov r8d,edi                            |
0000000152A7237D | 48 89 F2                           | mov rdx,rsi                            |
0000000152A72380 | FF 15 42 73 7E 06                  | call qword ptr ds:[<&ReadFile>]        |
*/
static BYTE lcpk_pattern3_at_read_file[23] =
    "\x48\x8b\x0b"
    "\x4c\x8d\x4c\x24\x60"
    "\x48\x83\x64\x24\x20\x00"
    "\x41\x89\xf8"
    "\x48\x89\xf2"
    "\xff\x15";
static int lcpk_offs3_at_read_file = 20;

static BYTE lcpk_pattern_at_get_size[18] =
    "\x85\xc0"
    "\x74\x24"
    "\x8b\x44\x24\x34"
    "\x89\x43\x04"
    "\x8b\x44\x24\x30"
    "\x89\x03";
static int lcpk_offs_at_get_size = 17;

/*
0000000151794085 | 48 8B 44 24 48                     | mov rax,qword ptr ss:[rsp+48]          |
000000015179408A | 48 89 47 08                        | mov qword ptr ds:[rdi+8],rax           |
000000015179408E | 33 C0                              | xor eax,eax                            |
0000000151794090 | 49 89 7D 00                        | mov qword ptr ds:[r13],rdi             |
*/
static BYTE lcpk_pattern_at_write_cpk_filesize[16] =
    "\x48\x8b\x44\x24\x48"
    "\x48\x89\x47\x08"
    "\x33\xc0"
    "\x49\x89\x7d\x00";
static int lcpk_offs_at_write_cpk_filesize = 0;

/*
0000000151794085 | 48 8B 44 24 48                     | mov rax,qword ptr ss:[rsp+48]          |
000000015179408A | 48 89 47 08                        | mov qword ptr ds:[rdi+8],rax           |
000000015179408E | 31 C0                              | xor eax,eax                            |
0000000151794090 | 49 89 7D 00                        | mov qword ptr ds:[r13],rdi             |
*/
static BYTE lcpk_pattern2_at_write_cpk_filesize[16] =
    "\x48\x8b\x44\x24\x48"
    "\x48\x89\x47\x08"
    "\x31\xc0"
    "\x49\x89\x7d\x00";

/*
000000014FA7839A | 4C 8B 01                           | mov r8,qword ptr ds:[rcx]              |
000000014FA7839D | 49 89 D9                           | mov r9,rbx                             |
000000014FA783A0 | 4D 01 D0                           | add r8,r10                             |
000000014FA783A3 | 4C 89 D9                           | mov rcx,r11                            |
*/
static BYTE lcpk_pattern_at_mem_copy[13] =
    "\x4c\x8b\x01"
    "\x49\x89\xd9"
    "\x4d\x01\xd0"
    "\x4c\x89\xd9";

/*
000000014FA7839A | 4C 8B 01                           | mov r8,qword ptr ds:[rcx]              |
000000014FA7839D | 4C 8B CB                           | mov r9,rbx                             |
000000014FA783A0 | 4D 03 C2                           | add r8,r10                             |
000000014FA783A3 | 49 8B CB                           | mov rcx,r11                            |
*/
static BYTE lcpk_pattern2_at_mem_copy[13] =
    "\x4c\x8b\x01"
    "\x4c\x8b\xcb"
    "\x4d\x03\xc2"
    "\x49\x8b\xcb";


static int lcpk_offs_at_mem_copy = 9;

/*
000000014FA5F32A | 48 8D 8F 08 01 00 00               | lea rcx,qword ptr ds:[rdi+108]         |
000000014FA5F331 | 49 89 F0                           | mov r8,rsi                             |
000000014FA5F334 | 48 8D 54 24 20                     | lea rdx,qword ptr ss:[rsp+20]          |
*/
static BYTE lcpk_pattern_at_lookup_file[16] =
    "\x48\x8d\x8f\x08\x01\x00\x00"
    "\x49\x89\xf0"
    "\x48\x8d\x54\x24\x20";

/*
000000014156B02E | 48 8D 8F 08 01 00 00               | lea rcx,qword ptr ds:[rdi+108]         |
000000014156B035 | 4C 8B C6                           | mov r8,rsi                             |
000000014156B038 | 48 8D 54 24 20                     | lea rdx,qword ptr ss:[rsp+20]          |
*/
static BYTE lcpk_pattern2_at_lookup_file[16] =
    "\x48\x8d\x8f\x08\x01\x00\x00"
    "\x4c\x8b\xc6"
    "\x48\x8d\x54\x24\x20";

static int lcpk_offs_at_lookup_file = 0;

/*
0000000150E98CB0 | 49 63 00                           | movsxd rax,dword ptr ds:[r8]         | at set team id
0000000150E98CB3 | 83 F8 02                           | cmp eax,2                            |
0000000150E98CB6 | 7D 16                              | jge pes2020.150E98CCE                |
0000000150E98CB8 | 4C 69 C0 54 06 00 00               | imul r8,rax,654                      |
0000000150E98CBF | 48 81 C1 38 01 00 00               | add rcx,138                          |
0000000150E98CC6 | 4C 01 C1                           | add rcx,r8                           |
0000000150E98CC9 | E9 12 0A DD EF                     | jmp pes2020.140C696E0                |
0000000150E98CCE | C3                                 | ret                                  |

00000001414C09D0 | 49:6300                  | movsxd rax,dword ptr ds:[r8]            | at set team id
00000001414C09D3 | 83F8 02                  | cmp eax,2                               |
00000001414C09D6 | 7D 16                    | jge pes2021.1414C09EE                   |
00000001414C09D8 | 4C:69C0 90060000         | imul r8,rax,690                         |
00000001414C09DF | 48:81C1 38010000         | add rcx,138                             |
00000001414C09E6 | 49:03C8                  | add rcx,r8                              |
00000001414C09E9 | E9 E24E65FF              | jmp pes2021.140B158D0                   |
00000001414C09EE | C3                       | ret                                     |
*/
static BYTE pattern_set_team_id[15] =
    //"\x49\x63\x00"
    //"\x83\xf8\x02"
    //"\x7d\x26"
    "\x4c\x69\xc0\x90\x06\x00\x00"
    "\x48\x81\xc1\x38\x01\x00\x00";
    //"\x4c\x01\xc1";
static int offs_set_team_id = -8;

static BYTE pattern_set_team_id_head[2] =
    "\x52"; // push rdx

/*
000000014D9310DD | 5A                                   | pop rdx                                  |
000000014D9310DE | 83 F8 02                             | cmp eax,2                                |
000000014D9310E1 | 7D 0B                                | jge pes2019.14D9310EE                    |
000000014D9310E3 | 90                                   | nop                                      |
000000014D9310E4 | 90                                   | nop                                      |
000000014D9310E5 | 90                                   | nop                                      |
000000014D9310E6 | 90                                   | nop                                      |
000000014D9310E7 | 90                                   | nop                                      |
000000014D9310E8 | 90                                   | nop                                      |
*/
static BYTE pattern_set_team_id_tail_1[13] =
    "\x5a"
    "\x83\xf8\x02"
    "\x7d\x0b"
    "\x90\x90\x90\x90\x90\x90";
static BYTE pattern_set_team_id_tail_2[13] =
    "\x5a"
    "\x83\xf8\x02"
    "\x7d\x1b"
    "\x90\x90\x90\x90\x90\x90";

static BYTE check_set_team_id_1[3] = "\x7d\x16";
static BYTE check_set_team_id_2[3] = "\x7d\x26";
static int offs_check_set_team_id = -2;

/*
0000000150E94EB3 | 48 8B 82 98 00 00 00               | mov rax,qword ptr ds:[rdx+98]        |
0000000150E94EBA | 48 89 81 98 00 00 00               | mov qword ptr ds:[rcx+98],rax        |
0000000150E94EC1 | 48 89 C8                           | mov rax,rcx                          | set_settings
0000000150E94EC4 | C3                                 | ret                                  |
*/
static BYTE pattern_set_settings[19] =
    "\x48\x8b\x82\x98\x00\x00\x00"
    "\x48\x89\x81\x98\x00\x00\x00"
    "\x48\x8b\xc1"
    "\xc3";
static BYTE pattern2_set_settings[19] =
    "\x48\x8b\x82\x98\x00\x00\x00"
    "\x48\x89\x81\x98\x00\x00\x00"
    "\x48\x89\xc8"
    "\xc3";
static int offs_set_settings = 0;
static BYTE pattern_set_settings_head[2] =
    "\x50";  // push rax
static BYTE pattern_set_settings_tail[2] =
    "\x58";   // pop rax

/*
000000015110A827 | 4C 8D A5 F0 C3 00 00                  | lea r12,qword ptr ss:[rbp+C3F0] |
000000015110A82E | 8B 8D 88 04 00 00                     | mov ecx,dword ptr ss:[rbp+488]  | t-check (4)
000000015110A834 | 41 80 E6 01                           | and r14b,1                      |
000000015110A838 | D1 FE                                 | sar esi,1                       |

0000000141EE81DE | 4C 8D AE 00 C4 00 00               | lea r13,qword ptr ds:[rsi+C400]        |
0000000141EE81E5 | 8B 8E 98 04 00 00                  | mov ecx,dword ptr ds:[rsi+498]         | c2
0000000141EE81EB | 41 80 E6 01                        | and r14b,1                             |
0000000141EE81EF | D1 FD                              | sar ebp,1                              |

0000000141F5184E | 4C:8DAB 00C40000         | lea r13,qword ptr ds:[rbx+C400]         |
0000000141F51855 | 8B8B 98040000            | mov ecx,dword ptr ds:[rbx+498]          |
0000000141F5185B | 41:80E6 01               | and r14b,1                              |
0000000141F5185F | D1FD                     | sar ebp,1                               |
*/
static BYTE pattern_trophy_check[20] =
    "\x4c\x8d\xab\x00\xc4\x00\x00"
    "\x8b\x8b\x98\x04\x00\x00"
    "\x41\x80\xe6\x01"
    "\xd1\xfd";
static int offs_trophy_check = 7;

/**
static BYTE pattern_trophy_check_head[5] =
    "\x48\x83\xec\x28";

static BYTE pattern_trophy_check_tail[10] =
    "\x48\x85\xd2"
    "\x0f\x84\x8d\x00\x00\x00";
**/

/*
0000000140A8FA94 | 48 89 8B 84 00 00 00               | mov qword ptr ds:[rbx+84],rcx        |
0000000140A8FA9B | 48 C7 83 C4 01 02 00 FF FF FF FF   | mov qword ptr ds:[rbx+201C4],FFFFFFF |
*/
static BYTE pattern_context_reset[19] =
    "\x48\x89\x8b\x84\x00\x00\x00"
    "\x48\xc7\x83\xc4\x01\x02\x00\xff\xff\xff\xff";
static int offs_context_reset = 0;

////// PES 2019 demo ///////////////////

/*
00007FF71BC1F33C | C7 47 58 00 00 80 3F                 | mov dword ptr ds:[rdi+58],3F800000         | set min time (float)
00007FF71BC1F343 | F3 0F 10 47 58                       | movss xmm0,dword ptr ds:[rdi+58]           |
*/
static BYTE pattern_set_min_time[13] =
    "\xc7\x47\x58\x00\x00\x80\x3f"
    "\xf3\x0f\x10\x47\x58";
static int offs_set_min_time = 0;

/*
00007FF71BC1F351 | C7 47 58 00 00 F0 41                 | mov dword ptr ds:[rdi+58],41F00000         | set max time (float)
00007FF71BC1F358 | 48 8B CE                             | mov rcx,rsi                                |
*/
static BYTE pattern_set_max_time[11] =
    "\xc7\x47\x58\x00\x00\xf0\x41"
    "\x48\x8b\xce";
static int offs_set_max_time = 0;

/*
00007FF71B036790 | 88 51 1C                             | mov byte ptr ds:[rcx+1C],dl                | set minutes
00007FF71B036793 | C3                                   | ret                                        |
00007FF71B036794 | CC                                   | int3                                       |
00007FF71B036795 | CC                                   | int3                                       |
00007FF71B036796 | CC                                   | int3                                       |
00007FF71B036797 | CC                                   | int3                                       |
00007FF71B036798 | CC                                   | int3                                       |
00007FF71B036799 | CC                                   | int3                                       |
*/
static BYTE pattern_set_minutes[11] =
    "\x88\x51\x1c"
    "\xc3"
    "\xcc\xcc\xcc\xcc\xcc\xcc";
static int offs_set_minutes = 0;
static BYTE patch_set_minutes[6] =
    "\xc6\x41\x1c\x0a"
    "\xc3";

/*
0000000140AD64ED | E8 DEEB5500              | call pes2021.1410350D0                     | set minutes
0000000140AD64F2 | E9 EB000000              | jmp pes2021.140AD65E2                      |
0000000140AD64F7 | 41:83F8 01               | cmp r8d,1                                  |
0000000140AD64FB | 48:8D5424 40             | lea rdx,qword ptr ss:[rsp+40]              |
0000000140AD6500 | 48:8BCD                  | mov rcx,rbp                                |
..
00000001410350D0 | 8851 1C                  | mov byte ptr ds:[rcx+1C],dl                |
00000001410350D3 | C3                       | ret                                        |
00000001410350D4 | CC                       | int3                                       |
00000001410350D5 | CC                       | int3                                       |
00000001410350D6 | CC                       | int3                                       |
00000001410350D7 | CC                       | int3                                       |
*/
static BYTE pattern_call_set_minutes[15] =
    "\xe9\xeb\x00\x00\x00"
    "\x41\x83\xf8\x01"
    "\x48\x8d\x54\x24\x40";
static int offs_call_set_minutes = -4;

// controller restrictions ("sider")
/*
0000000140C67D37 | F2:0F1000                | movsd xmm0,qword ptr ds:[rax]           |
0000000140C67D3B | F2:0F1183 A8000000       | movsd qword ptr ds:[rbx+A8],xmm0        |
*/
static BYTE pattern_sider[13] =
    "\xf2\x0f\x10\x00"
    "\xf2\x0f\x11\x83\xa8\x00\x00\x00";
static int offs_sider = 0;

// tournament_id --> trophy_id table
/*
0000000155BBBAD3 | 48 63 C1                           | movsxd rax,ecx                         |
0000000155BBBAD6 | 8B 44 C4 04                        | mov eax,dword ptr ss:[rsp+rax*8+4]     |
0000000155BBBADA | 48 8B 8D 40 09 00 00               | mov rcx,qword ptr ss:[rbp+940]         |
...
0000000155BBBAE9 | 4C 8D 9C 24 50 0A 00 00            | lea r11,qword ptr ss:[rsp+A50]         |
0000000155BBBAF1 | 49 8B 5B 30                        | mov rbx,qword ptr ds:[r11+30]          |
0000000155BBBAF5 | 49 8B 73 38                        | mov rsi,qword ptr ds:[r11+38]          |
0000000155BBBAF9 | 49 8B 7B 40                        | mov rdi,qword ptr ds:[r11+40]          |

0000000141E5528B | 48:63C1                  | movsxd rax,ecx                          |
0000000141E5528E | 8B44C4 04                | mov eax,dword ptr ss:[rsp+rax*8+4]      |
0000000141E55292 | 48:8B8D 50090000         | mov rcx,qword ptr ss:[rbp+950]          |
0000000141E55299 | 48:33CC                  | xor rcx,rsp                             |
0000000141E5529C | E8 3FA174FF              | call pes2021.14159F3E0                  |
0000000141E552A1 | 4C:8D9C24 600A0000       | lea r11,qword ptr ss:[rsp+A60]          |
0000000141E552A9 | 49:8B5B 30               | mov rbx,qword ptr ds:[r11+30]           |
0000000141E552AD | 49:8B73 38               | mov rsi,qword ptr ds:[r11+38]           |
0000000141E552B1 | 49:8B7B 40               | mov rdi,qword ptr ds:[r11+40]           |
*/
static BYTE pattern_trophy_table[15] =
    "\x48\x63\xc1"
    "\x8b\x44\xc4\x04"
    "\x48\x8b\x8d\x50\x09\x00\x00";
static int offs_trophy_table = 30;

// ball name

/*
000000014D9A0703 | 80 79 04 00                          | cmp byte ptr ds:[rcx+4],0        | before ball name copy
000000014D9A0707 | 48 8D 51 04                          | lea rdx,qword ptr ds:[rcx+4]     | rdx:"Ordem V EPL"
000000014D9A070B | 75 12                                | jne pes2019.14D9A071F            |
000000014D9A070D | 45 31 C0                             | xor r8d,r8d                      |
000000014D9A0710 | 48 89 C1                             | mov rcx,rax                      |
...
000000014D9A071F | 49 83 C8 FF                          | or r8,FFFFFFFFFFFFFFFF           |
000000014D9A0723 | 49 FF C0                             | inc r8                           |
000000014D9A0726 | 42 80 3C 02 00                       | cmp byte ptr ds:[rdx+r8],0       |
000000014D9A072B | 75 F6                                | jne pes2019.14D9A0723            |
000000014D9A072D | 48 89 C1                             | mov rcx,rax                      | rcx:dst,rdx:src,r8:len
*/
static BYTE pattern_ball_name[17] =
    "\x80\x79\x04\x00"
    "\x48\x8d\x51\x04"
    "\x75\x12"
    "\x45\x31\xc0"
    "\x48\x89\xc1";
static int offs_ball_name = 28;
static BYTE pattern_ball_name_head[3] = "\x50\x50";
static BYTE pattern_ball_name_tail[4] = "\x58\x58\x90";

/*
00000001415BD4A0 | 48 33 C4                             | xor rax,rsp                            |
00000001415BD4A3 | 48 89 84 24 E0 01 00 00              | mov qword ptr ss:[rsp+1E0],rax         |
00000001415BD4AB | 48 8B F9                             | mov rdi,rcx                            |
00000001415BD4AE | 48 8D 54 24 30                       | lea rdx,qword ptr ss:[rsp+30]          |
*/
static BYTE pattern_dxgi[20] =
    "\x48\x33\xc4"
    "\x48\x89\x84\x24\xe0\x01\x00\x00"
    "\x48\x8b\xf9"
    "\x48\x8d\x54\x24\x30";
static int offs_dxgi = 0x1a;

/*
0000000140ABC725 | C6 44 24 20 01                     | mov byte ptr ss:[rsp+20],1             |
0000000140ABC72A | 0F B7 D3                           | movzx edx,bx                           |
0000000140ABC72D | 48 8B CE                           | mov rcx,rsi                            |
0000000140ABC730 | E8 2B 58 9C 00                     | call pes2020.141481F60                 |
...
0000000141481F60 | E9 BB 19 C1 0D                     | jmp pes2020.14F093920                  |
0000000141481F65 | CC                                 | int3                                   |
0000000141481F66 | CC                                 | int3                                   |
0000000141481F67 | CC                                 | int3                                   |
0000000141481F68 | CC                                 | int3                                   |
0000000141481F69 | CC                                 | int3                                   |
0000000141481F6A | CC                                 | int3                                   |
0000000141481F6B | CC                                 | int3                                   |
0000000141481F6C | CC                                 | int3                                   |
0000000141481F6D | CC                                 | int3                                   |
0000000141481F6E | CC                                 | int3                                   |
0000000141481F6F | CC                                 | int3                                   |
...
000000014F093920 | 66 89 51 34                        | mov word ptr ds:[rcx+34],dx            | set stadium choice
000000014F093924 | C3                                 | ret                                    |
*/
static BYTE pattern_set_stadium_choice[13] =
    "\xc6\x44\x24\x20\x01"
    "\x0f\xb7\xd3"
    "\x48\x8b\xce"
    "\xe8";
static int offs_set_stadium_choice = 11;

/*
0000000140AB0222 | C6 44 24 20 01                     | mov byte ptr ss:[rsp+20],1             |
0000000140AB0227 | 41 0F B7 D7                        | movzx edx,r15w                         |
0000000140AB022B | 48 8B CE                           | mov rcx,rsi                            |
0000000140AB022E | E8 FD 25 6E 00                     | call pes2020.141192830                 |
...
0000000141192830 | E9 FB EA 81 0C                     | jmp pes2020.14D9B1330                  |
0000000141192835 | CC                                 | int3                                   |
0000000141192836 | CC                                 | int3                                   |
0000000141192837 | CC                                 | int3                                   |
0000000141192838 | CC                                 | int3                                   |
0000000141192839 | CC                                 | int3                                   |
000000014119283A | CC                                 | int3                                   |
000000014119283B | CC                                 | int3                                   |
000000014119283C | CC                                 | int3                                   |
000000014119283D | CC                                 | int3                                   |
000000014119283E | CC                                 | int3                                   |
000000014119283F | CC                                 | int3                                   |
...
000000014D9B1330 | 66 89 51 34                        | mov word ptr ds:[rcx+34],dx            | set stadium choice
000000014D9B1334 | C3                                 | ret                                    |
*/
static BYTE pattern2_set_stadium_choice[14] =
    "\xc6\x44\x24\x20\x01"
    "\x41\x0f\xb7\xd7"
    "\x48\x8b\xce"
    "\xe8";
static int offs2_set_stadium_choice = 12;

/*
000000014CFE6503 | 80 79 08 00             | cmp byte ptr ds:[rcx+8],0              | before stadium name copy
000000014CFE6507 | 48 8D 51 08             | lea rdx,qword ptr ds:[rcx+8]           | rdx:"Allianz Parque"
000000014CFE650B | 75 12                   | jne pes2019.14CFE651F                  |
000000014CFE650D | 45 31 C0                | xor r8d,r8d                            |
000000014CFE6510 | 48 89 C1                | mov rcx,rax                            |
000000014CFE6513 | E8 48 BE 4C F3          | call pes2019.1404B2360                 |
000000014CFE6518 | B0 01                   | mov al,1                               |
000000014CFE651A | 48 83 C4 28             | add rsp,28                             |
000000014CFE651E | C3                      | ret                                    |
000000014CFE651F | 49 83 C8 FF             | or r8,FFFFFFFFFFFFFFFF                 |
000000014CFE6523 | 49 FF C0                | inc r8                                 |
000000014CFE6526 | 42 80 3C 02 00          | cmp byte ptr ds:[rdx+r8],0             | rdx+r8*1:"Allianz Parque"
000000014CFE652B | 75 F6                   | jne pes2019.14CFE6523                  |
000000014CFE652D | 48 89 C1                | mov rcx,rax                            |

00000001414E09A3 | 8079 08 00               | cmp byte ptr ds:[rcx+8],0               |
00000001414E09A7 | 48:8D51 08               | lea rdx,qword ptr ds:[rcx+8]            |
00000001414E09AB | 75 12                    | jne pes2021.1414E09BF                   |
00000001414E09AD | 45:33C0                  | xor r8d,r8d                             |
00000001414E09B0 | 48:8BC8                  | mov rcx,rax                             |
00000001414E09B3 | E8 B819F2FE              | call pes2021.140402370                  |
00000001414E09B8 | B0 01                    | mov al,1                                |
00000001414E09BA | 48:83C4 28               | add rsp,28                              |
00000001414E09BE | C3                       | ret                                     |
00000001414E09BF | 49:83C8 FF               | or r8,FFFFFFFFFFFFFFFF                  |
00000001414E09C3 | 49:FFC0                  | inc r8                                  |
00000001414E09C6 | 42:803C02 00             | cmp byte ptr ds:[rdx+r8],0              |
00000001414E09CB | 75 F6                    | jne pes2021.1414E09C3                   |
00000001414E09CD | 48:8BC8                  | mov rcx,rax                             |
*/
static BYTE pattern_stadium_name[11] =
    "\x80\x79\x08\x00"
    "\x48\x8d\x51\x08"
    "\x75\x12";
static int offs_stadium_name = 28;
static BYTE pattern_stadium_name_head[3] = "\x50\x50";
static BYTE pattern_stadium_name_tail[4] = "\x58\x58\x90";

/*
00000001512FEF85 | 44 0F B7 80 58 02 00 00            | movzx r8d,word ptr ds:[rax+258]      |
00000001512FEF8D | 44 39 C5                           | cmp ebp,r8d                          | near def stadium name
00000001512FEF90 | 75 3A                              | jne pes2020.1512FEFCC                |
00000001512FEF92 | 48 89 C1                           | mov rcx,rax                          |
...
00000001512FEFDD | 48 85 C0                           | test rax,rax                         | before find stadium info
00000001512FEFE0 | 74 0D                              | je pes2020.1512FEFEF                 |
00000001512FEFE2 | 48 89 F2                           | mov rdx,rsi                          | rdx:DbgUiRemoteBreakin
00000001512FEFE5 | 48 89 C1                           | mov rcx,rax                          |
00000001512FEFE8 | E8 93 29 8A F0                     | call pes2020.141BA1980               |
00000001512FEFED | EB 12                              | jmp pes2020.1512FF001                |
00000001512FEFEF | 45 31 C0                           | xor r8d,r8d                          |
00000001512FEFF2 | 48 8D 15 3D 50 DA F0               | lea rdx,qword ptr ds:[1420A4036]     | rdx:DbgUiRemoteBreakin
00000001512FEFF9 | 48 89 F1                           | mov rcx,rsi                          |
00000001512FEFFC | E8 3F 0E 0F EF                     | call pes2020.1403EFE40               |
00000001512FF001 | 48 8B 5C 24 30                     | mov rbx,qword ptr ss:[rsp+30]        |
*/
// becomes:
/*
00000001512FEFDD | 48 85 C0                           | test rax,rax                         |
00000001512FEFE0 | 75 0C                              | jne pes2020.1512FEFEE                |
00000001512FEFE2 | 48 BA C2 0B D7 31 F8 7F 00 00      | mov rdx,sider.7FF831D70BC2           | rdx:DbgUiRemoteBreakin
00000001512FEFEC | FF D2                              | call rdx                             | rdx:DbgUiRemoteBreakin
00000001512FEFEE | 48 8B D6                           | mov rdx,rsi                          | rdx:DbgUiRemoteBreakin
00000001512FEFF1 | 48 8B C8                           | mov rcx,rax                          |
00000001512FEFF4 | E8 87 29 8A F0                     | call pes2020.141BA1980               |
00000001512FEFF9 | EB 06                              | jmp pes2020.1512FF001                |
00000001512FEFFB | 90                                 | nop                                  |
00000001512FEFFC | E8 3F 0E 0F EF                     | call pes2020.1403EFE40               |
00000001512FF001 | 48 8B 5C 24 30                     | mov rbx,qword ptr ss:[rsp+30]        |
*/

/*
00000001414E7455 | 44 0F B7 80 58 02 00 00            | movzx r8d,word ptr ds:[rax+258]        |
00000001414E745D | 41 3B E8                           | cmp ebp,r8d                            |
00000001414E7460 | 75 3A                              | jne pes2020.1414E749C                  |
00000001414E7462 | 48 8B C8                           | mov rcx,rax                            |
...
00000001414E74AD | 48 85 C0                           | test rax,rax                           |
00000001414E74B0 | 74 0D                              | je pes2020.1414E74BF                   |
00000001414E74B2 | 48 8B D6                           | mov rdx,rsi                            |
00000001414E74B5 | 48 8B C8                           | mov rcx,rax                            |
00000001414E74B8 | E8 03 11 FD FF                     | call pes2020.1414B85C0                 |
00000001414E74BD | EB 12                              | jmp pes2020.1414E74D1                  |
00000001414E74BF | 45 33 C0                           | xor r8d,r8d                            |
00000001414E74C2 | 48 8D 15 2D 25 43 01               | lea rdx,qword ptr ds:[1429199F6]       |
00000001414E74C9 | 48 8B CE                           | mov rcx,rsi                            |
00000001414E74CC | E8 2F 98 F1 FE                     | call pes2020.140400D00                 |
00000001414E74D1 | 48 8B 5C 24 30                     | mov rbx,qword ptr ss:[rsp+30]          |
*/
// becomes
/*
00000001414E74AD | 48 85 C0                           | test rax,rax                           |
00000001414E74B0 | 75 0C                              | jne pes2020.1414E74BE                  |
00000001414E74B2 | 48 BA A7 94 53 5D FA 7F 00 00      | mov rdx,sider.7FFA5D5394A7             | rdx:DbgUiRemoteBreakin
00000001414E74BC | FF D2                              | call rdx                               | rdx:DbgUiRemoteBreakin
00000001414E74BE | 48 8B D6                           | mov rdx,rsi                            | rdx:DbgUiRemoteBreakin
00000001414E74C1 | 48 8B C8                           | mov rcx,rax                            |
00000001414E74C4 | E8 F7 10 FD FF                     | call pes2020.1414B85C0                 |
00000001414E74C9 | EB 06                              | jmp pes2020.1414E74D1                  |
00000001414E74CB | 90                                 | nop                                    |
00000001414E74CC | E8 2F 98 F1 FE                     | call pes2020.140400D00                 |
00000001414E74D1 | 48 8B 5C 24 30                     | mov rbx,qword ptr ss:[rsp+30]          |


000000014150F545 | 44:0FB780 94020000       | movzx r8d,word ptr ds:[rax+294]         |
000000014150F54D | 41:3BE8                  | cmp ebp,r8d                             |
000000014150F550 | 75 3A                    | jne pes2021.14150F58C                   |
000000014150F552 | 48:8BC8                  | mov rcx,rax                             |
...
000000014150F59D | 48:85C0                  | test rax,rax                            |
000000014150F5A0 | 74 0D                    | je pes2021.14150F5AF                    |
000000014150F5A2 | 48:8BD6                  | mov rdx,rsi                             |
000000014150F5A5 | 48:8BC8                  | mov rcx,rax                             |
000000014150F5A8 | E8 6320FDFF              | call pes2021.1414E1610                  |
000000014150F5AD | EB 12                    | jmp pes2021.14150F5C1                   |
000000014150F5AF | 45:33C0                  | xor r8d,r8d                             |
000000014150F5B2 | 48:8D15 9DEF4601         | lea rdx,qword ptr ds:[14297E556]        |
000000014150F5B9 | 48:8BCE                  | mov rcx,rsi                             |
000000014150F5BC | E8 AF2DEFFE              | call pes2021.140402370                  |
000000014150F5C1 | 48:8B5C24 30             | mov rbx,qword ptr ss:[rsp+30]           |

becomes

*/

static BYTE pattern_def_stadium_name[9] = //[17] =
    "\x44\x0f\xb7\x80\x94\x02\x00\x00";
    //"\x44\x0f\xb6\x80\xda\x03\x00\x00";
    //"\x44\x39\xc5"
    //"\x75\x3a"
    //"\x48\x89\xc1";
static int offs_def_stadium_name = 0xa0 - 0x45; //0x53;
static BYTE pattern_def_stadium_name_head[3] = "\x75\x0c";
static BYTE pattern_def_stadium_name_tail[15] =
    "\x48\x8b\xd6"
    "\x48\x8b\xc8"
    "\xe8\x00\x00\x00\x00"
    "\xeb\x06"
    "\x90";
static int def_stadium_name_moved_call_offs_old = 0x08;
static int def_stadium_name_moved_call_offs_new = 0x14;

/*
0000000140C60B17 | 48 89 45 00                        | mov qword ptr ss:[rbp],rax             | before kit choice read
0000000140C60B1B | 41 C6 45 FE 01                     | mov byte ptr ds:[r13-2],1              |
0000000140C60B20 | 41 C6 45 00 00                     | mov byte ptr ds:[r13],0                |
0000000140C60B25 | 8B D6                              | mov edx,esi                            |
*/
static BYTE pattern_check_kit_choice[17] =
    "\x48\x89\x45\x00"
    "\x41\xc6\x45\xfe\x01"
    "\x41\xc6\x45\x00\x00"
    "\x8b\xd6";
static int offs_check_kit_choice = 4;

/*
0000000141E76770 | 49 FF C0                           | inc r8                                 |
0000000141E76773 | 42 38 04 03                        | cmp byte ptr ds:[rbx+r8],al            |
0000000141E76777 | 75 F7                              | jne pes2020.141E76770                  |
...
0000000141E76789 | 4C 8B 2D 38 AA 90 01               | mov r13,qword ptr ds:[1437811C8]       |
0000000141E76790 | 4D 85 ED                           | test r13,r13                           |
*/
static BYTE pattern_get_uniparam[10] =
    "\x49\xff\xc0"
    "\x42\x38\x04\x03"
    "\x75\xf7";
static int offs_get_uniparam = 0x20 - 4;

/*
0000000140F6E3A8 | 89 43 6C                           | mov dword ptr ds:[rbx+6C],eax        |
0000000140F6E3AB | C7 43 18 07 00 00 00               | mov dword ptr ds:[rbx+18],7          |
0000000140F6E3B2 | 83 7B 18 06                        | cmp dword ptr ds:[rbx+18],6          |
0000000140F6E3B6 | 75 08                              | jne pes2020.140F6E3C0                |
...
0000000140F6E3D1 | 48 8B 5C 24 30                     | mov rbx,qword ptr ss:[rsp+30]        |
0000000140F6E3D6 | 48 83 C4 20                        | add rsp,20                           |
0000000140F6E3DA | 5D                                 | pop rbp                              |
0000000140F6E3DB | C3                                 | ret                                  |
*/
static BYTE pattern_data_ready[17] =
    "\x89\x43\x6c"
    "\xc7\x43\x18\x07\x00\x00\x00"
    "\x83\x7b\x18\x06"
    "\x75\x08";
static int offs_data_ready = 0xd1-0xa8;

/*
0000000141C0DC50 | 85 C0                              | test eax,eax                         |
0000000141C0DC52 | 75 26                              | jne pes2020.141C0DC7A                |
0000000141C0DC54 | 8B D3                              | mov edx,ebx                          |
0000000141C0DC56 | 48 8B CF                           | mov rcx,rdi                          |
...
0000000141C0DC68 | E8 6F 07 36 FF                     | call pes2020.140F6E3DC               | call to move
*/
static BYTE pattern_call_to_move[10] =
    "\x85\xc0"
    "\x75\x26"
    "\x8b\xd3"
    "\x48\x8b\xcf";
static int offs_call_to_move = 0x68-0x50;

/*
00000001553111E4 | 85 C0                              | test eax,eax                           |
00000001553111E6 | 75 26                              | jne pes2020.15531120E                  |
00000001553111E8 | 89 DA                              | mov edx,ebx                            |
00000001553111EA | 48 89 F9                           | mov rcx,rdi                            |
...
00000001553111FC | E8 7F 0B 27 EC                     | call pes2020.141581D80                 | call to move
*/
static BYTE pattern2_call_to_move[10] =
    "\x85\xc0"
    "\x75\x26"
    "\x89\xda"
    "\x48\x89\xf9";
static int offs2_call_to_move = 0xfc-0xe4;

/*
00000001505F09CC | 44 0F B6 4B 4E                     | movzx r9d,byte ptr ds:[rbx+4E]       |
00000001505F09D1 | 44 0F B6 43 4D                     | movzx r8d,byte ptr ds:[rbx+4D]       |
00000001505F09D6 | 0F B6 53 4C                        | movzx edx,byte ptr ds:[rbx+4C]       |

0000000141E7521D | 44 0F B6 4B 4A                     | movzx r9d,byte ptr ds:[rbx+4A]         |
0000000141E75222 | 44 0F B6 43 49                     | movzx r8d,byte ptr ds:[rbx+49]         |
0000000141E75227 | 0F B6 53 48                        | movzx edx,byte ptr ds:[rbx+48]         |
*/
static BYTE pattern_kit_status[15] =
    "\x44\x0f\xb6\x4b\x4a"
    "\x44\x0f\xb6\x43\x49"
    "\x0f\xb6\x53\x48";
static int offs_kit_status = 0;

/*
0000000150A7259F | 31 C2                              | xor edx,eax                          |
0000000150A725A1 | 81 E2 FF 3F 00 00                  | and edx,3FFF                         |
0000000150A725A7 | 31 C2                              | xor edx,eax                          |
0000000150A725A9 | 41 89 51 10                        | mov dword ptr ds:[r9+10],edx         | set team id (edit mode?)

0000000141E747CC | 33 D0                              | xor edx,eax                            |
0000000141E747CE | 81 E2 FF 3F 00 00                  | and edx,3FFF                           |
0000000141E747D4 | 33 D0                              | xor edx,eax                            |
0000000141E747D6 | 41 89 11                           | mov dword ptr ds:[r9],edx              | set team id
0000000141E747D9 | 40 B6 01                           | mov sil,1                              |
*/
static BYTE pattern_set_team_for_kits[17] =
    "\x33\xd0"
    "\x81\xe2\xff\x3f\x00\x00"
    "\x33\xd0"
    "\x41\x89\x11"
    "\x40\xb6\x01";
static int offs_set_team_for_kits = 0;

/*
0000000150A74D73 | 89 8A FC FF FF FF                  | mov dword ptr ds:[rdx-4],ecx         | clear (reset) team id (for kits)
0000000150A74D79 | C7 42 18 FF FF 00 00               | mov dword ptr ds:[rdx+18],FFFF       |
0000000150A74D80 | C7 42 30 FF FF FF FF               | mov dword ptr ds:[rdx+30],FFFFFFFF   |

00000001561F1320 | 89 0A                              | mov dword ptr ds:[rdx],ecx             | clear team for kits
00000001561F1322 | C7 42 18 FF FF 00 00               | mov dword ptr ds:[rdx+18],FFFF         |
00000001561F1329 | C7 42 30 FF FF FF FF               | mov dword ptr ds:[rdx+30],FFFFFFFF     |
...
00000001561F1364 | 66 45 89 51 49                     | mov word ptr ds:[r9+49],r10w           |
00000001561F1369 | 45 88 51 48                        | mov byte ptr ds:[r9+48],r10b           |
00000001561F136D | C3                                 | ret                                    |
*/
static BYTE pattern_clear_team_for_kits[11] =
    //"\x89\x0a"
    //"\xc7\x42\x18\xff\xff\x00\x00"
    //"\xc7\x42\x30\xff\xff\xff\xff";
    "\x66\x45\x89\x51\x49"
    "\x45\x88\x51\x48"
    "\xc3";
static int offs_clear_team_for_kits = -(0x364-0x320);

/*
0000000141E40B7F | C7 44 24 40 01 00 00 00            | mov dword ptr ss:[rsp+40],1            | loaded uniparam
0000000141E40B87 | 45 33 C0                           | xor r8d,r8d                            |
0000000141E40B8A | 41 8D 50 20                        | lea edx,qword ptr ds:[r8+20]           |
0000000141E40B8E | 48 8D 4C 24 40                     | lea rcx,qword ptr ss:[rsp+40]          |
...
0000000141E40BAF | 48 89 46 50                        | mov qword ptr ds:[rsi+50],rax          | wrote +0x50
0000000141E40BB3 | C6 46 7B 01                        | mov byte ptr ds:[rsi+7B],1             |
0000000141E40BB7 | 48 8B 5C 24 48                     | mov rbx,qword ptr ss:[rsp+48]          |
0000000141E40BBC | 48 8B 74 24 50                     | mov rsi,qword ptr ss:[rsp+50]          |
*/
static BYTE pattern_uniparam_loaded[19] =
    "\x48\x89\x46\x50"
    "\xc6\x46\x7b\x01"
    "\x48\x8b\x5c\x24\x48"
    "\x48\x8b\x74\x24\x50";
static int offs_uniparam_loaded = -(0xbaf - 0xb7f - 8);

/*
00000001420A0D81 | 8B 47 0C                           | mov eax,dword ptr ds:[rdi+C]           |
00000001420A0D84 | 89 86 2C 01 00 00                  | mov dword ptr ds:[rsi+12C],eax         | copy clock 2
00000001420A0D8A | 8B 47 10                           | mov eax,dword ptr ds:[rdi+10]          |
00000001420A0D8D | 89 86 30 01 00 00                  | mov dword ptr ds:[rsi+130],eax         |
*/
static BYTE pattern_copy_clock[19] =
    "\x8b\x47\x0c"
    "\x89\x86\x2c\x01\x00\x00"
    "\x8b\x47\x10"
    "\x89\x86\x30\x01\x00\x00";
static int offs_copy_clock = 0;

/*
0000000156BF2F05 | 49 8D 9E E0 00 00 00               | lea rbx,qword ptr ds:[r14+E0]          | scoreboard clear routine
0000000156BF2F0C | 49 8D B6 D0 00 00 00               | lea rsi,qword ptr ds:[r14+D0]          |
0000000156BF2F13 | 8D 6F 02                           | lea ebp,qword ptr ds:[rdi+2]           |
*/
static BYTE pattern_clear_sc[18] =
    "\x49\x8d\x9e\xe0\x00\x00\x00"
    "\x49\x8d\xb6\xd0\x00\x00\x00"
    "\x8d\x6f\x02";
static int offs_clear_sc = 0;

/**
00000001403F8D27 | FF50 10                  | call qword ptr ds:[rax+10]                 |
00000001403F8D2A | 8B4B 0C                  | mov ecx,dword ptr ds:[rbx+C]               |
00000001403F8D2D | 48:8D55 B8               | lea rdx,qword ptr ss:[rbp-48]              |
00000001403F8D31 | E8 40C01A01              | call <JMP.&XInputGetState>                 | 1415A4D76
... then to:
00000001415A4D76 | FF25 AC02F900            | jmp qword ptr ds:[<&XInputGetState>]       |
00000001415A4D7C | FF25 AE02F900            | jmp qword ptr ds:[<&XInputSetState>]       |
00000001415A4D82 | CC                       | int3                                       |
**/
static BYTE pattern_xinput[12] =
    "\xff\x50\x10"
    "\x8b\x4b\x0c"
    "\x48\x8d\x55\xb8"
    "\xe8";
static int offs_xinput = 11;

#endif
