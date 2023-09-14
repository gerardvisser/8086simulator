;
; int 0x10
;
; loaded at: 0xC000:0x0010
;

; Variable offsets:

CGA_COLOURS = 0
EGA_COLOURS = 2
DEFAULT_256_COLOURS = 4
FONT_8 = 6
FONT_14 = 8
FONT_16 = 10

; Offsets relative to segment 0x0000:

INT_43 = 0x10C              ; 4 bytes
ACTIVE_MODE = 0x449         ; 1 byte
SCREEN_WIDTH = 0x44A        ; 2 bytes
REGEN_SIZE = 0x44C          ; 2 bytes
ACTIVE_PAGE_OFFSET = 0x44E  ; 2 bytes
CURSOR_LOCATIONS = 0x450    ; 16 bytes
ACTIVE_PAGE = 0x462         ; 1 byte
SCREEN_ROWS = 0x484         ; 1 byte
CHAR_HEIGHT = 0x485         ; 2 bytes


  cmp ah, 0
  jnz ah_not_0
  jmp set_video_mode

ah_not_0:
  cmp ah, 2
  jnz ah_not_2
  jmp set_cursor_location

ah_not_2:
  cmp ah, 3
  jnz ah_not_3
  jmp get_cursor_location

ah_not_3:
  cmp ah, 5
  jnz ah_not_5
  jmp set_active_page

ah_not_5:
  cmp ah, 6
  jnz ah_not_6
  jmp scroll_window_up

ah_not_6:
  cmp ah, 7
  jnz ah_not_7
  jmp scroll_window_down

ah_not_7:

  iret

;
; Set Video Mode
;
; Input:
;   AH = 0x00
;   AL = mode
;
set_video_mode:
  push ax
  push bx
  push cx
  push dx
  push si
  push di
  push es
  push ds

  cld

  push cs
  pop ds            ; ds = cs
  mov di, 0xA000
  mov es, di        ; es = 0xA000

  mov bx, ax        ; bl = mode
  mov dx, 0x3C4
  mov ax, 0x2101
  out dx, ax        ; disable video
  mov ax, 0x604
  out dx, ax        ; planar memory mode
  mov dx, 0x3CE
  xor ax, ax
  mov cx, 6

svm_loop_1:
  out dx, ax
  inc ax
  loop svm_loop_1

  mov ax, 0x406
  out dx, ax        ; memory map: 0xA000
  mov ax, 0xF07
  out dx, ax
  mov ax, 0xFF08
  out dx, ax

  cmp bl, 4
  jb set_text_mode
  jmp set_gfx_mode

set_text_mode:
  mov si, [FONT_16]
  xor di, di        ; es:di = 0xA000:0x0000

  mov dx, 0x3C4
  mov ax, 0x402
  out dx, ax        ; enable plane 2

; Copying the font to plane 2 in video memory
;
  mov cx, 0x100
svm_copy_font:
  push cx
  mov cx, 0x10
  rep movsb
  add di, 0x10
  pop cx
  loop svm_copy_font

  mov ax, 0x302
  out dx, ax        ; enable planes 1, 0
  mov ax, 0x204
  out dx, ax        ; odd/even memory mode

  xor di, di
  mov cx, 0x4000
  mov ax, 0x720
  rep stosw         ; initialise video memory for text modes

; setTextModeDac
;
  mov si, [EGA_COLOURS]
  mov dx, 0x3C8
  xor ax, ax
  out dx, al
  inc dx            ; dx = 0x3C9
  mov cx, 192
svm_set_text_mode_colours:
  lodsb
  out dx, al
  loop svm_set_text_mode_colours
  mov si, [DEFAULT_256_COLOURS]
  add si, 192
  mov cx, 576
svm_set_text_mode_colours_2:
  lodsb
  out dx, al
  loop svm_set_text_mode_colours_2

  call set_ega_palette
  mov bh, 4
  call set_remaining_attr_registers

  mov dx, 0x3D4
  cmp bl, 2
  jb set_text_mode_40_cols
  mov ax, 0x4B00
  out dx, ax
  mov ax, 0x4F01
  out dx, ax
  mov si, 0x1000    ; si = regen size
  jmp finish_setting_text_mode
set_text_mode_40_cols:
  mov ax, 0x2300
  out dx, ax
  mov ax, 0x2701
  out dx, ax
  mov si, 0x800     ; si = regen size
finish_setting_text_mode:
  mov ax, 0xDF06
  out dx, ax
  mov ax, 0x8F12
  out dx, ax
  mov ax, 0x0307
  out dx, ax
  mov ax, 0x4F09
  out dx, ax
  mov ax, 0x0117
  out dx, ax

  mov dx, 0x3CE
  mov ax, 0x0E06
  out dx, ax        ; memory map: 0xB800

  ; Preparing to end_set_video_mode
  mov ax, [FONT_16]
  mov bh, 0
  mov cx, 0x1018

end_set_video_mode:
  ; PRE:
  ;  ax = font offset
  ;  bl = video mode
  ;  bh = value for 0x3C4, 1: to enable video and set 8/9 pixels wide characters
  ;  ch = character height
  ;  cl = screen rows minus one
  ;  si = regen size
  xor dx, dx
  mov es, dx        ; es = 0x0000
  mov di, INT_43
  stosw
  push cs
  pop ax
  stosw
  ;
  mov di, ACTIVE_MODE
  mov al, bl
  stosb
  ;
  mov dx, 0x3D4
  ;
  mov ax, 0x000C
  out dx, ax        ; start address high register
  inc ax
  out dx, ax        ; start address low register
  ;
  mov al, 1         ; ax = 0x0001
  out dx, al
  inc dx
  in al, dx
  inc ax
  cmp bl, 0x13
  jnz svm_write_screen_width
  shr ax, 1         ; in 256 colour mode, the number columns in the horizontal end register is doubled.
svm_write_screen_width:
  stosw             ; writing SCREEN_WIDTH
  ;
  mov ax, si
  stosw             ; writing REGEN_SIZE
  ;
  push cx           ; push ch/cl = character height/screen rows minus one
  xor ax, ax
  mov cx, 0xA
  rep stosw         ; writing ACTIVE_PAGE_OFFSET, CURSOR_LOCATIONS and 'Cursor size/shape'.
  stosb             ; writing ACTIVE_PAGE
  mov ax, 0x3D4
  stosw             ; writing crt controller port (always 0x3D4, since mode 7 is not supported).
  ;
  pop ax            ; pop ah/al = character height/screen rows minus one
  mov di, SCREEN_ROWS
  stosw
  mov al, 0
  stosb             ; storing high byte of CHAR_HEIGHT
  ;
  mov dx, 0x3C4
  mov al, 0x01
  mov ah, bh
  out dx, ax        ; enable video

  pop ds
  pop es
  pop di
  pop si
  pop dx
  pop cx
  pop bx
  pop ax
  iret

set_gfx_mode:
  mov dx, 0x3C4
  mov ax, 0xF02
  out dx, ax        ; enable planes 3, 2, 1, 0

  xor di, di        ; es:di = 0xA000:0x0000
  mov cx, 0x8000
  xor ax, ax        ; ax = 0
  rep stosw         ; initialise video memory

  cmp bl, 5
  ja mode_above_5

  ; Modes 4 and 5
  mov ax, 0x302
  out dx, ax        ; (DX=0x3C4) enable planes 1, 0
  mov ax, 0x204
  out dx, ax        ; odd/even memory mode
  ;
  mov si, [CGA_COLOURS]
  call set_cga_ega_dac
  ;
  call set_cga_palette
  mov bh, 1
  call set_remaining_attr_registers
  ;
  mov dx, 0x3D4
  mov ax, 0x2800
  out dx, ax
  mov ax, 0x2701
  out dx, ax
  mov ax, 0xDF06
  out dx, ax
  mov ax, 0x8F12
  out dx, ax
  mov ax, 0x0307
  out dx, ax
  mov ax, 0xC109
  out dx, ax
  mov ax, 0x0017
  out dx, ax
  ;
  mov dx, 0x3CE
  mov ax, 0x3005
  out dx, ax        ; interleaved shift
  mov ax, 0x0F06
  out dx, ax        ; memory map: 0xB800, graphical
  ;
  mov ax, [FONT_8]
  mov bh, 1         ; value for 0x3C4, 1: to enable video and set 8 pixels wide characters
  mov cx, 0x818
  mov si, 0x4000    ; si = regen size
  jmp end_set_video_mode

mode_above_5:
  cmp bl, 6
  ja mode_above_6

  ; Mode 6
  mov ax, 0x102
  out dx, ax        ; (DX=0x3C4) enable plane 0
  ;
  mov si, [CGA_COLOURS]
  call set_cga_ega_dac
  ;
  mov dx, 0x3DA
  in al, dx         ; set 0x3C0 to expect an index
  mov dx, 0x3C0     ; dx = 0x3C0
  xor ax, ax
  out dx, al
  out dx, al
  mov cx, 15
svm_set_cga_2_palette:
  inc ax
  push ax
  out dx, al
  mov al, 0x17
  out dx, al
  pop ax
  loop svm_set_cga_2_palette
  mov bh, 1
  call set_remaining_attr_registers
  ;
  mov dx, 0x3D4
  mov ax, 0x5500
  out dx, ax
  mov ax, 0x4F01
  out dx, ax
  mov ax, 0xDF06
  out dx, ax
  mov ax, 0x8F12
  out dx, ax
  mov ax, 0x0307
  out dx, ax
  mov ax, 0xC109
  out dx, ax
  mov ax, 0x0017
  out dx, ax
  ;
  mov dx, 0x3CE
  mov ax, 0x0D06
  out dx, ax        ; memory map: 0xB800, graphical
  ;
  mov ax, [FONT_8]
  mov bh, 1         ; value for 0x3C4, 1: to enable video and set 8 pixels wide characters
  mov cx, 0x818
  mov si, 0x4000    ; si = regen size
  jmp end_set_video_mode

mode_above_6:
  cmp bl, 0xC
  ja mode_above_C

  ; Modes 7 - C are undefined.
  mov bl, 3
  jmp set_text_mode

mode_above_C:
  cmp bl, 0xE
  ja mode_above_E

  ; Modes D and E
  mov si, [EGA_COLOURS]
  call set_cga_ega_dac
  ;
  call set_ega_palette
  mov bh, 1
  call set_remaining_attr_registers
  ;
  mov dx, 0x3D4
  cmp bl, 0xD
  jz svm_set_40_cols_mode_D
  mov ax, 0x5500
  out dx, ax
  mov ax, 0x4F01
  out dx, ax
  mov si, 0x4000    ; si = regen size
  jmp svm_finish_setting_modes_D_and_E
svm_set_40_cols_mode_D:
  mov ax, 0x2800
  out dx, ax
  mov ax, 0x2701
  out dx, ax
  mov si, 0x2000    ; si = regen size
svm_finish_setting_modes_D_and_E:
  mov ax, 0xDF06
  out dx, ax
  mov ax, 0x8F12
  out dx, ax
  mov ax, 0x0307
  out dx, ax
  mov ax, 0xC009
  out dx, ax
  mov ax, 0x0117
  out dx, ax
  ;
  mov dx, 0x3CE
  mov ax, 0x0506
  out dx, ax        ; memory map: 0xA000, graphical
  ;
  mov ax, [FONT_8]
  mov bh, 1         ; value for 0x3C4, 1: to enable video and set 8 pixels wide characters
  mov cx, 0x818
  jmp end_set_video_mode

mode_above_E:
  cmp bl, 0x10
  ja mode_above_10

  ; Modes F and 10
  mov si, [EGA_COLOURS]
  call set_cga_ega_dac
  ;
  cmp bl, 0x10
  jz svm_set_640_350_16_palette
  mov bh, 7
  call set_ega_vga_2_colour_palette
  jmp svm_finish_setting_640_350_mode
svm_set_640_350_16_palette:
  call set_ega_palette
svm_finish_setting_640_350_mode:
  mov bh, 1
  call set_remaining_attr_registers
  ;
  mov dx, 0x3D4
  mov ax, 0x5500
  out dx, ax
  mov ax, 0x4F01
  out dx, ax
  mov ax, 0xDF06
  out dx, ax
  mov ax, 0x5D12
  out dx, ax
  mov ax, 0x0307
  out dx, ax
  mov ax, 0x4009
  out dx, ax
  mov ax, 0x0117
  out dx, ax
  ;
  mov dx, 0x3CE
  mov ax, 0x0506
  out dx, ax        ; memory map: 0xA000, graphical
  ;
  mov ax, [FONT_14]
  mov bh, 1         ; value for 0x3C4, 1: to enable video and set 8 pixels wide characters
  mov cx, 0xE18
  mov si, 0x8000    ; si = regen size
  jmp end_set_video_mode

mode_above_10:
  cmp bl, 0x12
  ja mode_above_12

  ; Modes 11 and 12
  mov si, [EGA_COLOURS]
  call set_cga_ega_dac
  ;
  cmp bl, 0x12
  jz svm_set_640_480_16_palette
  mov bh, 0x3F
  call set_ega_vga_2_colour_palette
  jmp svm_finish_setting_640_480_mode
svm_set_640_480_16_palette:
  call set_ega_palette
svm_finish_setting_640_480_mode:
  mov bh, 1
  call set_remaining_attr_registers
  ;
  mov dx, 0x3D4
  mov ax, 0x5500
  out dx, ax
  mov ax, 0x4F01
  out dx, ax
  mov ax, 0xDF06
  out dx, ax
  mov ax, 0xDF12
  out dx, ax
  mov ax, 0x0307
  out dx, ax
  mov ax, 0x4009
  out dx, ax
  mov ax, 0x0117
  out dx, ax
  ;
  mov dx, 0x3CE
  mov ax, 0x0506
  out dx, ax        ; memory map: 0xA000, graphical
  ;
  mov ax, [FONT_16]
  mov bh, 1         ; value for 0x3C4, 1: to enable video and set 8 pixels wide characters
  mov cx, 0x101D
  mov si, 0xA000    ; si = regen size
  jmp end_set_video_mode

mode_above_12:
  cmp bl, 0x13
  jnz mode_above_13

  ; Mode 13
  mov ax, 0xE04
  out dx, ax        ; (DX=0x3C4) chain-4 memory mode
  ;
  call set_vga_dac
  ;
  call set_vga_palette
  mov bh, 0x41
  call set_remaining_attr_registers
  ;
  mov dx, 0x3D4
  mov ax, 0x5500
  out dx, ax
  mov ax, 0x4F01
  out dx, ax
  mov ax, 0xDF06
  out dx, ax
  mov ax, 0x8F12
  out dx, ax
  mov ax, 0x0307
  out dx, ax
  mov ax, 0xC009
  out dx, ax
  mov ax, 0x0117
  out dx, ax
  ;
  mov dx, 0x3CE
  mov ax, 0x4005
  out dx, ax        ; 256 shift
  mov ax, 0x0506
  out dx, ax        ; memory map: 0xA000, graphical
  ;
  mov ax, [FONT_8]
  mov bh, 1         ; value for 0x3C4, 1: to enable video and set 8 pixels wide characters
  mov cx, 0x818
  mov si, 0xFA00    ; si = regen size
  jmp end_set_video_mode

mode_above_13:
  cmp bl, 0x14
  jnz mode_above_14

  ; Mode 14
  mov si, [EGA_COLOURS]
  call set_cga_ega_dac
  ;
  call set_ega_palette
  mov bh, 1
  call set_remaining_attr_registers
  ;
  mov dx, 0x3D4
  mov ax, 0xAF00
  out dx, ax
  mov ax, 0x6301
  out dx, ax
  mov ax, 0xBF06
  out dx, ax
  mov ax, 0x5F12
  out dx, ax
  mov ax, 0x6107
  out dx, ax
  mov ax, 0x4009
  out dx, ax
  mov ax, 0x0117
  out dx, ax
  ;
  mov dx, 0x3CE
  mov ax, 0x0506
  out dx, ax        ; memory map: 0xA000, graphical
  ;
  mov ax, [FONT_16]
  mov bh, 1         ; value for 0x3C4, 1: to enable video and set 8 pixels wide characters
  mov cx, 0x1025
  mov si, 0xF000    ; si = regen size
  jmp end_set_video_mode

mode_above_14:
  ; All other modes are undefined.
  mov bl, 3
  jmp set_text_mode

;
; Function: set_cga_ega_dac
; Input:
;   DS:SI = pointer to the colour value array
;
set_cga_ega_dac:
  mov dx, 0x3C8
  xor ax, ax
  out dx, al
  inc dx            ; dx = 0x3C9
  mov cx, 192
sced_set_cga_ega_colours:
  lodsb
  out dx, al
  loop sced_set_cga_ega_colours
  mov cx, 576
  xor ax, ax
sced_set_cga_ega_colours_2:
  out dx, al
  loop sced_set_cga_ega_colours_2
  ret

;
; Function: set_cga_palette
; Output:
;   DX = 0x3C0
;
set_cga_palette:
  mov dx, 0x3DA
  in al, dx         ; set 0x3C0 to expect an index
  mov dx, 0x3C0     ; dx = 0x3C0
  xor ax, ax
  out dx, al
  out dx, al
  inc ax
  out dx, al
  mov al, 0x13
  out dx, al
  mov al, 2
  out dx, al
  mov al, 0x15
  out dx, al
  mov al, 3
  out dx, al
  mov al, 0x17
  out dx, al
  mov al, 4
  out dx, al
  mov al, 0x02
  out dx, al
  mov al, 5
  out dx, al
  mov al, 0x04
  out dx, al
  mov al, 6
  out dx, al
  out dx, al
  inc ax
  out dx, al
  out dx, al
  inc ax
  mov cx, ax        ; cx = 8
svm_set_cga_palette_8_16:
  out dx, al
  add al, 8
  out dx, al
  sub al, 7
  loop svm_set_cga_palette_8_16
  ret

;
; Function: set_ega_palette
; Output:
;   DX = 0x3C0
;
set_ega_palette:
  mov dx, 0x3DA
  in al, dx         ; set 0x3C0 to expect an index
  mov dx, 0x3C0     ; dx = 0x3C0
  xor ax, ax
  mov cx, 6
sep_loop_1:
  out dx, al
  out dx, al
  inc ax
  loop sep_loop_1
  out dx, al
  mov al, 0x14
  out dx, al
  mov al, 0x7
  out dx, al
  out dx, al
  inc ax
  mov cx, 8
sep_loop_2:
  out dx, al
  add al, 0x30
  out dx, al
  sub al, 0x2F
  loop sep_loop_2
  ret

;
; Function: set_ega_vga_2_colour_palette
; Input:
;   BH = colour index
; Output:
;   DX = 0x3C0
;
set_ega_vga_2_colour_palette:
  mov dx, 0x3DA
  in al, dx         ; set 0x3C0 to expect an index
  mov dx, 0x3C0     ; dx = 0x3C0
  xor ax, ax
  mov cx, 8
sev2cp_loop_1:
  out dx, al
  push ax
  mov al, 0
  out dx, al
  pop ax
  add al, 2
  loop sev2cp_loop_1
  mov al, 1
  mov cx, 8
sev2cp_loop_2:
  out dx, al
  push ax
  mov al, bh
  out dx, al
  pop ax
  add al, 2
  loop sev2cp_loop_2
  ret

;
; Function: set_remaining_attr_registers
;   Sets registers 0x3C0, 0x10 and 0x3C0, 0x14, puts the index back at 0x20 and reads 0x3DA.
; Input:
;   DX = 0x3C0
;   BH = value for the mode control register (0x3C0, 0x10).
;
set_remaining_attr_registers:
  mov al, 0x30
  out dx, al
  mov al, bh
  out dx, al
  mov al, 0x34
  out dx, al
  mov al, 0
  out dx, al
  mov al, 0x20
  out dx, al
  mov dx, 0x3DA
  in al, dx         ; set 0x3C0 to expect an index
  ret

;
; Function: set_vga_dac
; Input:
;   DS = 0xC000
;
set_vga_dac:
  mov si, [DEFAULT_256_COLOURS]
  mov dx, 0x3C8
  xor ax, ax
  out dx, al
  inc dx            ; dx = 0x3C9
  mov cx, 768
svd_set_vga_colours:
  lodsb
  out dx, al
  loop svd_set_vga_colours
  ret

;
; Function: set_vga_palette
; Output:
;   DX = 0x3C0
;
set_vga_palette:
  mov dx, 0x3DA
  in al, dx         ; set 0x3C0 to expect an index
  mov dx, 0x3C0     ; dx = 0x3C0
  xor ax, ax
  mov cx, 16
svp_loop_1:
  out dx, al
  out dx, al
  inc ax
  loop svp_loop_1
  ret


;
; Set Cursor Location
;
; Input:
;   AH = 0x02
;   BH = page
;   DH = row (y)
;   DL = column (x)
;
set_cursor_location:
  push ax
  push bx
  push ds

  cmp bh, 7
  ja end_set_cursor_location

  xor ax, ax
  mov ds, ax        ; ds = 0x0000

  cmp dl, [SCREEN_WIDTH]
  jae end_set_cursor_location

  cmp dh, [SCREEN_ROWS]
  ja end_set_cursor_location

  shl bh, 1
  shr bx, 8
  add bx, CURSOR_LOCATIONS
  mov [bx], dx

end_set_cursor_location:
  pop ds
  pop bx
  pop ax
  iret


;
; Get Cursor Location
;
; Input:
;   AH = 0x03
;   BH = page
; Output:
;   CH = start of cursor (at 0x0000:0x0461)
;   CL = end of cursor (at 0x0000:0x0460)
;   DH = row (y)
;   DL = column (x)
;
get_cursor_location:
  push bx
  push ds

  cmp bh, 7
  ja end_get_cursor_location

  xor dx, dx
  mov ds, dx        ; ds = 0x0000

  mov cx, [ACTIVE_PAGE - 2]

  shl bh, 1
  shr bx, 8
  add bx, CURSOR_LOCATIONS
  mov dx, [bx]

end_get_cursor_location:
  pop ds
  pop bx
  iret


;
; Set Active Page
;
; Input:
;   AH = 0x05
;   AL = page
;
set_active_page:
  push ax
  push bx
  push dx
  push ds

  xor bx, bx
  mov ds, bx        ; ds = 0x0000

  mov ah, 0
  mov dx, [REGEN_SIZE]
  mov bl, [ACTIVE_MODE]
  cmp bl, 4
  jnb sap_mode_above_3

  ;set page for text modes
  cmp al, 7
  ja end_set_active_page
  mov [ACTIVE_PAGE], al
  mul dx
  mov [ACTIVE_PAGE_OFFSET], ax
  shr ax, 1         ; text modes use odd/even memory mode
  mov al, 0xC       ; assumes al = 0, so we can discard it
  mov dx, 0x3D4
  out dx, ax
  jmp end_set_active_page

sap_mode_above_3:
  cmp bl, 0xD
  jb end_set_active_page
  ja sap_mode_above_D

  ;set page for mode D
  cmp al, 7
  ja end_set_active_page
  call set_active_page_for_gfx_mode
  jmp end_set_active_page

sap_mode_above_D:
  cmp bl, 0xE
  ja sap_mode_above_E

  ;set page for mode E
  cmp al, 3
  ja end_set_active_page
  call set_active_page_for_gfx_mode
  jmp end_set_active_page

sap_mode_above_E:
  cmp bl, 0x10
  ja end_set_active_page

  ;set page for modes F and 10
  cmp al, 1
  ja end_set_active_page
  call set_active_page_for_gfx_mode

end_set_active_page:
  pop ds
  pop dx
  pop bx
  pop ax
  iret

;
; Function: set_active_page_for_gfx_mode
; Input:
;   AX = valid page number
;   DX = regen size
;   DS = 0x0000
;
set_active_page_for_gfx_mode:
  mov [ACTIVE_PAGE], al
  mul dx
  mov [ACTIVE_PAGE_OFFSET], ax
  mov al, 0xC       ; assumes al = 0, so we can discard it
  mov dx, 0x3D4
  out dx, ax
  ret


;
; Scroll Window Up
;
; Input:
;   AH = 0x06
;   AL = number of lines to scroll or 0 to clear the entire window
;   BH = attribute/colour for cleared lines
;   CH = y coordinate top left corner
;   CL = x coordinate top left corner
;   DH = y coordinate bottom right corner
;   DL = x coordinate bottom right corner
;
SWU_SCREEN_WIDTH = 0
SWU_WINDOW_WIDTH = 2
SWU_SCREEN_WIDTH_MINUS_WINDOW_WIDTH = 4
SWU_ACTIVE_PAGE_OFFSET = 6
SWU_CHAR_HEIGHT = 8
SWU_STACK_FRAME_SIZE = 10

SW_16CM_BACKGROUND_COLOUR = 0xFFF0

scroll_window_up:
  push ax
  push bx
  push cx
  push dx
  push bp
  push si
  push di
  push es
  push ds

  sub sp, SWU_STACK_FRAME_SIZE
  mov bp, sp

  cld

  xor di, di
  mov ds, di        ; ds = 0x0000

  mov di, [SCREEN_WIDTH]
  mov [bp + SWU_SCREEN_WIDTH], di
  mov di, [ACTIVE_PAGE_OFFSET]
  mov [bp + SWU_ACTIVE_PAGE_OFFSET], di
  mov di, [CHAR_HEIGHT]
  mov [bp + SWU_CHAR_HEIGHT], di

  cmp dh, [SCREEN_ROWS]
  jbe swu_check_dl
  mov dh, [SCREEN_ROWS]
swu_check_dl:
  cmp dl, [bp + SWU_SCREEN_WIDTH]
  jb swu_check_cx
  mov dl, [bp + SWU_SCREEN_WIDTH]
  dec dl
swu_check_cx:
  sub dh, ch
  jb end_scroll_window_up
  sub dl, cl
  jb end_scroll_window_up
  add dx, 0x101     ; inc dh, inc dl:
                    ; dh = height of window in rows, dl = width of window in characters
  cmp al, 0
  jnz swu_check_al_max
  mov al, dh
  jmp swu_al_checked
swu_check_al_max:
  cmp al, dh
  jbe swu_al_checked
  mov al, dh
swu_al_checked:
  mov ah, 0         ; ax = lines to scroll
  push dx
  mov dh, 0
  mov [bp + SWU_WINDOW_WIDTH], dx
  neg dx
  add dx, [bp + SWU_SCREEN_WIDTH]
  mov [bp + SWU_SCREEN_WIDTH_MINUS_WINDOW_WIDTH], dx
  pop dx
  shr dx, 8         ; dx = height of window in rows
  mov di, cx
  and di, 0x00FF    ; di = x coordinate top left corner
  shr cx, 8         ; cx = y coordinate top left corner
  xchg ax, cx       ; ax = y coordinate top left corner, cx = lines to scroll

  mov bl, [ACTIVE_MODE]
  cmp bl, 3
  ja swu_mode_above_3
  jmp swu_text_modes
swu_mode_above_3:
  cmp bl, 5
  ja swu_mode_above_5
  jmp swu_cga_4_colour_modes
swu_mode_above_5:
  cmp bl, 6
  ja swu_mode_above_6
  jmp swu_cga_2_colour_mode
swu_mode_above_6:
  cmp bl, 0x13
  jnz swu_16_colour_modes
  jmp swu_256_colour_mode

end_scroll_window_up:
  add sp, SWU_STACK_FRAME_SIZE
  pop ds
  pop es
  pop di
  pop si
  pop bp
  pop dx
  pop cx
  pop bx
  pop ax
  iret

swu_text_modes:
  ; optimalisatie mogelijk voor [bp+SWU_WINDOW_WIDTH]=[bp+SWU_SCREEN_WIDTH]
  mov si, 0xB800
  mov ds, si
  mov es, si
  push dx           ; push 'height of window in rows'
  mul word ptr [bp + SWU_SCREEN_WIDTH]
  pop dx            ; dx = height of window in rows
  add di, ax
  shl di, 1         ; es:di = address of top left corner (destination)
  add di, [bp + SWU_ACTIVE_PAGE_OFFSET]  ; adjusting di for active page
  shl word ptr [bp + SWU_SCREEN_WIDTH_MINUS_WINDOW_WIDTH], 1

  cmp cx, dx
  jz swu_tm_clear_area

  mov ax, cx        ; ax = lines to scroll
  push dx           ; push 'height of window in rows'
  mul word ptr [bp + SWU_SCREEN_WIDTH]
  pop dx            ; dx = height of window in rows
  shl ax, 1
  mov si, ax
  add si, di        ; ds:si = address of line that will become the top left corner of the window (source)
  sub dx, cx        ; dx = height of window in rows - lines to scroll
  xchg cx, dx       ; cx = height of window in rows - lines to scroll, dx = lines to scroll

swu_tm_copy_lines_loop:
  push cx
  mov cx, [bp + SWU_WINDOW_WIDTH]
  rep movsw
  add di, [bp + SWU_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  add si, [bp + SWU_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  pop cx
  loop swu_tm_copy_lines_loop

  mov cx, dx        ; cx = lines to scroll

  ; bh = attribute/colour for cleared lines
  ; cx = lines to scroll
  ; es:di = address where to start clearing
swu_tm_clear_area:
  mov ah, bh
  mov al, 0x20

swu_tm_clear_area_loop:
  push cx
  mov cx, [bp + SWU_WINDOW_WIDTH]
  rep stosw
  add di, [bp + SWU_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  pop cx
  loop swu_tm_clear_area_loop

  jmp end_scroll_window_up

swu_16_colour_modes:
  push dx           ; push 'height of window in rows'

  mov si, 0xA000
  mov ds, si
  mov es, si
  mul word ptr [bp + SWU_SCREEN_WIDTH]
  mul word ptr [bp + SWU_CHAR_HEIGHT]
  add di, ax        ; es:di = address of top left corner (destination)
  add di, [bp + SWU_ACTIVE_PAGE_OFFSET]  ; adjusting di for active page

  mov dx, 0x3CE
  mov ax, 0x0205
  out dx, ax        ; setting write mode 2
  mov ax, 0x0003
  out dx, ax        ; no logical operation
  mov ax, 0xFF08
  out dx, ax        ; bitmask 0xFF
  mov [SW_16CM_BACKGROUND_COLOUR], bh
  mov ax, 0x0105
  out dx, ax        ; setting write mode 1

  pop dx            ; dx = height of window in rows

  mov ax, cx        ; ax = lines to scroll
  cmp cx, dx
  jz swu_16cm_clear_area

  push dx           ; push 'height of window in rows'
  mul word ptr [bp + SWU_SCREEN_WIDTH]
  mul word ptr [bp + SWU_CHAR_HEIGHT]
  pop dx            ; dx = height of window in rows
  mov si, ax
  add si, di        ; ds:si = address of line that will become the top left corner of the window (source)
  sub dx, cx        ; dx = height of window in rows - lines to scroll
  mov ax, dx        ; ax = height of window in rows - lines to scroll
  mul word ptr [bp + SWU_CHAR_HEIGHT]   ; ax = char height * (height of window in rows - lines to scroll)
  xchg ax, cx       ; cx = char height * (height of window in rows - lines to scroll), ax = lines to scroll

swu_16cm_copy_lines_loop:
  push cx
  mov cx, [bp + SWU_WINDOW_WIDTH]
  rep movsb
  add di, [bp + SWU_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  add si, [bp + SWU_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  pop cx
  loop swu_16cm_copy_lines_loop

swu_16cm_clear_area:
  mul word ptr [bp + SWU_CHAR_HEIGHT]   ; ax = char height * lines to scroll
  mov cx, ax        ; cx = char height * lines to scroll
  mov al, [SW_16CM_BACKGROUND_COLOUR]   ; loading the latches

swu_16cm_clear_area_loop:
  push cx
  mov cx, [bp + SWU_WINDOW_WIDTH]
  rep stosb
  add di, [bp + SWU_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  pop cx
  loop swu_16cm_clear_area_loop

  jmp end_scroll_window_up

swu_cga_4_colour_modes:
  mov si, 0xB800
  mov ds, si
  mov es, si
  shr word ptr [bp + SWU_CHAR_HEIGHT], 1  ; dividing char. height by two because of cga addressing
  shl word ptr [bp + SWU_SCREEN_WIDTH_MINUS_WINDOW_WIDTH], 1  ; character width = 2 bytes
  push dx           ; push 'height of window in rows'
  mul word ptr [bp + SWU_SCREEN_WIDTH]
  mul word ptr [bp + SWU_CHAR_HEIGHT]
  pop dx            ; dx = height of window in rows
  add di, ax
  shl di, 1         ; (character width = 2 bytes) es:di = address of top left corner (destination)

  mov ax, cx        ; ax = lines to scroll
  cmp cx, dx
  jz swu_c4cm_clear_area

  push dx           ; push 'height of window in rows'
  mul word ptr [bp + SWU_SCREEN_WIDTH]
  mul word ptr [bp + SWU_CHAR_HEIGHT]
  pop dx            ; dx = height of window in rows
  shl ax, 1         ; character width = 2 bytes
  mov si, ax
  add si, di        ; ds:si = address of line that will become the top left corner of the window (source)

  sub dx, cx        ; dx = height of window in rows - lines to scroll
  mov ax, dx        ; ax = height of window in rows - lines to scroll
  mul word ptr [bp + SWU_CHAR_HEIGHT]   ; ax = char height * (height of window in rows - lines to scroll) / 2
  xchg ax, cx       ; cx = char height * (height of window in rows - lines to scroll) / 2, ax = lines to scroll

swu_c4cm_copy_lines_loop:
  push cx
  mov cx, [bp + SWU_WINDOW_WIDTH]
  rep movsw
  add di, [bp + SWU_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  add di, 0x2000 - 80
  add si, [bp + SWU_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  add si, 0x2000 - 80
  mov cx, [bp + SWU_WINDOW_WIDTH]
  rep movsw
  add di, [bp + SWU_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  sub di, 0x2000
  add si, [bp + SWU_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  sub si, 0x2000
  pop cx
  loop swu_c4cm_copy_lines_loop

swu_c4cm_clear_area:
  mul word ptr [bp + SWU_CHAR_HEIGHT]  ; ax = lines to scroll * char height / 2
  mov cx, ax        ; cx = lines to scroll * char height / 2

  and bh, 3
  mov al, bh
  shl al, 2
  or al, bh
  shl al, 2
  or al, bh
  shl al, 2
  or al, bh
  mov ah, al        ; ax now contains 8 pixels with the background colour

swu_c4cm_clear_area_loop:
  push cx
  mov cx, [bp + SWU_WINDOW_WIDTH]
  rep stosw
  add di, [bp + SWU_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  add di, 0x2000 - 80
  mov cx, [bp + SWU_WINDOW_WIDTH]
  rep stosw
  add di, [bp + SWU_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  sub di, 0x2000
  pop cx
  loop swu_c4cm_clear_area_loop

  jmp end_scroll_window_up

swu_cga_2_colour_mode:
  mov si, 0xB800
  mov ds, si
  mov es, si
  shr word ptr [bp + SWU_CHAR_HEIGHT], 1  ; dividing char. height by two because of cga addressing
  push dx           ; push 'height of window in rows'
  mul word ptr [bp + SWU_SCREEN_WIDTH]
  mul word ptr [bp + SWU_CHAR_HEIGHT]
  pop dx            ; dx = height of window in rows
  add di, ax        ; es:di = address of top left corner (destination)

  mov ax, cx        ; ax = lines to scroll
  cmp cx, dx
  jz swu_c2cm_clear_area

  push dx           ; push 'height of window in rows'
  mul word ptr [bp + SWU_SCREEN_WIDTH]
  mul word ptr [bp + SWU_CHAR_HEIGHT]
  pop dx            ; dx = height of window in rows
  mov si, ax
  add si, di        ; ds:si = address of line that will become the top left corner of the window (source)

  sub dx, cx        ; dx = height of window in rows - lines to scroll
  mov ax, dx        ; ax = height of window in rows - lines to scroll
  mul word ptr [bp + SWU_CHAR_HEIGHT]   ; ax = char height * (height of window in rows - lines to scroll) / 2
  xchg ax, cx       ; cx = char height * (height of window in rows - lines to scroll) / 2, ax = lines to scroll

swu_c2cm_copy_lines_loop:
  push cx
  mov cx, [bp + SWU_WINDOW_WIDTH]
  rep movsb
  add di, [bp + SWU_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  add di, 0x2000 - 80
  add si, [bp + SWU_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  add si, 0x2000 - 80
  mov cx, [bp + SWU_WINDOW_WIDTH]
  rep movsb
  add di, [bp + SWU_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  sub di, 0x2000
  add si, [bp + SWU_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  sub si, 0x2000
  pop cx
  loop swu_c2cm_copy_lines_loop

swu_c2cm_clear_area:
  mul word ptr [bp + SWU_CHAR_HEIGHT]  ; ax = lines to scroll * char height / 2
  mov cx, ax        ; cx = lines to scroll * char height / 2
  mov al, 0         ; background colour always 0 (bh ignored)

swu_c2cm_clear_area_loop:
  push cx
  mov cx, [bp + SWU_WINDOW_WIDTH]
  rep stosb
  add di, [bp + SWU_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  add di, 0x2000 - 80
  mov cx, [bp + SWU_WINDOW_WIDTH]
  rep stosb
  add di, [bp + SWU_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  sub di, 0x2000
  pop cx
  loop swu_c2cm_clear_area_loop

  jmp end_scroll_window_up

swu_256_colour_mode:
  mov si, 0xA000
  mov ds, si
  mov es, si
  shl word ptr [bp + SWU_SCREEN_WIDTH_MINUS_WINDOW_WIDTH], 3  ; character width = 8 bytes
  shl word ptr [bp + SWU_WINDOW_WIDTH], 2  ; character width = 8 bytes, copying will be done word by word
  push dx           ; push 'height of window in rows'
  mul word ptr [bp + SWU_SCREEN_WIDTH]
  mul word ptr [bp + SWU_CHAR_HEIGHT]
  pop dx            ; dx = height of window in rows
  add di, ax
  shl di, 3         ; (character width = 8 bytes) es:di = address of top left corner (destination)

  mov ax, cx        ; ax = lines to scroll
  cmp cx, dx
  jz swu_256cm_clear_area

  push dx           ; push 'height of window in rows'
  mul word ptr [bp + SWU_SCREEN_WIDTH]
  mul word ptr [bp + SWU_CHAR_HEIGHT]
  pop dx            ; dx = height of window in rows
  shl ax, 3         ; character width = 8 bytes
  mov si, ax
  add si, di        ; ds:si = address of line that will become the top left corner of the window (source)

  sub dx, cx        ; dx = height of window in rows - lines to scroll
  mov ax, dx        ; ax = height of window in rows - lines to scroll
  mul word ptr [bp + SWU_CHAR_HEIGHT]   ; ax = char height * (height of window in rows - lines to scroll)
  xchg ax, cx       ; cx = char height * (height of window in rows - lines to scroll), ax = lines to scroll

swu_256cm_copy_lines_loop:
  push cx
  mov cx, [bp + SWU_WINDOW_WIDTH]
  rep movsw
  add di, [bp + SWU_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  add si, [bp + SWU_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  pop cx
  loop swu_256cm_copy_lines_loop

swu_256cm_clear_area:
  mul word ptr [bp + SWU_CHAR_HEIGHT]   ; ax = char height * lines to scroll
  mov cx, ax        ; cx = char height * lines to scroll
  mov al, bh
  mov ah, bh        ; ax now contains two pixels with the background colour

swu_256cm_clear_area_loop:
  push cx
  mov cx, [bp + SWU_WINDOW_WIDTH]
  rep stosw
  add di, [bp + SWU_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  pop cx
  loop swu_256cm_clear_area_loop

  jmp end_scroll_window_up


;
; Scroll Window Down
;
; Input:
;   AH = 0x07
;   AL = number of lines to scroll or 0 to clear the entire window
;   BH = attribute/colour for cleared lines
;   CH = y coordinate top left corner
;   CL = x coordinate top left corner
;   DH = y coordinate bottom right corner
;   DL = x coordinate bottom right corner
;
SWD_SCREEN_WIDTH = 0
SWD_WINDOW_WIDTH = 2
SWD_SCREEN_WIDTH_MINUS_WINDOW_WIDTH = 4
SWD_ACTIVE_PAGE_OFFSET = 6
SWD_CHAR_HEIGHT = 8
SWD_STACK_FRAME_SIZE = 10

scroll_window_down:
  push ax
  push bx
  push cx
  push dx
  push bp
  push si
  push di
  push es
  push ds

  sub sp, SWD_STACK_FRAME_SIZE
  mov bp, sp

  std

  xor di, di
  mov ds, di        ; ds = 0x0000

  mov di, [SCREEN_WIDTH]
  mov [bp + SWD_SCREEN_WIDTH], di
  mov di, [ACTIVE_PAGE_OFFSET]
  mov [bp + SWD_ACTIVE_PAGE_OFFSET], di
  mov di, [CHAR_HEIGHT]
  mov [bp + SWD_CHAR_HEIGHT], di

  cmp dh, [SCREEN_ROWS]
  jbe swd_check_dl
  mov dh, [SCREEN_ROWS]
swd_check_dl:
  cmp dl, [bp + SWD_SCREEN_WIDTH]
  jb swd_check_cx
  mov dl, [bp + SWD_SCREEN_WIDTH]
  dec dl
swd_check_cx:
  mov di, dx        ; di = coordinates bottom right corner
  sub dh, ch
  jb end_scroll_window_down
  sub dl, cl
  jb end_scroll_window_down
  add dx, 0x101     ; inc dh, inc dl:
                    ; dh = height of window in rows, dl = width of window in characters
  mov cx, di        ; ch = y coordinate bottom right corner, cl = x coordinate bottom right corner
  cmp al, 0
  jnz swd_check_al_max
  mov al, dh
  jmp swd_al_checked
swd_check_al_max:
  cmp al, dh
  jbe swd_al_checked
  mov al, dh
swd_al_checked:
  mov ah, 0         ; ax = lines to scroll
  push dx
  mov dh, 0
  mov [bp + SWD_WINDOW_WIDTH], dx
  neg dx
  add dx, [bp + SWD_SCREEN_WIDTH]
  mov [bp + SWD_SCREEN_WIDTH_MINUS_WINDOW_WIDTH], dx
  pop dx
  shr dx, 8         ; dx = height of window in rows
  mov di, cx
  and di, 0x00FF    ; di = x coordinate bottom right corner
  shr cx, 8         ; cx = y coordinate bottom right corner
  xchg ax, cx       ; ax = y coordinate bottom right corner, cx = lines to scroll

  mov bl, [ACTIVE_MODE]
  cmp bl, 3
  ja swd_mode_above_3
  jmp swd_text_modes
swd_mode_above_3:
  cmp bl, 5
  ja swd_mode_above_5
  jmp swd_cga_4_colour_modes
swd_mode_above_5:
  cmp bl, 6
  ja swd_mode_above_6
  jmp swd_cga_2_colour_mode
swd_mode_above_6:
  cmp bl, 0x13
  jnz swd_16_colour_modes
  jmp swd_256_colour_mode

end_scroll_window_down:
  add sp, SWD_STACK_FRAME_SIZE
  pop ds
  pop es
  pop di
  pop si
  pop bp
  pop dx
  pop cx
  pop bx
  pop ax
  iret

swd_text_modes:
  mov si, 0xB800
  mov ds, si
  mov es, si
  push dx           ; push 'height of window in rows'
  mul word ptr [bp + SWD_SCREEN_WIDTH]
  pop dx            ; dx = height of window in rows
  add di, ax
  shl di, 1         ; (character width = 2 bytes) es:di = address of bottom right corner (destination)
  add di, [bp + SWD_ACTIVE_PAGE_OFFSET]  ; adjusting di for active page
  shl word ptr [bp + SWD_SCREEN_WIDTH_MINUS_WINDOW_WIDTH], 1  ; character width = 2 bytes

  cmp cx, dx
  jz swd_tm_clear_area

  mov ax, cx        ; ax = lines to scroll
  push dx           ; push 'height of window in rows'
  mul word ptr [bp + SWD_SCREEN_WIDTH]
  pop dx            ; dx = height of window in rows
  shl ax, 1
  mov si, di
  sub si, ax        ; ds:si = address of line that will become the bottom right corner of the window (source)
  sub dx, cx        ; dx = height of window in rows - lines to scroll
  xchg cx, dx       ; cx = height of window in rows - lines to scroll, dx = lines to scroll

swd_tm_copy_lines_loop:
  push cx
  mov cx, [bp + SWD_WINDOW_WIDTH]
  rep movsw
  sub di, [bp + SWD_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  sub si, [bp + SWD_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  pop cx
  loop swd_tm_copy_lines_loop

  mov cx, dx        ; cx = lines to scroll

swd_tm_clear_area:
  mov ah, bh
  mov al, 0x20

swd_tm_clear_area_loop:
  push cx
  mov cx, [bp + SWD_WINDOW_WIDTH]
  rep stosw
  sub di, [bp + SWD_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  pop cx
  loop swd_tm_clear_area_loop

  jmp end_scroll_window_down

swd_16_colour_modes:
  push dx           ; push 'height of window in rows'

  mov si, 0xA000
  mov ds, si
  mov es, si
  mul word ptr [bp + SWD_SCREEN_WIDTH]
  mul word ptr [bp + SWD_CHAR_HEIGHT]
  add di, ax
  mov ax, [bp + SWD_CHAR_HEIGHT]
  dec ax
  mul word ptr [bp + SWD_SCREEN_WIDTH]
  add di, ax        ; es:di = address of bottom right corner (destination)
  add di, [bp + SWD_ACTIVE_PAGE_OFFSET]  ; adjusting di for active page

  mov dx, 0x3CE
  mov ax, 0x0205
  out dx, ax        ; setting write mode 2
  mov ax, 0x0003
  out dx, ax        ; no logical operation
  mov ax, 0xFF08
  out dx, ax        ; bitmask 0xFF
  mov [SW_16CM_BACKGROUND_COLOUR], bh
  mov ax, 0x0105
  out dx, ax        ; setting write mode 1

  pop dx            ; dx = height of window in rows

  mov ax, cx        ; ax = lines to scroll
  cmp cx, dx
  jz swd_16cm_clear_area

  push dx           ; push 'height of window in rows'
  mul word ptr [bp + SWD_SCREEN_WIDTH]
  mul word ptr [bp + SWD_CHAR_HEIGHT]
  pop dx            ; dx = height of window in rows
  mov si, di
  sub si, ax        ; ds:si = address of line that will become the bottom right corner of the window (source)
  sub dx, cx        ; dx = height of window in rows - lines to scroll
  mov ax, dx        ; ax = height of window in rows - lines to scroll
  mul word ptr [bp + SWD_CHAR_HEIGHT]   ; ax = char height * (height of window in rows - lines to scroll)
  xchg ax, cx       ; cx = char height * (height of window in rows - lines to scroll), ax = lines to scroll

swd_16cm_copy_lines_loop:
  push cx
  mov cx, [bp + SWD_WINDOW_WIDTH]
  rep movsb
  sub di, [bp + SWD_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  sub si, [bp + SWD_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  pop cx
  loop swd_16cm_copy_lines_loop

swd_16cm_clear_area:
  mul word ptr [bp + SWD_CHAR_HEIGHT]   ; ax = char height * lines to scroll
  mov cx, ax        ; cx = char height * lines to scroll
  mov al, [SW_16CM_BACKGROUND_COLOUR]   ; loading the latches

swd_16cm_clear_area_loop:
  push cx
  mov cx, [bp + SWD_WINDOW_WIDTH]
  rep stosb
  sub di, [bp + SWD_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  pop cx
  loop swd_16cm_clear_area_loop

  jmp end_scroll_window_down

swd_cga_4_colour_modes:
  push dx           ; push 'height of window in rows'

  mov si, 0xB800
  mov ds, si
  mov es, si
  shr word ptr [bp + SWD_CHAR_HEIGHT], 1  ; dividing char. height by two because of cga addressing
  shl word ptr [bp + SWD_SCREEN_WIDTH_MINUS_WINDOW_WIDTH], 1  ; character width = 2 bytes
  mul word ptr [bp + SWD_SCREEN_WIDTH]
  mul word ptr [bp + SWD_CHAR_HEIGHT]
  add di, ax
  mov ax, [bp + SWD_CHAR_HEIGHT]
  dec ax
  mul word ptr [bp + SWD_SCREEN_WIDTH]
  add di, ax
  shl di, 1         ; character width = 2 bytes
  add di, 0x2000    ; es:di = address of bottom right corner (destination)

  pop dx            ; dx = height of window in rows

  mov ax, cx        ; ax = lines to scroll
  cmp cx, dx
  jz swd_c4cm_clear_area

  push dx           ; push 'height of window in rows'
  mul word ptr [bp + SWD_SCREEN_WIDTH]
  mul word ptr [bp + SWD_CHAR_HEIGHT]
  pop dx            ; dx = height of window in rows
  shl ax, 1         ; character width = 2 bytes
  mov si, di
  sub si, ax        ; ds:si = address of line that will become the bottom right corner of the window (source)

  sub dx, cx        ; dx = height of window in rows - lines to scroll
  mov ax, dx        ; ax = height of window in rows - lines to scroll
  mul word ptr [bp + SWD_CHAR_HEIGHT]   ; ax = char height * (height of window in rows - lines to scroll) / 2
  xchg ax, cx       ; cx = char height * (height of window in rows - lines to scroll) / 2, ax = lines to scroll

swd_c4cm_copy_lines_loop:
  push cx
  mov cx, [bp + SWD_WINDOW_WIDTH]
  rep movsw
  sub di, [bp + SWD_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  sub di, 0x2000 - 80
  sub si, [bp + SWD_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  sub si, 0x2000 - 80
  mov cx, [bp + SWD_WINDOW_WIDTH]
  rep movsw
  sub di, [bp + SWD_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  add di, 0x2000
  sub si, [bp + SWD_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  add si, 0x2000
  pop cx
  loop swd_c4cm_copy_lines_loop

swd_c4cm_clear_area:
  mul word ptr [bp + SWD_CHAR_HEIGHT]  ; ax = lines to scroll * char height / 2
  mov cx, ax        ; cx = lines to scroll * char height / 2

  and bh, 3
  mov al, bh
  shl al, 2
  or al, bh
  shl al, 2
  or al, bh
  shl al, 2
  or al, bh
  mov ah, al        ; ax now contains 8 pixels with the background colour

swd_c4cm_clear_area_loop:
  push cx
  mov cx, [bp + SWD_WINDOW_WIDTH]
  rep stosw
  sub di, [bp + SWD_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  sub di, 0x2000 - 80
  mov cx, [bp + SWD_WINDOW_WIDTH]
  rep stosw
  sub di, [bp + SWD_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  add di, 0x2000
  pop cx
  loop swd_c4cm_clear_area_loop

  jmp end_scroll_window_down

swd_cga_2_colour_mode:
  push dx           ; push 'height of window in rows'

  mov si, 0xB800
  mov ds, si
  mov es, si
  shr word ptr [bp + SWD_CHAR_HEIGHT], 1  ; dividing char. height by two because of cga addressing
  mul word ptr [bp + SWD_SCREEN_WIDTH]
  mul word ptr [bp + SWD_CHAR_HEIGHT]
  add di, ax
  mov ax, [bp + SWD_CHAR_HEIGHT]
  dec ax
  mul word ptr [bp + SWD_SCREEN_WIDTH]
  add di, ax
  add di, 0x2000    ; es:di = address of bottom right corner (destination)

  pop dx            ; dx = height of window in rows

  mov ax, cx        ; ax = lines to scroll
  cmp cx, dx
  jz swd_c2cm_clear_area

  push dx           ; push 'height of window in rows'
  mul word ptr [bp + SWD_SCREEN_WIDTH]
  mul word ptr [bp + SWD_CHAR_HEIGHT]
  pop dx            ; dx = height of window in rows
  mov si, di
  sub si, ax        ; ds:si = address of line that will become the bottom right corner of the window (source)

  sub dx, cx        ; dx = height of window in rows - lines to scroll
  mov ax, dx        ; ax = height of window in rows - lines to scroll
  mul word ptr [bp + SWD_CHAR_HEIGHT]   ; ax = char height * (height of window in rows - lines to scroll) / 2
  xchg ax, cx       ; cx = char height * (height of window in rows - lines to scroll) / 2, ax = lines to scroll

swd_c2cm_copy_lines_loop:
  push cx
  mov cx, [bp + SWD_WINDOW_WIDTH]
  rep movsb
  sub di, [bp + SWD_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  sub di, 0x2000 - 80
  sub si, [bp + SWD_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  sub si, 0x2000 - 80
  mov cx, [bp + SWD_WINDOW_WIDTH]
  rep movsb
  sub di, [bp + SWD_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  add di, 0x2000
  sub si, [bp + SWD_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  add si, 0x2000
  pop cx
  loop swd_c2cm_copy_lines_loop

swd_c2cm_clear_area:
  mul word ptr [bp + SWD_CHAR_HEIGHT]  ; ax = lines to scroll * char height / 2
  mov cx, ax        ; cx = lines to scroll * char height / 2
  mov al, 0         ; background colour always 0 (bh ignored)

swd_c2cm_clear_area_loop:
  push cx
  mov cx, [bp + SWD_WINDOW_WIDTH]
  rep stosb
  sub di, [bp + SWD_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  sub di, 0x2000 - 80
  mov cx, [bp + SWD_WINDOW_WIDTH]
  rep stosb
  sub di, [bp + SWD_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  add di, 0x2000
  pop cx
  loop swd_c2cm_clear_area_loop

  jmp end_scroll_window_down

swd_256_colour_mode:
  push dx           ; push 'height of window in rows'

  mov si, 0xA000
  mov ds, si
  mov es, si
  shl word ptr [bp + SWD_SCREEN_WIDTH_MINUS_WINDOW_WIDTH], 3  ; character width = 8 bytes
  shl word ptr [bp + SWD_WINDOW_WIDTH], 2  ; character width = 8 bytes, copying will be done word by word
  mul word ptr [bp + SWD_SCREEN_WIDTH]
  mul word ptr [bp + SWD_CHAR_HEIGHT]
  add di, ax
  mov ax, [bp + SWD_CHAR_HEIGHT]
  dec ax
  mul word ptr [bp + SWD_SCREEN_WIDTH]
  add di, ax
  shl di, 3         ; character width = 8 bytes
  add di, 6         ; (character width = 8 bytes, copying word by word) es:di = address of bottom right corner (destination)

  pop dx            ; dx = height of window in rows

  mov ax, cx        ; ax = lines to scroll
  cmp cx, dx
  jz swd_256cm_clear_area

  push dx           ; push 'height of window in rows'
  mul word ptr [bp + SWD_SCREEN_WIDTH]
  mul word ptr [bp + SWD_CHAR_HEIGHT]
  pop dx            ; dx = height of window in rows
  shl ax, 3         ; character width = 8 bytes
  mov si, di
  sub si, ax        ; ds:si = address of line that will become the bottom right corner of the window (source)

  sub dx, cx        ; dx = height of window in rows - lines to scroll
  mov ax, dx        ; ax = height of window in rows - lines to scroll
  mul word ptr [bp + SWD_CHAR_HEIGHT]   ; ax = char height * (height of window in rows - lines to scroll)
  xchg ax, cx       ; cx = char height * (height of window in rows - lines to scroll), ax = lines to scroll

swd_256cm_copy_lines_loop:
  push cx
  mov cx, [bp + SWD_WINDOW_WIDTH]
  rep movsw
  sub di, [bp + SWD_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  sub si, [bp + SWD_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  pop cx
  loop swd_256cm_copy_lines_loop

swd_256cm_clear_area:
  mul word ptr [bp + SWD_CHAR_HEIGHT]   ; ax = char height * lines to scroll
  mov cx, ax        ; cx = char height * lines to scroll
  mov al, bh
  mov ah, bh        ; ax now contains two pixels with the background colour

swd_256cm_clear_area_loop:
  push cx
  mov cx, [bp + SWD_WINDOW_WIDTH]
  rep stosw
  sub di, [bp + SWD_SCREEN_WIDTH_MINUS_WINDOW_WIDTH]
  pop cx
  loop swd_256cm_clear_area_loop

  jmp end_scroll_window_down
