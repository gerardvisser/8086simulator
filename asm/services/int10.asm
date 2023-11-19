;
; int 0x10
;
; loaded at: 0xC000:0x0011
;

; Variable offsets (relative to cs):

CGA_COLOURS = 0x00                 ; 2 bytes
EGA_COLOURS = 0x02                 ; 2 bytes
DEFAULT_256_COLOURS = 0x04         ; 2 bytes
FONT_8 = 0x06                      ; 2 bytes
FONT_14 = 0x08                     ; 2 bytes
FONT_16 = 0x0A                     ; 2 bytes
SCREEN_WIDTH_IN_PIXELS = 0x0C      ; 2 bytes
SCREEN_HEIGHT_IN_PIXELS = 0x0E     ; 2 bytes
GFX_MODE_BACKGROUND_COLOUR = 0x10  ; 1 byte

; Offsets relative to segment 0x0000:

INT_43 = 0x10C              ; 4 bytes
ACTIVE_MODE = 0x449         ; 1 byte
SCREEN_WIDTH = 0x44A        ; 2 bytes, screen width in characters
REGEN_SIZE = 0x44C          ; 2 bytes
ACTIVE_PAGE_OFFSET = 0x44E  ; 2 bytes
CURSOR_LOCATIONS = 0x450    ; 16 bytes
ACTIVE_PAGE = 0x462         ; 1 byte
SCREEN_ROWS = 0x484         ; 1 byte, screen height in text rows minus one
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
  cmp ah, 8
  jnz ah_not_8
  jmp read_character_and_attribute

ah_not_8:
  cmp ah, 9
  jnz ah_not_9
  jmp draw_character_and_attribute

ah_not_9:
  cmp ah, 0xA
  jnz ah_not_A
  jmp draw_character

ah_not_A:
  cmp ah, 0xB
  jnz ah_not_B
  jmp set_cga_palette

ah_not_B:
  cmp ah, 0xC
  jnz ah_not_C
  jmp put_pixel

ah_not_C:
  cmp ah, 0xD
  jnz ah_not_D
  jmp get_pixel

ah_not_D:
  cmp ah, 0xE
  jnz ah_not_E
  jmp draw_character_in_teletype_mode

ah_not_E:
  cmp ah, 0xF
  jnz ah_not_F
  jmp get_video_mode

ah_not_F:
  cmp ah, 0x10
  jnz ah_not_10
  jmp ah_is_10

ah_not_10:

  iret


ah_is_10:
  cmp al, 0
  jnz ah_is_10_al_not_0
  jmp set_palette_register

ah_is_10_al_not_0:

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
  ;  ds = cs
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
  mov byte ptr [GFX_MODE_BACKGROUND_COLOUR], 0
  call determine_resolution
  mov [SCREEN_WIDTH_IN_PIXELS], cx
  mov [SCREEN_HEIGHT_IN_PIXELS], si
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
  call set_cga_default_palette
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
; Function: set_cga_default_palette
; Output:
;   DX = 0x3C0
;
set_cga_default_palette:
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
svm_set_cga_default_palette_8_16:
  out dx, al
  add al, 8
  out dx, al
  sub al, 7
  loop svm_set_cga_default_palette_8_16
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
;   Sets registers 0x3C0, 0x10; 0x3C0, 0x11 and 0x3C0, 0x14, puts the index back at 0x20 and reads 0x3DA.
; Input:
;   DX = 0x3C0
;   BH = value for the mode control register (0x3C0, 0x10).
;
set_remaining_attr_registers:
  mov al, 0x30
  out dx, al
  mov al, bh
  out dx, al
  ;
  mov al, 0x31
  out dx, al
  mov al, 0
  out dx, al
  ;
  mov al, 0x34
  out dx, al
  mov al, 0
  out dx, al
  ;
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
; Function: determine_resolution
; Input:
;   BH = value of register 0x3C4, 1
; Output:
;   CX = screen width in pixels
;   SI = screen height in pixels
; Affected registers:
;   AX, DX
;
determine_resolution:
  push ds
  ;
  xor si, si        ; si = 0
  mov ds, si        ; ds = 0x0000
  ;
  mov ax, [SCREEN_WIDTH]  ; ax = screen width in characters
  mov cx, 8
  test bh, 1        ; are characters 8 or 9 pixels wide?
  jnz dr_char_width_in_cx
  inc cx
dr_char_width_in_cx:
  mul cx            ; ax = screen width in pixels
  mov cx, ax        ; cx = screen width in pixels
  ;
  mov dx, 0x3D4     ; dx = 0x3D4
  mov al, 7
  out dx, al        ; selecting overflow register
  inc dx            ; dx = 0x3D5
  in al, dx
  dec dx            ; dx = 0x3D4
  test al, 0x40     ; vertical end bit 9
  jz dr_ve_bit_9_set
  inc si
dr_ve_bit_9_set:
  shl si, 1
  test al, 0x02     ; vertical end bit 8
  jz dr_ve_bit_8_set
  or si, 1
dr_ve_bit_8_set:
  shl si, 8
  ;
  mov ax, 0x0012
  out dx, al        ; selecting vertical end register
  inc dx            ; dx = 0x3D5
  in al, dx
  dec dx            ; dx = 0x3D4
  or si, ax
  inc si            ; si = screen height
  ;
  mov al, 9
  out dx, al        ; selecting maximum scan line register
  inc dx            ; dx = 0x3D5
  in al, dx
  test al, 0x80     ; is scan doubling bit set?
  jz dr_si_contains_screen_height_in_pixels
  shr si, 1
  ;
dr_si_contains_screen_height_in_pixels:
  pop ds
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


;
; Read Character And Attribute
;
; Input:
;   AH = 0x08
;   BH = page
; Output:
;   AH = attribute
;   AL = character code (code page 437)
;
RCAA_STACK_FRAME_SIZE = 16

read_character_and_attribute:
  push bx
  push cx
  push dx
  push bp
  push si
  push di
  push es
  push ds

  sub sp, RCAA_STACK_FRAME_SIZE
  mov bp, sp

  cld

  xor ax, ax
  mov ds, ax        ; ds = 0x0000

  mov bl, [ACTIVE_MODE]
  cmp bl, 3
  ja rcaa_mode_above_3
  jmp rcaa_text_modes
rcaa_mode_above_3:
  cmp bl, 5
  ja rcaa_mode_above_5
  jmp rcaa_cga_4_colour_modes
rcaa_mode_above_5:
  cmp bl, 6
  ja rcaa_mode_above_6
  jmp rcaa_cga_2_colour_mode
rcaa_mode_above_6:
  cmp bl, 0x13
  jnz rcaa_16_colour_modes
  jmp rcaa_256_colour_mode

rcaa_text_modes:
  cmp bh, 7
  ja end_read_character_and_attribute  ; no valid page specified: return 0 (ax should be 0 here!)
  mov ax, [REGEN_SIZE]
  shr bx, 8         ; bx = page
  mul bx            ; ax = offset of top left character in page
  mov si, ax        ; si = offset of top left character in page
  ;
  shl bx, 1         ; bx = 2 * page
  mov cx, [bx + CURSOR_LOCATIONS]  ; ch = row (y), cl = column (x)
  mov al, ch
  mov ah, 0         ; ax = row (y)
  mov ch, 0         ; cx = column (x)
  mul word ptr [SCREEN_WIDTH]
  add ax, cx        ; ax = y * [SCREEN_WIDTH] + x
  shl ax, 1         ; character width = 2 bytes
  add si, ax        ; si = location of character and attribute
  ;
  mov ax, 0xB800
  mov ds, ax        ; ds = 0xB800
  lodsw             ; ah = attribute, al = character code

end_read_character_and_attribute:
  add sp, RCAA_STACK_FRAME_SIZE
  pop ds
  pop es
  pop di
  pop si
  pop bp
  pop dx
  pop cx
  pop bx
  iret

rcaa_16_colour_modes:
  cmp bl, 0x11
  jb rcaa_mode_below_11
  mov cx, [CURSOR_LOCATIONS]  ; ch = row (y), cl = column (x)
  xor si, si        ; si = page offset
  jmp rcaa_si_equals_page_offset
rcaa_mode_below_11:
  cmp bl, 0xF
  jb rcaa_mode_below_F
  cmp bh, 2
  jb rcaa_valid_page
  jmp end_read_character_and_attribute  ; no valid page specified: return 0 (ax should be 0 here!)
rcaa_mode_below_F:
  cmp bl, 0xE
  jb rcaa_mode_below_E
  cmp bh, 4
  jb rcaa_valid_page
  jmp end_read_character_and_attribute  ; no valid page specified: return 0 (ax should be 0 here!)
rcaa_mode_below_E:
  cmp bh, 7
  ja end_read_character_and_attribute  ; no valid page specified: return 0 (ax should be 0 here!)
rcaa_valid_page:
  mov ax, [REGEN_SIZE]
  shr bx, 8         ; bx = page
  mul bx            ; ax = offset of top left pixel in page (page offset)
  mov si, ax        ; si = page offset
  shl bx, 1         ; bx = 2 * page
  mov cx, [bx + CURSOR_LOCATIONS]  ; ch = row (y), cl = column (x)
rcaa_si_equals_page_offset:
  mov al, ch
  mov ah, 0         ; ax = row (y)
  mov ch, 0         ; cx = column (x)
  mul word ptr [SCREEN_WIDTH]
  mul word ptr [CHAR_HEIGHT]
  add ax, cx        ; ax = y * [SCREEN_WIDTH] * [CHAR_HEIGHT] + x
  add si, ax        ; si = location of character's first eight pixels
  ;
  mov bx, [SCREEN_WIDTH]  ; bx = screen width in characters
  dec bx            ; bx = screen width in characters - 1
  mov cx, [CHAR_HEIGHT]   ; cx = character height
  push [INT_43 + 2] ; push 'segment of font table'
  push [INT_43]     ; push 'offset of font table'
  mov ax, 0xA000
  mov ds, ax        ; ds = 0xA000
  push ss
  pop es            ; es = ss
  mov di, bp
  ;
  mov dx, 0x3CE
  mov ax, 0x0805
  out dx, ax        ; setting read mode 1
  mov ax, 0x0F07
  out dx, ax        ; colour don't care 0xF
  cs: mov ah, [GFX_MODE_BACKGROUND_COLOUR]
  mov al, 2
  out dx, ax        ; colour compare is background colour
  ;
  push cx           ; push 'character height'
rcaa_16cm_read_char_bits:
  lodsb
  xor al, 0xFF      ; al = eight character bits
  stosb
  add si, bx        ; si = location of character's next eight pixels
  loop rcaa_16cm_read_char_bits
  pop bx            ; bx = character height
  ;
  pop si            ; si = offset of font table
  pop ds            ; ds = segment of font table
  call find_character_code
  jmp end_read_character_and_attribute

rcaa_cga_4_colour_modes:
  mov cx, [CURSOR_LOCATIONS]  ; ch = row (y), cl = column (x)
  mov al, ch
  mov ah, 0         ; ax = row (y)
  mov ch, 0         ; cx = column (x)
  shl cx, 1         ; cx = 2 * x
  mul word ptr [SCREEN_WIDTH]
  mul word ptr [CHAR_HEIGHT]
  add ax, cx        ; ax = y * 2 * [SCREEN_WIDTH] * [CHAR_HEIGHT] / 2 + 2 * x
  mov si, ax        ; si = location of character's first four pixels
  ;
  mov bx, [SCREEN_WIDTH]  ; bx = screen width in characters
  shl bx, 1         ; bx = screen width in bytes
  mov cx, [CHAR_HEIGHT]   ; cx = character height
  push [INT_43 + 2] ; push 'segment of font table'
  push [INT_43]     ; push 'offset of font table'
  mov ax, 0xB800
  mov ds, ax        ; ds = 0xB800
  push ss
  pop es            ; es = ss
  mov di, bp
  cs: mov dl, [GFX_MODE_BACKGROUND_COLOUR]
  ;
  push cx           ; push 'character height'
  shr cx, 1         ; two scan lines per loop
rcaa_c4cm_read_char_bits:
  call get_character_line_pattern_cga_4_colour_mode
  stosb
  add si, 0x1FFE
  call get_character_line_pattern_cga_4_colour_mode
  stosb
  sub si, 0x2002
  add si, bx
  loop rcaa_c4cm_read_char_bits
  pop bx            ; bx = character height
  ;
  pop si            ; si = offset of font table
  pop ds            ; ds = segment of font table
  call find_character_code
  jmp end_read_character_and_attribute

rcaa_cga_2_colour_mode:
  mov cx, [CURSOR_LOCATIONS]  ; ch = row (y), cl = column (x)
  mov al, ch
  mov ah, 0         ; ax = row (y)
  mov ch, 0         ; cx = column (x)
  mul word ptr [SCREEN_WIDTH]
  mul word ptr [CHAR_HEIGHT]
  shr ax, 1         ; ax = y * [SCREEN_WIDTH] * [CHAR_HEIGHT] / 2
  add ax, cx        ; ax = y * [SCREEN_WIDTH] * [CHAR_HEIGHT] / 2 + x
  mov si, ax        ; si = location of character's first eight pixels
  ;
  mov bx, [SCREEN_WIDTH]  ; bx = screen width in characters (and bytes)
  mov cx, [CHAR_HEIGHT]   ; cx = character height
  push [INT_43 + 2] ; push 'segment of font table'
  push [INT_43]     ; push 'offset of font table'
  mov ax, 0xB800
  mov ds, ax        ; ds = 0xB800
  push ss
  pop es            ; es = ss
  mov di, bp
  ;
  push cx           ; push 'character height'
  shr cx, 1         ; two scan lines per loop
rcaa_c2cm_read_char_bits:
  movsb
  add si, 0x1FFF
  movsb
  sub si, 0x2001
  add si, bx
  loop rcaa_c2cm_read_char_bits
  pop bx            ; bx = character height
  ;
  pop si            ; si = offset of font table
  pop ds            ; ds = segment of font table
  call find_character_code
  jmp end_read_character_and_attribute

rcaa_256_colour_mode:
  mov cx, [CURSOR_LOCATIONS]  ; ch = row (y), cl = column (x)
  mov al, ch
  mov ah, 0         ; ax = row (y)
  mov ch, 0         ; cx = column (x)
  mul word ptr [SCREEN_WIDTH]
  mul word ptr [CHAR_HEIGHT]
  add ax, cx        ; ax = y * [SCREEN_WIDTH] * [CHAR_HEIGHT] + x
  shl ax, 3         ; ax = 8 * (y * [SCREEN_WIDTH] * [CHAR_HEIGHT] + x)
  mov si, ax        ; si = location of character's first pixel
  ;
  mov bx, [SCREEN_WIDTH]  ; bx = screen width in characters
  dec bx            ; bx = screen width in characters - 1
  shl bx, 3         ; bx = screen width in bytes - 8
  mov cx, [CHAR_HEIGHT]   ; cx = character height
  push [INT_43 + 2] ; push 'segment of font table'
  push [INT_43]     ; push 'offset of font table'
  mov ax, 0xA000
  mov ds, ax        ; ds = 0xA000
  push ss
  pop es            ; es = ss
  mov di, bp
  cs: mov dl, [GFX_MODE_BACKGROUND_COLOUR]
  ;
  push cx           ; push 'character height'
rcaa_256cm_read_char_bits:
  call get_character_line_pattern_256_colour_mode
  stosb
  add si, bx
  loop rcaa_256cm_read_char_bits
  pop bx            ; bx = character height
  ;
  pop si            ; si = offset of font table
  pop ds            ; ds = segment of font table
  call find_character_code
  jmp end_read_character_and_attribute

;
; Function: find_character_code
; Input:
;   BX = character height
;   ES:BP = pointer to the character bit pattern to identify
;   DS:SI = pointer to font table
; Output:
;   AX = character code (code page 437)
; Affected registers:
;   CX, DX, SI, DI, DS
;
find_character_code:
  add si, bx        ; si = offset for character 0x01
  ;
  mov cx, 0xFF
fcc_check_char:
  mov ax, cx        ; ax = loop count
  mov dx, si        ; dx = offset of character to compare
  mov di, bp
  mov cx, bx        ; cx = character height
  repz cmpsb
  jnz fcc_checked_char_not_equal
  mov cx, ax        ; restore loop count
  jmp fcc_checking_chars_done
fcc_checked_char_not_equal:
  mov si, dx
  add si, bx        ; si = offset of next character to compare
  mov cx, ax        ; restore loop count
  loop fcc_check_char
  ;
fcc_checking_chars_done:
  mov ax, 0x100
  sub ax, cx
  mov ah, 0
  ret

;
; Function: get_character_line_pattern_cga_4_colour_mode
; Input:
;   DL = background colour
;   DS:SI = pointer to two bytes in video memory
; Output:
;   AL = character line pattern: background bits are 0
; Affected registers:
;   AH, DH, SI
;
get_character_line_pattern_cga_4_colour_mode:
  push bx
  push cx
  ;
  lodsw
  xchg ah, al
  mov bx, ax
  xor ax, ax
  mov cx, 8
gclpc4cm_loop:
  mov dh, 0
  rcl bx, 1
  rcl dh, 1
  rcl bx, 1
  rcl dh, 1
  cmp dh, dl
  jz gclpc4cm_clc
  stc
  jmp gclpc4cm_add_bit
gclpc4cm_clc:
  clc
gclpc4cm_add_bit:
  rcl al, 1
  loop gclpc4cm_loop
  ;
  pop cx
  pop bx
  ret

;
; Function: get_character_line_pattern_256_colour_mode
; Input:
;   DL = background colour
;   DS:SI = pointer to eight bytes in video memory
; Output:
;   AL = character line pattern: background bits are 0
; Affected registers:
;   SI
;
get_character_line_pattern_256_colour_mode:
  push bx
  push cx
  ;
  mov cx, 8
gclp256cm_loop:
  lodsb
  cmp al, dl
  jz gclp256cm_clc
  stc
  jmp gclp256cm_add_bit
gclp256cm_clc:
  clc
gclp256cm_add_bit:
  rcl bl, 1
  loop gclp256cm_loop
  mov al, bl
  ;
  pop cx
  pop bx
  ret


;
; Draw Character And Attribute
;
; Input:
;   AH = 0x09
;   AL = character code (code page 437)
;   BH = page
;   BL = attribute/colour
;   CX = repeat count
;
DCAA_COLOUR_AND_CHAR_CODE = 0
DCAA_COLOUR = 1
DCAA_STACK_FRAME_SIZE = 2

draw_character_and_attribute:
  push ax
  push bx
  push cx
  push dx
  push di
  push es
  push ds

  jcxz end_draw_character_and_attribute

  cld

  xor dx, dx
  mov ds, dx        ; ds = 0x0000

  mov dl, [ACTIVE_MODE]
  cmp dl, 3
  ja dcaa_gfx_modes

  cmp bh, 7
  ja end_draw_character_and_attribute

  mov ah, bl        ; ah = attribute, al = character code
  push ax           ; push 'ah = attribute, al = character code'
  push cx           ; push 'repeat count'
  shr bx, 8         ; bx = page
  mov ax, [REGEN_SIZE]
  mul bx            ; ax = offset of top left character in page
  mov di, ax        ; di = offset of top left character in page

  shl bx, 1         ; bx = 2 * page
  mov cx, [bx + CURSOR_LOCATIONS]  ; ch = row (y), cl = column (x)
  mov al, ch
  mov ah, 0         ; ax = row (y)
  mov ch, 0         ; cx = column (x)
  mul word ptr [SCREEN_WIDTH]
  add ax, cx        ; ax = y * [SCREEN_WIDTH] + x
  shl ax, 1         ; character width = 2 bytes
  add di, ax        ; di = location of character and attribute

  mov ax, 0xB800
  mov es, ax        ; es = 0xB800
  pop cx            ; cx = repeat count
  pop ax            ; ah = attribute, al = character code
  rep stosw

end_draw_character_and_attribute:
  pop ds
  pop es
  pop di
  pop dx
  pop cx
  pop bx
  pop ax
  iret

end_dcaa_gfx_modes:
  add sp, DCAA_STACK_FRAME_SIZE
  pop si
  pop bp
  jmp end_draw_character_and_attribute

dcaa_gfx_modes:
  push bp
  push si

  sub sp, DCAA_STACK_FRAME_SIZE
  mov bp, sp

  mov ah, bl        ; ah = colour, al = character code
  mov [bp + DCAA_COLOUR_AND_CHAR_CODE], ax

  cmp dl, 5
  ja dcaa_mode_above_5
  jmp dcaa_cga_4_colour_modes
dcaa_mode_above_5:
  cmp dl, 6
  ja dcaa_mode_above_6
  jmp dcaa_cga_2_colour_mode
dcaa_mode_above_6:
  cmp dl, 0x13
  jnz dcaa_16_colour_mode
  jmp dcaa_256_colour_mode

dcaa_16_colour_mode:
  push cx           ; push 'repeat count'
  ;
  mov di, 0xA000
  mov es, di        ; es = 0xA000
  ;
  cmp dl, 0x11
  jb dcaa_mode_below_11
  mov cx, [CURSOR_LOCATIONS]  ; ch = row (y), cl = column (x)
  xor di, di        ; di = page offset
  jmp dcaa_di_equals_page_offset
dcaa_mode_below_11:
  cmp dl, 0xF
  jb dcaa_mode_below_F
  cmp bh, 2
  jb dcaa_valid_page
  jmp end_dcaa_gfx_modes
dcaa_mode_below_F:
  cmp dl, 0xE
  jb dcaa_mode_below_E
  cmp bh, 4
  jb dcaa_valid_page
  jmp end_dcaa_gfx_modes
dcaa_mode_below_E:
  cmp bh, 7
  ja end_dcaa_gfx_modes
dcaa_valid_page:
  mov ax, [REGEN_SIZE]
  shr bx, 8         ; bx = page
  mul bx            ; ax = offset of top left pixel in page (page offset)
  mov di, ax        ; di = page offset
  shl bx, 1         ; bx = 2 * page
  mov cx, [bx + CURSOR_LOCATIONS]  ; ch = row (y), cl = column (x)
dcaa_di_equals_page_offset:
  mov bx, [CHAR_HEIGHT]  ; bx = character height
  mov al, ch
  mov ah, 0         ; ax = row (y)
  mov ch, 0         ; cx = column (x)
  mul word ptr [SCREEN_WIDTH]
  mul bx            ; ax *= [CHAR_HEIGHT]
  add ax, cx        ; ax = y * [SCREEN_WIDTH] * [CHAR_HEIGHT] + x
  add di, ax        ; es:di = location of character's first eight pixels
  ;
  mov ax, [bp + DCAA_COLOUR_AND_CHAR_CODE]
  mov ah, 0
  mul bx            ; ax *= [CHAR_HEIGHT] = character code * character height
  mov si, [INT_43]
  add si, ax        ; si = location of character within font table
  ;
  mov dx, 0x3CE
  mov ax, 0xFF08
  out dx, ax        ; bitmask 0xFF
  mov ax, [bp + DCAA_COLOUR_AND_CHAR_CODE]
  mov al, 0
  out dx, ax        ; set/reset register filled with draw colour
  test ah, 0x80     ; is colour bit 7 set?
  jz dcaa_16cm_do_not_use_any_logical_operation
  ;
  mov ax, 0x1803
  out dx, ax        ; no rotation, logical operation = xor
  mov ax, 0x0305
  out dx, ax        ; write mode 3
  ;
  mov dx, [SCREEN_WIDTH]
  dec dx            ; dx = screen width - 1
  pop cx            ; cx = repeat count
  mov ds, [INT_43 + 2]  ; ds = segment of font table
  ;
  ; es:di = screen location
  ; ds:si = position in font table
  ; cx = repeat count
  ; bx = char height
  ; dx = screen width - 1
dcaa_16cm_put_char_repeat_loop_1:
  push cx
  push si
  push di
  mov cx, bx        ; cx = character height
dcaa_16cm_put_char_loop_1:
  es: mov al, [di]  ; loading the latches
  movsb
  add di, dx
  loop dcaa_16cm_put_char_loop_1
  pop di
  inc di
  pop si
  pop cx
  loop dcaa_16cm_put_char_repeat_loop_1
  jmp end_dcaa_gfx_modes
  ;
dcaa_16cm_do_not_use_any_logical_operation:
  mov ax, 0x0003
  out dx, ax        ; no rotation, no logical operation
  mov ax, 0x0205
  out dx, ax        ; write mode 2
  cs: mov al, [GFX_MODE_BACKGROUND_COLOUR]
  es: mov [di], al
  es: mov al, [di]  ; loading the latches
  mov ax, 0x0305
  out dx, ax        ; write mode 3
  ;
  mov dx, [SCREEN_WIDTH]
  dec dx            ; dx = screen width - 1
  pop cx            ; cx = repeat count
  mov ds, [INT_43 + 2]  ; ds = segment of font table
  ;
  ; es:di = screen location
  ; ds:si = position in font table
  ; cx = repeat count
  ; bx = char height
  ; dx = screen width - 1
dcaa_16cm_put_char_repeat_loop_2:
  push cx
  push si
  push di
  mov cx, bx        ; cx = character height
dcaa_16cm_put_char_loop_2:
  movsb
  add di, dx
  loop dcaa_16cm_put_char_loop_2
  pop di
  inc di
  pop si
  pop cx
  loop dcaa_16cm_put_char_repeat_loop_2
  jmp end_dcaa_gfx_modes

dcaa_256_colour_mode:
  push cx           ; push 'repeat count'
  ;
  mov di, 0xA000
  mov es, di        ; es = 0xA000
  ;
  mov cx, [CURSOR_LOCATIONS]  ; ch = row (y), cl = column (x)
  mov bx, [CHAR_HEIGHT]  ; bx = character height
  mov al, ch
  mov ah, 0         ; ax = row (y)
  mov ch, 0         ; cx = column (x)
  mul word ptr [SCREEN_WIDTH]
  mul bx            ; ax *= [CHAR_HEIGHT]
  add ax, cx        ; ax = y * [SCREEN_WIDTH] * [CHAR_HEIGHT] + x
  shl ax, 3         ; (character width = 8 bytes) ax = 8 * (y * [SCREEN_WIDTH] * [CHAR_HEIGHT] + x)
  mov di, ax        ; es:di = location of character's first pixel
  ;
  mov ax, [bp + DCAA_COLOUR_AND_CHAR_CODE]
  mov ah, 0
  mul bx            ; ax *= [CHAR_HEIGHT] = character code * character height
  mov si, [INT_43]
  add si, ax        ; si = location of character within font table
  ;
  mov dx, [SCREEN_WIDTH]
  dec dx            ; dx = screen width in chars - 1
  shl dx, 3         ; dx = char width in bytes * (screen width in chars - 1)
  pop cx            ; cx = repeat count
  mov ds, [INT_43 + 2]  ; ds = segment of font table
  ;
  ; es:di = screen location
  ; ds:si = position in font table
  ; cx = repeat count
  ; bx = char height
  ; dx = 8 * (screen width in chars - 1)
dcaa_256cm_put_char_repeat_loop:
  push cx
  push si
  push di
  ;
  mov cx, bx        ; cx = character height
  push bx
dcaa_256cm_put_char_loop:
  push cx
  ;
  mov bh, 0x80
  mov cx, 8
dcaa_256cm_put_char_line_loop:
  test [si], bh
  jnz dcaa_256cm_set_fgcolour
  cs: mov al, [GFX_MODE_BACKGROUND_COLOUR]
  jmp dcaa_256cm_put_pixel
dcaa_256cm_set_fgcolour:
  mov al, [bp + DCAA_COLOUR]
dcaa_256cm_put_pixel:
  stosb
  shr bh, 1
  loop dcaa_256cm_put_char_line_loop
  ;
  add di, dx        ; let di point to next line of character
  inc si            ; let si point to next line of character bit pattern
  pop cx
  loop dcaa_256cm_put_char_loop
  ;
  pop bx
  pop di
  add di, 8
  pop si
  pop cx
  loop dcaa_256cm_put_char_repeat_loop
  jmp end_dcaa_gfx_modes

dcaa_cga_4_colour_modes:
  push cx           ; push 'repeat count'
  ;
  mov di, 0xB800
  mov es, di        ; es = 0xB800
  ;
  mov cx, [CURSOR_LOCATIONS]  ; ch = row (y), cl = column (x)
  mov bx, [CHAR_HEIGHT]  ; bx = character height
  mov al, ch
  mov ah, 0         ; ax = row (y)
  mov ch, 0         ; cx = column (x)
  shl cx, 1         ; (character width = 2 bytes) cx = 2 * x
  mul word ptr [SCREEN_WIDTH]
  mul bx            ; ax *= [CHAR_HEIGHT]
  add ax, cx        ; ax = y * 2 * [SCREEN_WIDTH] * [CHAR_HEIGHT] / 2 + 2 * x
  mov di, ax        ; es:di = location of character's first four pixels
  ;
  mov ax, [bp + DCAA_COLOUR_AND_CHAR_CODE]
  mov ah, 0
  mul bx            ; ax *= [CHAR_HEIGHT] = character code * character height
  mov si, [INT_43]
  add si, ax        ; si = location of character within font table
  ;
  mov dx, [SCREEN_WIDTH]
  shl dx, 1         ; dx = screen width in bytes
  shr bx, 1         ; bx = char height / 2
  pop cx            ; cx = repeat count
  mov ds, [INT_43 + 2]  ; ds = segment of font table
  ;
  test byte ptr [bp + DCAA_COLOUR], 0x80  ; is colour bit 7 set?
  jnz dcaa_c4cm_use_xor_operation
  ;
  and byte ptr [bp + DCAA_COLOUR], 3
  ;
  ; es:di = screen location
  ; ds:si = position in font table
  ; cx = repeat count
  ; bx = char height / 2
  ; dx = screen width in bytes = screen width in characters * character width in bytes
dcaa_c4cm_put_char_repeat_loop_normal:
  push bx
  push cx
  push si
  push di
  mov cx, bx
  cs: mov bl, [GFX_MODE_BACKGROUND_COLOUR]  ; Value should be 0, 1, 2 or 3.
  ;
dcaa_c4cm_put_char_loop_normal:
  call create_character_line_pixels_cga_4_colour_mode
  stosw
  add di, 0x1FFE
  call create_character_line_pixels_cga_4_colour_mode
  stosw
  sub di, 0x2002
  add di, dx
  loop dcaa_c4cm_put_char_loop_normal
  ;
  pop di
  add di, 2
  pop si
  pop cx
  pop bx
  loop dcaa_c4cm_put_char_repeat_loop_normal
  jmp end_dcaa_gfx_modes
  ;
dcaa_c4cm_use_xor_operation:
  and byte ptr [bp + DCAA_COLOUR], 3
  ;
  ; es:di = screen location
  ; ds:si = position in font table
  ; cx = repeat count
  ; bx = char height / 2
  ; dx = screen width in bytes = screen width in characters * character width in bytes
dcaa_c4cm_put_char_repeat_loop_xor:
  push bx
  push cx
  push si
  push di
  mov cx, bx
  mov bl, 0
  ;
dcaa_c4cm_put_char_loop_xor:
  call create_character_line_pixels_cga_4_colour_mode
  es: xor [di], ax
  add di, 0x2000
  call create_character_line_pixels_cga_4_colour_mode
  es: xor [di], ax
  sub di, 0x2000
  add di, dx
  loop dcaa_c4cm_put_char_loop_xor
  ;
  pop di
  add di, 2
  pop si
  pop cx
  pop bx
  loop dcaa_c4cm_put_char_repeat_loop_xor
  jmp end_dcaa_gfx_modes

dcaa_cga_2_colour_mode:
  push cx           ; push 'repeat count'
  ;
  mov di, 0xB800
  mov es, di        ; es = 0xB800
  ;
  mov cx, [CURSOR_LOCATIONS]  ; ch = row (y), cl = column (x)
  mov bx, [CHAR_HEIGHT]  ; bx = character height
  mov al, ch
  mov ah, 0         ; ax = row (y)
  mov ch, 0         ; cx = column (x)
  mul word ptr [SCREEN_WIDTH]
  mul bx            ; ax *= [CHAR_HEIGHT]
  shr ax, 1
  add ax, cx        ; ax = y * [SCREEN_WIDTH] * [CHAR_HEIGHT] / 2 + x
  mov di, ax        ; es:di = location of character's first eight pixels
  ;
  mov ax, [bp + DCAA_COLOUR_AND_CHAR_CODE]
  mov ah, 0
  mul bx            ; ax *= [CHAR_HEIGHT] = character code * character height
  mov si, [INT_43]
  add si, ax        ; si = location of character within font table
  ;
  mov dx, [SCREEN_WIDTH]
  shr bx, 1         ; bx = char height / 2
  pop cx            ; cx = repeat count
  mov ds, [INT_43 + 2]  ; ds = segment of font table
  ;
  test byte ptr [bp + DCAA_COLOUR], 0x80  ; is colour bit 7 set?
  jnz dcaa_c2cm_use_xor_operation
  ;
  ; es:di = screen location
  ; ds:si = position in font table
  ; cx = repeat count
  ; bx = char height / 2
  ; dx = screen width
dcaa_c2cm_put_char_repeat_loop_normal:
  push cx
  push si
  push di
  mov cx, bx
  ;
dcaa_c2cm_put_char_loop_normal:
  movsb
  add di, 0x1FFF
  movsb
  sub di, 0x2001
  add di, dx
  loop dcaa_c2cm_put_char_loop_normal
  ;
  pop di
  inc di
  pop si
  pop cx
  loop dcaa_c2cm_put_char_repeat_loop_normal
  jmp end_dcaa_gfx_modes
  ;
dcaa_c2cm_use_xor_operation:
  ;
  ; es:di = screen location
  ; ds:si = position in font table
  ; cx = repeat count
  ; bx = char height / 2
  ; dx = screen width
dcaa_c2cm_put_char_repeat_loop_xor:
  push cx
  push si
  push di
  mov cx, bx
  ;
dcaa_c2cm_put_char_loop_xor:
  lodsb
  es: xor [di], al
  add di, 0x2000
  lodsb
  es: xor [di], al
  sub di, 0x2000
  add di, dx
  loop dcaa_c2cm_put_char_loop_xor
  ;
  pop di
  inc di
  pop si
  pop cx
  loop dcaa_c2cm_put_char_repeat_loop_xor
  jmp end_dcaa_gfx_modes

;
; Function: create_character_line_pixels_cga_4_colour_mode
; Input:
;   BL = background colour
;   DS:SI = position in font table
;   SS:BP+DCAA_COLOUR = foreground colour
; Output:
;   AX = character line pixels (AL = left four pixels, AH = right four pixels)
;   SI += 1
; Affected registers:
;   -
;
create_character_line_pixels_cga_4_colour_mode:
  push cx
  push dx
  ;
  mov cx, 8
  mov dl, 0x80
cclpc4cm_loop:
  shl ax, 2
  test [si], dl
  jz cclpc4cm_set_bgcolour
  or al, [bp + DCAA_COLOUR]
  shr dl, 1
  loop cclpc4cm_loop
  jmp cclpc4cm_loop_end
cclpc4cm_set_bgcolour:
  or al, bl
  shr dl, 1
  loop cclpc4cm_loop
cclpc4cm_loop_end:
  xchg ah, al
  inc si
  ;
  pop dx
  pop cx
  ret


;
; Draw Character
;
; Input:
;   AH = 0x0A
;   AL = character code (code page 437)
;   BH = page
;   BL = colour
;   CX = repeat count
;
draw_character:
  push ax
  push bx
  push cx
  push dx
  push di
  push es
  push ds

  jcxz end_draw_character

  cld

  xor dx, dx
  mov ds, dx        ; ds = 0x0000

  mov dl, [ACTIVE_MODE]
  cmp dl, 4
  jb dc_text_modes
  jmp dcaa_gfx_modes

dc_text_modes:
  cmp bh, 7
  ja end_draw_character
  ;
  push ax           ; push 'al = character code'
  push cx           ; push 'repeat count'
  shr bx, 8         ; bx = page
  mov ax, [REGEN_SIZE]
  mul bx            ; ax = offset of top left character in page
  mov di, ax        ; di = offset of top left character in page
  ;
  shl bx, 1         ; bx = 2 * page
  mov cx, [bx + CURSOR_LOCATIONS]  ; ch = row (y), cl = column (x)
  mov al, ch
  mov ah, 0         ; ax = row (y)
  mov ch, 0         ; cx = column (x)
  mul word ptr [SCREEN_WIDTH]
  add ax, cx        ; ax = y * [SCREEN_WIDTH] + x
  shl ax, 1         ; character width = 2 bytes
  add di, ax        ; di = location of character and attribute
  ;
  mov ax, 0xB800
  mov es, ax        ; es = 0xB800
  pop cx            ; cx = repeat count
  pop ax            ; al = character code
dc_put_char_repeat_loop:
  stosb
  inc di
  loop dc_put_char_repeat_loop

end_draw_character:
  jmp end_draw_character_and_attribute


;
; Set CGA Palette
;
; Especially useful for changing colours in modes 4 and 5 (320 x 200 four colour modes).
;
; Input:
;   AH = 0x0B
;   BH = 0 (background colour and foreground brightness)
;        1 (palette)
;   BL = (when BH = 0) bits 3 - 0: background colour, bit 4: foreground brightness
;      = (when BH = 1) 0: background, green, red, yellow
;                      1: background, cyan, magenta, white
;
set_cga_palette:
  push ax
  push bx
  push dx
  push ds

  xor dx, dx
  mov ds, dx        ; ds = 0x0000

  mov dl, [ACTIVE_MODE]
  cmp dl, 3
  ja scp_gfx_modes

  mov dx, 0x3DA
  in al, dx         ; set 0x3C0 to expect an index
  mov dx, 0x3C0     ; dx = 0x3C0
  mov al, 0x31
  out dx, al
  mov al, bl
  out dx, al        ; set overscan colour

end_set_cga_palette:
  mov al, 0x20
  out dx, al
  mov dx, 0x3DA
  in al, dx         ; set 0x3C0 to expect an index
  ;
  pop ds
  pop dx
  pop bx
  pop ax
  iret

scp_gfx_modes:
  cmp dl, 6
  jz scp_cga_2_colour_mode

  test bh, 1
  jz scp_set_background_colour

  mov dx, 0x3DA
  in al, dx         ; set 0x3C0 to expect an index
  mov dx, 0x3C0     ; dx = 0x3C0
  and bl, 1         ; two palettes can be chosen
  mov ah, 0xFE
  call set_four_colour_palette
  jmp end_set_cga_palette

scp_set_background_colour:
  mov bh, bl
  and bx, 0x0F10    ; bh = background colour, bl = foreground brightness
  test bh, 0x08
  jz scp_background_dac_index_in_bh
  and bh, 0x07
  or bh, 0x10
scp_background_dac_index_in_bh:
  mov dx, 0x3DA
  in al, dx         ; set 0x3C0 to expect an index
  mov dx, 0x3C0     ; dx = 0x3C0
  mov al, 0
  out dx, al
  mov al, bh
  out dx, al
  ;
  mov ah, 0xEF
  call set_four_colour_palette
  jmp end_set_cga_palette

scp_cga_2_colour_mode:
  and bl, 0x0F      ; the foreground colour
  test bl, 0x08
  jz scp_foreground_dac_index_in_bl
  and bl, 0x07
  or bl, 0x10
scp_foreground_dac_index_in_bl:
  push cx
  ;
  mov dx, 0x3DA
  in al, dx         ; set 0x3C0 to expect an index
  mov dx, 0x3C0     ; dx = 0x3C0
  mov cx, 15
scp_c2cm_set_foreground_colour_loop:
  mov al, cl
  out dx, al
  mov al, bl
  out dx, al
  loop scp_c2cm_set_foreground_colour_loop
  ;
  pop cx
  jmp end_set_cga_palette

;
; Function: set_four_colour_palette
; Input:
;   AH = mask to clear the palette bit
;   BL = the palette bit
;   DX = 0x3C0 (and port 0x3C0 expects an index)
; Output:
;   Port 0x3C0 expects an index
; Affected registers:
;   AL, BH
;
set_four_colour_palette:
  mov bh, 1
sfcp_set_palette_loop:
  mov al, bh
  out dx, al
  inc dx            ; dx = 0x3C1
  in al, dx
  dec dx            ; dx = 0x3C0
  and al, ah
  or al, bl
  out dx, al
  inc bh
  cmp bh, 4
  jb sfcp_set_palette_loop
  ret


;
; Put Pixel
;
; Input:
;   AH = 0x0C
;   AL = colour
;   BH = page
;   CX = x-coordinate
;   DX = y-coordinate
;
put_pixel:
  push ax
  push bx
  push cx
  push dx
  push di
  push ds

  xor di, di
  mov ds, di        ; ds = 0x0000

  mov bl, [ACTIVE_MODE]
  cmp bl, 3
  ja pp_gfx_modes
  jmp end_put_pixel

pp_gfx_modes:
  cs: cmp cx, [SCREEN_WIDTH_IN_PIXELS]
  jb pp_check_y_coordinate
  jmp end_put_pixel ; done if specified x-coordinate is too large
pp_check_y_coordinate:
  cs: cmp dx, [SCREEN_HEIGHT_IN_PIXELS]
  jnb end_put_pixel ; done if specified y-coordinate is too large

  cmp bl, 5
  ja pp_mode_above_5
  jmp pp_cga_4_colour_modes
pp_mode_above_5:
  cmp bl, 6
  ja pp_mode_above_6
  jmp pp_cga_2_colour_mode
pp_mode_above_6:
  cmp bl, 0x13
  jz pp_256_colour_mode

  cmp bl, 0x11
  jb pp_mode_below_11
  ;
  push ax           ; push 'colour'
  xor di, di        ; di = page offset
  jmp pp_di_equals_page_offset
pp_mode_below_11:
  cmp bl, 0xF
  jb pp_mode_below_F
  cmp bh, 2
  jb pp_valid_page
  jmp end_put_pixel
pp_mode_below_F:
  cmp bl, 0xE
  jb pp_mode_below_E
  cmp bh, 4
  jb pp_valid_page
  jmp end_put_pixel
pp_mode_below_E:
  cmp bh, 7
  ja end_put_pixel
pp_valid_page:
  push ax           ; push 'colour'
  push dx           ; push 'y-coordinate'
  ;
  mov ax, [REGEN_SIZE]
  shr bx, 8         ; bx = page
  mul bx            ; ax = offset of top left pixel in page (page offset)
  mov di, ax        ; di = page offset
  pop dx            ; dx = y-coordinate
pp_di_equals_page_offset:
  mov ax, 0xA000
  mov ds, ax        ; ds = 0xA000
  ;
  cs: mov ax, [SCREEN_WIDTH_IN_PIXELS]
  shr ax, 3         ; ax = screen width in bytes
  mul dx            ; multiply by y-coordinate
  add di, ax
  mov bx, cx        ; bx = x-coordinate
  and cl, 0x7       ; cl = x-coordinate % 8
  shr bx, 3         ; bx = x-coordinate / 8
  add di, bx        ; ds:di = pointer to byte to change
  ;
  mov ah, 0x80
  shr ah, cl        ; ah = bit mask of pixel to set
  mov dx, 0x3CE
  mov al, 8
  out dx, ax        ; set bitmask
  mov ax, 0x0003
  out dx, ax        ; no rotation, no logical operation
  mov ax, 0x0205
  out dx, ax        ; write mode 2
  ;
  mov al, [di]      ; loading the latches
  pop ax            ; al = colour
  mov [di], al      ; writing pixel

end_put_pixel:
  pop ds
  pop di
  pop dx
  pop cx
  pop bx
  pop ax
  iret

pp_256_colour_mode:
  push ax           ; push 'colour'
  mov ax, 0xA000
  mov ds, ax        ; ds = 0xA000
  ;
  cs: mov ax, [SCREEN_WIDTH_IN_PIXELS]  ; ax = screen width in bytes
  mul dx            ; multiply by y-coordinate
  mov di, ax
  add di, cx        ; ds:di = pointer to byte to change
  ;
  pop ax            ; al = colour
  mov [di], al      ; writing pixel
  ;
  jmp end_put_pixel

pp_cga_4_colour_modes:
  and al, 3         ; only 4 colours available
  push ax           ; push 'colour'
  mov ax, 0xB800
  mov ds, ax        ; ds = 0xB800
  ;
  xor di, di
  test dx, 1        ; is y-coordinate odd?
  jz pp_c4cm_di_contains_base_offset
  mov di, 0x2000
pp_c4cm_di_contains_base_offset:
  shr dx, 1         ; divide y-coordinate by 2
  cs: mov ax, [SCREEN_WIDTH_IN_PIXELS]
  shr ax, 2         ; ax = screen width in bytes
  mul dx            ; multiply by y-coordinate / 2
  add di, ax
  mov bx, cx        ; bx = x-coordinate
  and cl, 0x3       ; cl = x-coordinate % 4
  shr bx, 2         ; bx = x-coordinate / 4
  add di, bx        ; ds:di = pointer to byte to change
  ;
  shl cl, 1         ; we want to shift two bits per pixel
  mov al, 0xC0
  shr al, cl
  xor al, 0xFF      ; the bit mask for the pixel to change
  ;
  and [di], al
  pop ax            ; al = colour
  shl al, 6
  shr al, cl
  or [di], al
  ;
  jmp end_put_pixel

pp_cga_2_colour_mode:
  and al, 1         ; only 2 colours available
  push ax           ; push 'colour'
  mov ax, 0xB800
  mov ds, ax        ; ds = 0xB800
  ;
  xor di, di
  test dx, 1        ; is y-coordinate odd?
  jz pp_c2cm_di_contains_base_offset
  mov di, 0x2000
pp_c2cm_di_contains_base_offset:
  shr dx, 1         ; divide y-coordinate by 2
  cs: mov ax, [SCREEN_WIDTH_IN_PIXELS]
  shr ax, 3         ; ax = screen width in bytes
  mul dx            ; multiply by y-coordinate / 2
  add di, ax
  mov bx, cx        ; bx = x-coordinate
  and cl, 0x7       ; cl = x-coordinate % 8
  shr bx, 3         ; bx = x-coordinate / 8
  add di, bx        ; ds:di = pointer to byte to change
  ;
  mov al, 0x80
  shr al, cl
  xor al, 0xFF      ; the bit mask for the pixel to change
  ;
  and [di], al
  pop ax            ; al = colour
  shl al, 7
  shr al, cl
  or [di], al
  ;
  jmp end_put_pixel


;
; Get Pixel
;
; Input:
;   AH = 0x0D
;   BH = page
;   CX = x-coordinate
;   DX = y-coordinate
; Output:
;   AL = colour
;
get_pixel:
  push bx
  push cx
  push dx
  push si
  push ds

  xor si, si
  mov ds, si        ; ds = 0x0000

  mov bl, [ACTIVE_MODE]
  cmp bl, 3
  ja gp_gfx_modes
  jmp end_get_pixel

gp_gfx_modes:
  cs: cmp cx, [SCREEN_WIDTH_IN_PIXELS]
  jb gp_check_y_coordinate
  jmp end_get_pixel ; done if specified x-coordinate is too large
gp_check_y_coordinate:
  cs: cmp dx, [SCREEN_HEIGHT_IN_PIXELS]
  jb gp_coordinates_valid
  jmp end_get_pixel ; done if specified y-coordinate is too large

gp_coordinates_valid:
  cmp bl, 5
  ja gp_mode_above_5
  jmp gp_cga_4_colour_modes
gp_mode_above_5:
  cmp bl, 6
  ja gp_mode_above_6
  jmp gp_cga_2_colour_mode
gp_mode_above_6:
  cmp bl, 0x13
  jz gp_256_colour_mode

  cmp bl, 0x11
  jb gp_mode_below_11
  ;
  xor si, si        ; si = page offset
  jmp gp_si_equals_page_offset
gp_mode_below_11:
  cmp bl, 0xF
  jb gp_mode_below_F
  cmp bh, 2
  jb gp_valid_page
  jmp end_get_pixel
gp_mode_below_F:
  cmp bl, 0xE
  jb gp_mode_below_E
  cmp bh, 4
  jb gp_valid_page
  jmp end_get_pixel
gp_mode_below_E:
  cmp bh, 7
  ja end_get_pixel
gp_valid_page:
  push dx           ; push 'y-coordinate'
  ;
  mov ax, [REGEN_SIZE]
  shr bx, 8         ; bx = page
  mul bx            ; ax = offset of top left pixel in page (page offset)
  mov si, ax        ; si = page offset
  pop dx            ; dx = y-coordinate
gp_si_equals_page_offset:
  mov ax, 0xA000
  mov ds, ax        ; ds = 0xA000
  ;
  cs: mov ax, [SCREEN_WIDTH_IN_PIXELS]
  shr ax, 3         ; ax = screen width in bytes
  mul dx            ; multiply by y-coordinate
  add si, ax
  mov bx, cx        ; bx = x-coordinate
  and cl, 0x7       ; cl = x-coordinate % 8
  shr bx, 3         ; bx = x-coordinate / 8
  add si, bx        ; ds:si = pointer to byte to read
  mov bx, 0x0080    ; bh = 0
  shr bl, cl        ; bl = bit mask of pixel to read
  ;
  mov dx, 0x3CE
  mov ax, 0x0005
  out dx, ax        ; read mode 0
  mov ax, 0x0304
  ;
  mov cx, 4
gp_16cm_append_bit_loop:
  shl bh, 1
  out dx, ax        ; select a plane to read
  test [si], bl
  jz gp_16cm_bit_appended
  or bh, 1
gp_16cm_bit_appended:
  dec ah            ; ah = next plane to read
  loop gp_16cm_append_bit_loop
  ;
  mov al, bh        ; al = pixel colour

end_get_pixel:
  mov ah, 0x0D      ; restoring ah
  pop ds
  pop si
  pop dx
  pop cx
  pop bx
  iret

gp_256_colour_mode:
  mov ax, 0xA000
  mov ds, ax        ; ds = 0xA000
  ;
  cs: mov ax, [SCREEN_WIDTH_IN_PIXELS]  ; ax = screen width in bytes
  mul dx            ; multiply by y-coordinate
  mov si, ax
  add si, cx        ; ds:si = pointer to byte to read
  lodsb             ; reading pixel
  ;
  jmp end_get_pixel

gp_cga_4_colour_modes:
  mov ax, 0xB800
  mov ds, ax        ; ds = 0xB800
  ;
  xor si, si
  test dx, 1        ; is y-coordinate odd?
  jz gp_c4cm_si_contains_base_offset
  mov si, 0x2000
gp_c4cm_si_contains_base_offset:
  shr dx, 1         ; divide y-coordinate by 2
  cs: mov ax, [SCREEN_WIDTH_IN_PIXELS]
  shr ax, 2         ; ax = screen width in bytes
  mul dx            ; multiply by y-coordinate / 2
  add si, ax
  mov bx, cx        ; bx = x-coordinate
  shr bx, 2         ; bx = x-coordinate / 4
  add si, bx        ; ds:si = pointer to byte to read
  ;
  and cl, 0x3       ; cl = x-coordinate % 4
  shl cl, 1         ; we want to shift two bits per pixel
  xor cl, 0xFF      ; cl = -2*(x-coordinate % 4) - 1
  add cl, 7         ; cl = 6 - 2*(x-coordinate % 4)
  ;
  lodsb             ; reading four pixels
  shr al, cl
  and al, 3         ; al = pixel colour
  ;
  jmp end_get_pixel

gp_cga_2_colour_mode:
  mov ax, 0xB800
  mov ds, ax        ; ds = 0xB800
  ;
  xor si, si
  test dx, 1        ; is y-coordinate odd?
  jz gp_c2cm_si_contains_base_offset
  mov si, 0x2000
gp_c2cm_si_contains_base_offset:
  shr dx, 1         ; divide y-coordinate by 2
  cs: mov ax, [SCREEN_WIDTH_IN_PIXELS]
  shr ax, 3         ; ax = screen width in bytes
  mul dx            ; multiply by y-coordinate / 2
  add si, ax
  mov bx, cx        ; bx = x-coordinate
  shr bx, 3         ; bx = x-coordinate / 8
  add si, bx        ; ds:si = pointer to byte to read
  ;
  and cl, 0x7       ; cl = x-coordinate % 8
  xor cl, 0xFF      ; cl = -(x-coordinate % 8) - 1
  add cl, 8         ; cl = 7 - (x-coordinate % 8)
  ;
  lodsb             ; reading eight pixels
  shr al, cl
  and al, 1         ; al = pixel colour
  ;
  jmp end_get_pixel


;
; Draw Character In Teletype Mode
;
; Input:
;   AH = 0x0E
;   AL = character code (code page 437)
;   [BH = page: ignored, active page assumed]
;   BL = colour
;
draw_character_in_teletype_mode:
  push ax
  push bx
  push cx
  push ds
  ;
  xor cx, cx
  mov ds, cx        ; ds = 0x0000
  ;
  mov bh, [ACTIVE_PAGE]  ; bh = active page

  cmp al, 0x0D
  jnz dcitm_check_for_line_feed

  ; carriage return
  shr bx, 8         ; bx = active page
  shl bx, 1         ; bx = 2 * active page
  mov cx, [bx + CURSOR_LOCATIONS]  ; cl = cursor's x-coordinate, ch = cursor's y-coordinate
  mov cl, 0
  jmp dcitm_set_cursor_and_end

dcitm_check_for_line_feed:
  cmp al, 0x0A
  jnz dcitm_check_for_bell

  ; line feed
  shr bx, 8         ; bx = active page
  shl bx, 1         ; bx = 2 * active page
  mov cx, [bx + CURSOR_LOCATIONS]  ; cl = cursor's x-coordinate, ch = cursor's y-coordinate
  jmp dcitm_move_cursor_to_next_line

dcitm_check_for_bell:
  cmp al, 0x07
  jnz dcitm_check_for_backspace

  ; bell
  jmp end_draw_character_in_teletype_mode

dcitm_check_for_backspace:
  cmp al, 0x08
  jz dcitm_backspace

  mov cx, 1
  ;
  pushf
  push cs
  call draw_character
  ;
  shr bx, 8         ; bx = active page
  shl bx, 1         ; bx = 2 * active page
  mov cx, [bx + CURSOR_LOCATIONS]  ; cl = cursor's x-coordinate, ch = cursor's y-coordinate
  inc cl            ; increase cursor's x-coordinate
  cmp cl, [SCREEN_WIDTH]
  jb dcitm_set_cursor_and_end
  ;
  mov cl, 0
dcitm_move_cursor_to_next_line:
  cmp ch, [SCREEN_ROWS]
  jnb dcitm_scroll_needed
  ;
  inc ch            ; increase cursor's y-coordinate
  jmp dcitm_set_cursor_and_end
dcitm_scroll_needed:
  push bx
  push cx
  push dx
  ;
  mov dl, [ACTIVE_MODE]
  cmp dl, 3
  ja dcitm_scroll_needed_gfx_mode
  ;
  mov al, [SCREEN_ROWS]
  inc al
  shl al, 1
  mul byte ptr [SCREEN_WIDTH]
  dec ax
  push ds
  mov bx, 0xB800
  mov ds, bx        ; ds = 0xB800
  mov bx, ax
  mov al, [bx]
  pop ds            ; ds = 0x0000
  mov bh, al
  jmp dcitm_end_scroll_needed
dcitm_scroll_needed_gfx_mode:
  cs: mov bh, [GFX_MODE_BACKGROUND_COLOUR]
dcitm_end_scroll_needed:
  mov al, 1
  xor cx, cx
  mov dx, 0xFFFF
  ;
  pushf
  push cs
  call scroll_window_up
  ;
  pop dx
  pop cx
  pop bx

dcitm_set_cursor_and_end:
  ; PRE:
  ;  bx = cursor to change (= 2 * active page)
  ;  cl = cursor's x-coordinate
  ;  ch = cursor's y-coordinate
  mov [bx + CURSOR_LOCATIONS], cx

end_draw_character_in_teletype_mode:
  pop ds
  pop cx
  pop bx
  pop ax
  iret

dcitm_backspace:
  mov ax, bx        ; ah/al = active page/colour
  shr bx, 8         ; bx = active page
  shl bx, 1         ; bx = 2 * active page
  mov cx, [bx + CURSOR_LOCATIONS]  ; cl = cursor's x-coordinate, ch = cursor's y-coordinate
  jcxz end_draw_character_in_teletype_mode
  ;
  cmp cl, 0
  jnz dcitm_move_cursor_one_position_back
  ;
  mov cl, [SCREEN_WIDTH]
  dec ch
  ;
dcitm_move_cursor_one_position_back:
  dec cl
  mov [bx + CURSOR_LOCATIONS], cx
  ;
  mov bx, ax        ; bh = active page, bl = colour
  mov cx, 1
  mov al, 0x20      ; al = space
  ;
  pushf
  push cs
  call draw_character
  ;
  jmp end_draw_character_in_teletype_mode


;
; Get Video Mode
;
; Input:
;   AH = 0x0F
; Output:
;   AH = screen width in characters
;   AL = active video mode
;   BH = active page
;
get_video_mode:
  push ds
  ;
  xor ax, ax
  mov ds, ax        ; ds = 0x0000
  ;
  mov ah, [SCREEN_WIDTH]
  mov al, [ACTIVE_MODE]
  mov bh, [ACTIVE_PAGE]
  ;
  pop ds
  iret


;
; Set Palette Register
;
; Input:
;   AH = 0x10
;   AL = 0x00
;   BH = colour value (6 bit value)
;   BL = palette register (0x0 - 0xF)
;
set_palette_register:
  push ax
  push dx
  ;
  mov dx, 0x3DA
  in al, dx         ; set 0x3C0 to expect an index
  mov dx, 0x3C0     ; dx = 0x3C0
  mov al, bl
  and al, 0xF
  out dx, al
  mov al, bh
  out dx, al
  ;
  mov al, 0x20
  out dx, al
  mov dx, 0x3DA
  in al, dx         ; set 0x3C0 to expect an index
  ;
  pop dx
  pop ax
  iret
