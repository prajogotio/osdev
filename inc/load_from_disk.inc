
; al = number of sectors
; cl = starting sector
; dl = boot drive
; es:bx = read buffer
LoadFromDisk:
  pusha

ResetDisk:
  push ax
  push dx

  mov ah, 0
  mov dl, 0
  int 0x13
  jc ResetDisk

  pop dx
  pop ax

ReadFromDisk:
  
  mov ah, 0x02
  mov ch, 0
  mov dh, 0
  int 0x13

  jc ReadFromDisk

  popa
  ret