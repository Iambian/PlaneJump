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
;Note: 0E30200h is the start of the 512 byte color palette. (RGB 1555)
;NOte: 0E30800h is the start of CursorImage (1KB of high speed memory)

prev_scanline   EQU 4  ;re-use argument space for temp memory, as the ...
cur_track_block EQU 5  ;... variable this overwrites isn't being used again

tr_offset EQU 3
px_offset EQU 6
x_offset  EQU 9

;Simple acceleration by copying to and running from high speed memory
_drawGameField:
	ld de,0E30800h
	push de
		ld hl,dgf_start
		ld bc,dgf_end-dgf_start
		ldir
	pop hl
	jp (hl)
dgf_start:
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
		ld (iy+cur_track_block),a
		push bc
			push de
				ld bc,(iy+x_offset)
				add hl,bc     ;apply x offset
				ld bc,(0E30014h)
				add hl,bc     ;add start address to buffer
				ld bc,(ix+3)  ;
				add hl,bc     ;add start x. BCU and B known zeroes here.
				lea ix,ix+6
				push hl
				pop de
				inc de        ;precompute HL+1 and keep tracking it
				ld a,4
dgf_drawloop:	
				ld c,(ix+0)
				rrc (iy+cur_track_block)
				jr c,dgf_drawscan
dgf_skipscan:	
				add hl,bc
				ex de,hl
				add hl,bc
				ex de,hl
				jr dgf_finish
dgf_drawscan:	
				dec c
				ld (hl),b
				ldir
				inc de
				inc hl
dgf_finish:		
				inc ix
				dec a
				jr nz,dgf_drawloop
				lea ix,ix-10
			pop de
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
dgf_end:
	
;accel notes.
;106 pushes plus 2 more bytes writes to a complete row
;
;less inner: modify value
;inner:      push value 106 times, increment sp, push once more
;
;
;
	
;Try to accelerate this later. Just accept the 30fps for now.
_drawBG:
	di
	ld de,0E30800h
	ld hl,drawbg_seg1_start
	ld bc,drawbg_seg1_end-drawbg_seg1_start
	ldir
	push hl
		push de
		pop hl
		inc de
		ld bc,105
		ld (hl),0D5h  ;PUSH DE
		ldir
	pop hl  ;points to next step: drawbg_seg2_start
	ld bc,drawbg_seg2_end-drawbg_seg2_start
	ldir
drawbg_routine_start:
	ld iy,0
	add iy,sp
	ld hl,(0E30014h)
	ld de,320*240
	add hl,de
	ld sp,hl
	ld a,240
	jp 0E30800h
	
;segment 1: loads DE with value to be pushing. Use IY-6 as scratch
drawbg_seg1_start:
	ld (iy-6+2),a ;3
	ld de,(iy-6)  ;3: get A into DEU
	ld e,a        ;1
	ld d,a        ;1
drawbg_seg1_end:
;Immediately after seg1 is 106 instances of PUSH DE
drawbg_seg2_start:
	inc sp        ;1
	push de       ;1
	dec a         ;1
	db 020h,-(11+106+2)  ;jr nz,[explicit offset] (+2 for itself)
	ld sp,iy
	ei
	ret
drawbg_seg2_end:
	
	
	
	