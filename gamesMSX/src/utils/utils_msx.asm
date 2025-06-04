SECTION code_user

DATA_PORT EQU 0x98
CRTL_PORT EQU 0x99
        
PUBLIC _vdp_set_address
_vdp_set_address:
    ld a, l
    out (CRTL_PORT), a        
    ld a, h
    or 0x40              
    out (CRTL_PORT), a
    REPT 12
      nop
    ENDR
    ret

PUBLIC _vdp_write_byte
_vdp_write_byte:
    ld   c, DATA_PORT
    ld   a, l 
    out  (c), a
    ret

PUBLIC _vdp_read_byte
_vdp_read_byte:
    ld c, DATA_PORT
    in a, (c)
    ld l, a        
    ld h, 0        
    ret

PUBLIC _vdp_write_bytes
_vdp_write_bytes:
    pop  bc 
    pop  de
    pop  hl
    push bc
    ld   c, DATA_PORT

vwb_loop:            
    ld   a, (hl)         
    out  (c), a          
    inc  hl              
    dec  de              
    ld   a, d
    or   e
    jr   nz, vwb_loop
    ret 
    
PUBLIC _vdp_write_bytes_otir
_vdp_write_bytes_otir:
    pop  bc          ; ret
    pop  de          ; len  (DE)
    pop  hl          ; src  (HL)
    push bc
    ld   c, DATA_PORT

vwbf_loop256:
    ld   a, d
    or   e           ; len == 0?
    ret  z
    ld   b, 0        ; 256
    ld   a, d
    or   a
    jr   z, vwbf_last
    otir             ; 256-byte burst
    dec  d
    jp   vwbf_loop256

vwbf_last:
    ld   b, e        ; remainder (<256)
    otir
    ret

PUBLIC _vdp_blast_line
_vdp_blast_line:
    ld c, DATA_PORT           
    ld b, 32             
vbl_loop:
    ld a, (hl)           
    out (c), a           
    inc hl               
    dec b                
    jp nz, vbl_loop
    ret

PUBLIC _vdp_blast_tilemap
_vdp_blast_tilemap:
    ld c, DATA_PORT           
    ld de, 768           
vbt_loop:
    ld a, (hl)           
    out (c), a           
    inc hl               
    dec de               
    ld a, d
    or e                 
    jp nz, vbt_loop
    ret

PUBLIC _suspend_interrupts
_suspend_interrupts:
    di
    ret

PUBLIC _resume_interrupts
_resume_interrupts:
    ei
    ret

PUBLIC _do_nop
_do_nop:
    nop
    ret
