bits 32

section .text

global restore_process
restore_process:
    push ebp
    mov ebp, esp

    mov ax, (4 * 8) | 3 ; ring 3 data with bottom 2 bits set for ring 3
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax ; SS is handled by iret

    ; set up stack frame iret expects
    mov esp, [ebp + 24]
    mov eax, esp
    push (4 * 8) | 3 ; data
    push eax
    pushf
    push (3 * 8) | 3 ; code
    push dword [ebp + 40]
    mov eax, [ebp + 8]
    mov ebx, [ebp + 12]
    mov ecx, [ebp + 16]
    mov edx, [ebp + 20]
    ; mov esp, [ebp + 24]
    mov esi, [ebp + 32]
    mov edi, [ebp + 36]
    mov ebp, [ebp + 28]
    iret

global flush_tss
flush_tss:
    mov ax, (5 * 8) | 0 ; fifth 8-byte selector, symbolically OR-ed with 0 to set the RPL (requested privilege level).
    ltr ax
    ret

test_user_function:
   mov eax, 2
   mov ebx, 3
   mov ecx, 4
   mov edx, 5
   int 0x80


global jump_usermode
jump_usermode:
    mov ax, (4 * 8) | 3 ; ring 3 data with bottom 2 bits set for ring 3
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax ; SS is handled by iret

    ; set up the stack frame iret expects
    mov eax, esp
    push (4 * 8) | 3 ; data selector
    push eax ; current esp
    pushf ; eflags
    push (3 * 8) | 3 ; code selector (ring 3 code with bottom 2 bits set for ring 3)
    push test_user_function ; instruction address to return to
    iret


; This macro creates a routine that will call our isr_handler function
; in Rust, passing the number of the interrupt recieved.
extern isr_handler
%macro handler_macro 1
    ; Commented instrs: we should probably be saving all of these segments and stuff,
    ; but we don't have a userspace right now, so it's ok for a minute
    pusha
    mov ebx, %1
    cmp ebx, 0x80
    jne push_zero_syscall_args%1
    mov ecx, [esp + 44] ; ecx now points to the userspace esp
    push dword [ecx + 12]
    push dword [ecx + 8]
    push dword [ecx + 4]
    push dword [ecx]
    jmp call_isr_handler%1
push_zero_syscall_args%1:
    push 0
    push 0
    push 0
    push 0
call_isr_handler%1:
    push dword [esp + 4]
    push %1
    ; Here's what the stack looks like at the time of calling isr_handler:
    ; interrupt_num
    ; info
    ; syscall1
    ; syscall2
    ; syscall3
    ; syscall4
    ; edi
    ; esi
    ; ebp fe88
    ; *kernel* esp - not the saved userspace esp
    ; ebx
    ; edx
    ; ecx
    ; eax
    ; eip
    ; cs
    ; eflags
    ; esp - this is sthe esp from userspace - was pushed by the hardware
    ; ds
    call isr_handler
    add esp, (4 * 6)

    mov ebx, %1
    cmp ebx, 0x80
    jne isr_handler_ret%1
syscall_modify_eax%1:
    mov [esp + 28], eax ; write eax for popa
isr_handler_ret%1:
	popa
	iret ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP

.end:
%endmacro

global int0
int0: handler_macro 0
global int1
int1: handler_macro 1
global int2
int2: handler_macro 2
global int3
int3: handler_macro 3
global int4
int4: handler_macro 4
global int5
int5: handler_macro 5
global int6
int6: handler_macro 6
global int7
int7: handler_macro 7
global int8
int8: handler_macro 8
global int9
int9: handler_macro 9
global int10
int10: handler_macro 10
global int11
int11: handler_macro 11
global int12
int12: handler_macro 12
global int13
int13: handler_macro 13
global int14
int14: handler_macro 14
global int15
int15: handler_macro 15
global int16
int16: handler_macro 16
global int17
int17: handler_macro 17
global int18
int18: handler_macro 18
global int19
int19: handler_macro 19
global int20
int20: handler_macro 20
global int21
int21: handler_macro 21
global int22
int22: handler_macro 22
global int23
int23: handler_macro 23
global int24
int24: handler_macro 24
global int25
int25: handler_macro 25
global int26
int26: handler_macro 26
global int27
int27: handler_macro 27
global int28
int28: handler_macro 28
global int29
int29: handler_macro 29
global int30
int30: handler_macro 30
global int31
int31: handler_macro 31
global int32
int32: handler_macro 32
global int33
int33: handler_macro 33
global int34
int34: handler_macro 34
global int35
int35: handler_macro 35
global int36
int36: handler_macro 36
global int37
int37: handler_macro 37
global int38
int38: handler_macro 38
global int39
int39: handler_macro 39
global int40
int40: handler_macro 40
global int41
int41: handler_macro 41
global int42
int42: handler_macro 42
global int43
int43: handler_macro 43
global int44
int44: handler_macro 44
global int45
int45: handler_macro 45
global int46
int46: handler_macro 46
global int47
int47: handler_macro 47
global int48
int48: handler_macro 48
global int49
int49: handler_macro 49
global int50
int50: handler_macro 50
global int51
int51: handler_macro 51
global int52
int52: handler_macro 52
global int53
int53: handler_macro 53
global int54
int54: handler_macro 54
global int55
int55: handler_macro 55
global int56
int56: handler_macro 56
global int57
int57: handler_macro 57
global int58
int58: handler_macro 58
global int59
int59: handler_macro 59
global int60
int60: handler_macro 60
global int61
int61: handler_macro 61
global int62
int62: handler_macro 62
global int63
int63: handler_macro 63
global int64
int64: handler_macro 64
global int65
int65: handler_macro 65
global int66
int66: handler_macro 66
global int67
int67: handler_macro 67
global int68
int68: handler_macro 68
global int69
int69: handler_macro 69
global int70
int70: handler_macro 70
global int71
int71: handler_macro 71
global int72
int72: handler_macro 72
global int73
int73: handler_macro 73
global int74
int74: handler_macro 74
global int75
int75: handler_macro 75
global int76
int76: handler_macro 76
global int77
int77: handler_macro 77
global int78
int78: handler_macro 78
global int79
int79: handler_macro 79
global int80
int80: handler_macro 80
global int81
int81: handler_macro 81
global int82
int82: handler_macro 82
global int83
int83: handler_macro 83
global int84
int84: handler_macro 84
global int85
int85: handler_macro 85
global int86
int86: handler_macro 86
global int87
int87: handler_macro 87
global int88
int88: handler_macro 88
global int89
int89: handler_macro 89
global int90
int90: handler_macro 90
global int91
int91: handler_macro 91
global int92
int92: handler_macro 92
global int93
int93: handler_macro 93
global int94
int94: handler_macro 94
global int95
int95: handler_macro 95
global int96
int96: handler_macro 96
global int97
int97: handler_macro 97
global int98
int98: handler_macro 98
global int99
int99: handler_macro 99
global int100
int100: handler_macro 100
global int101
int101: handler_macro 101
global int102
int102: handler_macro 102
global int103
int103: handler_macro 103
global int104
int104: handler_macro 104
global int105
int105: handler_macro 105
global int106
int106: handler_macro 106
global int107
int107: handler_macro 107
global int108
int108: handler_macro 108
global int109
int109: handler_macro 109
global int110
int110: handler_macro 110
global int111
int111: handler_macro 111
global int112
int112: handler_macro 112
global int113
int113: handler_macro 113
global int114
int114: handler_macro 114
global int115
int115: handler_macro 115
global int116
int116: handler_macro 116
global int117
int117: handler_macro 117
global int118
int118: handler_macro 118
global int119
int119: handler_macro 119
global int120
int120: handler_macro 120
global int121
int121: handler_macro 121
global int122
int122: handler_macro 122
global int123
int123: handler_macro 123
global int124
int124: handler_macro 124
global int125
int125: handler_macro 125
global int126
int126: handler_macro 126
global int127
; int128 is defined specially above
int127: handler_macro 127
global int128
int128: handler_macro 128
global int129
int129: handler_macro 129
global int130
int130: handler_macro 130
global int131
int131: handler_macro 131
global int132
int132: handler_macro 132
global int133
int133: handler_macro 133
global int134
int134: handler_macro 134
global int135
int135: handler_macro 135
global int136
int136: handler_macro 136
global int137
int137: handler_macro 137
global int138
int138: handler_macro 138
global int139
int139: handler_macro 139
global int140
int140: handler_macro 140
global int141
int141: handler_macro 141
global int142
int142: handler_macro 142
global int143
int143: handler_macro 143
global int144
int144: handler_macro 144
global int145
int145: handler_macro 145
global int146
int146: handler_macro 146
global int147
int147: handler_macro 147
global int148
int148: handler_macro 148
global int149
int149: handler_macro 149
global int150
int150: handler_macro 150
global int151
int151: handler_macro 151
global int152
int152: handler_macro 152
global int153
int153: handler_macro 153
global int154
int154: handler_macro 154
global int155
int155: handler_macro 155
global int156
int156: handler_macro 156
global int157
int157: handler_macro 157
global int158
int158: handler_macro 158
global int159
int159: handler_macro 159
global int160
int160: handler_macro 160
global int161
int161: handler_macro 161
global int162
int162: handler_macro 162
global int163
int163: handler_macro 163
global int164
int164: handler_macro 164
global int165
int165: handler_macro 165
global int166
int166: handler_macro 166
global int167
int167: handler_macro 167
global int168
int168: handler_macro 168
global int169
int169: handler_macro 169
global int170
int170: handler_macro 170
global int171
int171: handler_macro 171
global int172
int172: handler_macro 172
global int173
int173: handler_macro 173
global int174
int174: handler_macro 174
global int175
int175: handler_macro 175
global int176
int176: handler_macro 176
global int177
int177: handler_macro 177
global int178
int178: handler_macro 178
global int179
int179: handler_macro 179
global int180
int180: handler_macro 180
global int181
int181: handler_macro 181
global int182
int182: handler_macro 182
global int183
int183: handler_macro 183
global int184
int184: handler_macro 184
global int185
int185: handler_macro 185
global int186
int186: handler_macro 186
global int187
int187: handler_macro 187
global int188
int188: handler_macro 188
global int189
int189: handler_macro 189
global int190
int190: handler_macro 190
global int191
int191: handler_macro 191
global int192
int192: handler_macro 192
global int193
int193: handler_macro 193
global int194
int194: handler_macro 194
global int195
int195: handler_macro 195
global int196
int196: handler_macro 196
global int197
int197: handler_macro 197
global int198
int198: handler_macro 198
global int199
int199: handler_macro 199
global int200
int200: handler_macro 200
global int201
int201: handler_macro 201
global int202
int202: handler_macro 202
global int203
int203: handler_macro 203
global int204
int204: handler_macro 204
global int205
int205: handler_macro 205
global int206
int206: handler_macro 206
global int207
int207: handler_macro 207
global int208
int208: handler_macro 208
global int209
int209: handler_macro 209
global int210
int210: handler_macro 210
global int211
int211: handler_macro 211
global int212
int212: handler_macro 212
global int213
int213: handler_macro 213
global int214
int214: handler_macro 214
global int215
int215: handler_macro 215
global int216
int216: handler_macro 216
global int217
int217: handler_macro 217
global int218
int218: handler_macro 218
global int219
int219: handler_macro 219
global int220
int220: handler_macro 220
global int221
int221: handler_macro 221
global int222
int222: handler_macro 222
global int223
int223: handler_macro 223
global int224
int224: handler_macro 224
global int225
int225: handler_macro 225
global int226
int226: handler_macro 226
global int227
int227: handler_macro 227
global int228
int228: handler_macro 228
global int229
int229: handler_macro 229
global int230
int230: handler_macro 230
global int231
int231: handler_macro 231
global int232
int232: handler_macro 232
global int233
int233: handler_macro 233
global int234
int234: handler_macro 234
global int235
int235: handler_macro 235
global int236
int236: handler_macro 236
global int237
int237: handler_macro 237
global int238
int238: handler_macro 238
global int239
int239: handler_macro 239
global int240
int240: handler_macro 240
global int241
int241: handler_macro 241
global int242
int242: handler_macro 242
global int243
int243: handler_macro 243
global int244
int244: handler_macro 244
global int245
int245: handler_macro 245
global int246
int246: handler_macro 246
global int247
int247: handler_macro 247
global int248
int248: handler_macro 248
global int249
int249: handler_macro 249
global int250
int250: handler_macro 250
global int251
int251: handler_macro 251
global int252
int252: handler_macro 252
global int253
int253: handler_macro 253
global int254
int254: handler_macro 254
global int255
int255: handler_macro 255

global isr_handler_table
isr_handler_table:
%assign i 0
%rep    256
    dd int%+i ; use DQ instead if targeting 64-bit
%assign i i+1
%endrep
