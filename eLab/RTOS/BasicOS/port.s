    AREA |.text|, CODE, READONLY, ALIGN=2
    THUMB
    REQUIRE8
    PRESERVE8

PendSV_Handler   PROC

    EXPORT PendSV_Handler

    IMPORT      eos_current         ; extern variable */
    IMPORT      eos_next            ; extern variable */
    IMPORT      addr_target         ; extern variable */
    IMPORT      addr_source         ; extern variable */
    IMPORT      copy_size           ; extern variable */

    CPSID       I                   ; disable interrupts (set PRIMASK) */

    LDR         r1,=eos_current     ; if (eos_current != 0)
                                    ; { */
    LDR         r1,[r1,#0x00]
    CMP         r1, #0
    BEQ         PendSV_restore
    NOP
    PUSH        {r4-r7}             ;       push r4-r11 into stack */
    MOV         r4, r8
    MOV         r5, r9
    MOV         r6, r10
    MOV         r7, r11
    PUSH        {r4-r7}
    LDR         r1,=eos_current     ;       eos_current->sp = sp; */
    LDR         r1,[r1,#0x00]
    MOV         r2, SP
    STR         r2,[r1,#0x00]       ; } */

PendSV_restore
    LDR         r4, =addr_source
    LDR         r0, [R4]
    LDR         r4, =addr_target
    LDR         r1, [R4]
    LDR         r4, =copy_size
    LDR         r2, [R4]
    CMP         r0, r1              ; Check the target addr is at front of the source.
    BNE         LoopStart           ; If yes, copy from front to back.
    ADD         r0, r2              ; If not, copy from back to front.
    SUBS        r1, r1, r2          ; Calculate the new source address.

LoopStart
    MOV         r4, r0              ; Save the source address.
    MOV         r5, r1              ; Save the target address.
    MOV         r2, r2              ; Save the copying size.
    LDR         r3, =0              ; clear counting.
    
Loop2
    LDR         r0, [r4]            ; Read one byte from the source address.
    STR         r0, [r5]            ; Write the byte into target address.
    ADDS        r3, r3, #4           ; Counting + 1
    ADDS        r4, r4, #4
    ADDS        r5, r5, #4
    CMP         r3, r2              ; Check the end of copying.
    BNE         Loop2               ; If not end, continue

    LDR         r1,=eos_next        ; sp = eos_next->sp; */
    LDR         r1,[r1,#0x00]
    LDR         r0,[r1,#0x00]
    MOV         SP, r0
    LDR         r1,=eos_next        ; eos_current = eos_next; */
    LDR         r1,[r1,#0x00]
    LDR         r2,=eos_current
    STR         r1,[r2,#0x00]
    POP         {r4-r7}
    MOV         r8, r4
    MOV         r9, r5
    MOV         r10,r6
    MOV         r11,r7
    POP         {r4-r7}
    CPSIE       I                   ; enable interrupts (clear PRIMASK) */
    BX          lr                  ; return to the next task */
    ENDP

    ALIGN   4

    END

