;
; High speed graphics routines for Plane Jump.
; TODO: UPDATE THIS FILE TO WORK WITH CE C TOOLCHAIN v12.1
;


assume adl=1

;an array of structs 8 bytes wide, containing 32+240+32 entries
;+0 ypos, +1 sx, +4 w1, +5 w2, +6 w3, +7 w4 
;array of bytes, bits representing the encoded track: xxxx_1234. 32 entries
extern _translate
;do: ld iy,0 \ add iy,sp to set argument stack
;iy+3=arg0, iy+6=arg1, iy+9=arg2 ...
extern _track

OTHER_COLOR EQU 243
DRAW_BUFFER EQU 0E30014h
LCD_PALETTE_MEMORY EQU 0E30200h		;RGB1555 palette, 512 bytes
CURSOR_IMAGE_MEMORY EQU 0E30800h	;1KB high speed memory

;-----------------------------------------------------------------------------
; void drawGameField(uint8_t tile_offset,uint8_t pixel_offset,int x_offset);
; This function draws the scrolling game field based the contents of the `track`
; array (local) and the `translate` translation table (global).
; This is a high speed routine that copies itself to high speed memory.

section .text

cur_track_block EQU 5  ;... variable this overwrites isn't being used again

TRANSLATE_STRIDE EQU 8
tr_offset EQU 3
px_offset EQU 6
x_offset  EQU 9

public _drawGameField
_drawGameField:
	;ret	;DEBUG: App is crashing. Eliminating variables.
	ld  de,CURSOR_IMAGE_MEMORY
	push de
		ld  hl,dgf_start
		ld  bc,dgf_end-dgf_start
		ldir
	pop hl
	jp  (hl)
dgf_start:
	di
	ld  iy,0
	add iy,sp
	push ix
		;preconditioning
		ld  bc,(iy+x_offset)
		ld  hl,(DRAW_BUFFER)
		add hl,bc     ;add start address to buffer
		ld  (dgf_loadoffset),hl
		;start address on translation
		ld  a,(iy+px_offset)
		ld  b,a
		ld  c,TRANSLATE_STRIDE
		mlt bc
		ld  ix,_translate
		add ix,bc
		;get starting address on track array
		ld  c,(iy+tr_offset)
		ld  b,0
		ld  hl,_track
		add hl,bc
		ex de,hl
		;init values for main loop
		ld  b,a
		inc b
		neg
		srl c
		ld  c,a  ;px offset. some additional testing, yes?
		sbc a,a
		and a,OTHER_COLOR ;A is 0 or 244
		ld  L,a
		ld  a,b
		cp  33
		ld  a,L
		jr  nc,$+4
		xor a,OTHER_COLOR  ;there has to be a better way to suppress the flashing.
		ld  (dgf_color_ins),a
		ld  a,255
		ld  (dgf_prevscanline),a
		
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
		ld  l,(ix+0)      ;ypos
		inc l
		jr  z,dgf_skipscanline
		dec l
		;The section below solves a line skipping problem
dgf_prevscanline = ($-dgf_start+1)+CURSOR_IMAGE_MEMORY
		ld  a,0
		cp  l
		jr  z,dgf_skipscanline   ;if repeating same line, skip. for performance
		inc a    ;If L==(A+1), is on track
		jr  z,dgf_skip_repro
		add a,7
		cp  a,L    ;but if A is still greater than L, interpolate. Else, cancel.
		jr  c,dgf_skip_repro
		sub a,7
		ld  L,a
		lea ix,ix-8
		inc b
		inc c
dgf_skip_repro:
		ld  a,L
		ld  (dgf_prevscanline),a
		;end section
		ld  h,160   ;half of screen width (320)
		mlt hl
		add hl,hl  ;finish multiply by 320
		ld  a,(de)
		ex  af,af'
		push bc
			push de
dgf_loadoffset = ($-dgf_start+1)+CURSOR_IMAGE_MEMORY
				ld  bc,0
				add hl,bc
				ld  bc,(ix+1)  ;
				add hl,bc     ;add start x. BCU and B known zeroes here.
				lea ix,ix+4
				push hl
				pop de
				inc de        ;precompute HL+1 and keep tracking it
dgf_color_ins = ($-dgf_start+1)+CURSOR_IMAGE_MEMORY
				ld  a,0
				ld  iyh,4
dgf_drawloop:	
				ld  c,(ix+0)
				ex  af,af'
				rrca
				jr  c,dgf_drawscan
dgf_skipscan:	ex  af,af'
				add hl,bc
				ex  de,hl
				add hl,bc
				ex  de,hl
				jr  dgf_finish
dgf_drawscan:	ex  af,af'
				dec c
				ld  (hl),a
				ldir
				inc de
				inc hl
dgf_finish:		
				xor a,OTHER_COLOR
				inc ix
				dec iyh
				jr  nz,dgf_drawloop
				lea ix,ix-8
			pop de
		pop bc
dgf_skipscanline:
		dec c
		jr  z,dgf_endnow
		lea ix,ix+8
dgf_godjnznow:
		djnz dgf_mainloop
		ld  b,65  ;Why does this number have to be twice as large as max px_offset?
		inc de
		ld  a,(dgf_color_ins)
		xor a,OTHER_COLOR
		ld  (dgf_color_ins),a
		jr  dgf_godjnznow
dgf_endnow:
	pop ix
	ei
	ret
dgf_end:
	

;-----------------------------------------------------------------------------
; void drawBG(void);
; Constructs the background renderer in high speed memory, then runs it.
; The background is a simple gradient of increasing palette indices.
; The actual animation is done by modifying the palette entries.



section .text
require hue_to_1555_internal
public _drawBG
_drawBG:
	;ret  ;DEBUG: App is crashing. Eliminating variables.
	di
	; The renderer is constructed in three parts.
	; The first part loads DE with the value to be pushed.
	ld  de,CURSOR_IMAGE_MEMORY
	ld  hl,drawbg_seg1_start
	ld  bc,drawbg_seg1_end-drawbg_seg1_start
	ldir
	; The second part writes 106 instances of PUSH DE.
	; Running that would write 318 bytes which is almost one row on the screen.
	push hl		;preserve read address for third part
		push de
		pop hl
		inc de
		ld  bc,105
		ld  (hl),0D5h  ;PUSH DE
		ldir
	pop hl
	; The third part finishes the row.
	;copy in the h1555 routine as well.
	ld  bc,hto1555_seg1_end-drawbg_seg2_start
	ldir
	; Select the base color to be used for the background gradient
	;
drawBG_circularHueSMC = $+1
	ld  a,0
	inc a
	ld	(drawBG_circularHueSMC),a
	ld  c,a
	call hue_to_1555_internal
	; Run palette filler routine, accepting the result
	; of the hue to 1555 conversion in HL
	ld a,MB      ;save MBASE
	push af
		ld a,0E3h
		ld MB,a
		jp.sis hto1555_ramseg	;in: HL=1555 color
drawBG_exitPaletteFiller:
	pop af
	ld MB,a
	; Finish setting up pointers and registers for the renderer.
	; Then run it. NOTE: This is a tail call optimization.
	ld  iy,0
	add iy,sp	;Preserve current stack frame (and use it)
	ld  hl,(DRAW_BUFFER)
	ld  de,320*240
	add hl,de
	ld  sp,hl
	ld  b,240
	jp  CURSOR_IMAGE_MEMORY
	

;================================
;================================
;START COMBINED FAST RAM SEGMENTS
	
;segment 1: loads DE with value to be pushing. Use IY-6 as scratch
drawbg_seg1_start:
	ld  (iy-6+2),b ;3
	ld  de,(iy-6)  ;3: get A into DEU
	ld  e,b        ;1
	ld  d,b        ;1 = 8
drawbg_seg1_end:

;
; Between these two segments is 106 instances of PUSH DE.
;

;segment 2: Finishes the row by effectively writing 2 more bytes to the row.
drawbg_seg2_start:
	inc sp        ;1
	push de       ;1 = 2
	db 10h,-(8+2+106+2)		;Manually construct DJNZ instruction.
	ld  sp,iy
	ei
	ret
drawbg_seg2_end:

;segment 2.5: Constructs the static background color gradient through the palette.
;input: HL=1555 color to write to palette.
;output: Palette now has a color gradient.
;NOTE: The actual animation is carried in the SMC portion of
;hue_to_1555_internal, which is called first by this routine's caller.
hto1555_seg1_start:
assume adl=0
hto1555_ramseg = (hto1555_seg1_start-drawbg_seg1_start+106)+0800h  ;sis jump
	ld  b,30		;Loop counter. 8*30 = 240 (number of rows)
	ld  sp,0200h+01E2h  ;16 bit address of LCD palette memory, at 241st element.
	ld  de,32    ;const val for green channel increment
hto1555_reloc_loop:
	ld  a,h    	;get red
	and 124   	;isolate red
	cp  a,124
	jr  z,hto1555_noinc_red
	add a,4
hto1555_noinc_red:
	xor a,h
	and 252
	xor a,h
	ld  h,a
	
	push hl		;write 3 palette entries to the palette.
	push hl
	push hl
	
	ld  a,L    	;get blue value, disregard green
	and a,31   	;isolate blue
	cp  a,31
	jr  z,hto1555_noinc_blue
	inc a
hto1555_noinc_blue:
	xor a,L     	;begin combine low byte with (possibly) new blue
	and a,31    	;clear green bits so next xor will write them, leaving blue
	xor a,L
	ld  L,a
	
	push hl		;write 2 palette entries to the palette.
	push hl
	
	xor a,h     ;begin combine low with high to isolate all green bits
	and a,224   ;keep high bits of L
	xor a,h     ;to write out all green bits on lower (from high)
	and a,227   ;to remove all non-green bits
	cp  a,227
	jr  z,hto1555_noinc_green
	add hl,de
hto1555_noinc_green:
	push hl		;write the final 3 entries to the palette,
	push hl		;making a total of 8 entries. Loops 30 times. 8*30=240.
	push hl
	djnz hto1555_reloc_loop
	jp.lil drawBG_exitPaletteFiller
assume adl=1
hto1555_seg1_end:

;END COMBINED FAST RAM SEGMENTS
;==============================
;==============================


;-----------------------------------------------------------------------------
; uint16_t hue_to_1555RGB(uint8_t hue);
; Converts a hue value (0-255) to a 1555 RGB value. The conversion is done in a
; way that produces a smooth gradient when the hue is incremented, and the 
; output is suitable for direct use in the RGB1555 palette.

section .text
require hto1555_single
private hue_to_1555_internal
public _hue_to_1555RGB
_hue_to_1555RGB:
	di		;1555: IRRR_RRGG:GGGB_BBBB
	pop hl   
	pop bc	;C = hue (0-255)
	push bc
	push hl
	ld a,c
hue_to_1555_internal:
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
	ret

;-----------------------------------------------------------------------------
; Internal routine. Do not expose to the C runtime. Called by hue_to_1555RGB.

;in:  A= HSV hue channel (0-255)
;out: A= RGB green chanel
;destroys: AF, DE.
;NOTES: To get RGB red channel, add 171 to input hue. For blue, add 85.

;Computing the gradients is done in 8.8 fixed point math
;across a maybe-hexagonal color wheel.
;NOTE: 1524 = (65536/(256/6)), used as a const for scaling.
;  0- 43: N*1524.        Rising edge occupies 1/6 of the color wheel
; 44-127: 65535.         Max value. Occupies next 2/6 of the color wheel.
;128-171: -(N-171)*1524. Falling edge occupies next 1/6 of the color wheel.
;172-255: 0.             Min value. Occupies last 2/6 of the color wheel.

section .text
private hto1555_single
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
	