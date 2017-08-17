;---------------------------------------------------------;
;                    Alotware Kernel		 	  ;
;                                                         ;
;             Written by: Muazzam Ali Kazmi		  ;
;                     Public Domain                       ;
;                                                         ;
;		   BIOS Interrupt calls			  ;
;---------------------------------------------------------;

;________________________________________________
;Real-mode BIOS interrupt 10h for screen related functions.
;IN/OUT:  Same as BIOS interrupt 10h
;
int10h:
	use32
	cli
	
	mov word[axreg], ax
	mov word[bxreg], bx
	mov word[cxreg], cx
	mov word[dxreg], dx
	mov word[direg], di
	mov word[sireg], si
	mov dword[ebpreg], ebp
	mov dword[espreg], esp

	push eax
	push edx
	
	call go16
	use16

	mov ax, word[axreg]
	mov bx, word[bxreg]
	mov cx, word[cxreg]
	mov dx, word[dxreg]
	mov si, word[sireg]
	mov di, word[direg]
	
	int 10h
	call go32
	use32

	mov ax, 0x10
	mov ds, ax
	mov ax, 0x18			;Setup es,ss,gs base to 0
	mov ss, ax
	mov es, ax	
	mov gs, ax
	mov esp, dword[espreg]
	sub esp, 4*2
	
	pop edx
	pop eax

	mov ebp, dword[ebpreg]
	sti
 	ret


;________________________________________________
;Real-mode BIOS interrupt 13h for disk related functions.
;IN/OUT:  Same as BIOS interrupt 13h 
;
int13h:
	use32
	cli
	mov word[axreg], ax
	mov word[bxreg], bx
	mov word[cxreg], cx
	mov word[dxreg], dx
	mov word[direg], di
	mov word[sireg], si
	mov dword[ebpreg], ebp
	mov dword[espreg], esp

	push eax
	push edx
	
	call go16
	use16

	mov bx, word[bxreg]
	mov cx, word[cxreg]
	mov dx, word[dxreg]
	mov si, word[sireg]
	mov di, word[direg]
	mov ax, word[axreg]
	
	int 13h
	
	pushf
	pop ax
	
	mov word[flagreg], ax		;Save flag (for error checking)

	mov word[axreg], ax
	call go32
	use32

	mov ax, 0x10
	mov ds, ax
	mov ax, 0x18			;Setup es,gs,ss base to 0
	mov ss, ax
	mov gs, ax
	mov es, ax	
	mov esp, dword[espreg]
	sub esp, 4*2
	
	pop edx
	pop eax

	mov ebp, dword[ebpreg]

	pushfd
	pop eax
	
	or ax, word[flagreg]
	
	push eax
	popfd
	
	mov ax, word[axreg]
	sti
 	ret



;--------------Registers----------------;
axreg:		 dw 	0
bxreg:		 dw 	0
cxreg:		 dw	0
dxreg:		 dw	0
sireg:		 dw	0
direg:		 dw	0
ebpreg:	         dd 	0
espreg:	         dd 	0
flagreg:	 dd	0
