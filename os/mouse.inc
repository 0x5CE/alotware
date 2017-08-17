;---------------------------------------------------------;
;                    Alotware Kernel		 	  ;
;                                                         ;
;             Written by: Muazzam Ali Kazmi		  ;
;                     Public Domain                       ;
;                                                         ;
;	         Mouse Related Functions		  ;
;---------------------------------------------------------;

;_______________________________________
;Initialize the ps2 mouse.
;IN/OUT: nothing
;
initMouse:
	push eax
	
;Enable Mouse IRQ
;
	call waitPS2Write	;Wait if PS2 is busy
	mov al, 0x20		;Get Compaq status byte
	out 0x64, al		;0x64 is status register
	
	call waitPS2Read	
	in al, 0x60
	or al, 2		;Set 2nd bit 1 to enable IRQ12
	mov bl, al		;Save modified byte
	
	call waitPS2Write
	mov al, 0x60		;Set compaq status byte
	out 0x64, al		

	call waitPS2Write
	mov al, bl		;Send modified byte
	out 0x60, al

;Enable auxiliary mouse device
;
	call waitPS2Write
	mov al, 0xa8		;Enable auxiliary mouse device
	out 0x64, al
	
;Use default settings
;
	mov al, 0xf6		;Set default
	call ps2Send
	
	call waitPS2Read
	in al, 0x60		;Get acknowledge byte

;Set resolution
;
	mov al, 0xe8		;Set resolution
	call ps2Send
	
	call waitPS2Read
	in al, 0x60		;Get acknowledge byte

	mov al, 3		;8 count/mm
	call ps2Send

	call waitPS2Read
	in al, 0x60		;Get acknowledge byte

	
;Enable packets
;
	mov al, 0xf4		;Enable packets
	call ps2Send
	
	call waitPS2Read
	in al, 0x60		;Get acknowledge byte

	mov ax, word[resolution.y]
	mov word[ps2MouseHandler.mouseY], ax

	mov esi, ps2MouseHandler
	mov eax, 0x74		;IRQ12
	call installISR

	pop eax
	ret

;__________________________________________
;Get current mouse position and button states
;IN: nothing
;OUT:	eax Mouse X
;	ebx Mouse Y
;	edx Mouse Buttons (bit #0 left button, bit #1 right button)
;	
mouseGet:
	mov eax, [mouseX]
	mov ebx, [mouseY]
	mov edx, 0 ; byte[ps2MouseHandler.data]

	ret

;__________________________________________
;Set new mouse position
;IN:	eax Mouse X
;	ebx Mouse Y
;OUT:	nothing
;	
mouseSet:
	mov [mouseX], eax
	mov [mouseY], ebx
	mov byte[ps2MouseHandler.data], 0
	ret
	
;__________________________________________
;Initialize the Synaptics touchpad mouse.
;IN/OUT: nothing
;
initSynaptics:
	push eax

	mov al, 0xf5		;Disable
	call ps2Send

	call waitPS2Read
	in al, 0x60		;Get acknowledge byte

	mov al, 0xe8
	call ps2Send
	
	call waitPS2Read
	in al, 0x60		;Get acknowledge byte

	mov al, 0x03
	call ps2Send

	call waitPS2Read
	in al, 0x60		;Get acknowledge byte

	mov al, 0xe8
	call ps2Send

	call waitPS2Read
	in al, 0x60		;Get acknowledge byte

	mov al, 0x00
	call ps2Send

	call waitPS2Read
	in al, 0x60		;Get acknowledge byte

	mov al, 0xe8
	call ps2Send

	call waitPS2Read
	in al, 0x60		;Get acknowledge byte

	mov al, 0x00
	call ps2Send

	call waitPS2Read
	in al, 0x60		;Get acknowledge byte

	mov al, 0xe8
	call ps2Send

	call waitPS2Read
	in al, 0x60		;Get acknowledge byte

	mov al, 0x01
	call ps2Send

	call waitPS2Read
	in al, 0x60		;Get acknowledge byte

	mov al, 0xf3
	call ps2Send

	call waitPS2Read
	in al, 0x60		;Get acknowledge byte

	mov al, 0x14
	call ps2Send

	call waitPS2Read
	in al, 0x60		;Get acknowledge byte

	mov al, 0xf4		;Enable
	call ps2Send

	call waitPS2Read
	in al, 0x60		;Get acknowledge byte

	mov esi, synapticsHandler
	mov eax, 0x74		;IRQ12
	call installISR

	pop eax
	ret
;__________________________________________
;Send command or data to ps/2 controller
;IN: al command
;OUT: nothing
;
ps2Send:
	xchg bl, al		;Save al

	call waitPS2Write
	mov al, 0xd4		;We are sending a command
	out 0x64, al

	call waitPS2Write	
	xchg bl, al		;Get al back
	out 0x60, al

	ret
;__________________________________________
;Wait for mouse event and get mouse values
;IN: nothing
;OUT:	eax Mouse X
;	ebx Mouse Y
;	edx Mouse Buttons (bit #0 left button, bit #1 right button)
;
waitMouse:
	sti

	mov byte[ps2MouseHandler.changed], 0
.wait:
	cmp byte[ps2MouseHandler.changed], 1	;Check if mouse state changed
	hlt
	jne .wait
		
	mov eax, [mouseX]
	mov ebx, [mouseY]
	movzx edx, byte[ps2MouseHandler.data]
	
	ret

mouseX:	dd 0
mouseY: dd 0
