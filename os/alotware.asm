;---------------------------------------------------------;
;                    Alotware Kernel		 	  ;
;                                                         ;
;             Written by: Muazzam Ali Kazmi		  ;
;                     Public Domain                       ;
;						          ;
;		      Main functions		    	  ;
;---------------------------------------------------------;

use16					;Tell assembler to generate 16-bit code

;Setup Segments	
;
	mov ax, 50h
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

;Setup Stack	
;
	cli
	mov ax, 0x5000
	mov ss, ax
	mov sp, 0
	
	mov byte[bootDrive], dl
	mov dword[BPBAddress], ebp
	
;Enable a20 line to access full 4-gb memory
;
	mov ax, 0x2401
	int 0x15

	call go32			;Setup 32-bit mode

use32					;Tell assembler to generate 32-bit code

KStart:
	mov ax, 0x10
	mov ds, ax
	mov ax, 0x18			;Setup es base to 0
	mov ss, ax
	mov fs, ax
	mov gs, ax
	mov es, ax	
	mov esp, 0x10000		;Setup Stack pointer

	cli
	
	call initKeyboard
	call initMouse
	;call initSynaptics
	call initDisk

	;Set timer frequency
	mov eax, 100			;Set timer frequency 1.19 mhz / eax
	out 0x40, al			;First send lower byte
	
	mov al, ah			;Now send higher byte
	out 0x40, al

	call setTextMode

	;Install IRQ Handlers
	mov esi, timerHandler
	mov eax, 8			;IRQ0
	call installISR

	mov esi, kbdHandler	
	mov eax, 9			;IRQ1
	call installISR
	
	mov esi, APIint100
	mov eax, 100
	call installISR

	sti				;Enable interrupts
		
;Enable SSE
;	
	mov eax, cr0
	or eax, 10b			;Coprocessor monitor
	and ax, 1111111111111011b	;Disable coprocessor emulation
	mov cr0, eax
	
	mov eax, cr4
;Floating point exceptions
;
	or ax, 001000000000b
	or ax, 010000000000b
	mov cr4, eax

;Set graphics or text mode
;	
	call askForVideoMode

;Load shell
;
	mov eax, 0			;No arguments
	mov esi, defaultShell
	clc
	call loadProgram

	jnc .end

.notFound:
	mov esi, noShell
	mov dx, 0
	call setCursor
	call printString
	
	hlt
	jmp $

.end:
	call clearScreen

;Print shell finished message and halt the operating system
;
	mov esi, CLIfinished
	mov dx, 0
	call setCursor
	call printString	

	hlt
	jmp $

progExtension:  db '.bin', 0		;Default extension of applications
progAddress:	dd 0x1000000	 	;Address of next program to load

videoBuffer1: dd 0x100000
videoBuffer2: dd 0x100000

defaultShell:	db 'cli.bin', 0		;First program to execute

noShell:	db 'Shell not found!',0
CLIfinished:	db 'Shell exited - Operating System halted', 0

isUserMode:	db 0
isGraphicsMode:	db 0

programCount:  	dw 0

PCB.esp:	times 5 dd 0
.pointer:	dd 0

PCB.size:	times 5 dd 0
.pointer:	dd 0

;________________________________________________
;Ask for video mode and select it.
;IN/OUT: nothing
;
askForVideoMode:
	mov esi, .prompt
	call printString
	
.getKey:
	call waitKeyboard
	
	cmp al, 'a'
	je .text
	cmp al, 'A'
	je .text
	
	cmp al, 'b'
	je .graphics1
	cmp al, 'B'
	je .graphics1
	
	cmp al, 'c'
	je .graphics2
	cmp al, 'C'
	je .graphics2
	
	jmp .getKey
	
.text:	ret				;Already in text mode
	
.graphics1:
	mov word[VBEMode], 0x115	;I should not assume video mode number
	call setGraphicsMode
	ret

.graphics2:
	mov word[VBEMode], 0x118	;I should not assume video mode number
	call setGraphicsMode

	ret
		
.prompt: db 10, "Tip: Choose 'B' for emulators, 'C' for virtual machines and real hardware.", 10, 10
	 db "Video mode (A: Text mode), (B: 800x600), (C: 1024x768):", 0
;________________________________________________
;Load a program from disk and execute it.
;IN: 	esi Program name,
;	edi Program arguments.
;	eax=0 if no arguments.
;OUT: 	CF sets on error or not exists
;	clears on success
;
loadProgram:
	pusha

;Copy program arguments to a well-known address
;
	cmp eax, 0
	je .noArguments
	
	push esi
	push es
	
	mov esi, edi
	call stringLength
	
	mov ecx, eax
	inc ecx
	
	push 0x18
	pop es
	
	mov esi, edi
	
	mov edi, PROG_ARGS

	rep movsb		;Copy ecx characters string from esi to edi
	
	pop es
	pop esi
	
	jmp .next1
	
.noArguments:
	mov byte[gs:PROG_ARGS], 0
	
.next1:
	call fileExists
	jc .error
	
	mov ebx, eax

	add ebx, [.lastProgSize]

	mov eax, [PCB.size.pointer]
	add eax, PCB.size
	mov dword[eax], ebx
	
	mov dword[.lastProgSize], ebx

	add dword[PCB.size.pointer], 4

	cmp dword[PCB.size.pointer], 4*4
	ja .error

	add dword[progAddress], ebx

	mov edi, dword[progAddress]
	sub edi, 0x500
	call fileLoad
	jc .error
	

;Now we have to calculate program's data and code base address
;and put it into program's data and code GDT entry
;

	mov eax, dword[progAddress]
	mov edx, eax
	and eax, 0xffff
	
	mov word[GDT.programCode+2], ax	
	mov word[GDT.programData+2], ax	

	mov eax, edx
	shr eax, 16
	and eax, 0xff

	mov byte[GDT.programCode+4], al
	mov byte[GDT.programData+4], al
	
	mov eax, edx
	shr eax, 24
	and eax, 0xff
	
	mov byte[GDT.programCode+7], al
	mov byte[GDT.programData+7], al

	lgdt[GDTReg]


	mov eax, [PCB.esp.pointer]
	add eax, PCB.esp
	mov dword[eax], esp

	add dword[PCB.esp.pointer], 4
	cmp dword[PCB.esp.pointer], 4*4
	ja .error

	sti			;Make sure interrupts are available
	pushfd			;New flag
	push 0x30		;New CS
	push dword 0		;New IP
	
	inc word[programCount]

	mov edi, PROG_ARGS
	sub edi, dword[progAddress]
	mov ax, 0x38
	mov ds, ax
	iret

.programEnded:

	dec word[programCount]	
	
	mov ax, 0x10
	mov ds, ax
	
	mov eax, [PCB.size.pointer]
	add eax, PCB.size
	sub eax, 4
	mov ebx, dword[eax]

	sub dword[progAddress], ebx

	sub dword[PCB.size.pointer], 4


	mov eax, dword[bytesAllocated]
	sub dword[progAddress], eax

	mov dword[bytesAllocated], 0

;Calculate program's data and code base address
;and put it into program's data and code GDT entry
;

	mov eax, dword[progAddress]
	mov edx, eax
	and eax, 0xffff
	
	mov word[GDT.programCode+2], ax	
	mov word[GDT.programData+2], ax	

	mov eax, edx
	shr eax, 16
	and eax, 0xff

	mov byte[GDT.programCode+4], al
	mov byte[GDT.programData+4], al
	
	mov eax, edx
	shr eax, 24
	and eax, 0xff
	
	mov byte[GDT.programCode+7], al
	mov byte[GDT.programData+7], al

	lgdt[GDTReg]

	sub dword[PCB.esp.pointer], 4

	clc
	jmp short .end
.error:
	stc
.end:
	popa
	ret
.lastProgSize: 	dd 0

;________________________________________________
;Generate a pseudo random 
;IN: eax maximum
;OUT: eax number
;
rand:
	mov ecx, eax
	
	mov eax, [.rn]
	mov ebx, 1103515245
	mul ebx
	add eax, 12345
	mov [.rn], eax
	
	mov ebx, 65536
	mov edx, 0
	div ebx
	
	mov ebx, ecx
	mov edx, 0
	div ebx
	mov eax, edx
	ret
.rn:	dd 1
	
;________________________________________________
;Seed the random number generator
;IN: eax number
;OUT: nothing
;
srand:
	mov [rand.rn], eax
	ret

;________________________________________________
;Restart the computer
;IN/OUT: nothing
;
reboot:
.waitLoop:
	in al, 0x64		;0x64 is status register
	bt ax, 1		;Check 2nd bit to become 0
	jnc .OK
	jmp .waitLoop
.OK:
	mov al, 0xfe
	out 0x64, al

	cli
	jmp $
	ret

.status: db 0

;________________________________________________
;Terminate the current program
;IN/OUT: nothing
;
terminate:
	pop eax

	mov ax, 0x10
	mov ds, ax
	
	cmp byte[isGraphicsMode], 0
	je .noGraphicsMode

.noGraphicsMode:
	
	mov eax, [PCB.esp.pointer]
	add eax, PCB.esp
	sub eax, 4
	mov esp, dword[eax]

	mov eax, loadProgram.programEnded
	push 0x08
	push eax
	retf

;________________________________________________
;Allocate memory to program
;IN:  eax number 4-kib blocks to allocate
;OUT: esi address of allocated memory
;
memoryAllocate:
	mov esi, 0
	ret

bytesAllocated: dd 0

;________________________________________________
;deallocate allocated memory
;IN:  eax number of 4-kib blocks to deallocate
;OUT: nothing
;
memoryDeallocate:
	mov esi, 0
	ret

;________________________________________________

use32				;Tell assembler to generate 32-bit code
;-----------Include features and functions------------
include "string.inc"		;Functions for strings
include "video.inc"		;Functions for video
include "keyboard.inc"		;Functions for keyboard
include "isr.inc"		;Interrupt service routines and IRQ Handlers
include "cpumodes.inc"		;IDT, GDT, functions for set pmode and real mode
include "biosint.inc"		;Real mode BIOS interrupts
include "disk.inc"		;Functions to read/write disks
include "api.inc"		;Application Programming Interface
include "graphics.inc"		;Functions for graphics
include "mouse.inc"		;Functions for Mouse
include "fonts.inc"		;Graphics mode fonts
;----------------------------------------------------

kEnd:
VBE_MODE_BLOCK	=	kEnd+0		;VBE Mode Block Buffer
DISK_BUFFER	=	kEnd+1024	;Disk Buffer for loading sectors
PROG_ARGS	=	kEnd+60000+0x500
