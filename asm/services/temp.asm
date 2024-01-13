

scroll_window_up: ;tijdelijk


SCREEN_WIDTH_IN_PIXELS = 0x0C      ; 2 bytes
SCREEN_HEIGHT_IN_PIXELS = 0x0E     ; 2 bytes
GFX_MODE_BACKGROUND_COLOUR = 0x10  ; 1 byte

; Offsets relative to segment 0x0000:

INT_1F = 0x07C              ; 4 bytes
INT_43 = 0x10C              ; 4 bytes
ACTIVE_MODE = 0x449         ; 1 byte
SCREEN_WIDTH = 0x44A        ; 2 bytes, screen width in characters
REGEN_SIZE = 0x44C          ; 2 bytes
ACTIVE_PAGE_OFFSET = 0x44E  ; 2 bytes
CURSOR_LOCATIONS = 0x450    ; 16 bytes
ACTIVE_PAGE = 0x462         ; 1 byte
SCREEN_ROWS = 0x484         ; 1 byte, screen height in text rows minus one
CHAR_HEIGHT = 0x485         ; 2 bytes
VGA_FLAGS = 0x489           ; 1 byte


;
; Draw String In Teletype Mode
;
; Input:
;   AH = 0x13
;   AL = write mode:
;        0: attribute/colour in BL, string only contains characters
;           cursor not updated
;        1: attribute/colour in BL, string only contains characters
;           cursor updated
;        2: string contains characters and attributes in turn
;           cursor not updated
;        3: string contains characters and attributes in turn
;           cursor updated
;   [BH = page: ignored, active page assumed]
;   BL = attribute/colour (writing modes 0 and 1)
;   CX = length of the string
;   DH = y coordinate to start drawing
;   DL = x coordinate to start drawing
;   ES:BP = pointer to the string
;
DSITM_CURSOR = 0               ; 2 bytes: cursor's x- and y-coordinate
DSITM_SCREEN_WIDTH_CHARS = 2   ; 2 bytes: screen width in characters
DSITM_MAX_Y = 4                ; 1 byte: screen height in text rows minus one
DSITM_BACKGROUND_COLOUR = 5    ; 1 byte: background colour/attribute
DSITM_CHAR_HEIGHT = 6          ; 2 bytes: character height
DSITM_CHAR_WIDTH = 8           ; 2 bytes: character width in bytes
DSITM_SCREEN_WIDTH_BYTES = 10  ; 2 bytes: screen width in bytes = character width in bytes * screen width in characters
DSITM_LINE_HEIGHT = 12         ; 2 bytes: line height = character height * screen width in bytes
DSITM_DRAW_CHAR = 14           ; 2 bytes: offset of a function that draws a single character
DSITM_SET_COLOUR = 16          ; 2 bytes: offset of a function that sets the foreground colour
DSITM_FONT_TABLE = 18          ; 4 bytes: pointer to a font table
DSITM_STACK_FRAME_SIZE = 22

DSITM_16CM_BACKGROUND_COLOUR = 0xFFF0

draw_string_in_teletype_mode:
  push ax
  push bx
  push cx
  push dx
  push bp
  push si
  push di
  push es
  push ds
  sub sp, DSITM_STACK_FRAME_SIZE
  ;
  cld
  mov si, bp        ; es:si = pointer to the string to draw
  mov bp, sp
  ;
  xor di, di
  mov ds, di        ; ds = 0x0000
  mov bh, al        ; bh = write mode
  ;
  cmp dh, [SCREEN_ROWS]
  jna dsitm_check_cursor_x
  jmp end_draw_string_in_teletype_mode
dsitm_check_cursor_x:
  cmp dl, [SCREEN_WIDTH]
  jb dsitm_cursor_within_bounds
  jmp end_draw_string_in_teletype_mode
dsitm_cursor_within_bounds:
  mov [bp + DSITM_CURSOR], dx
  mov di, [ACTIVE_PAGE_OFFSET]  ; di = offset of top of screen
  cmp cx, 0
  jnz dsitm_string_length_positive
  jmp dsitm_write_cursor_if_needed_and_end
  ;
dsitm_string_length_positive:
  mov ax, [SCREEN_WIDTH]  ; ax = screen width in characters
  mov [bp + DSITM_SCREEN_WIDTH_CHARS], ax
  mov dl, [SCREEN_ROWS]
  mov [bp + DSITM_MAX_Y], dl
  ;
  mov dl, [ACTIVE_MODE]
  cmp dl, 3
  ja dsitm_gfx_modes
  ;
  mov [bp + DSITM_BACKGROUND_COLOUR], bl
  mov word ptr [bp + DSITM_CHAR_WIDTH], 2
  shl ax, 1         ; ax = screen width in bytes
  mov [bp + DSITM_SCREEN_WIDTH_BYTES], ax
  mov [bp + DSITM_LINE_HEIGHT], ax
  push es
  pop ds            ; ds:si = pointer to the string to draw
  mov ax, 0xB800
  mov es, ax        ; es:di = top of screen
  call draw_string_in_teletype_mode_text
  jmp dsitm_write_cursor_if_needed_and_end

dsitm_gfx_modes:
  cs: mov dh, [GFX_MODE_BACKGROUND_COLOUR]
  mov [bp + DSITM_BACKGROUND_COLOUR], dh
  push ax           ; push 'screen width in characters'
  mov ax, [INT_43]
  mov [bp + DSITM_FONT_TABLE], ax
  mov ax, [INT_43 + 2]
  mov [bp + DSITM_FONT_TABLE + 2], ax
  mov ax, [CHAR_HEIGHT]
  mov [bp + DSITM_CHAR_HEIGHT], ax
  pop ax            ; ax = screen width in characters
  ;
  cmp dl, 5
  ja dsitm_mode_above_5
  mov word ptr [bp + DSITM_CHAR_WIDTH], 2
  shl ax, 1         ; ax = screen width in bytes
  mov [bp + DSITM_SCREEN_WIDTH_BYTES], ax
  shl ax, 2         ; ax = line height
  mov [bp + DSITM_LINE_HEIGHT], ax
  mov word ptr [bp + DSITM_DRAW_CHAR], draw_character_c4cm
  mov word ptr [bp + DSITM_SET_COLOUR], dsitm_set_colour_c4cm
  push es
  pop ds            ; ds:si = pointer to the string to draw
  mov ax, 0xB800
  mov es, ax        ; es:di = top of screen
  and bl, 3         ; colour should be 0, 1, 2 or 3
  call draw_string_in_teletype_mode_gfx
  jmp dsitm_write_cursor_if_needed_and_end

dsitm_mode_above_5:
  cmp dl, 6
  ja dsitm_mode_above_6
  mov word ptr [bp + DSITM_CHAR_WIDTH], 1
  mov [bp + DSITM_SCREEN_WIDTH_BYTES], ax
  shl ax, 2         ; ax = line height
  mov [bp + DSITM_LINE_HEIGHT], ax
  mov word ptr [bp + DSITM_DRAW_CHAR], draw_character_c2cm
  mov word ptr [bp + DSITM_SET_COLOUR], dsitm_set_colour_c2cm
  push es
  pop ds            ; ds:si = pointer to the string to draw
  mov ax, 0xB800
  mov es, ax        ; es:di = top of screen
  and bl, 1         ; colour should be 0 or 1
  call draw_string_in_teletype_mode_gfx
  jmp dsitm_write_cursor_if_needed_and_end

dsitm_mode_above_6:
  cmp dl, 0x13
  jnz dsitm_planar_mode
  mov word ptr [bp + DSITM_CHAR_WIDTH], 8
  shl ax, 3         ; ax = screen width in bytes
  mov [bp + DSITM_SCREEN_WIDTH_BYTES], ax
  shl ax, 3         ; ax = line height
  mov [bp + DSITM_LINE_HEIGHT], ax
  mov word ptr [bp + DSITM_DRAW_CHAR], draw_character_256cm
  mov word ptr [bp + DSITM_SET_COLOUR], dsitm_set_colour_256cm
  push es
  pop ds            ; ds:si = pointer to the string to draw
  mov ax, 0xA000
  mov es, ax        ; es:di = top of screen
  call draw_string_in_teletype_mode_gfx
  jmp dsitm_write_cursor_if_needed_and_end

dsitm_planar_mode:
  mov word ptr [bp + DSITM_CHAR_WIDTH], 1
  mov [bp + DSITM_SCREEN_WIDTH_BYTES], ax
  mul word ptr [bp + DSITM_CHAR_HEIGHT]  ; ax = line height
  mov [bp + DSITM_LINE_HEIGHT], ax
  mov word ptr [bp + DSITM_DRAW_CHAR], draw_character_16cm
  mov word ptr [bp + DSITM_SET_COLOUR], dsitm_set_colour_16cm
  push es
  pop ds            ; ds:si = pointer to the string to draw
  mov ax, 0xA000
  mov es, ax        ; es:di = top of screen
  ;
  mov dx, 0x3CE
  mov ax, 0xFF08
  out dx, ax        ; bitmask 0xFF
  mov ah, bl
  mov al, 0
  out dx, ax        ; set/reset register filled with draw colour
  mov ax, 0x0003
  out dx, ax        ; no rotation, no logical operation
  mov ax, 0x0205
  out dx, ax        ; vga write mode 2
  mov al, [bp + DSITM_BACKGROUND_COLOUR]
  es: mov [DSITM_16CM_BACKGROUND_COLOUR], al
  es: mov al, [DSITM_16CM_BACKGROUND_COLOUR]  ; loading the latches
  mov ax, 0x0305
  out dx, ax        ; vga write mode 3
  call draw_string_in_teletype_mode_gfx

dsitm_write_cursor_if_needed_and_end:
  ; PRE:
  ;  bh = write mode
  test bh, 1        ; should cursor be updated?
  jz end_draw_string_in_teletype_mode
  xor ax, ax
  mov ds, ax        ; ds = 0x0000
  mov bl, [ACTIVE_PAGE]
  mov bh, 0
  shl bx, 1
  mov ax, [bp + DSITM_CURSOR]
  mov [bx + CURSOR_LOCATIONS], ax
end_draw_string_in_teletype_mode:
  add sp, DSITM_STACK_FRAME_SIZE
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

;
; Function: dsitm_set_colour_16cm
; Input:
;   AH = colour
; Output:
;   set/reset register filled with draw colour
; Affected registers:
;   DX
;
dsitm_set_colour_16cm:
  push ax
  ;
  mov dx, 0x3CE
  mov al, 0
  out dx, ax        ; set/reset register filled with draw colour
  ;
  pop ax
  ret

;
; Function: dsitm_set_colour_256cm
; Input:
;   AH = colour
; Output:
;   BL = colour
; Affected registers:
;   -
;
dsitm_set_colour_256cm:
  mov bl, ah
  ret

;
; Function: dsitm_set_colour_c2cm
; Input:
;   AH = colour
; Output:
;   BL = colour
; Affected registers:
;   -
;
dsitm_set_colour_c2cm:
  mov bl, ah
  and bl, 1
  ret

;
; Function: dsitm_set_colour_c4cm
; Input:
;   AH = colour
; Output:
;   BL = colour
; Affected registers:
;   -
;
dsitm_set_colour_c4cm:
  mov bl, ah
  and bl, 3
  ret

;
; Function: draw_string_in_teletype_mode_gfx
; Input:
;   BH = write mode:
;        0: attribute/colour in BL, string only contains characters
;           cursor not updated
;        1: attribute/colour in BL, string only contains characters
;           cursor updated
;        2: string contains characters and attributes in turn
;           cursor not updated
;        3: string contains characters and attributes in turn
;           cursor updated
;   BL = attribute/colour (writing modes 0 and 1)
;   CX = length of the string
;   DS:SI = pointer to the string to draw
;   ES:DI = top of screen
;
;   SS:BP+DSITM_CURSOR = cursor's x-coordinate
;   SS:BP+DSITM_CURSOR+1 = cursor's y-coordinate
;   SS:BP+DSITM_SCREEN_WIDTH_CHARS = screen width in characters
;   SS:BP+DSITM_CHAR_WIDTH = character width in bytes
;   SS:BP+DSITM_SCREEN_WIDTH_BYTES = screen width in bytes
;   SS:BP+DSITM_LINE_HEIGHT = line height = character height * screen width in bytes
;   SS:BP+DSITM_MAX_Y = screen height in text rows minus one
;   SS:BP+DSITM_BACKGROUND_COLOUR = background colour/attribute
;
;   SS:BP+DSITM_CHAR_HEIGHT = character height
;   SS:BP+DSITM_DRAW_CHAR = offset of a function that draws a single character
;   SS:BP+DSITM_SET_COLOUR = offset of a function that sets the foreground colour
;   SS:BP+DSITM_FONT_TABLE = pointer to a font table
; Output:
;   -
; Affected registers:
;   AX, CX, DX, BL
;
draw_string_in_teletype_mode_gfx:
  call dsitm_calc_string_destination_address
  test bh, 2        ; what data is in the string?
  jz dsitmg_chars_only
  call draw_string_in_teletype_mode_gfx_chars_and_attrs
  ret
dsitmg_chars_only:
  call draw_string_in_teletype_mode_gfx_chars_only
  ret

;
; Function: draw_string_in_teletype_mode_text
; Input:
;   BH = write mode:
;        0: attribute/colour in BL, string only contains characters
;           cursor not updated
;        1: attribute/colour in BL, string only contains characters
;           cursor updated
;        2: string contains characters and attributes in turn
;           cursor not updated
;        3: string contains characters and attributes in turn
;           cursor updated
;   BL = attribute/colour (writing modes 0 and 1)
;   CX = length of the string
;   DS:SI = pointer to the string to draw
;   ES:DI = top of screen
;
;   SS:BP+DSITM_CURSOR = cursor's x-coordinate
;   SS:BP+DSITM_CURSOR+1 = cursor's y-coordinate
;   SS:BP+DSITM_SCREEN_WIDTH_CHARS = screen width in characters
;   SS:BP+DSITM_CHAR_WIDTH = character width in bytes
;   SS:BP+DSITM_SCREEN_WIDTH_BYTES = screen width in bytes
;   SS:BP+DSITM_LINE_HEIGHT = line height = character height * screen width in bytes
;   SS:BP+DSITM_MAX_Y = screen height in text rows minus one
;   SS:BP+DSITM_BACKGROUND_COLOUR = background colour/attribute
; Output:
;   -
; Affected registers:
;   AX, DX
;
draw_string_in_teletype_mode_text:
  call dsitm_calc_string_destination_address
  test bh, 2        ; what data is in the string?
  jz dsitmt_chars_only
  call draw_string_in_teletype_mode_text_chars_and_attrs
  ret
dsitmt_chars_only:
  mov ah, bl        ; ah = attribute
  call draw_string_in_teletype_mode_text_chars_only
  ret

;
; Function: dsitm_calc_string_destination_address
; Input:
;   ES:DI = top of screen
; Output:
;   ES:DI = position where to draw the string
; Affected registers:
;   AX, DX
;
dsitm_calc_string_destination_address:
  mov dx, [bp + DSITM_CURSOR]
  mov al, dl        ; al = cursor's x-coordinate
  mul byte ptr [bp + DSITM_CHAR_WIDTH]
  add di, ax
  mov ax, dx
  shr ax, 8         ; ax = cursor's y-coordinate
  mul word ptr [bp + DSITM_LINE_HEIGHT]
  add di, ax        ; es:di = position where to draw the string
  ret

;
; Function: draw_string_in_teletype_mode_gfx_chars_and_attrs
; Input:
;   CX = length of the string
;   DS:SI = pointer to the string to draw
;   ES:DI = position where to draw the string
;
;   latches should be filled with background colour
;   write mode 3
; Output:
;   -
; Affected registers:
;   AX, CX, DX, BL
;
draw_string_in_teletype_mode_gfx_chars_and_attrs:
  lodsw             ; load a character in al and colour in ah
  ;
  cmp al, 0x0D
  jnz dsitmgcaa_check_for_line_feed
  call cursor_carriage_return
  jmp dsitmgcaa_continue_loop
dsitmgcaa_check_for_line_feed:
  cmp al, 0x0A
  jnz dsitmgcaa_check_for_bell
  call cursor_line_feed
  jmp dsitmgcaa_continue_loop
dsitmgcaa_check_for_bell:
  cmp al, 0x07
  jz dsitmgcaa_continue_loop
  ;
  cmp al, 0x08
  jnz dsitmgcaa_other_character
  call cursor_decrease
  jc dsitmgcaa_continue_loop
  mov al, 0x20
  call [bp + DSITM_DRAW_CHAR]
  jmp dsitmgcaa_continue_loop
dsitmgcaa_other_character:
  call [bp + DSITM_SET_COLOUR]
  call [bp + DSITM_DRAW_CHAR]
  call cursor_increase
dsitmgcaa_continue_loop:
  loop draw_string_in_teletype_mode_gfx_chars_and_attrs
  ret

;
; Function: draw_string_in_teletype_mode_gfx_chars_only
; Input:
;   BL = draw colour
;   CX = length of the string
;   DS:SI = pointer to the string to draw
;   ES:DI = position where to draw the string
;
;   set/reset register filled with draw colour
;   latches should be filled with background colour
;   write mode 3
; Output:
;   -
; Affected registers:
;   AX, CX, DX
;
draw_string_in_teletype_mode_gfx_chars_only:
  lodsb             ; load a character in al
  ;
  cmp al, 0x0D
  jnz dsitmgco_check_for_line_feed
  call cursor_carriage_return
  jmp dsitmgco_continue_loop
dsitmgco_check_for_line_feed:
  cmp al, 0x0A
  jnz dsitmgco_check_for_bell
  call cursor_line_feed
  jmp dsitmgco_continue_loop
dsitmgco_check_for_bell:
  cmp al, 0x07
  jz dsitmgco_continue_loop
  ;
  cmp al, 0x08
  jnz dsitmgco_other_character
  call cursor_decrease
  jc dsitmgco_continue_loop
  mov al, 0x20
  call [bp + DSITM_DRAW_CHAR]
  jmp dsitmgco_continue_loop
dsitmgco_other_character:
  call [bp + DSITM_DRAW_CHAR]
  call cursor_increase
dsitmgco_continue_loop:
  loop draw_string_in_teletype_mode_gfx_chars_only
  ret

;
; Function: draw_string_in_teletype_mode_text_chars_and_attrs
; Input:
;   CX = length of the string
;   DS:SI = pointer to the string to draw
;   ES:DI = position where to draw the string
; Output:
;   -
; Affected registers:
;   AX, CX
;
draw_string_in_teletype_mode_text_chars_and_attrs:
  lodsw             ; load a character in al and attribute in ah
  ;
  cmp al, 0x0D
  jnz dsitmtcaa_check_for_line_feed
  call cursor_carriage_return
  jmp dsitmtcaa_continue_loop
dsitmtcaa_check_for_line_feed:
  cmp al, 0x0A
  jnz dsitmtcaa_check_for_bell
  call cursor_line_feed
  jmp dsitmtcaa_continue_loop
dsitmtcaa_check_for_bell:
  cmp al, 0x07
  jz dsitmtcaa_continue_loop
  ;
  cmp al, 0x08
  jnz dsitmtcaa_other_character
  call cursor_decrease
  jc dsitmtcaa_continue_loop
  mov al, 0x20
  es: mov [di], ax
  jmp dsitmtcaa_continue_loop
dsitmtcaa_other_character:
  es: mov [di], ax
  call cursor_increase
dsitmtcaa_continue_loop:
  loop draw_string_in_teletype_mode_text_chars_and_attrs
  ret

;
; Function: draw_string_in_teletype_mode_text_chars_only
; Input:
;   AH = attribute
;   CX = length of the string
;   DS:SI = pointer to the string to draw
;   ES:DI = position where to draw the string
; Output:
;   -
; Affected registers:
;   AX, CX
;
draw_string_in_teletype_mode_text_chars_only:
  lodsb             ; load a character in al
  ;
  cmp al, 0x0D
  jnz dsitmtco_check_for_line_feed
  call cursor_carriage_return
  jmp dsitmtco_continue_loop
dsitmtco_check_for_line_feed:
  cmp al, 0x0A
  jnz dsitmtco_check_for_bell
  call cursor_line_feed
  jmp dsitmtco_continue_loop
dsitmtco_check_for_bell:
  cmp al, 0x07
  jz dsitmtco_continue_loop
  ;
  cmp al, 0x08
  jnz dsitmtco_other_character
  call cursor_decrease
  jc dsitmtco_continue_loop
  mov al, 0x20
  es: mov [di], ax
  jmp dsitmtco_continue_loop
dsitmtco_other_character:
  es: mov [di], ax
  call cursor_increase
dsitmtco_continue_loop:
  loop draw_string_in_teletype_mode_text_chars_only
  ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;
; Function: draw_character_16cm
; Input:
;   AL = character to draw
;   ES:DI = position where to write character
;   set/reset register filled with draw colour
;   latches should be filled with background colour
;   write mode 3
; Output:
;   -
; Affected registers:
;   AX, DX
;
draw_character_16cm:
  push cx
  push si
  push di
  push ds
  ;
  mov cx, [bp + DSITM_CHAR_HEIGHT]  ; cx = cl = character height
  mul cl            ; ax = offset in font table
  mov si, [bp + DSITM_FONT_TABLE]
  add si, ax
  mov ds, [bp + DSITM_FONT_TABLE + 2]  ; ds:si = position in font table
  mov dx, [bp + DSITM_SCREEN_WIDTH_BYTES]
  dec dx            ; dx = screen width - 1
dc16cm_loop:
  movsb
  add di, dx
  loop dc16cm_loop
  ;
  pop ds
  pop di
  pop si
  pop cx
  ret

;
; Function: draw_character_256cm
; Input:
;   AL = character to draw
;   BL = draw colour
;   ES:DI = position where to write character
; Output:
;   -
; Affected registers:
;   AX, DX
;
draw_character_256cm:
  push cx
  push si
  push di
  push ds
  ;
  mov cx, [bp + DSITM_CHAR_HEIGHT]  ; cx = cl = character height
  mul cl            ; ax = offset in font table
  mov si, [bp + DSITM_FONT_TABLE]
  add si, ax
  mov ds, [bp + DSITM_FONT_TABLE + 2]  ; ds:si = position in font table
  mov dx, [bp + DSITM_SCREEN_WIDTH_BYTES]
  sub dx, 8         ; dx = screen width in bytes - char width
dc256cm_put_char_loop:
  push cx
  mov ah, 0x80
  mov cx, 8
dc256cm_put_char_line_loop:
  test [si], ah
  jnz dc256cm_set_fgcolour
  mov al, [bp + DSITM_BACKGROUND_COLOUR]
  jmp dc256cm_put_pixel
dc256cm_set_fgcolour:
  mov al, bl
dc256cm_put_pixel:
  stosb
  shr ah, 1
  loop dc256cm_put_char_line_loop
  inc si
  add di, dx
  pop cx
  loop dc256cm_put_char_loop
  ;
  pop ds
  pop di
  pop si
  pop cx
  ret

;
; Function: draw_character_c2cm
; Input:
;   AL = character to draw
;   BL = draw colour (ignored)
;   ES:DI = position where to write character
; Output:
;   -
; Affected registers:
;   AX
;
draw_character_c2cm:
  push cx
  push si
  push di
  push ds
  ;
  mov cx, [bp + DSITM_CHAR_HEIGHT]  ; cx = cl = character height
  mul cl            ; ax = offset in font table
  mov si, [bp + DSITM_FONT_TABLE]
  add si, ax
  mov ds, [bp + DSITM_FONT_TABLE + 2]  ; ds:si = position in font table
  shr cx, 1         ; cx = character height / 2
dcc2cm_loop:
  movsb
  add di, 0x1FFF
  movsb
  sub di, 0x2001
  add di, [bp + DSITM_SCREEN_WIDTH_BYTES]
  loop dcc2cm_loop
  ;
  pop ds
  pop di
  pop si
  pop cx
  ret

;
; Function: draw_character_c4cm
; Input:
;   AL = character to draw
;   BL = draw colour
;   ES:DI = position where to write character
; Output:
;   -
; Affected registers:
;   AX, DX
;
draw_character_c4cm:
  push cx
  push si
  push di
  push ds
  ;
  mov cx, [bp + DSITM_CHAR_HEIGHT]  ; cx = cl = character height
  mul cl            ; ax = offset in font table
  mov si, [bp + DSITM_FONT_TABLE]
  add si, ax
  mov ds, [bp + DSITM_FONT_TABLE + 2]  ; ds:si = position in font table
  shr cx, 1         ; cx = character height / 2
dcc4cm_loop:
  call dcc4cm_create_character_line_pixels
  stosw
  add di, 0x1FFE
  call dcc4cm_create_character_line_pixels
  stosw
  sub di, 0x2002
  add di, [bp + DSITM_SCREEN_WIDTH_BYTES]
  loop dcc4cm_loop
  ;
  pop ds
  pop di
  pop si
  pop cx
  ret

;
; Function: dcc4cm_create_character_line_pixels
; Input:
;   BL = draw colour
;   DS:SI = position in font table
;   SS:BP+DSITM_BACKGROUND_COLOUR = background colour/attribute
; Output:
;   AX = character line pixels (AL = left four pixels, AH = right four pixels)
;   SI += 1
; Affected registers:
;   DX
;
dcc4cm_create_character_line_pixels:
  push cx
  ;
  mov cx, 8
  mov dl, 0x80
dcc4cm_cclp_loop:
  shl ax, 2
  test [si], dl
  jz dcc4cm_cclp_set_bgcolour
  or al, bl
  shr dl, 1
  loop dcc4cm_cclp_loop
  jmp dcc4cm_cclp_loop_end
dcc4cm_cclp_set_bgcolour:
  or al, [bp + DSITM_BACKGROUND_COLOUR]
  shr dl, 1
  loop dcc4cm_cclp_loop
dcc4cm_cclp_loop_end:
  xchg ah, al
  inc si
  ;
  pop cx
  ret

;
; The cursor_ functions all expect the following constants to be avaiable on
; the stack:
;   SS:BP+DSITM_SCREEN_WIDTH_CHARS = screen width in characters
;   SS:BP+DSITM_SCREEN_WIDTH_BYTES = screen width in bytes
;   SS:BP+DSITM_CHAR_WIDTH = character width in bytes
;   SS:BP+DSITM_LINE_HEIGHT = line height = character height * screen width in bytes
;   SS:BP+DSITM_MAX_Y = screen height in text rows minus one
;   SS:BP+DSITM_BACKGROUND_COLOUR = background colour/attribute
;

;
; Function: cursor_carriage_return
; Input:
;   SS:BP+DSITM_CURSOR = cursor's x-coordinate
;   SS:BP+DSITM_CURSOR+1 = cursor's y-coordinate
;   DI = corresponding position in video memory
; Output:
;   SS:BP+DSITM_CURSOR = updated cursor's x-coordinate
;   SS:BP+DSITM_CURSOR+1 = updated cursor's y-coordinate
;   DI = corresponding position in video memory
; Affected registers:
;   -
;
cursor_carriage_return:
  push ax
  ;
  mov al, [bp + DSITM_CURSOR]
  mov byte ptr [bp + DSITM_CURSOR], 0
  mul byte ptr [bp + DSITM_CHAR_WIDTH]
  sub di, ax
  ;
  pop ax
  ret

;
; Function: cursor_decrease
; Input:
;   SS:BP+DSITM_CURSOR = cursor's x-coordinate
;   SS:BP+DSITM_CURSOR+1 = cursor's y-coordinate
;   DI = corresponding position in video memory
; Output:
;   SS:BP+DSITM_CURSOR = updated cursor's x-coordinate
;   SS:BP+DSITM_CURSOR+1 = updated cursor's y-coordinate
;   DI = corresponding position in video memory
;   Carry = 0: when the operation succeeded
;           1: when nothing happened, because the cursor's coordinates were (0, 0)
; Affected registers:
;   -
;
cursor_decrease:
  push bx
  mov bx, [bp + DSITM_CURSOR]  ; bl = cursor's x-coordinate, bh = cursor's y-coordinate
  ;
  cmp bx, 1
  jc end_cursor_decrease
  ;
  cmp bl, 0
  jnz cd_move_cursor_one_position_back
  sub di, [bp + DSITM_LINE_HEIGHT]
  add di, [bp + DSITM_SCREEN_WIDTH_BYTES]
  mov bl, [bp + DSITM_SCREEN_WIDTH_CHARS]
  dec bh
cd_move_cursor_one_position_back:
  sub di, [bp + DSITM_CHAR_WIDTH]  ; this operation should clear the carry flag!
  dec bl
end_cursor_decrease:
  mov [bp + DSITM_CURSOR], bx
  pop bx
  ret

;
; Function: cursor_increase
; Input:
;   SS:BP+DSITM_CURSOR = cursor's x-coordinate
;   SS:BP+DSITM_CURSOR+1 = cursor's y-coordinate
;   DI = corresponding position in video memory
; Output:
;   SS:BP+DSITM_CURSOR = updated cursor's x-coordinate
;   SS:BP+DSITM_CURSOR+1 = updated cursor's y-coordinate
;   DI = corresponding position in video memory
; Affected registers:
;   -
;
cursor_increase:
  push bx
  mov bx, [bp + DSITM_CURSOR]  ; bl = cursor's x-coordinate, bh = cursor's y-coordinate
  ;
  add di, [bp + DSITM_CHAR_WIDTH]
  inc bl            ; increase cursor's x-coordinate
  cmp bl, [bp + DSITM_SCREEN_WIDTH_CHARS]
  jb end_cursor_increase
  ;
  sub di, [bp + DSITM_SCREEN_WIDTH_BYTES]
  mov bl, 0
  cmp bh, [DSITM_MAX_Y]
  jnb ci_scroll_needed
  ;
  add di, [bp + DSITM_LINE_HEIGHT]
  inc bh
  jmp end_cursor_increase
ci_scroll_needed:
  call cursor_scroll_up
end_cursor_increase:
  mov [bp + DSITM_CURSOR], bx
  pop bx
  ret

;
; Function: cursor_line_feed
; Input:
;   SS:BP+DSITM_CURSOR = cursor's x-coordinate
;   SS:BP+DSITM_CURSOR+1 = cursor's y-coordinate
;   DI = corresponding position in video memory
; Output:
;   SS:BP+DSITM_CURSOR = updated cursor's x-coordinate
;   SS:BP+DSITM_CURSOR+1 = updated cursor's y-coordinate
;   DI = corresponding position in video memory
; Affected registers:
;   -
;
cursor_line_feed:
  push bx
  ;
  mov bx, [bp + DSITM_CURSOR]  ; bl = cursor's x-coordinate, bh = cursor's y-coordinate
  cmp bh, [DSITM_MAX_Y]
  jb clf_increase_y_coordinate
  call cursor_scroll_up
  jmp end_cursor_line_feed
clf_increase_y_coordinate:
  add di, [bp + DSITM_LINE_HEIGHT]
  inc byte ptr [bp + DSITM_CURSOR + 1]
end_cursor_line_feed:
  pop bx
  ret

;
; Function: cursor_scroll_up
; Input:
;   -
; Output:
;   -
; Affected registers:
;   -
;
cursor_scroll_up:
  push ax
  push bx
  push cx
  push dx
  mov al, 1
  mov bh, [bp + DSITM_BACKGROUND_COLOUR]
  xor cx, cx
  mov dx, 0xFFFF
  pushf
  push cs
  call scroll_window_up
  pop dx
  pop cx
  pop bx
  pop ax
  ret
