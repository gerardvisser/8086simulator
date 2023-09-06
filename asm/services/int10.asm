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

INT_43_OFFSET = 0x10C
ACTIVE_MODE_OFFSET = 0x449
SCREEN_WIDTH_OFFSET = 0x44A


  cmp ah, 0
  jnz ah_not_0
  jmp set_video_mode

ah_not_0:

  iret

;
; Set Video Mode
;
; AH = 0
; AL = mode
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
  pushf

  cli
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
  mov al, 0x30
  out dx, al
  mov al, 0x4
  out dx, al
  mov al, 0x34
  out dx, al
  mov al, 0
  out dx, al
  mov al, 0x20
  out dx, al
  mov dx, 0x3DA
  in al, dx         ; set 0x3C0 to expect an index

  mov dx, 0x3D4
  cmp bl, 2
  jb set_text_mode_40_cols
  mov ax, 0x4B00
  out dx, ax
  mov ax, 0x4F01
  out dx, ax
  jmp finish_setting_text_mode
set_text_mode_40_cols:
  mov ax, 0x2300
  out dx, ax
  mov ax, 0x2701
  out dx, ax
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

end_set_video_mode:
  ; PRE:
  ; ax = font offset
  ; bl = video mode
  ; bh = value for 0x3C4, 1: to enable video and set 8/9 pixels wide characters
  xor cx, cx
  mov es, cx        ; es = 0x0000
  mov di, INT_43_OFFSET
  stosw
  push cs
  pop ax
  stosw
  ;
  mov di, ACTIVE_MODE_OFFSET
  mov al, bl
  stosb
  ;
  mov dx, 0x3D4
  mov ax, 1         ; ax = 0x0001
  out dx, al
  inc dx
  in al, dx
  inc ax
  stosw             ; writing SCREEN_WIDTH_OFFSET
  ;
  mov dx, 0x3C4
  mov al, 0x01
  mov ah, bh
  out dx, ax        ; enable video

  popf
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
  mov al, 0x30
  out dx, al
  mov al, 0x1
  out dx, al
  mov al, 0x34
  out dx, al
  mov al, 0
  out dx, al
  mov al, 0x20
  out dx, al
  mov dx, 0x3DA
  in al, dx         ; set 0x3C0 to expect an index
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
  mov al, 0x30
  out dx, al
  mov al, 0x1
  out dx, al
  mov al, 0x34
  out dx, al
  mov al, 0
  out dx, al
  mov al, 0x20
  out dx, al
  mov dx, 0x3DA
  in al, dx         ; set 0x3C0 to expect an index
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
  mov al, 0x30
  out dx, al
  mov al, 0x1
  out dx, al
  mov al, 0x34
  out dx, al
  mov al, 0
  out dx, al
  mov al, 0x20
  out dx, al
  mov dx, 0x3DA
  in al, dx         ; set 0x3C0 to expect an index
  ;
  mov dx, 0x3D4
  cmp bl, 0xD
  jz svm_set_40_cols_mode_D
  mov ax, 0x5500
  out dx, ax
  mov ax, 0x4F01
  out dx, ax
  jmp svm_finish_setting_modes_D_and_E
svm_set_40_cols_mode_D:
  mov ax, 0x2800
  out dx, ax
  mov ax, 0x2701
  out dx, ax
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
  mov al, 0x30
  out dx, al
  mov al, 0x1
  out dx, al
  mov al, 0x34
  out dx, al
  mov al, 0
  out dx, al
  mov al, 0x20
  out dx, al
  mov dx, 0x3DA
  in al, dx         ; set 0x3C0 to expect an index
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
  mov al, 0x30
  out dx, al
  mov al, 0x1
  out dx, al
  mov al, 0x34
  out dx, al
  mov al, 0
  out dx, al
  mov al, 0x20
  out dx, al
  mov dx, 0x3DA
  in al, dx         ; set 0x3C0 to expect an index
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
  jmp end_set_video_mode

mode_above_12:
  cmp bl, 0x13
  jnz mode_above_13
  ;TODO

mode_above_13:
  cmp bl, 0x14
  jnz mode_above_14
  ;TODO

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
  out dx, al
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
