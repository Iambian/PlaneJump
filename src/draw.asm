.assume adl=1
XDEF _drawGameField
XDEF _drawBG

;an array of structs 10 bytes wide, containing 32+240+32 entries
;+0 ypos, +3 sx, +6 w1, +7 w2, +8 w3, +9 w4 
XREF _translate
;array of bytes, bits representing the encoded track: xxxx_1234. 32 entries
XREF _track

;do: ld iy,0 \ add iy,sp to set argument stack
;iy+3=arg0, iy+6=arg1, iy+9=arg2 ...
;
;Note: 0E30014h is a pointer for the buffer


prev_scanline EQU -12  ;whyyyyy???? why can't it be -3 or -6?

tr_offset EQU 3
px_offset EQU 6


_drawGameField:
	ld iy,0
	add iy,sp
	push ix
		;start address on translation
		ld a,(iy+px_offset)
		ld b,a
		ld c,10
		mlt bc
		ld ix,_translate
		add ix,bc
		;get starting address on track array
		ld c,(iy+tr_offset)
		ld b,0
		ld hl,_track
		add hl,bc
		ex de,hl
		;init values for main loop
		ld b,a
		neg
		;add a,240
		ld c,a
		ld (iy+prev_scanline),255
;draw 240 scanlines.
; on entry: B=rows of first tile row to draw, C=240
;           IX=&translate[t_offset], DE=&track[p_offset]
dgf_mainloop:
		;ypos set to 255 if draw coordinate is outside the screen.
		;since screen height is only 240. (probably?)Can't do -1 as it was 
		;defined as a uint8_t in the C program these values were genned in.
		ld hl,(ix+0)      ;ypos
		inc l
		jr z,dgf_skipscanline
		dec l
		;The section below solves a line skipping problem
		ld a,(iy+prev_scanline) ;if prev_scanline+2 == L, dec L and set
		cp l
		jr z,dgf_skipscanline   ;if repeating same line, skip. for performance
		inc a    ;If L==(A+1), is on track
		jr z,dgf_skip_repro
		add a,3
		cp l    ;but if A is still greater than L, interpolate. Else, cancel.
		jr c,dgf_skip_repro
		sub a,3
		ld l,a
		lea ix,ix-10
		inc b
		inc c
dgf_skip_repro:
		ld (iy+prev_scanline),l
		;end section
		ld h,160   ;half of screen width (320)
		mlt hl
		add hl,hl  ;finish multiply by 320
		ld a,(de)
		push bc
			ld bc,(ix+3)  ;startx
			add hl,bc     ;offset complete.
			ld bc,(0E30014h)
			add hl,bc     ;address complete
			ld bc,0       ;easy way to clear BCU
			;first row
			rra
			ld c,(ix+6)
			jr c,dgf_drawr1
			add hl,bc
			jr dgf_finishr1
dgf_drawr1:
			ld b,c
			ld c,0
dgf_loopr1:
			ld (hl),c
			inc hl
			djnz dgf_loopr1
dgf_finishr1:
			;second row
			rra
			ld c,(ix+7)
			jr c,dgf_drawr2
			add hl,bc   ;b will have been set to 0 regardless of prior path
			jr dgf_finishr2
dgf_drawr2:
			ld b,c
			ld c,0
dgf_loopr2:
			ld (hl),c
			inc hl
			djnz dgf_loopr2
dgf_finishr2:
			;third row
			rra
			ld c,(ix+8)
			jr c,dgf_drawr3
			add hl,bc
			jr dgf_finishr3
dgf_drawr3:
			ld b,c
			ld c,0
dgf_loopr3:
			ld (hl),c
			inc hl
			djnz dgf_loopr3
dgf_finishr3:
			;fourth (last) row
			rra
			ld c,(ix+9)
			jr c,dgf_drawr4
			add hl,bc
			jr dgf_finishr4
dgf_drawr4:
			ld b,c
			ld c,0
dgf_loopr4:
			ld (hl),c
			inc hl
			djnz dgf_loopr4
dgf_finishr4:
		pop bc
dgf_skipscanline:
		dec c
		jr z,dgf_endnow
		lea ix,ix+10
		djnz dgf_mainloop
		ld b,64  ;Why does this number have to be twice as large as max px_offset?
		inc de
		jr dgf_mainloop
dgf_endnow:
	pop ix
	ret
	
	
;Try to accelerate this later. Just accept the 30fps for now.
_drawBG:
	ld a,240
	ld hl,(0E30014h)
drawBG_loop:
	ld (hl),a
	ld bc,319
	push hl
	pop de
	inc de
	ldir
	dec a
	jr nz,drawBG_loop
	ret
	
	
	
	
	
	