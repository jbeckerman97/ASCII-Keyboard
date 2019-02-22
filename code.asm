; Final Project
; Written by Jacob Beckerman
; CSI-370-02
; Capture input from keyboard and pass to final.cpp which will display a keyboard with the appropriate buttons pushed down
; Due 4/30/18
; All references used are included in documentation

includelib C:\Irvine\Irvine32.lib
include C:\Irvine\Irvine32.inc
include C:\Irvine\Macros.inc
include C:\Irvine\VirtualKeys.inc

displayKeyboard PROTO C, key:DWORD, flag:DWORD

.DATA
keyCode WORD 0
flag DWORD 0

.CODE
   asmMain PROC C
      push ebp
      mov ebp, esp

      call displayKeyboard

      KeyPress:
         mov eax, 70
         call Delay

         xor edx, edx
         call ReadKey ; This is a no-wait function call
         jz KeyPress ; Without this, displayKeyboard would print until program crashes

         mov keyCode, dx
         mov flag, ebx

         cmp dx, VK_ESCAPE
         jz ExitProg

         xor ebx, ebx ; Clear ebx to hold just flag
         mov ebx, flag

         xor edx, edx ; Clear dx to hold just keyCode
         mov dx, keyCode

         push dx  ; Push virtual key code
         push ebx ; Push flag

         call displayKeyboard
         add esp, 48 ; 32 for ebx, 16 for dx

         jmp KeyPress
      
      ExitProg:
         pop ebp
         exit
   asmMain ENDP
END