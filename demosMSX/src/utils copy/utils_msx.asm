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
    REPT 8
      nop
    ENDR
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
