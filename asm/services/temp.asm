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
DSITM_FONT_TABLE = 16          ; 4 bytes: pointer to a font table
DSITM_STACK_FRAME_SIZE = 20

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
  ja end_draw_string_in_teletype_mode
  cmp dl, [SCREEN_WIDTH]
  jae end_draw_string_in_teletype_mode
  mov [bp + DSITM_CURSOR], dx
  mov di, [ACTIVE_PAGE_OFFSET]  ; di = offset of top of screen
  jcxz dsitm_write_cursor_if_needed_and_end
  ;
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
  shr word [bp + DSITM_CHAR_HEIGHT], 1  ; because of cga addressing
  mov word ptr [bp + DSITM_CHAR_WIDTH], 2
  shl ax, 1         ; ax = screen width in bytes
  mov [bp + DSITM_SCREEN_WIDTH_BYTES], ax
  shl ax, 2         ; ax = line height
  mov [bp + DSITM_LINE_HEIGHT], ax
  mov [bp + DSITM_DRAW_CHAR], TODO
  push es
  pop ds            ; ds:si = pointer to the string to draw
  mov ax, 0xB800
  mov es, ax        ; es:di = top of screen
  call draw_string_in_teletype_mode_gfx
  jmp dsitm_write_cursor_if_needed_and_end

dsitm_mode_above_5:
  cmp dl, 6
  ja dsitm_mode_above_6
  shr word [bp + DSITM_CHAR_HEIGHT], 1  ; because of cga addressing
  mov word ptr [bp + DSITM_CHAR_WIDTH], 1
  mov [bp + DSITM_SCREEN_WIDTH_BYTES], ax
  shl ax, 2         ; ax = line height
  mov [bp + DSITM_LINE_HEIGHT], ax
  mov [bp + DSITM_DRAW_CHAR], TODO
  push es
  pop ds            ; ds:si = pointer to the string to draw
  mov ax, 0xB800
  mov es, ax        ; es:di = top of screen
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
  mov [bp + DSITM_DRAW_CHAR], TODO
  push es
  pop ds            ; ds:si = pointer to the string to draw
  mov ax, 0xA000
  mov es, ax        ; es:di = top of screen
  call draw_string_in_teletype_mode_gfx
  jmp dsitm_write_cursor_if_needed_and_end

dsitm_planar_mode:
  mov word ptr [bp + DSITM_CHAR_WIDTH], 1
  mov [bp + DSITM_SCREEN_WIDTH_BYTES], ax
  mul [bp + DSITM_CHAR_HEIGHT]  ; ax = line height
  mov [bp + DSITM_LINE_HEIGHT], ax
  mov [bp + DSITM_DRAW_CHAR], TODO
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
;   SS:BP+DSITM_FONT_TABLE = pointer to a font table
; Output:
;   -
; Affected registers:
;   AX, DX
;
draw_string_in_teletype_mode_gfx:
  push bx
  ;
  call dsitm_calc_string_destination_address
  test bh, 2        ; what data is in the string?
  jz dsitmg_chars_only
  call ...
  jmp end_draw_string_in_teletype_mode_gfx
dsitmg_chars_only:
  call ...
end_draw_string_in_teletype_mode_gfx:
  pop bx
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
  push bx
  ;
  call dsitm_calc_string_destination_address

  ;
  pop bx
  ret

;
; Function: dsitm_calc_string_destination_address
; Input:
;   ES:DI = top of screen
;
;   SS:BP+DSITM_CURSOR = cursor's x-coordinate
;   SS:BP+DSITM_CURSOR+1 = cursor's y-coordinate
;   SS:BP+DSITM_CHAR_WIDTH = character width in bytes
;   SS:BP+DSITM_LINE_HEIGHT = line height = character height * screen width in bytes
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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;
; Function: draw_character_16cm
; Input:
;   CX = character height
;   DX = screen width - 1
;   DS:SI = position in font table
;   ES:DI = position where to write character
;   set/reset register filled with draw colour
;   latches should be filled with background colour
;   write mode 3
; Output:
; Affected registers:
;   SI
;
draw_character_16cm:
  push cx
  push di
  ;
dc16cm_loop:
  movsb
  add di, dx
  loop dc16cm_loop
  ;
  pop di
  pop cx
  ret


;
; Function: cursor_carriage_return
; Input:
;   SS:BP+DSITM_CURSOR = cursor's x-coordinate
;   SS:BP+DSITM_CURSOR+1 = cursor's y-coordinate
;   DI = corresponding position in video memory
;   SS:BP+DSITM_CHAR_WIDTH = character width in bytes
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
;   SS:BP+DSITM_SCREEN_WIDTH_CHARS = screen width in characters
;   SS:BP+DSITM_CHAR_WIDTH = character width in bytes
;   SS:BP+DSITM_SCREEN_WIDTH_BYTES = screen width in bytes
;   SS:BP+DSITM_LINE_HEIGHT = line height = character height * screen width in bytes
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
;   SS:BP+DSITM_SCREEN_WIDTH_CHARS = screen width in characters
;   SS:BP+DSITM_CHAR_WIDTH = character width in bytes
;   SS:BP+DSITM_SCREEN_WIDTH_BYTES = screen width in bytes
;   SS:BP+DSITM_LINE_HEIGHT = line height = character height * screen width in bytes
;   SS:BP+DSITM_MAX_Y = screen height in text rows minus one
;   SS:BP+DSITM_BACKGROUND_COLOUR = background colour/attribute
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
;   SS:BP+DSITM_LINE_HEIGHT = line height = character height * screen width in bytes
;   SS:BP+DSITM_MAX_Y = screen height in text rows minus one
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
;   SS:BP+DSITM_BACKGROUND_COLOUR = background colour/attribute
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
