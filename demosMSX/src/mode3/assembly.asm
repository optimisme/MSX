;──────────────────────────────────────────────────────────────
; void draw_pixel_to_buffer_asm(uint8_t x, uint8_t y, uint8_t col)
; Convec. z88dk "ccall": [ret][col][y][x]
;──────────────────────────────────────────────────────────────
 EXTERN _nt_buffer
 EXTERN _row_base
 EXTERN _mask_tbl
 EXTERN _set_tbl
 PUBLIC _draw_pixel_to_buffer_asm

_draw_pixel_to_buffer_asm:
 pop bc ; ret
 pop de ; E = colour
 pop hl ; L = y
 pop ix ; IXL = x
 push bc ; restore ret
 ld b, e ; B = colour

; --- clip -----------------------------------------------------
 ld a, ixl
 cp 64
 ret nc ; x ≥ 64
 ld a, l
 cp 48
 ret nc ; y ≥ 48

; --- quadrant q = ((y&1)<<1)|(x&1) ----------------------------
 ld a, l
 and 1
 add a, a ; (y&1)<<1
 ld c, a
 ld a, ixl
 and 1
 or c
 ld c, a ; C = q (0..3)

; --- idx = row_base[y] + (x>>1) -------------------------------
; row_base is uint16_t array, so we need y*2 for byte offset
 ld a, l
 add a, a ; y * 2 (for uint16_t array)
 ld e, a
 ld d, 0
 ld hl, _row_base
 add hl, de ; HL -> &row_base[y]
 ld e, (hl)
 inc hl
 ld d, (hl) ; DE = row_base[y] (16-bit value)
 
 ld a, ixl
 srl a ; x >> 1
 ld l, a
 ld h, 0
 add hl, de ; HL = idx
 ld de, _nt_buffer
 add hl, de ; HL -> nt_buffer[idx]

; --- old byte -------------------------------------------------
 ld a, (hl)
 push af ; save old
 push hl ; save pointer

; --- mask = mask_tbl[q] ---------------------------------------
 ld a, c
 ld e, a
 ld d, 0
 ld hl, _mask_tbl
 add hl, de
 ld d, (hl) ; D = mask

; --- set = set_tbl[q][col] -----------------------------------
; set_tbl is [4][4] array, so offset = q*4 + col
 ld a, c
 add a, a
 add a, a ; q * 4
 add a, b ; + colour
 ld e, a
 ld d, 0
 ld hl, _set_tbl
 add hl, de
 ld e, (hl) ; E = set

; --- compose & store -----------------------------------------
 pop hl ; pointer
 pop af ; A = old
 and d ; old & mask
 or e ; | set
 ld (hl), a
 ret