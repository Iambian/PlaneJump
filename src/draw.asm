.assume adl=1
XDEF _drawGameField
XDEF _drawTrapezoid

;an array of structs 10 bytes wide, containing 32+240+32 entries
;+0 ypos, +3 sx, +6 w1, +7 w2, +8 w3, +9 w4 
XREF _translate
;array of bytes, bits representing the encoded track: xxxx_1234. 32 entries
XREF _track

;do: ld iy,0 \ add iy,sp to set argument stack
;iy+3=arg0, iy+6=arg1, iy+9=arg2 ...
;
;Note: 0E30014h is a pointer for the buffer


temp1 EQU -3
temp2 EQU -6
curbuf       EQU -9
curwidth	 EQU -12
curtilefield EQU -15


tr_offset EQU 3
px_offset EQU 6


_drawGameField:
	ld iy,0
	add iy,sp
	push ix
		;start address on translation
		ld b,(iy+px_offset)
		push bc
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
		pop af
		neg
		;add a,240
		ld c,a
		;ld c,240
		ld b,(iy+px_offset)  ;used to determine how many rows of top tile to draw
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
		ld h,160   ;half of screen width (320)
		mlt hl
		add hl,hl  ;finish multiply by 320
		ld a,(de)
		ld (iy+curtilefield),a
		push bc
			push de
				ld de,(ix+1)  ;startx
				add hl,de     ;get starting position on scanline
				ld bc,(0E30014h)
				add hl,bc
				ld bc,(ix+4)        ;width in B
				ld (iy+curwidth),bc ;store for later
				xor a
dgf_drawloop:	inc a
				push af
					ld e,(iy+curwidth)
					ld d,a
					mlt de              ;get offset error in D
					ld e,d
					ld d,0
					ld c,(iy+curwidth+1)
					ld b,a              ;
					ld a,c              ;save width for later
					add a,e             ;width plus curerr for current block
					mlt bc              ;get total offset in bc
					ex de,hl
					add hl,bc           ;find total true offset
					add hl,de           ;now have actual address. (keep DE)
					srl (iy+curtilefield)
					jr nc,dgf_skipfield
					ld b,a
					xor a
dgf_drawsubloop:	ld (hl),a
					inc hl
					djnz dgf_drawsubloop
dgf_skipfield:	pop af
				cp 4
				jr c,dgf_drawloop
				
				
				
				
				
				
				
				
				
				
				
			
			
			
			
			ld bc,(ix+3)  ;startx
			add hl,bc     ;offset complete.
			ld bc,(0E30014h)
			add hl,bc     ;address complete
			ld bc,0       ;easy way to clear BCU
			;first row
			rra
			ld c,(ix+6)
			jr nc,dgf_drawr1
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
			jr nc,dgf_drawr2
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
			jr nc,dgf_drawr3
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
			jr nc,dgf_drawr4
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
	
;attempt acceleration with this.

tx1 EQU 3
ty1 EQU 6
tx2 EQU 9
ty2 EQU 12
tw1 EQU 15
tw2 EQU 18
color EQU 21

invslope1 EQU -12
invslope2 EQU -15

tempcx1 EQU -20
tempcx2 EQU -25	


_drawTrapezoid:
	ld iy,0
	add iy,sp
	ld hl,(iy+ty2)
	ld de,(iy+ty1)
	or a
	sbc hl,de
	ret z           ;zero height object. not allowed.
	ld c,l
	ld hl,(iy+tx1)
	ld de,(iy+tx2)
	push hl
		push de
			call findinvslope
			ld (iy+invslope1),hl
		pop hl  ;tx2
		ld de,(iy+tw2)
		add hl,de
		ex de,hl
	pop hl
	ld a,c
	ld bc,(iy+tw1)
	add hl,bc
	push hl
		ld c,a
		call findinvslope
		ld (iy+invslope2),hl
;copy tx1 and tx1+tw1 to tempcx1 and tempcx2 as 24.8
		xor a
		ld hl,(iy+tx1)
		ld (iy+tempcx1+1),hl
		ld (iy+tempcx1+0),a  ;24.8, fractional part cleared.
	pop hl  ;tx1+tw1
	ld (iy+tempcx2+1),hl
	ld (iy+tempcx2+0),a
	ld a,c  ;loop counter to A
	;Find offset into buffer
	inc a   ;account for initial run
	jr dtpz_findnewoffset
dtpz_mainloop:
	;calc number of pixels to draw
	ex de,hl
	ld hl,(iy+tempcx2+1)
	ld bc,(iy+tempcx1+1)
	or a
	sbc hl,bc         ;HL=width-to-draw. Assumes less than 256
	ld b,L
	ex de,hl
	ld c,(iy+color)
dtpz_subloop_draw:
	ld (hl),c
	inc hl
	djnz dtpz_subloop_draw
	;find new tempcx1 and 2, then recalc HL
	ld hl,(iy+tempcx2+0)
	ld de,(iy+invslope2)
	add hl,de
	ld (iy+tempcx2+0),hl
	ld hl,(iy+tempcx1+0)
	ld de,(iy+invslope1)
	add hl,de
	ld (iy+tempcx1+0),hl
dtpz_findnewoffset:
	ld h,(iy+ty1)
	inc (iy+ty1)
	ld l,160
	mlt hl
	add hl,hl
	ld de,(iy+tempcx1+1)
	add hl,de
	ld de,(0E30014h)  ;Address of screen buffer
	add hl,de         ;Current address.
	dec a
	jr nz,dtpz_mainloop
	ret
	
	
;in: HL=x1, DE=x2, C=yheight
;out: HL= 8.8 quotient
findinvslope:
	or a
	sbc hl,de
	jr c,findinvnegslope
	ld h,l
	ld l,0
	jr div24_8
findinvnegslope:
	add hl,de   ;undo initial sbc
	ex de,hl
	or a
	sbc hl,de   ;make positive
	call div24_8
	ex de,hl
	or a
	sbc hl,hl
	sbc hl,de   ;make negative
	ret

;hl/c = hl. destroys a,bc
;can get away with code for 16/8 because ez80.
div24_8:
	xor a
	ld b,24
div24_8_start:
	add	hl,hl	; unroll 24 times
	rla			; ...
	cp	c		; ...
	jr	c,div24_8_skip
	sub	c		; ...
	inc	l		; ...
div24_8_skip:
	djnz div24_8_start
	ret


