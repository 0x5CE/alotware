;---------------------------------------------------------;
;                    Alotware Kernel		 	  ;
;                                                         ;
;             Written by: Muazzam Ali Kazmi		  ;
;                     Public Domain                       ;
;                                                         ;
;	 Disks and File systems Related Functions	  ;
;---------------------------------------------------------;

use32					;Tell assembler to generate 32-bit code
;________________________________________________
;Load sector from disk using extended BIOS functions
;IN: 	eax Number of sectors
;	esi LBA
;	edi Buffer to load
;	cx Real-Mode Segment
;	dl Drive
;
;OUT: 	nothing
;
readSectorBIOS:
	push eax
	push esi
	
	mov dword[.DAP.totalSectors], eax	;Total sectors to load
	mov dword[.DAP.LBA], esi		;Linear Block Addres (LBA)

	mov eax, edi
	shr eax, 4
	add cx, ax
	and edi, 0xf
	
	mov word[.DAP.segment], cx		;Real-mode segment
	mov word[.DAP.offset], di
		
	mov esi, .DAP
	mov ah, 0x42				;BIOS extended read
	
	call int13h				;BIOS disk services
	jnc .noError
	
;Print error and halt
;	
	mov esi, diskError
	call printString
	
	cli
	hlt	
.noError:

	pop esi
	pop eax
	ret
.DAP:
.DAP.size:		db 16
.DAP.reserved:		db 0
.DAP.totalSectors:	dw 0
.DAP.offset:		dw 0x0000
.DAP.segment:		dw 0
.DAP.LBA:		dd 0
			dd 0

;________________________________________________
;Write sector to disk using extended BIOS functions
;IN: 	eax Number of sectors
;	esi LBA
;	edi Buffer to write
;	cx Real-Mode Segment
;	dl Drive
;
;OUT: 	nothing
;
writeSectorBIOS:
	push eax
	push esi
	
	mov dword[.DAP.totalSectors], eax	;Total sectors to write
	mov dword[.DAP.LBA], esi		;Linear Block Addres (LBA)
	
	mov eax, edi
	shr eax, 4
	add cx, ax
	and edi, 0xf
	
	mov word[.DAP.offset], di
	mov word[.DAP.segment], cx		;Real-mode segment
	
	mov esi, .DAP
	mov ah, 0x43				;BIOS extended write
	mov al, 0

	call int13h				;BIOS disk services
	jnc .noError
	
;Print error and halt
;	
	mov esi, diskError
	call printString
	
	cli
	hlt	
.noError:

	pop esi
	pop eax
	ret
.DAP:
.DAP.size:		db 16
.DAP.reserved:		db 0
.DAP.totalSectors:	dw 0
.DAP.offset:		dw 0x0000
.DAP.segment:		dw 0
.DAP.LBA:		dd 0
			dd 0

;________________________________________________
;Initialise the disk
;IN/OUT: nothing
;
initDisk:
	mov dx, 0

;Get volume information from BPB and save it
;
	mov esi, dword[BPBAddress]
	
	mov ax, word[es:esi+8]			;Bytes Per Sector
	mov word[bytesPerSector], ax

	mov al, byte[es:esi+10]			;Sectors Per Cluster
	mov byte[sectPerCluster], al
	
	mov ax, word[es:esi+11]			;Reserved Sectors
	mov word[reservedSectors], ax

	mov al, byte[es:esi+13]			;Number of FAT tables
	mov byte[totalFATs], al

	mov ax, word[es:esi+14]			;Root directory entries
	mov word[rootDirEntries], ax

	mov ax, word[es:esi+19]			;Sectors Per FAT
	mov word[sectorsPerFAT], ax

	mov eax, dword[es:esi+29]		;Total sectors
	mov dword[totalSectors], eax

;Calculate root directory size
;
;Formula:
;Size  = (rootDirEntries * 32) / bytesPerSector
;
	mov ax, word[rootDirEntries]
	shl ax, 5			;Multiply by 32
	mov bx, word[bytesPerSector]
	xor dx, dx			;dx = 0
	div bx				;ax = ax / bx
	mov word[rootDirSize], ax	;Save root dir size

;Calculate size of all (both) FAT tables
;
;Formula:
;Size  = totalFATs * sectorsPerFAT
;
	mov ax, word[sectorsPerFAT]
	movzx bx, byte[totalFATs]
	xor dx, dx			;dx = 0
	mul bx				;ax = ax * bx
	mov word[FATsSize], ax		;Save FATs' size

;Calculate data area address
;
;Forumula:
;reservedSectors + FATsSize + rootDirSize
;
	movzx eax, word[reservedSectors]	
	add ax, word[FATsSize]
	add ax, word[rootDirSize]
	mov dword[dataArea], eax
	
	
;Calculate root directory's LBA address
;
;Formula:
;LBA  = reservedSectors + FATsSize
;
	movzx esi, word[reservedSectors]
	add si, word[FATsSize]
	mov dword[rootDir], esi

;Calculate File Allocation Table's LBA address
;
;Formula:
;LBA  = reservedSectors
;
	movzx esi, word[reservedSectors]
	mov dword[FAT], esi	
	
;Calculate cluster size in bytes
;
;Forumula:
;sectPerCluster * bytesPerSector
;
	movzx eax, byte[sectPerCluster]
	movzx ebx, word[bytesPerSector]
	xor edx, edx
	mul ebx				;ax = ax * bx	
	mov dword[clusterSize], eax

	ret

;________________________________________________
;Convert FAT file name to human readable
;IN:	esi pointer to 11 char file name
;OUT:	Will modify file name
;     	Carry flag sets on wrong file name
;
FATFileNameToFileName:
	push eax
	push ebx
	push ecx
	push edi
	push esi

;Check for empty string
;
	cmp byte[esi], 0
	je .wrongFileName	;If string is empty

	cmp byte[esi+8], ' '
	jne .thereIsExtension
	call stringTrim
	
	jmp .successfull
	
.thereIsExtension:
	
;Clear temporary buffer from previous execution
;
	mov ax, ' '
	mov ecx, 12
	mov edi, .fileNameBuffer + 0x500	;Clear tempBuffer
	cld
	rep stosb
	
;Copy file name to temporary buffer
;
	pop esi			;Restore esi
	push esi
	mov edi, .fileNameBuffer+0x500
	mov ecx, 11
	rep movsb		;Copy ecx bytes from esi to edi

;Get file name without extension
;
	mov esi, .fileNameBuffer
	mov byte[esi+8], 0
	call stringTrim

;Add dot
;
	call stringLength
	mov byte[esi+eax], '.'

;Get extension
;
	pop esi
	push esi		;Restore esi
	add esi, 8
	
	mov byte[esi+3], 0
	call stringTrim

	mov ebx, eax		;Save file name length (without extension)
	
	call stringLength

;Put extension and file name together 
;
	lea edi, [.fileNameBuffer + 0x500 + ebx + 1]

	mov ecx, eax
	rep movsb		;mov ecx bytes from edi to esi

;Copy temporary buffer to actual address
;
	pop esi
	push esi	
	
	mov edi, esi
	add edi, 0x500		;ES segment
	mov esi, .fileNameBuffer
	mov ecx, 12
	rep movsb

	pop esi
	push esi
	
	add eax, ebx		;File name length + extension length
	inc eax			;Add '.' length
	mov byte[esi+eax], 0

.successfull:
	pop esi
	push esi
	
	;call stringLowerCase
	
	clc 			;Clear carry flag
	jmp .end

.wrongFileName:
	stc			;Set carry flag
.end:
	pop esi
	pop edi
	pop ecx
	pop ebx
	pop eax
	ret

.fileNameBuffer: times 13 db ' '
db 0

;________________________________________________
;Convert file name to FAT compaitable
;IN:	esi File name
;OUT:	Will modify file name
;     	Carry flag sets on wrong file name
;
fileNameToFATFileName:
	push eax
	push ebx
	push ecx
	push edx
	push edi
	push esi	
	
;Check for empty string
;
	cmp byte[esi], 0
	je .wrongFileName			;If string is empty

;Check for dot (.)
;
	mov al, '.'		;Character to find
	call stringFindChar
	jnc .dot
	
	call stringLength
	
	cmp eax, 8		;More than 8 characters not allowed
	ja .wrongFileName
	
	call stringUpperCase
	
	mov ecx, 11
	sub ecx, eax
	
	mov edx, eax
	
	pop esi
	push esi
	
	push es
	push ds
	pop es			;Make sure es = ds
	
;Make sure file name is exectly 11-char
;
	mov edi, esi
	add edi, eax
	mov al, ' '
	rep stosb
	
	pop es
	
	clc
	jmp .end

.dot:
	push eax
	
;Clear temporary buffer from previous execution
;
	mov al, ' '
	mov ecx, 12
	mov edi, .fileNameBuffer + 0x500	;Clear tempBuffer
	cld
	rep stosb
	pop eax
	
	cmp al, 1
	ja .wrongFileName	;If dot occurred more than one time

	call stringUpperCase	;File names in FAT are upper case

;Check '.' position
;
	mov ebx, 0		;ebx is dot position counter in string
.dotFindLoop:
	mov al, byte[esi]
	cmp al, '.'
	je .dotFound
	inc esi
	inc ebx
	jmp .dotFindLoop

.dotFound:
	cmp ebx, 8
	ja .wrongFileName	;If file name is more than 8 character long

	cmp ebx, 1
	jb .wrongFileName	;If file name is less than 1 character long

;Save file name to temporary buffer (without extension)
;
	pop esi			;Restore esi
	push esi	
	mov edi, .fileNameBuffer+0x500
	mov ecx, ebx
	cld
	rep movsb		;mov ecx characters from esi to buffer
	
;Now check extension
;
	pop esi			;Restore esi
	push esi
	add esi, ebx		;ebx for file name length
	add esi, 1		;1 for '.' character		

	call stringLength	;Check extension length

	cmp eax, 1
	jb .wrongFileName	;If extension is less than 1 character long
	
	cmp eax, 3
	ja .wrongFileName	;If extension is more than 3 character long

;Save extension to temporary buffer
;
	mov edi, .fileNameBuffer+0x500+8
	mov ecx, eax
	cld
	rep movsb		;mov ecx characters from esi to buffer
	
	mov byte[.fileNameBuffer+11], 0	
.successfull:
	
;Save temporary buffer to orignal esi position
;
	pop esi			;Restore esi
	push esi
	mov edi, esi
	add edi, 0x500
	mov esi, .fileNameBuffer
	mov ecx, 12
	cld
	rep movsb		;mov ecx characters from buffer to edi
	
	clc 			;Clear carry flag
	jmp .end

.wrongFileName:
	stc			;Set carry flag
.end:
	pop esi
	pop edi
	pop edx
	pop ecx
	pop ebx
	pop eax
	ret
.fileNameBuffer: times 13 db ' '
db 0

;________________________________________________
;Check if file exists
;IN: 	esi Name of file to check
;
;OUT: 	eax File size in bytes
;	ebx Pointer to Root Dir Entry
;	CF sets on file does not exists or wrong file name
;
fileExists:
	push ecx
	push edx
	push edi
	push esi
	
	call stringLength
	cmp eax, 12			
	ja .failture			;If wrong file name
	
	inc eax				;File name including 0 

;Copy file name to temporary buffer
;
	mov edi, .fileNameBuffer+0x500
	mov ecx, eax			;File name length
	cld
	rep movsb			;Times ecx mov esi string to edi

;Make file name FAT compaitable
;
	mov esi, .fileNameBuffer
	call fileNameToFATFileName
	jc .failture			;If wrong file name
	
;Load root directory from disk to find file
;
	movzx eax, word[rootDirSize]	;Sectors to load
	mov esi, dword[rootDir]		;Root directory LBA
	mov cx, 0x50			;Segment
	mov edi, DISK_BUFFER+20000	;Offset
	mov dl, byte[bootDrive]

	call readSectorBIOS

;Search the file from all entries
;
	movzx edx, word[rootDirEntries]	;Total files or folders in root dir
	mov ebx, DISK_BUFFER + 0x500 + 20000

	cld				;Clear direction flag
.fileSearchLoop:
	mov ecx, 11			;11 characters in file name
	mov edi, ebx
	mov esi, .fileNameBuffer
	rep cmpsb			;Compare ecx characters at edi and esi
	je .fileFound	

	add ebx, 32

	dec edx
	jnz .fileSearchLoop

	jmp .failture			;File not found

.fileFound:
	mov eax, dword[es:ebx+28]	;File size
	sub ebx, 0x500			;ES segment
.success:
	clc 				;Clear carry flag
	jmp .end

.failture:
	stc				;Set carry flag
	jmp .end
.end:
	pop esi
	pop edi
	pop edx
	pop ecx	
	ret
.fileNameBuffer: times 13 db ' '
db 0


;________________________________________________
;Load a file in memory
;IN: 	esi Name of file to load
;	edi Address to load file
;
;OUT: 	eax file size in bytes
;	CF sets on file not exists or wrong file name
;	
fileLoad:
	push ebx
	push ecx
	push edx
	push edi
	push esi

	mov dword[.loadAddress], edi
	
;Check if file exists and get starting cluster
;
	call fileExists	
	jc .failture
	mov [.fileSize], eax		;Save file's size
	
	mov ax, word[ebx+26]		;ebx is pointer to Root dir entry
	mov word[.cluster], ax		;Save starting cluster

;Load FAT table from disk to find all clusters of file
;
	movzx eax, word[sectorsPerFAT]	;Sectors to load
	mov esi, dword[FAT]		;Root directory LBA
	mov ecx, 0x50			;Segment
	mov edi, DISK_BUFFER+20000	;Offset
	
	mov dl, byte[bootDrive]

	call readSectorBIOS

	mov ebp, dword[clusterSize]	;Save cluster size 
	mov cx,  0x00			;Real mode segment
	mov edi, dword[.loadAddress]	;Offset
	
;Find clusters and load cluster chain
;
.clustersLoadLoop:

;Convert logical address [cluster] to LBA (physical address)
;
;Forumula:
;((cluster - 2) * sectPerCluster) + dataArea
; 
	movzx esi, word[.cluster]		
	sub esi, 2

	movzx eax, byte[sectPerCluster]		
	xor edx, edx			;dx = 0
	mul esi				;(cluster - 2) * sectPerCluster
	mov esi, eax	

	add esi, dword[dataArea]

	movzx ax, byte[sectPerCluster]	;Total sectors to load

	mov dl, byte[bootDrive]

;Load cluster at a temporary buffer
;
	push edi
	mov edi, DISK_BUFFER+0x500
	call readSectorBIOS
	pop edi

;Copy cluster to its orignal location
;
	push edi
	add edi, 0x500
	mov esi, DISK_BUFFER
	mov ecx, ebp			;ebp is bytes per cluster
	cld
	rep movsb			;mov ecx bytes from esi to edi
	pop edi

;Find next cluster from FAT table
;
	movzx ebx, word[.cluster]
	
	shl ebx, 1			;bx * 2 (2 bytes in entry)
	add ebx, DISK_BUFFER+20000	;FAT location

	mov si, word[ebx]		;SI contains next cluster

	mov word[.cluster], si		;Save it

	cmp si, 0xfff8			;0xfff8 is End Of File (EOF) marker
	jae .success

;Add free space size for new cluster 
;
	add edi, ebp			;ebp is bytes per cluster
	jmp .clustersLoadLoop

.success:
	mov eax, [.fileSize]
	clc 				;Clear carry flag
	jmp .end

.failture:
	stc				;Set carry flag
	jmp .end
.end:
	pop esi
	pop edi
	pop edx
	pop ecx
	pop ebx
	ret
.cluster	dw 0
.loadAddress:	dd 0
.fileSize:	dd 0

;________________________________________________
;Get the list of Files in root directory
;IN: 	nothing
;OUT: 	esi Pointer to files list
;	eax number of total files
filesList:
	push ebx
	push ecx
	push edx
	push edi

;Load root directory
;
	movzx eax, word[rootDirSize]	;Sectors to load
	mov esi, dword[rootDir]		;Root directory LBA
	mov cx, 0x50			;Segment
	mov edi, DISK_BUFFER+20000	;Offset
	mov dl, byte[bootDrive]

	call readSectorBIOS
	

;Build the list
;
	mov edx, DISK_BUFFER+0x500	;Index in new list
	mov ebx, 0			;Files counter
	mov esi, DISK_BUFFER+20000	;Root dir offset
	sub esi, 32

.buildListLoop:
	add esi, 32			;Next entry (32 bytes per entry)

	mov al, byte[esi+11]		;File attributes

	bt ax, 4			;If Sub directory then skip
	jc .buildListLoop
	bt ax, 3			;If Volume label then skip
	jc .buildListLoop

	cmp byte[esi+11], 0x0f		;If long file name then skip
	je .buildListLoop
	cmp byte[esi], 229		;If deleted file then skip
	je .buildListLoop
	cmp byte[esi], 0		;If last file then end
	je .listEnded			

	call FATFileNameToFileName	;Convert name to human readable

;Add file entry to the list
;
	call stringLength		;Find entry's length

	push esi
	mov edi, edx
	mov ecx, eax			;eax is string entry's string length
	rep movsb			;mov ecx bytes from esi to edi
	pop esi

;Add return character
;
	mov byte[es:edx+eax], 10
	
	inc eax				;String length + 1 character
	inc ebx				;Update files counter
	add edx, eax			;Update index into list
	jmp .buildListLoop		;Get next files

.listEnded:
	mov byte[edx-0x500], 0		;End of string
	
	mov esi, DISK_BUFFER
	mov eax, ebx
.end:
	pop edi
	pop edx
	pop ecx
	pop ebx
	ret

;________________________________________________
;Save a file on disk.
;IN: 	esi Pointer to file Name
;	edi Pointer to file Data
;	eax File Size (in bytes)
;
;OUT: 	CF sets on error or file exists,
;	clears on success
;
fileSave:
	push eax
	push ebx
	push ecx
	push edx
	push edi
	push esi
	
	mov ebp, edi			;Save edi
	mov dword[.fileSize], eax	;Save file size
	
;Create new file
;
	call fileNew
	jc .failture			;If file exists then end
	
;Load FAT table from disk
;
	movzx eax, word[sectorsPerFAT]	;Sectors to load
	mov esi, dword[FAT]		;Root directory LBA
	mov ecx, 0x50			;Segment
	mov edi, DISK_BUFFER+20000	;Offset

	mov dl, byte[bootDrive]

	call readSectorBIOS

;Calculate required clusters
;
;Forumula:
;Required Clusters = .fileSize / clusterSize
; 
	mov eax, dword[.fileSize]
	mov ebx, dword[clusterSize]
	mov edx, 0
	div ebx				;.fileSize / clusterSize
	inc eax
	
	mov dword[.requiredClusters], eax
	
	mov ecx, eax			;Loop counter
	
	mov esi, DISK_BUFFER+20000		
	add esi, (3*2)			;Reserved clusters
	mov edx, 3			;Logical cluster counter
	mov edi, DISK_BUFFER+0x500	;Pointer to free clusters list
	mov eax, 0
;Find free clusters from FAT table
;
.freeClustersFindLoop:
	mov ax, word[esi]		;Load FAT entry
		
	or ax, ax			;Compare ax with 0
	jz .freeClusterFound
	
	add esi, 2			;Next FAT entry
	inc edx
	jmp .freeClustersFindLoop
	
.freeClusterFound:

;Store free cluster into list
;
	mov word[esi], 0xffff
	
	mov ax, dx
	stosw				;mov word[es:edi], ax & add edi, 2
	
	loop .freeClustersFindLoop

	movzx edx, word[DISK_BUFFER]
	push edx			;Free cluster

;All required free clusters found
;

;Create cluster chain in FAT table
;
	mov ecx, dword[.requiredClusters]
	mov esi, DISK_BUFFER		;List of free clusters (words)
	
.createClusterChain:

	mov dx, word[esi]		;Current cluster
	
	mov edi, DISK_BUFFER+20000	;FAT table address
	shl dx, 1			;Multiply by 2
	add di, dx			;edi is pointer to current FAT entry
	
	cmp ecx, 1			;Almost done
	je .clusterChainDone
	
	mov ax, word[esi+2]		;Next cluster
	mov word[edi], ax		;Save next cluster to FAT entry

	add esi, 2			;Next free cluster
	loop .createClusterChain
	
.clusterChainDone:	
	mov word[edi], 0xffff		;0xffff last cluster marker
	
;Write FAT table to disk
;
	movzx eax, word[sectorsPerFAT]	;Sectors to write
	mov esi, dword[FAT]		;Root directory LBA
	mov ecx, 0x50			;Segment
	mov edi, DISK_BUFFER+20000	;Offset

	mov dl, byte[bootDrive]

	call writeSectorBIOS

	pop ecx				;Free cluster
	
;Get root dir entry
;
	pop esi				;Resotre esi
	push esi
	
	call fileExists
	jc .failture
	
;ebx is Pointer to root dir entry
;	
	mov eax, dword[.fileSize]
	mov dword[ebx+28], eax		;Size
	mov word[ebx+26], cx		;First cluster

;Write modified root dir to disk
;
	movzx eax, word[rootDirSize]	;Sectors to write
	mov esi, dword[rootDir]		;Root directory LBA
	mov cx, 0x50			;Segment
	mov edi, DISK_BUFFER+20000	;Offset
	mov dl, byte[bootDrive]

	call writeSectorBIOS

;Save data to free clusters
;
	
	mov ebx, DISK_BUFFER		;Free clusters List
	movzx ecx, word[.requiredClusters]
	
;Convert logical address [cluster] to LBA (physical address)
;
;Forumula:
;((cluster - 2) * sectPerCluster) + dataArea
;
.writeDataToClusters:	

	push ecx

;Copy actual data to temporary buffer
;	
	mov esi, ebp
	mov edi, DISK_BUFFER+0x500+20000
	mov ecx, dword[clusterSize]
	rep movsb
	
	movzx esi, word[ebx]
	sub esi, 2

	movzx eax, byte[sectPerCluster]		
	xor edx, edx			;dx = 0
	mul esi				;(cluster - 2) * sectPerCluster
	mov esi, eax	

	add esi, dword[dataArea]

	movzx ax, byte[sectPerCluster]	;Total sectors write

	mov dl, byte[bootDrive]
	
;Write temporary buffer
;
	mov edi, DISK_BUFFER+0x500+20000
	mov ecx, 0			;Real mode segment
	
	call writeSectorBIOS
	pop ecx
	
	
	add ebp, dword[clusterSize]	;Next data block
	add ebx, 2			;Next free cluster
	
	loop .writeDataToClusters
.success:
	clc 				;Clear carry flag
	jmp .end

.failture:
	stc				;Set carry flag
	jmp .end
.end:
	pop esi
	pop edi
	pop edx
	pop ecx
	pop ebx
	pop eax
	ret
.fileSize: 		dd 0
.requiredClusters: 	dd 0

;________________________________________________
;Remove a file from the disk
;IN:    esi Pointer to File name
;OUT: 	nothing
;
fileDelete:
	pushad
	
	call fileExists			
	jc .end				;If file already exists
	
;Root dir entry is already loaded by file exists
;
	mov ax, word[ebx+26]		;Get Starting cluster
	mov word[.cluster], ax		;Save it

;Mark the file as deleted
;
	mov byte[ebx], 229
	
;Write modified root dir to disk
;
	movzx eax, word[rootDirSize]	;Sectors to write
	mov esi, dword[rootDir]		;Root directory LBA
	mov cx, 0x50			;Segment
	mov edi, DISK_BUFFER+20000	;Offset
	mov dl, byte[bootDrive]

	call writeSectorBIOS
	
;Clean file's allocated clusters from FAT table
;

;Load FAT table from disk
;
	movzx eax, word[sectorsPerFAT]	;Sectors to load
	mov esi, dword[FAT]		;Root directory LBA
	mov ecx, 0x50			;Segment
	mov edi, DISK_BUFFER+20000	;Offset

	mov dl, byte[bootDrive]

	call readSectorBIOS

.nextCluster:

;Calculate next cluster
;
	mov edi, DISK_BUFFER+20000	;FAT table
	movzx esi, word[.cluster]
	shl esi, 1			;Multiply by 2
	add edi, esi
	
	mov ax, word[edi]
	
	mov word[.cluster], ax
	
	mov word[edi], 0		;Mark clusters as free
	
	cmp ax, 0xfff8			;0xfff8 is End Of File (EOF) marker
	jae .allClustersDeleted
	
	jmp .nextCluster
	
.allClustersDeleted:

;Write FAT table to disk
;
	movzx eax, word[sectorsPerFAT]	;Sectors to write
	mov esi, dword[FAT]		;Root directory LBA
	mov ecx, 0x50			;Segment
	mov edi, DISK_BUFFER+20000	;Offset

	mov dl, byte[bootDrive]

	call writeSectorBIOS
.end:
	popad
	ret
	
.cluster:	dw 0

;________________________________________________
;Create a new empty file.
;IN:    esi Pointer to File name
;OUT: 	esi Pointer to root dir entry
;	CF sets on error or file exists,
;	clears on success
;
fileNew:
	pushad
	
;Check if file already exists
;
	call fileExists
	jnc .failture

	call stringLength
	cmp eax, 12			
	ja .failture			;If wrong file name
	
	inc eax				;File name including 0 

;Copy file name to temporary buffer
;
	mov edi, .fileNameBuffer+0x500
	mov ecx, eax			;File name length
	cld
	rep movsb			;Times ecx mov esi string to edi

;Make file name FAT compaitable
;
	mov esi, .fileNameBuffer
	call fileNameToFATFileName
	jc .failture			;If wrong file name

	push esi
;Load root directory from disk
;
	movzx eax, word[rootDirSize]	;Sectors to load
	mov esi, dword[rootDir]		;Root directory LBA
	mov cx, 0x50			;Segment
	mov edi, DISK_BUFFER+20000	;Offset
	mov dl, byte[bootDrive]

	call readSectorBIOS

	mov edi, DISK_BUFFER+20000
	movzx ecx, word[rootDirEntries]
	
;Search free root dir entry
;
.freeEntryFindLoop:

	cmp byte[edi], 229		;Deleted file
	je .freeEntryFound
	
	cmp byte[edi], 0		;Unused entry
	je .freeEntryFound

	add edi, 32
	loop .freeEntryFindLoop

.freeEntryNotFound:
	jmp .failture

.freeEntryFound:

;Copy file name to root directory buffer
;	
	pop esi				;Restore esi
	mov ecx, 11			;File name length
	push edi
	add edi, 0x500			;ES segment
	rep movsb			;mov ecx bytes from esi to edi

	pop edi				;Restore edi
	push edi
	
;Clean root dir entry's other values
;
	add edi, 0x500+11
	mov ecx, 32-11
	mov al, 0
	cld
	rep stosb			;mov al into ecx bytes of edi
	
;Write modified root dir to disk
;
	movzx eax, word[rootDirSize]	;Sectors to write
	mov esi, dword[rootDir]		;Root directory LBA
	mov cx, 0x50			;Segment
	mov edi, DISK_BUFFER+20000	;Offset
	mov dl, byte[bootDrive]

	call writeSectorBIOS

	pop esi				;Pointer to root dir entry
.success:
	clc 				;Clear carry flag
	jmp .end

.failture:
	stc				;Set carry flag
	jmp .end
.end:
	popad
	ret
.fileNameBuffer: times 13 db ' '
db 0

;
;To be filled latter
;
BPBAddress:		dd 0		;Memory Address to BIOS parameter block
bytesPerSector:		dw 0		;Number of bytes in each sector
sectPerCluster:		db 0		;Sectors in one cluster (allocation unit)
reservedSectors:	dw 0		;Sectors reserved After boot sector
totalFATs:		db 0		;Number of FAT tables.
rootDirEntries:		dw 0		;Total files or folders in root directory
sectorsPerFAT:		dw 0		;Sectors used in FAT
totalSectors:		dd 0		;Sectors in Disk
rootDirSize:		dw 0		;Root directory size (in sectors)
rootDir:		dd 0		;Root directory address (LBA)
FATsSize:		dw 0		;FAT tables size (in sectors)
FAT:			dd 0		;File Allocatin Table (FAT)'s address (LBA)
dataArea:		dd 0		;dataArea start address (LBA)
clusterSize:		dd 0		;Cluster size in bytes
bootDrive:		db 0		;Drive number: 0 for floppy, 0x80 for hard disk

diskError:		db "Disk error! Halting.", 0
