%ifndef __TIO_OS_GDT_INC__
%define __TIO_OS_GDT_INC__

bits 16
InstallGdt:
  cli
  pusha
  lgdt [GdtPointer]
  sti
  popa
  ret
  

GdtTableStart:
GdtNullDesc:
  dd 0
  dd 0
GdtCodeDesc:
  dw 0xFFFF
  dw 0
  db 0
  db 10011010b
  db 11001111b
  db 0
GdtDataDesc:
  dw 0xFFFF
  dw 0
  db 0
  db 10010010b
  db 11001111b
  db 0
GdtTableEnd:
GdtPointer:
  dw GdtTableEnd - GdtTableStart - 1
  dd GdtTableStart

%endif ; __TIO_OS_GDT_INC__
