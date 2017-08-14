;---------------------------------------------------------;
;                    Alotware Kernel		 	  ;
;                                                         ;
;             Written by: Muazzam Ali Kazmi		  ;
;                     Public Domain                       ;
;                                                         ;
;             P-Mode, Real-Mode and GDT, IDT		  ;
;---------------------------------------------------------;

;______________________________________________________________________________________________________
;Go to 32-bit protected Mode
;IN/OUT:  nothing

go32:
use16					;Still in 16-bit mode
	cli

	pop bp				;Return address
	
;Load descriptors
;
	lgdt[GDTReg] 			;Load GDT
	lidt[IDTReg]			;Load IDT

;Enter 32-bit now
;
	mov eax, cr0
	or eax, 1			;Set protected mode bit 1
	mov cr0, eax

;Return
;
	push 0x08			;New cs
	push bp				;New eip
	retf


GDT:		dd 0, 0		;null Descriptor
.Code:		dw 0xffff	;Limit (0:15)	
		dw 0x0500	;Base (0:15)
		db 0		;Base (16:23)	
		db 10011010b	;Pr=1,Privl=00,Resr=1,ex=1,C=0,R=1,Ac=0
		db 11001111b	;Gr=1,Sz=1,Resr=00,Limit(16:19)
		db 0		;Base (24:31)


;Data Descriptor base on 500h
.Data:		dw 0xffff	;Limit (0:15)	
		dw 0x0500	;Base (0:15)
		db 0		;Base (16:23)	
		db 10010010b	;Pr=1,Privl=00,Resr=1,ex=0,D=0,W=1,Ac=0
		db 11001111b	;Gr=1,Sz=1,Resr=00,Limit(16:19)
		db 0		;Base (24:31)

;Data Descriptor base on 0h
.linear:	dw 0xffff	;Limit (0:15)	
		dw 0		;Base (0:15)
		db 0		;Base (16:23)	
		db 10010010b	;Pr=1,Privl=00,Resr=1,ex=0,D=0,W=1,Ac=0
		db 11001111b	;Gr=1,Sz=1,Resr=00,Limit(16:19)
		db 0		;Base (24:31)

;Code Descriptor for 16-bit protected mode
.codePM16:	dw 0xffff	;Limit (0:15)	
		dw 0x0500	;Base (0:15)
		db 0		;Base (16:23)	
		db 10011010b	;Pr=1,Privl=00,Resr=1,ex=1,C=0,R=1,Ac=0
		db 0		;Gr=1,Sz=1,Resr=00,Limit(16:19)
		db 0		;Base (24:31)

;Data Descriptor for 16-bit protected mode
.dataPM16:	dw 0xffff	;Limit (0:15)	
		dw 0		;Base (0:15)
		db 0		;Base (16:23)	
		db 10010010b	;Pr=1,Privl=00,Resr=1,ex=0,D=0,W=1,Ac=0
		db 0		;Gr=1,Sz=1,Resr=00,Limit(16:19)
		db 0		;Base (24:31)

;Program's Code
.programCode:	dw 0xffff	;Limit (0:15)	
		dw 0		;Base (0:15)
		db 0		;Base (16:23)	
		db 10011010b	;Pr=1,Privl=00,Resr=1,ex=1,C=0,R=1,Ac=0
		db 11001111b	;Gr=1,Sz=1,Resr=00,Limit(16:19)
		db 0		;Base (24:31)


;Program's Data
.programData:	dw 0xffff	;Limit (0:15)	
		dw 0		;Base (0:15)
		db 0		;Base (16:23)	
		db 10010010b	;Pr=1,Privl=00,Resr=1,ex=0,D=0,W=1,Ac=0
		db 11001111b	;Gr=1,Sz=1,Resr=00,Limit(16:19)
		db 0		;Base (24:31)



GDTReg:		dw 63		;GDT Size - 1
		dd GDT+0x500	;GDT Offset


IDT:		times 256 dw unhandled, 0x0008, 0x8e00, 0
		;unhandled:	Unhandled is Offset (0:15)
		;0x0008:	0x08 is selector
		;0x8e00:	8 is p=1,dpl=00,s=1, e is i386 interrupt gate, 00 is reserved
		;0:		Offset (16:31)

IDTReg:		dw 2047		;IDT Size - 1
		dd IDT+0x500	;IDT Offset

;________________________________________________
;Go to 16-bit real Mode
;IN/OUT:  nothing
;
go16:
use32					;Tell assembler to generate 32-bit code
	
	cli			;Clear interrupts
	pop edx			;save return location in edx
	
	jmp 0x20:PM16		;Load CS with selector 0x20

;To go to 16-bit real mode, first we have to go to 16-bit protected mode
;

use16				;Tell assembler to generate 16-bit code
PM16:
	mov ax, 0x28		;0x28 is 16-bit protected mode selector.
	mov ss, ax	
	mov sp, 0x5000		;Stack hase base

	mov eax, cr0
	and eax, 0xfffffffe	;Clear protected enable bit in cr0
	mov cr0, eax		;Disable 32-bit mode

	jmp 0x50:realMode	;Load CS and IP

realMode:
;Load segment registers with 16-bit Values.
;
	mov ax, 0x50
	mov ds, ax
	mov ax, 0x6000
	mov ss, ax
	mov ax, 0
	mov es, ax
	mov sp, 0
	
	cli
	lidt[.idtR]		;Load real mode interrupt vector table
	sti
	
	push 0x50
	push dx			;Return to saved location in EDX
	retf			;Start real mode

;Real mode interrupt vector table
;
.idtR:	dw 0xffff		;Limit
	dd 0			;Base

use32					;Tell assembler to generate 32-bit code
