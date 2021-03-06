.assume adl=1
XDEF _drawGameField
XDEF _drawBG
XDEF _hue_to_1555RGB
OTHER_COLOR EQU 252


TRANSLATE_STRIDE EQU 8
;an array of structs 8 bytes wide, containing 32+240+32 entries
;+0 ypos, +1 sx, +4 w1, +5 w2, +6 w3, +7 w4 
XREF _translate
;array of bytes, bits representing the encoded track: xxxx_1234. 32 entries
XREF _track

;do: ld iy,0 \ add iy,sp to set argument stack
;iy+3=arg0, iy+6=arg1, iy+9=arg2 ...
;
;Note: 0E30014h is a pointer for the buffer
;Note: 0E30200h is the start of the 512 byte color palette. (RGB 1555)
;NOte: 0E30800h is the start of CursorImage (1KB of high speed memory)

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
	di
	ld iy,0
	add iy,sp
	push ix
		;preconditioning
		ld bc,(iy+x_offset)
		ld hl,(0E30014h)
		add hl,bc     ;add start address to buffer
		ld (dgf_loadoffset),hl
		;start address on translation
		ld a,(iy+px_offset)
		ld b,a
		ld c,TRANSLATE_STRIDE
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
		inc b
		neg
		srl c
		ld c,a  ;px offset. some additional testing, yes?
		sbc a,a
		and a,OTHER_COLOR ;A is 0 or 252
		ld L,a
		ld a,b
		cp 33
		ld a,L
		jr nc,$+4
		xor a,OTHER_COLOR  ;there has to be a better way to suppress the flashing.
		ld (dgf_color_ins),a
		ld a,255
		ld (dgf_prevscanline),a
		
;		inc b
;		dec b
;		jr z,dgf_skipscanline
;draw 240 scanlines.
; on entry: B=rows of first tile row to draw, C=240
;           IX=&translate[t_offset], DE=&track[p_offset]
dgf_mainloop:
		;ypos set to 255 if draw coordinate is outside the screen.
		;since screen height is only 240. (probably?)Can't do -1 as it was 
		;defined as a uint8_t in the C program these values were genned in.
		ld l,(ix+0)      ;ypos
		inc l
		jr z,dgf_skipscanline
		dec l
		;The section below solves a line skipping problem
dgf_prevscanline EQU ($-dgf_start+1)+0E30800h
		ld a,0
		cp l
		jr z,dgf_skipscanline   ;if repeating same line, skip. for performance
		inc a    ;If L==(A+1), is on track
		jr z,dgf_skip_repro
		add a,7
		cp l    ;but if A is still greater than L, interpolate. Else, cancel.
		jr c,dgf_skip_repro
		sub a,7
		ld l,a
		lea ix,ix-8
		inc b
		inc c
dgf_skip_repro:
		ld a,l
		ld (dgf_prevscanline),a
		;end section
		ld h,160   ;half of screen width (320)
		mlt hl
		add hl,hl  ;finish multiply by 320
		ld a,(de)
		ex af,af'
		push bc
			push de
dgf_loadoffset EQU ($-dgf_start+1)+0E30800h
				ld bc,0
				add hl,bc
				ld bc,(ix+1)  ;
				add hl,bc     ;add start x. BCU and B known zeroes here.
				lea ix,ix+4
				push hl
				pop de
				inc de        ;precompute HL+1 and keep tracking it
dgf_color_ins EQU ($-dgf_start+1)+0E30800h
				ld a,0
				ld iyh,4
dgf_drawloop:	
				ld c,(ix+0)
				ex af,af'
				rrca
				jr c,dgf_drawscan
dgf_skipscan:	ex af,af'
				add hl,bc
				ex de,hl
				add hl,bc
				ex de,hl
				jr dgf_finish
dgf_drawscan:	ex af,af'
				dec c
				ld (hl),a
				ldir
				inc de
				inc hl
dgf_finish:		
				xor OTHER_COLOR
				inc ix
				dec iyh
				jr nz,dgf_drawloop
				lea ix,ix-8
			pop de
		pop bc
dgf_skipscanline:
		dec c
		jr z,dgf_endnow
		lea ix,ix+8
dgf_godjnznow:
		djnz dgf_mainloop
		ld b,65  ;Why does this number have to be twice as large as max px_offset?
		inc de
		ld a,(dgf_color_ins)
		xor OTHER_COLOR
		ld (dgf_color_ins),a
		jr dgf_godjnznow
dgf_endnow:
	pop ix
	ei
	ret
dgf_end:
	
	
	
	
	
	
	
	
	
	
	
;accel notes.
;106 pushes plus 2 more bytes writes to a complete row
;less inner: modify value
;inner:      push value 106 times, increment sp, push once more
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
	;copy in the h1555 routine as well.
	ld bc,hto1555_seg1_end-drawbg_seg2_start
	ldir
	call hue_to_1555_internal
	ld iy,0
	add iy,sp
	ld hl,(0E30014h)
	ld de,320*240
	add hl,de
	ld sp,hl
	ld b,240
	jp 0E30800h
	
	
	
;=============================================================================
;START COMBINED FAST RAM SEGMENTS
	
;segment 1: loads DE with value to be pushing. Use IY-6 as scratch
drawbg_seg1_start:
	ld (iy-6+2),b ;3
	ld de,(iy-6)  ;3: get A into DEU
	ld e,b        ;1
	ld d,b        ;1 = 8
drawbg_seg1_end:
;Immediately after seg1 is 106 instances of PUSH DE
drawbg_seg2_start:
	inc sp        ;1
	push de       ;1 = 2
	db 10h,-(8+2+106+2)
	ld sp,iy
	ei
	ret
drawbg_seg2_end:

hto1555_seg1_start:
.assume adl=0
hto1555_ramseg EQU (hto1555_seg1_start-drawbg_seg1_start+106)+0800h  ;sis jump
	ld b,30
	ld sp,0200h+01E2h  ;just after the 241st element
	ld de,32     ;to increment green
hto1555_reloc_loop:
	ld a,h    ;get red
	and 124   ;isolate red
	cp 124
	jr z,hto1555_noinc_red
	add a,4
hto1555_noinc_red:
	xor h
	and 252
	xor h
	ld h,a
	
	push hl
	push hl
	push hl
	
	ld a,L    ;get blue value, disregard green
	and 31    ;isolate blue
	cp 31
	jr z,hto1555_noinc_blue
	inc a
hto1555_noinc_blue:
	xor L     ;begin combine low byte with (possibly) new blue
	and 31    ;clear green bits so next xor will write them, leaving blue
	xor L
	ld L,a
	
	push hl
	push hl
	
	xor h     ;begin combine low with high to isolate all green bits
	and 224   ;keep high bits of L
	xor h     ;to write out all green bits on lower (from high)
	and 227   ;to remove all non-green bits
	cp 227
	jr z,hto1555_noinc_green
	add hl,de
hto1555_noinc_green:
	; push hl
	; push hl
	; push hl
	; push hl
	; push hl
	push hl
	push hl
	push hl
	djnz hto1555_reloc_loop
	jp.lil hto1555_continue
.assume adl=1
hto1555_seg1_end:

;END COMBINED FAST RAM SEGMENT
;=============================================================================


;Computing most of this in 8.8 fixed point, represented as an int.
;Note: 1524 comes from 65536/43, where 43 is roughly (256/6), or 1/6th of
;a complete rotation around our "special" color wheel.
;  0- 43: N*1524
; 44-127: 65535
;128-171: -(N-171)*1524
;172-255: 0
;
;1555: IRRR_RRGG:GGGB_BBBB
hue_to_1555_internal:
hto1555_smc EQU $+1
	ld a,0
	inc a
	ld (hto1555_smc),a
	ld c,a
	jr hto1555_internal_skip
_hue_to_1555RGB:
	di
	ld hl,hto1555_seg1_start
	ld de,0E30800h+(hto1555_seg1_start-drawbg_seg1_start)+106
	ld bc,hto1555_seg1_end-hto1555_seg1_start
	ldir
	;start calculating
	pop bc   ;C = hue (0-255)
	push bc
	ld a,c
hto1555_internal_skip:
	call hto1555_single  ;get G. bits in GGGG_G***. Moveto: GGG*_**GG
	and a,248    ;GGGG_G***
	rlca         ;GGGG_***G
	rlca         ;GGG*_**GG
	ld h,a       ;save raw to high (reprocess later to replace upper bits
	and a,224    ;GGG0_0000
	ld l,a       ;save processed to low
	ld a,c
	sub a,171    ;blue offset
	call hto1555_single ;get B. bits in BBBB_B***. Moveto: ***B_BBBB
	and a,248    ;BBBB_B***
	rrca         ;*BBB_BB**
	rrca         ;**BB_BBB*
	rrca         ;***B_BBBB
	or L
	ld L,a       ;GGGB_BBBB (low byte)
	ld a,c
	sub a,85     ;red offset
	call hto1555_single ;get R. bits in RRRR_R***. Moveto: 1RRR_RR**
	and a,248    ;RRRR_R000
	inc a        ;RRRR_R001
	rrca         ;1RRR_RR00
	xor h        ;1R^G
	and 252      ;####_##00
	xor h        ;1RRR_RRGG
	ld h,a       ;HL is now set up for stuffs.
	;get ready to write to the buffer thing.
	ld a,MB      ;save MBASE
	push af
		ld a,0E3h
		ld MB,a
		jp.sis hto1555_ramseg
hto1555_continue:
	pop af
	ld MB,a
	ret

;in: A=hue, out A= (H_GREEN).
;(To get red, pass in A = A-85. To get blue, pass in A = A-171
;destroys HL
hto1555_single:
	cp 44
	jr nc,hto1555s_skipasc
hto1555_slide:
	ld  e,a
	ld  d,a
	add a,a
	add a,a
	add a,e  ;high byte const of 1524 (5) as repeated adds
	ld  e,224 ;low byte const of 1524 (224)
	mlt de
	add a,d ;10b
	ret
hto1555s_skipasc:
	bit 7,a
	jr nz,hto1555s_skipmax
	ld a,255
	ret
hto1555s_skipmax:
	sub a,172
	jr nc,hto1555s_skiptomin
	cpl     ; -(N-171) but we can -(N-172)+1. I think?
	jr hto1555_slide
hto1555s_skiptomin:
	xor a
	ret
	
	
;Note: 0E30200h is the start of the 512 byte color palette. (RGB 1555)
;NOte: 0E30800h is the start of CursorImage (1KB of high speed memory)
