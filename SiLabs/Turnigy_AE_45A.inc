;**** **** **** **** ****
;
; BLHeli program for controlling brushless motors in helicopters
;
; Copyright 2011, 2012 Steffen Skaug
; This program is distributed under the terms of the GNU General Public License
;
; This file is part of BLHeli.
;
; BLHeli is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
;
; BLHeli is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with BLHeli.  If not, see <http://www.gnu.org/licenses/>.
;
;**** **** **** **** ****
;
; Turnigy AE 45A hardware definition file
;
;**** **** **** **** ****



;*********************
; Device SiLabs F310
;*********************
$include (c8051f310.inc)

;**** **** **** **** ****
; Uses internal calibrated oscillator set to 24Mhz
;**** **** **** **** ****

;**** **** **** **** ****
; Constant definitions
;**** **** **** **** ****
CSEG AT 1A40h
Eep_ESC_Layout:		DB	"#TurnigyAE45A#  "	; ESC layout tag
CSEG AT 1A50h
Eep_ESC_MCU:			DB	"#BLHELI#F310#   "	; Project and MCU tag (16 Bytes)

PORT3_EXIST			EQU	1	; Set to 1 if MCU has port3
DAMPED_MODE_ENABLE		EQU	0	; Damped mode disabled
NFETON_DELAY			EQU	10	; Wait delay from pfets off to nfets on
PFETON_DELAY			EQU	10	; Wait delay from nfets off to pfets on
COMP_PWM_HIGH_ON_DELAY	EQU	30	; Wait delay from pwm on until comparator can be read (for high pwm frequency)
COMP_PWM_HIGH_OFF_DELAY	EQU	60	; Wait delay from pwm off until comparator can be read (for high pwm frequency)
COMP_PWM_LOW_ON_DELAY	EQU	10	; Wait delay from pwm on until comparator can be read (for low pwm frequency)
COMP_PWM_LOW_OFF_DELAY	EQU	20	; Wait delay from pwm off until comparator can be read (for low pwm frequency)
ADC_LIMIT_L			EQU 	85	; Power supply measurement ADC value for which main motor power is limited (low byte)
ADC_LIMIT_H			EQU	0	; Power supply measurement ADC value for which main motor power is limited (2 MSBs)
TEMP_LIMIT_L			EQU 	246	; Temperature measurement ADC value for which main motor power is limited (low byte)
TEMP_LIMIT_H			EQU	1	; Temperature measurement ADC value for which main motor power is limited (2 msbs)
TEMP_LIMIT_STEP		EQU	6	; Temperature measurement ADC value increment for which main motor power is further limited


;*********************
; PORT 0 definitions *
;*********************  
BnFET		EQU	7	;o
BpFET		EQU	6	;o
Rcp_In		EQU	5	;i
;			EQU	4	;i
;			EQU	3	;i
ApFET		EQU	2	;o
CnFET		EQU	1	;o
CpFET		EQU	0	;o

P0_DIGITAL	EQU	0FFh
P0_INIT		EQU	NOT((1 SHL BnFET)+(1 SHL CnFET)+(1 SHL ApFET)+(1 SHL BpFET)+(1 SHL CpFET)) AND 0FFh
P0_PUSHPULL	EQU	(1 SHL BnFET)+(1 SHL CnFET)+(1 SHL ApFET)+(1 SHL BpFET)+(1 SHL CpFET)
P0_SKIP		EQU	NOT(1 SHL Rcp_In)

MACRO Read_Rcp_Int
	mov	A, P0
	jnb	Flags3.PGM_RCP_PWM_POL, ($+4)	; Is pwm polarity negative?
	cpl	A						; Yes - invert
ENDM
MACRO Rcp_Int_Enable
	orl	PCA0CPM0, #01h				; Interrupt enabled
ENDM
MACRO Rcp_Int_Disable
	anl	PCA0CPM0, #0FEh			; Interrupt disabled
ENDM
MACRO Rcp_Int_First
	anl	PCA0CPM0, #0CFh			
	jb	Flags3.PGM_RCP_PWM_POL, ($+6)	; Is pwm polarity positive?
	orl	PCA0CPM0, #20h				; Capture rising edge
	jnb	Flags3.PGM_RCP_PWM_POL, ($+6)	; Is pwm polarity negative?
	orl	PCA0CPM0, #10h				; Capture falling edge
ENDM
MACRO Rcp_Int_Second
	anl	PCA0CPM0, #0CFh			
	jb	Flags3.PGM_RCP_PWM_POL, ($+6)	; Is pwm polarity positive?
	orl	PCA0CPM0, #10h				; Capture falling edge
	jnb	Flags3.PGM_RCP_PWM_POL, ($+6)	; Is pwm polarity negative?
	orl	PCA0CPM0, #20h				; Capture rising edge
ENDM
MACRO Rcp_Clear_Int_Flag
	clr 	CCF0						; Clear interrupt flag
ENDM		


;*********************
; PORT 1 definitions *
;*********************
Mux_B1		EQU	7	;i
Adc_Ip		EQU	6	;i
Mux_A3		EQU	5	;i
Mux_A2		EQU	4	;i
Mux_A1		EQU	3	;i
;			EQU	2	;i
Comp_Com2		EQU	1	;i
Comp_Com1		EQU	0	;i

P1_DIGITAL	EQU	00h
P1_INIT		EQU 	0FFh		
P1_PUSHPULL	EQU	00h
P1_SKIP		EQU	0

;*********************
; PORT 2 definitions *
;*********************
;			EQU	7	;i
;			EQU	6	;i
Mux_C3		EQU	5	;i
Mux_C2		EQU	4	;i
Mux_C1		EQU	3	;i
;			EQU	2	;i
Mux_B3		EQU	1	;i
Mux_B2		EQU	0	;i

P2_DIGITAL	EQU	00h
P2_INIT		EQU 	0FFh			
P2_PUSHPULL	EQU	00h
P2_SKIP		EQU	0


MACRO AnFET_on
	mov	A, Current_Pwm_Limited
	jz	($+12)
	jb	Flags3.PGM_DIR_REV, ($+5)
	setb	P3.AnFET
	jnb	Flags3.PGM_DIR_REV, ($+5)
	setb	P0.CnFET
ENDM
MACRO AnFET_off
	jb	Flags3.PGM_DIR_REV, ($+5)
	clr	P3.AnFET
	jnb	Flags3.PGM_DIR_REV, ($+5)
	clr	P0.CnFET
ENDM
MACRO BnFET_on
	mov	A, Current_Pwm_Limited
	jz	($+4)
	setb	P0.BnFET
ENDM
MACRO BnFET_off
	clr	P0.BnFET
ENDM
MACRO CnFET_on
	mov	A, Current_Pwm_Limited
	jz	($+12)
	jb	Flags3.PGM_DIR_REV, ($+5)
	setb	P0.CnFET
	jnb	Flags3.PGM_DIR_REV, ($+5)
	setb	P3.AnFET
ENDM
MACRO CnFET_off
	jb	Flags3.PGM_DIR_REV, ($+5)
	clr	P0.CnFET
	jnb	Flags3.PGM_DIR_REV, ($+5)
	clr	P3.AnFET
ENDM
MACRO All_nFETs_Off
	clr	P3.AnFET
	clr	P0.BnFET
	clr	P0.CnFET
ENDM

MACRO ApFET_on
	jb	Flags3.PGM_DIR_REV, ($+5)
	setb	P0.ApFET
	jnb	Flags3.PGM_DIR_REV, ($+5)
	setb	P0.CpFET
ENDM
MACRO ApFET_off
	jb	Flags3.PGM_DIR_REV, ($+5)
	clr	P0.ApFET
	jnb	Flags3.PGM_DIR_REV, ($+5)
	clr	P0.CpFET
ENDM
MACRO BpFET_on
	setb	P0.BpFET
ENDM
MACRO BpFET_off
	clr	P0.BpFET
ENDM
MACRO CpFET_on
	jb	Flags3.PGM_DIR_REV, ($+5)
	setb	P0.CpFET
	jnb	Flags3.PGM_DIR_REV, ($+5)
	setb	P0.ApFET
ENDM
MACRO CpFET_off
	jb	Flags3.PGM_DIR_REV, ($+5)
	clr	P0.CpFET
	jnb	Flags3.PGM_DIR_REV, ($+5)
	clr	P0.ApFET
ENDM
MACRO All_pFETs_Off
	clr	P0.ApFET
	clr	P0.BpFET
	clr	P0.CpFET
ENDM
MACRO All_pFETs_On
	setb	P0.ApFET
	setb	P0.BpFET
	setb	P0.CpFET
ENDM

MACRO Set_Comp_Phase_A
	jb	Flags3.PGM_DIR_REV, ($+6)
	mov	CPT0MX, #01h	; Set comparator multiplexer to phase A
	jnb	Flags3.PGM_DIR_REV, ($+6)
	mov	CPT0MX, #03h
ENDM
MACRO Set_Comp_Phase_B
	mov	CPT0MX, #02h	; Set comparator multiplexer to phase B
ENDM
MACRO Set_Comp_Phase_C
	jb	Flags3.PGM_DIR_REV, ($+6)
	mov	CPT0MX, #03h	; Set comparator multiplexer to phase C
	jnb	Flags3.PGM_DIR_REV, ($+6)
	mov	CPT0MX, #01h
ENDM
MACRO Read_Comp_Out
	mov	A, CPT0CN		; Read comparator output
ENDM
MACRO Set_Comp_Hyst_Low
	mov	CPT0CN, #83h 	; Set 20mV negative hysteresis
ENDM
MACRO Set_Comp_Hyst_High
	mov	CPT0CN, #8Ch 	; Set 20mV positive hysteresis
ENDM


;*********************
; PORT 3 definitions *
;*********************
LED			EQU	4	;o
Bec_6V		EQU	3	;o
;			EQU	2	;i
AnFET		EQU	1	;o
DebugPin		EQU	0	;o

P3_DIGITAL	EQU	0FFh
P3_INIT		EQU 	NOT((1 SHL LED)+(1 SHL Bec_6V)+(1 SHL AnFET)+(1 SHL DebugPin))
P3_PUSHPULL	EQU	(1 SHL LED)+(1 SHL Bec_6V)+(1 SHL AnFET)+(1 SHL DebugPin)


;**********************
; MCU specific macros *
;**********************
MACRO Interrupt_Table_Definition
CSEG AT 0				; Code segment start
	jmp reset
CSEG AT 0Bh			; Timer0 interrupt	
	jmp	t0_int	
CSEG AT 2Bh			; Timer2 interrupt	
	jmp	t2_int	
CSEG AT 5Bh			; PCA interrupt	
	jmp	pca_int	
CSEG AT 73h			; Timer3 interrupt	
	jmp	t3_int	
ENDM

MACRO Initialize_Adc
	mov	REF0CN, #0Eh	; Set vdd (3.3V) as reference. Enable temp sensor and bias
	mov	ADC0CF, #0B8h	; ADC clock 1MHz
	mov	AMX0P, #Adc_Ip	; Select positive input
	mov	AMX0N, #1Fh	; Select negative input as ground
	mov	ADC0CN, #80h	; ADC enabled
ENDM
MACRO Set_Adc_Ip_Volt
	mov	AMX0P, #Adc_Ip	; Select positive input
ENDM
MACRO Set_Adc_Ip_Temp
	mov	AMX0P, #1Eh	; Select temp sensor input
ENDM
MACRO Start_Adc
	mov	ADC0CN, #90h	; ADC start
ENDM
MACRO Get_Adc_Status
	mov	A, ADC0CN		
ENDM
MACRO Read_Adc_Result
	mov	Temp1, ADC0L
	mov	Temp2, ADC0H
ENDM
MACRO Stop_Adc
ENDM