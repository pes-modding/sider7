;------------------------------------------
;Descr  : hooking utils
;
;Small functions to help with hooking
;Typically, they would call a function
;from sider, which does all the actual work
;------------------------------------------

extern sider_read_file:proc
extern sider_get_size:proc
extern sider_mem_copy:proc
extern sider_lookup_file:proc
extern sider_set_team_id:proc
extern sider_set_settings:proc
extern sider_trophy_check:proc
extern sider_context_reset:proc
extern sider_free_select:proc
extern sider_trophy_table:proc
extern sider_ball_name:proc
extern sider_stadium_name:proc
extern sider_def_stadium_name:proc
extern sider_set_stadium_choice:proc
extern sider_check_kit_choice:proc
extern sider_data_ready:proc
extern sider_kit_status:proc
extern sider_set_team_for_kits:proc
extern sider_clear_team_for_kits:proc
extern sider_loaded_uniparam:proc
extern sider_clear_sc:proc
extern sider_set_edit_team_id:proc
extern sider_custom_event:proc

extern _sci:dq

.code
sider_read_file_hk proc

        mov     rax,[rsp+28h]
        sub     rsp,38h
        mov     [rsp+20h],rax
        mov     rax,[r9+20h]
        mov     [rsp+28h],rax
        call    sider_read_file
        ;mov     r12,[rsp+28h]
        add     rsp,38h
        ret

sider_read_file_hk endp

sider_get_size_hk proc

        sub     rsp,28h
        mov     [rsp+20h],rdx
        mov     rcx,rsi
        mov     rdx,rbx
        call    sider_get_size
        mov     rcx,qword ptr [rdi+1d0h]
        mov     eax,1
        mov     rdx,[rsp+20h]
        add     rsp,28h
        ret

sider_get_size_hk endp

sider_extend_cpk_hk proc

        mov     rax,1000000000000000h
        mov     qword ptr [rdi+8],rax
        mov     qword ptr [r13],rdi
        xor     rax,rax
        ret

sider_extend_cpk_hk endp

sider_mem_copy_hk proc

        push    rsp
        sub     rsp,20h
        mov     rcx,r11
        call    sider_mem_copy
        mov     qword ptr [rdi+10h],rbx
        add     rsp,28h
        ret

sider_mem_copy_hk endp

sider_lookup_file_hk proc

        push    rax
        sub     rsp,20h
        call    sider_lookup_file
        lea     rcx,qword ptr [rdi+108h]
        mov     r8,rsi
        lea     rdx,qword ptr [rsp+50h]
        add     rsp,20h
        pop     rax
        ret

sider_lookup_file_hk endp

;00000001414C09D0 | 49:6300                  | movsxd rax,dword ptr ds:[r8]            | at set team id
;00000001414C09D3 | 83F8 02                  | cmp eax,2                               |
;00000001414C09D6 | 7D 16                    | jge pes2021.1414C09EE                   |
;00000001414C09D8 | 4C:69C0 90060000         | imul r8,rax,690                         |
;00000001414C09DF | 48:81C1 38010000         | add rcx,138                             |
;00000001414C09E6 | 49:03C8                  | add rcx,r8                              |
;00000001414C09E9 | E9 E24E65FF              | jmp pes2021.140B158D0                   |
;00000001414C09EE | C3                       | ret                                     |

sider_set_team_id_hk proc

        mov     rdx,[rsp+8h]
        push    r9
        push    r10
        push    r11
        sub     rsp,40h
        movsxd  rax,dword ptr [r8]
        mov     [rsp+30h],rax
        cmp     eax,2
        jge     done
        imul    r8,rax,690h
        add     rcx,138h
        add     rcx,r8
        mov     [rsp+20h],rcx
        mov     [rsp+28h],r8
        call    sider_set_team_id
        mov     rcx,[rsp+20h]
        mov     r8,[rsp+28h]
        mov     rax,[rsp+30h]
done:   add     rsp,40h
        pop     r11
        pop     r10
        pop     r9
        ret

sider_set_team_id_hk endp

;0000000150E94EB3 | 48 8B 82 98 00 00 00               | mov rax,qword ptr ds:[rdx+98]        |
;0000000150E94EBA | 48 89 81 98 00 00 00               | mov qword ptr ds:[rcx+98],rax        |
;0000000150E94EC1 | 48 89 C8                           | mov rax,rcx                          | set_settings
;0000000150E94EC4 | C3                                 | ret                                  |

sider_set_settings_hk proc

        pushfq
        push    rcx
        push    rdx
        push    r8
        push    r9
        push    r10
        push    r11
        sub     rsp,20h
        mov     rax,qword ptr [rdx+98h]
        mov     qword ptr [rcx+98h],rax
        call    sider_set_settings
        add     rsp,20h
        pop     r11
        pop     r10
        pop     r9
        pop     r8
        pop     rdx
        pop     rcx
        popfq
        ret

sider_set_settings_hk endp

;0000000141F5184E | 4C:8DAB 00C40000         | lea r13,qword ptr ds:[rbx+C400]         |
;0000000141F51855 | 8B8B 98040000            | mov ecx,dword ptr ds:[rbx+498]          | c2
;0000000141F5185B | 41:80E6 01               | and r14b,1                              |
;0000000141F5185F | D1FD                     | sar ebp,1                               |

sider_trophy_check_hk proc

        push    rax
        sub     rsp,20h
        mov     ecx,dword ptr [rbx+498h]
        call    sider_trophy_check
        mov     ecx,eax
        and     r14b,1
        sar     ebp,1
        add     rsp,20h
        pop     rax
        ret

sider_trophy_check_hk endp

;0000000141F5659F | 4C:8DAB 00C40000         | lea r13,qword ptr ds:[rbx+C400]            |
;0000000141F565A6 | 44:0FB6B3 04E90000       | movzx r14d,byte ptr ds:[rbx+E904]          |
;0000000141F565AE | 48:8D83 04EC0000         | lea rax,qword ptr ds:[rbx+EC04]            |
;0000000141F565B5 | D1FD                     | sar ebp,1                                  |
;0000000141F565B7 | 41:80E6 01               | and r14b,1                                 |
;...
;0000000141F565DF | 8B8B 98040000            | mov ecx,dword ptr ds:[rbx+498]             |
;0000000141F565E5 | 898F 70510000            | mov dword ptr ds:[rdi+5170],ecx            |

sider_trophy_check2_hk proc

        push    rax
        sub     rsp,20h
        mov     ecx,dword ptr [rbx+498h]
        call    sider_trophy_check
        mov     ecx,eax
        mov     dword ptr [rdi+5170h],ecx
        add     rsp,20h
        pop     rax
        ret

sider_trophy_check2_hk endp

sider_context_reset_hk proc

        push    rcx
        push    rdx
        push    r8
        push    r9
        push    r10
        push    r11
        sub     rsp,28h
        mov     qword ptr [rbx+84h],rcx
        mov     qword ptr [rbx+201c4h],0ffffffffh
        call    sider_context_reset
        add     rsp,28h
        pop     r11
        pop     r10
        pop     r9
        pop     r8
        pop     rdx
        pop     rcx
        ret

sider_context_reset_hk endp

sider_free_select_hk proc

        push    rax
        sub     rsp,20h
        movsd   xmm0,qword ptr [rax]
        movsd   qword ptr [rbx+0a8h],xmm0
        lea     rcx,[rbx+0a8h]
        call    sider_free_select
        add     rsp,20h
        pop     rax
        ret

sider_free_select_hk endp

sider_trophy_table_hk proc

        push    rax
        push    r11
        sub     rsp,28h
        mov     rbx,qword ptr [r11+30h]
        mov     rsi,qword ptr [r11+38h]
        mov     rdi,qword ptr [r11+40h]
        lea     rcx,[rsp+40h]
        call    sider_trophy_table
        add     rsp,28h
        pop     r11
        pop     rax
        ret

sider_trophy_table_hk endp

;000000014D9A071F | 49 83 C8 FF                          | or r8,FFFFFFFFFFFFFFFF           |
;000000014D9A0723 | 49 FF C0                             | inc r8                           |
;000000014D9A0726 | 42 80 3C 02 00                       | cmp byte ptr ds:[rdx+r8],0       |
;000000014D9A072B | 75 F6                                | jne pes2019.14D9A0723            |
;000000014D9A072D | 48 89 C1                             | mov rcx,rax                      | rcx:dst,rdx:src,r8:len

sider_ball_name_hk proc

        sub     rsp,38h
        mov     [rsp+20h],rcx
        mov     [rsp+28h],rdx
        mov     rcx,rdx
        call    sider_ball_name
        mov     rdx,rax
        mov     rcx,[rsp+20h]
        or      r8,0ffffffffffffffffh
next:   inc     r8
        cmp     byte ptr [rdx+r8],0
        jne     next
        mov     rax,[rsp+40h]
        mov     rcx,rax
        add     rsp,38h
        ret

sider_ball_name_hk endp

;000000014CFE651F | 49 83 C8 FF                  | or r8,FFFFFFFFFFFFFFFF                 |
;000000014CFE6523 | 49 FF C0                     | inc r8                                 |
;000000014CFE6526 | 42 80 3C 02 00               | cmp byte ptr ds:[rdx+r8],0             | rdx+r8*1:"Allianz Parque"
;000000014CFE652B | 75 F6                        | jne pes2019.14CFE6523                  |
;000000014CFE652D | 48 89 C1                     | mov rcx,rax                            | rcx:dst,rdx:src,r8:len

sider_stadium_name_hk proc

        sub     rsp,38h
        mov     [rsp+20h],rcx
        mov     [rsp+28h],rdx
        ;mov     rcx,rdx
        push    r8
        push    r9
        mov     r8,rdi
        mov     r9,rbx
        call    sider_stadium_name
        pop     r9
        pop     r8
        mov     rdx,rax
        mov     rcx,[rsp+20h]
        or      r8,0ffffffffffffffffh
next:   inc     r8
        cmp     byte ptr [rdx+r8],0
        jne     next
        mov     rax,[rsp+40h]
        mov     rcx,rax
        add     rsp,38h
        ret

sider_stadium_name_hk endp

;000000014D33447D | 48 85 C0                             | test rax,rax                           |
;000000014D334480 | 74 0D                                | je pes2019.14D33448F                   |
;000000014D334482 | 48 89 F2                             | mov rdx,rsi                            |
;000000014D334485 | 48 89 C1                             | mov rcx,rax                            |
;000000014D334488 | E8 63 C4 C4 F4                       | call pes2019.141F808F0                 |
;000000014D33448D | EB 12                                | jmp pes2019.14D3344A1                  |
;000000014D33448F | 45 31 C0                             | xor r8d,r8d                            |
;000000014D334492 | 48 8D 15 E9 F0 1F F5                 | lea rdx,qword ptr ds:[142533582]       |
;000000014D334499 | 48 89 F1                             | mov rcx,rsi                            |
;000000014D33449C | E8 6F E9 17 F3                       | call pes2019.1404B2E10                 |

sider_def_stadium_name_hk proc

        push    rcx
        push    rdx
        push    r8
        push    r9
        push    r10
        push    r11
        sub     rsp,28h
        call    sider_def_stadium_name
        add     rsp,28h
        pop     r11
        pop     r10
        pop     r9
        pop     r8
        pop     rdx
        pop     rcx
        ret

sider_def_stadium_name_hk endp

sider_set_stadium_choice_hk proc

        push    rcx
        push    rdx
        push    r8
        push    r9
        push    r10
        push    r11
        sub     rsp,28h   ; we get here via jmp, but stack still needs correct alignment
        call    sider_set_stadium_choice
        add     rsp,28h
        pop     r11
        pop     r10
        pop     r9
        pop     r8
        pop     rdx
        pop     rcx
        ret

sider_set_stadium_choice_hk endp

;0000000140C60B17 | 48 89 45 00                        | mov qword ptr ss:[rbp],rax             |
;0000000140C60B1B | 41 C6 45 FE 01                     | mov byte ptr ds:[r13-2],1              |
;0000000140C60B20 | 41 C6 45 00 00                     | mov byte ptr ds:[r13],0                |
;0000000140C60B25 | 8B D6                              | mov edx,esi                            |

sider_check_kit_choice_hk proc

        push    rcx
        push    r8
        push    r9
        push    r10
        push    r11
        push    r14
        sub     rsp,28h
        mov     rcx,r14   ;mis - match info struct
        mov     rdx,r10   ;0/1 - home/away
        call    sider_check_kit_choice
        add     rsp,28h
        pop     r14
        mov     byte ptr [r13-2],1
        mov     byte ptr [r13],0
        mov     edx,esi
        pop     r11
        pop     r10
        pop     r9
        pop     r8
        pop     rcx
        ret

sider_check_kit_choice_hk endp

;000000014000E877 | C7 43 18 07 00 00 00            | mov dword ptr ds:[rbx+18],7            |
;000000014000E87E | 89 43 6C                        | mov dword ptr ds:[rbx+6C],eax          |
;000000014000E881 | 83 7B 18 06                     | cmp dword ptr ds:[rbx+18],6            |
;000000014000E885 | 75 08                           | jne pes2019.14000E88F                  |
;...
;000000014000E8A0 | 48 8B 5C 24 30                  | mov rbx,qword ptr ss:[rsp+30]          |
;000000014000E8A5 | 48 83 C4 20                     | add rsp,20                             |
;000000014000E8A9 | 5D                              | pop rbp                                |
;000000014000E8AA | C3                              | ret                                    |

sider_data_ready_hk proc

        push    rcx
        push    rdx
        push    r8
        push    r9
        push    r10
        push    r11
        sub     rsp,20h   ; we get here via jmp, so stack already aligned
        mov     rcx,rbx   ; buffer structure
        call    sider_data_ready
        add     rsp,20h
        pop     r11
        pop     r10
        pop     r9
        pop     r8
        pop     rdx
        pop     rcx
        mov     rbx,qword ptr [rsp+30h]  ; original replaced code
        add     rsp,20h
        pop     rbp
        ret

sider_data_ready_hk endp

;00000001505F09CC | 44 0F B6 4B 4E                     | movzx r9d,byte ptr ds:[rbx+4E]       |
;00000001505F09D1 | 44 0F B6 43 4D                     | movzx r8d,byte ptr ds:[rbx+4D]       |
;00000001505F09D6 | 0F B6 53 4C                        | movzx edx,byte ptr ds:[rbx+4C]       |

;0000000141E7521D | 44 0F B6 4B 4A                     | movzx r9d,byte ptr ds:[rbx+4A]         |
;0000000141E75222 | 44 0F B6 43 49                     | movzx r8d,byte ptr ds:[rbx+49]         |
;0000000141E75227 | 0F B6 53 48                        | movzx edx,byte ptr ds:[rbx+48]         |

sider_kit_status_hk proc

        push    rcx
        push    r10
        push    r11
        push    rax
        sub     rsp,28h
        mov     rcx,rbx
        mov     rdx,rax
        call    sider_kit_status
        movzx   r9d, byte ptr [rbx+4ah]
        movzx   r8d, byte ptr [rbx+49h]
        movzx   rdx, byte ptr [rbx+48h]
        add     rsp,28h
        pop     rax
        pop     r11
        pop     r10
        pop     rcx
        ret

sider_kit_status_hk endp

;0000000141E747CC | 33 D0                              | xor edx,eax                            |
;0000000141E747CE | 81 E2 FF 3F 00 00                  | and edx,3FFF                           |
;0000000141E747D4 | 33 D0                              | xor edx,eax                            |
;0000000141E747D6 | 41 89 11                           | mov dword ptr ds:[r9],edx              | set team id (for kits)

sider_set_team_for_kits_hk proc

        push    r8
        push    r9
        push    r10
        push    r11
        sub     rsp,28h
        xor     edx,eax
        and     edx,3fffh
        xor     edx,eax
        mov     dword ptr [r9],edx
        mov     rcx,rbx
        call    sider_set_team_for_kits
        mov     rcx,3fffh
        add     rsp,28h
        pop     r11
        pop     r10
        pop     r9
        pop     r8
        ret

sider_set_team_for_kits_hk endp

;00000001561F1320 | 89 0A                              | mov dword ptr ds:[rdx],ecx             | clear team for kits
;00000001561F1322 | C7 42 18 FF FF 00 00               | mov dword ptr ds:[rdx+18],FFFF         |
;00000001561F1329 | C7 42 30 FF FF FF FF               | mov dword ptr ds:[rdx+30],FFFFFFFF     |

sider_clear_team_for_kits_hk proc

        push    rcx
        push    rdx
        push    r8
        push    r9
        push    r10
        push    r11
        sub     rsp,28h
        mov     dword ptr [rdx],ecx
        mov     dword ptr [rdx+18h],0ffffh
        mov     dword ptr [rdx+30h],0ffffffffh
        mov     rcx,rbx
        call    sider_clear_team_for_kits
        mov     rax,3fffh
        add     rsp,28h
        pop     r11
        pop     r10
        pop     r9
        pop     r8
        pop     rdx
        pop     rcx
        ret

sider_clear_team_for_kits_hk endp

;00000001404BE666 | 45 33 C0                           | xor r8d,r8d                            |
;00000001404BE669 | 41 8D 50 20                        | lea edx,qword ptr ds:[r8+20]           |
;00000001404BE66D | 48 8D 4C 24 40                     | lea rcx,qword ptr ss:[rsp+40]          |

sider_loaded_uniparam_hk proc

        push    rcx
        push    rdx
        push    r8
        push    r9
        push    r10
        push    r11
        sub     rsp,28h
        mov     rcx,rax
        call    sider_loaded_uniparam
        mov     [rsi+48h],rax
        add     rsp,28h
        pop     r11
        pop     r10
        pop     r9
        pop     r8
        pop     rdx
        pop     rcx
        xor     r8d,r8d
        lea     edx,qword ptr [r8+20h]
        lea     rcx,qword ptr [rsp+48h]
        ret

sider_loaded_uniparam_hk endp

;00000001420A0D81 | 8B 47 0C                           | mov eax,dword ptr ds:[rdi+C]           |
;00000001420A0D84 | 89 86 2C 01 00 00                  | mov dword ptr ds:[rsi+12C],eax         | copy clock 2
;00000001420A0D8A | 8B 47 10                           | mov eax,dword ptr ds:[rdi+10]          |
;00000001420A0D8D | 89 86 30 01 00 00                  | mov dword ptr ds:[rsi+130],eax         |

sider_copy_clock_hk proc

        mov     _sci,rsi
        mov     eax,dword ptr [rdi+0ch]
        mov     dword ptr [rsi+12ch],eax
        mov     eax,dword ptr [rdi+10h]
        mov     dword ptr [rsi+130h],eax
        ret

sider_copy_clock_hk endp

;0000000156BF2F05 | 49 8D 9E E0 00 00 00               | lea rbx,qword ptr ds:[r14+E0]          | scoreboard clear routine
;0000000156BF2F0C | 49 8D B6 D0 00 00 00               | lea rsi,qword ptr ds:[r14+D0]          |
;0000000156BF2F13 | 8D 6F 02                           | lea ebp,qword ptr ds:[rdi+2]           |

sider_clear_sc_hk proc

        push    rcx
        push    rdx
        push    r8
        push    r9
        push    r10
        push    r11
        sub     rsp,28h
        mov     rcx,r14
        call    sider_clear_sc
        add     rsp,28h
        pop     r11
        pop     r10
        pop     r9
        pop     r8
        pop     rdx
        pop     rcx
        lea rbx, qword ptr [r14+0e0h]
        lea rsi, qword ptr [r14+0d0h]
        ret

sider_clear_sc_hk endp

;0000000141F0DA7D | 25 00C0FFFF                   | and eax,FFFFC000                           |
;0000000141F0DA82 | 3D 0040FEFF                   | cmp eax,FFFE4000                           |
;0000000141F0DA87 | 75 09                         | jne pes2021.141F0DA92                      |
;0000000141F0DA89 | C741 20 00000D00              | mov dword ptr ds:[rcx+20],D0000            |
;0000000141F0DA90 | EB 11                         | jmp pes2021.141F0DAA3                      |
;0000000141F0DA92 | 48:8D41 20                    | lea rax,qword ptr ds:[rcx+20]              |
;0000000141F0DA96 | 4C:8D5424 18                  | lea r10,qword ptr ss:[rsp+18]              |
;0000000141F0DA9B | 49:3BC2                       | cmp rax,r10                                |
;0000000141F0DA9E | 74 03                         | je pes2021.141F0DAA3                       |
;0000000141F0DAA0 | 44:8900                       | mov dword ptr ds:[rax],r8d                 | set edit team id
;0000000141F0DAA3 | 8B4424 28                     | mov eax,dword ptr ss:[rsp+28]              |
;0000000141F0DAA7 | 8941 28                       | mov dword ptr ds:[rcx+28],eax              |
;0000000141F0DAAA | 8B4424 30                     | mov eax,dword ptr ss:[rsp+30]              |
;0000000141F0DAAE | 8941 2C                       | mov dword ptr ds:[rcx+2C],eax              |
;0000000141F0DAB1 | B0 01                         | mov al,1                                   |
;0000000141F0DAB3 | 44:8949 24                    | mov dword ptr ds:[rcx+24],r9d              |
;0000000141F0DAB7 | 48:8951 10                    | mov qword ptr ds:[rcx+10],rdx              |
;0000000141F0DABB | 48:C701 02000000              | mov qword ptr ds:[rcx],2                   |
;0000000141F0DAC2 | C3                            | ret                                        |

sider_set_edit_team_id_hk proc

        push    rcx
        push    rdx
        push    r8
        push    r9
        push    r10
        push    r11
        sub     rsp,28h
        mov     rcx,r8
        and     rcx,0ffffffffh
        call    sider_set_edit_team_id
        mov     al,1
        add     rsp,28h
        pop     r11
        pop     r10
        pop     r9
        pop     r8
        pop     rdx
        pop     rcx
        ret

sider_set_edit_team_id_hk endp

; Generic custom event trigger
; Expects two parameters on the stack
; - original value of rcx (1st param)
; - original value of rbx (2nd param)
sider_custom_event_rbx_hk proc

        mov     rbx,[rsp+10h]   ;original rbx value
        push    rsp
        push    r15
        push    r14
        push    r13
        push    r12
        push    r11
        push    r10
        push    r9
        push    r8
        push    rbp
        push    rdi
        push    rsi
        push    rdx
        push    rcx
        push    rbx
        push    rax
        pushfq
        sub     rsp,10h
        movdqu  [rsp],xmm15
        sub     rsp,10h
        movdqu  [rsp],xmm14
        sub     rsp,10h
        movdqu  [rsp],xmm13
        sub     rsp,10h
        movdqu  [rsp],xmm12
        sub     rsp,10h
        movdqu  [rsp],xmm11
        sub     rsp,10h
        movdqu  [rsp],xmm10
        sub     rsp,10h
        movdqu  [rsp], xmm9
        sub     rsp,10h
        movdqu  [rsp],xmm8
        sub     rsp,10h
        movdqu  [rsp],xmm7
        sub     rsp,10h
        movdqu  [rsp],xmm6
        sub     rsp,10h
        movdqu  [rsp],xmm5
        sub     rsp,10h
        movdqu  [rsp],xmm4
        sub     rsp,10h
        movdqu  [rsp],xmm3
        sub     rsp,10h
        movdqu  [rsp],xmm2
        sub     rsp,10h
        movdqu  [rsp],xmm1
        sub     rsp,10h
        movdqu  [rsp],xmm0
        mov     rax,[rsp+190h]  ;original rcx value
        mov     [rsp+118h],rax  ;copy to saved location (REGISTERS.rcx)

        mov     rdx,rsp         ;params: 1st: rcx - already has value, 2nd: rdx - pointer to REGISTERS struct

        ; ensure correct stack alignment (on 10h boundary, before call is made)
        mov     rax,rsp
        and     rax,8h
        cmp     rax,8h
        je      alignment8

        sub     rsp,20h
        call    sider_custom_event
        add     rsp,20h
        jmp     restore_registers

alignment8:
        sub     rsp,28h
        call    sider_custom_event
        add     rsp,28h

restore_registers:
        movdqu  xmm0,[rsp]
        add     rsp,10h
        movdqu  xmm1,[rsp]
        add     rsp,10h
        movdqu  xmm2,[rsp]
        add     rsp,10h
        movdqu  xmm3,[rsp]
        add     rsp,10h
        movdqu  xmm4,[rsp]
        add     rsp,10h
        movdqu  xmm5,[rsp]
        add     rsp,10h
        movdqu  xmm6,[rsp]
        add     rsp,10h
        movdqu  xmm7,[rsp]
        add     rsp,10h
        movdqu  xmm8,[rsp]
        add     rsp,10h
        movdqu  xmm9,[rsp]
        add     rsp,10h
        movdqu  xmm10,[rsp]
        add     rsp,10h
        movdqu  xmm11,[rsp]
        add     rsp,10h
        movdqu  xmm12,[rsp]
        add     rsp,10h
        movdqu  xmm13,[rsp]
        add     rsp,10h
        movdqu  xmm14,[rsp]
        add     rsp,10h
        movdqu  xmm15,[rsp]
        add     rsp,10h
        popfq
        pop     rax
        pop     rbx
        pop     rcx
        pop     rdx
        pop     rsi
        pop     rdi
        pop     rbp
        pop     r8
        pop     r9
        pop     r10
        pop     r11
        pop     r12
        pop     r13
        pop     r14
        pop     r15
        pop     rsp
        ret

sider_custom_event_rbx_hk endp

end
