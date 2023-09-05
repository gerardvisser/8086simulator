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
  pop ds
  mov di, 0xA000
  mov es, di

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

  cmp bx, 4
  jb set_text_mode
  jmp set_gfx_mode

set_text_mode:
  mov si, [FONT_16]
  xor di, di

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
  cmp bx, 2
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
  mov dx, 0x3D4     ; TODO: KUNNEN WE DIT ALS GEZET AANNEMEN?
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
  ;TODO

  jmp end_set_video_mode

;
; Function: set_ega_palette
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
