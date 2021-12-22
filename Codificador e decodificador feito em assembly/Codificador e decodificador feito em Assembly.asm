; Nome: Gabriel Santos Teixeira				Matr�cula: 190087587

;-------------------------------------------------------------------------------
; MSP430 Assembler Code Template for use with TI Code Composer Studio
;
;
;-------------------------------------------------------------------------------
            .cdecls C,LIST,"msp430.h"       ; Include device header file
            
;-------------------------------------------------------------------------------
            .def    RESET                   ; Export program entry-point to
                                            ; make it known to linker.
;-------------------------------------------------------------------------------
            .text                           ; Assemble into program memory.
            .retain                         ; Override ELF conditional linking
                                            ; and retain current section.
            .retainrefs                     ; And retain any sections that have
                                            ; references to current section.

;-------------------------------------------------------------------------------
RESET       mov.w   #__STACK_END,SP         ; Initialize stackpointer
StopWDT     mov.w   #WDTPW|WDTHOLD,&WDTCTL  ; Stop watchdog timer


;-------------------------------------------------------------------------------
; Main loop here
;-------------------------------------------------------------------------------

RT_TAM 		.equ 	26 						; Tamanho dos rotores (26 letras)
QTD_ROTORES .equ	3						; Quantidade de rotores que ser�o usados.

VISTO1:		MOV 	#MSG	,R5				; Move endere�o da mensagem para R5.
 			MOV 	#GSM	,R6				; Move endere�o da msg cifrada para R6.
 			CALL 	#ENIGMA 				; Cifrar

			CALL 	#RESETE 				; Restaurar posi��o original dos rotores.

 			MOV 	#GSM	,R5				; Move endere�o da msg cifrada para R5.
 			MOV 	#DCF	,R6				; Move endere�o da msg decifrada para R6.
 			CALL 	#ENIGMA 				; Decifrar
 			JMP $							; Trava o programa nesta linha.
 			NOP


; Defini��o da chave do Enigma
CHAVE: 		.byte	2, 6, 3, 8, 5, 12, 2

;--ENIGMA-----------------------------------------------------------------------

; Sua rotina ENIGMA
ENIGMA:		MOV		#RT_TAM	,R9				; Inicializa R9 com tamanho dos rotores.
			MOV		#CHAVE  ,R8				; Inicializa R8 com chave do enigma.
			CALL	#RRD					; Configura o primeiro rotor.
			CALL	#RRD					; Configura o segundo rotor.
			CALL	#RRD					; Configura o terceiro rotor.
			MOV		#CHAVE  ,R8				; Move chave do enigma para R8.
			MOV		#COPIAS	,R12			; Move endere�o que salva valor dos rotores para R12.
			CALL	#BACKUP					; Salva valor dos rotores.
			MOV		#CHAVE  ,R8				; Move chave do enigma para R8.

LOOP_1:		TST.B	0(R5)					; Verifica condi��o de parada da string.
			JZ		FIM						; Vai para label FIM se alcan�ar a condi��o.

			CMP.B	#'A'	,0(R5)			; Condi��o para s� cifrar letras.
			JHS		PROXIMO_1
			JMP		AJUSTE

PROXIMO_1:	CMP.B	#'['	,0(R5)			; Condi��o para s� cifrar letras.
			JLO		PROXIMO_2
			JMP		AJUSTE

AJUSTE:		MOV.B	@R5+	,0(R6)			; Ajuste para onde n�o tiver letras,
			INC		R6						; n�o ser alterado.
			JMP		LOOP_1

PROXIMO_2:	MOV.B	@R5+	,R10			; Move letras do #MSG para R10.
			SUB.B	#'A'	,R10			; Obt�m posi��o referente ao primeiro rotor.

			CALL	#SWITCH_R				; Obt�m o primeiro rotor que ser� utilizado.

			INC		R8						; Movimenta o vetor chave para o pr�x. rotor.
			ADD		R7		,R10			; Encontra endere�o escolhido.
			MOV.B	0(R10)	,R10			; Joga valor do endere�o escolhido para R10.

			CALL	#SWITCH_R				; Obt�m o segundo rotor que ser� utilizado.

			INC		R8						; Movimenta o vetor chave para o pr�x. rotor.
			ADD		R7		,R10			; Encontra endere�o escolhido.
			MOV.B	0(R10)	,R10			; Joga valor do endere�o escolhido para R10.

			CALL	#SWITCH_R				; Obt�m o terceiro rotor que ser� utilizado.

			INC		R8						; Movimenta o vetor chave para o refletor.
			ADD		R7		,R10			; Encontra endere�o escolhido.
			MOV.B	0(R10)	,R7				; Joga valor do endere�o escolhido para R7.

			MOV.B	0(R8)	,R15			; Escolhe refletor que ser� usado.
			MOV		#REFLETORES ,R12		; Move endere�o dos refletores para R12.

REFL:		DEC		R15
			TST		R15						; Condi��o de parada do seletor de refletor.
			JZ		PROXIMO_3

			ADD		#RT_TAM	,R12			; Puxa o refletor escolhido.
			JMP		REFL

PROXIMO_3:	ADD		R7		,R12			; Encontra endere�o escolhido.
			MOV.B	0(R12)	,R7				; Joga valor do endere�o escolhido para R7.

			DECD 	R8						; Movimenta chave para terceiro rotor.
			CALL	#INDICE					; Consulta �ndice do valor de R7.
			DECD	R8						; Movimenta chave para segundo rotor.
			CALL	#INDICE					; Consulta �ndice do resultado do �ndice anterior.
			DECD	R8						; Movimenta chave para primeiro rotor.
			CALL	#INDICE					; Consulta �ndice do resultado do �ndice anterior.

			ADD.B	#'A'	,R7				; Obt�m letra cifrada.
			MOV.B	R7		,0(R6)			; Move letra cifrada para #GSM.

			INC 	R6						; Move para pr�xima posi��o de #GSM.

			CALL	#ROT_ROTOR				; Faz o giro do rotor � cada letra.

			JMP 	LOOP_1					; Loop para cifrar todas as letras.

FIM:		RET								; Quando termina volta para a main.

;--ROT_ROTOR--------------------------------------------------------------------

; Ser� feito rota��o para � esquerda, � cada letra cifrada.
ROT_ROTOR:	PUSH	R14						; Salva R14 na pilha.
			PUSH	R13						; Salva R13 na pilha.
			PUSH	R12						; Salva R12 na pilha.
			DEC		R9

INICIO_RT:	TST		R9						; Condi��o para girar o rotor do meio.
			JZ		GIRO_M
			CALL	#SWITCH_R				; Puxa rotor escolhido.
			INC		R8						; Move para o pr�ximo rotor escolhido.
			MOV		R7		,R14			; Move para ajustar registros � subrotina.
			MOV		#RT_TAM	,R12			; Registro auxiliar da condi��o de parada.
			MOV.B	#RT_TAM ,R13			; Move tamanho do vetor para R13.
			ADD		R14		,R13			; Mover� registro para final do vetor.
			DEC		R13						; Ajuste pois passa da posi��o final do vetor.
			MOV.B	@R13	,R7				; Guarda o valor do mais � direita.
			DEC		R12						; Para n�o rotacionar mais que o necess�rio.

LOOP_RI:	TST		R12						; Condi��o de parada, ap�s rotacionar, com
			JZ		FIM_RI					; exce��o do �ltimo, todos os valores do rotor.
			DEC		R12

			MOV.B	-1(R13)	,0(R13)			; Movimenta valores do rotor para a esquerda.
			DEC		R13						; Movimenta o vetor para pr�xima posi��o a
			JMP		LOOP_RI					; rotacionar.

GIRO_M:		CALL	#SWITCH_R				; Ajuste para puxar rotor do meio escolhido,
			INC		R8						; caso contr�rio seria escolhido o vetor
			CALL	#SWITCH_R				; da esquerda.
			MOV		#RT_TAM	,R9				; Reseta R9.
			MOV		R7		,R14
			MOV		#RT_TAM	,R12			; Faz mesma inicializa��o vista acima,
			MOV.B	#RT_TAM ,R13			; para rotacionar o rotor do meio para
			ADD		R14		,R13			; a esquerda.
			DEC		R13
			MOV.B	@R13	,R7
			DEC		R12

LOOP_MI:	TST		R12
			JZ		RMV_MI
			DEC		R12
											; Rotaciona valores para esquerda como
			MOV.B	-1(R13)	,0(R13)			; visto acima. At� aqui sem novidades.
			DEC		R13
			JMP		LOOP_MI

RMV_MI:		MOV.B	R7		,0(R13)			; Move o �ltimo valor para a primeira
											; posi��o do rotor do meio.
			SUB		#3		,R8				; Move chave para primeira posi��o.
			JMP		INICIO_RT

FIM_RI:		MOV.B	R7		,0(R13)			; Move o �ltimo valor para a primeira
											; posi��o do rotor da esquerda.
			DECD	R8						; Move chave para primeira posi��o.
			POP		R12						; Retorna R12 da pilha.
			POP		R13						; Retorna R13 da pilha.
			POP		R14						; Retorna R14 da pilha.
			RET								; Volta para subrotina Enigma.

;--BACKUP-----------------------------------------------------------------------

; Salva na mem�ria valores dos rotores.
BACKUP:		MOV		#QTD_ROTORES,R14		; Quantos rotores ser�o salvos na mem�ria.

LOOP_BE:	MOV		#RT_TAM	,R13			; Tamanho do rotor para R13.
			TST		R14						; Condi��o de parada, ap�s todos rotores
			JZ		FIM_B					; terem sido salvos na mem�ria.
			DEC		R14
			CALL	#SWITCH_R				; Puxa rotor escolhido.
			INC		R8						; Move para o pr�ximo rotor escolhido.

LOOP_B:		TST		R13						; Condi��o de parada ap�s configura��o
			JZ		LOOP_BE					; do rotor ser copiada.
			DEC		R13

			MOV.B	@R7+	,0(R12)			; Copia valor do rotor configurado para
			INC		R12						; espa�o separado na mem�ria.
			JMP		LOOP_B

FIM_B:		RET								; Retorna para subrotina main.

;--RESETE-----------------------------------------------------------------------

; Reseta rotores para estado inicial.
RESETE:		MOV		#QTD_ROTORES,R14		; Quantos rotores ser�o resetados.
			MOV		#COPIAS	,R12			; Move endere�o das c�pias para R12.

LOOP_ERST:	MOV		#RT_TAM	,R13			; Tamanho do rotor para R13.
			TST		R14						; Condi��o de parada, ap�s todos rotores
			JZ		FIM_RST					; terem sido resetados para valores
			DEC		R14						; obtidos ap�s configura��o inicial.
			CALL	#SWITCH_R				; Puxa rotor escolhido.
			INC		R8						; Move para o pr�ximo rotor escolhido.

LOOP_RST:	TST		R13						; Condi��o de parada ap�s configura��o
			JZ		LOOP_ERST				; do rotor ser resetada.
			DEC		R13

			MOV.B	@R12+	,0(R7)			; Move valor salvo na mem�ria para rotores
			INC		R7						; previamente escolhidos.
			JMP		LOOP_RST

FIM_RST:	SUB		#6		,R8				; Move chave para primeira posi��o.
			CALL	#RRE
 			CALL	#RRE					; Reverte as rota��es feitas na primeira
 			CALL	#RRE					; configura��o.
			RET								; Volta para subrotina Enigma.

;--SWICH_R----------------------------------------------------------------------

; Retorna para a fun��o o rotor escolhido.
SWITCH_R:	MOV.B	0(R8)	,R15			; Puxa de chaves o rotor escolhido.
			INC		R8						; Move chave para pr�xima posi��o.
			MOV		#ROTORES,R7				; Move endere�o de rotores para R7.

SWITCH_I:	DEC		R15						; Condi��o de parada para quando
			TST		R15						; chegar no rotor escolhido.
			JZ		FIM_S

			ADD		#RT_TAM	,R7				; Adiciona tamanho do rotor ao vetor
			JMP		SWITCH_I				; em R7, para chegar no rotor escolhido.

FIM_S:		RET								; Volta para subrotinas em que foi chamado.

;--RRD--------------------------------------------------------------------------

; Faz rota��o dos rotores para direita.
RRD:		PUSH	R5						; Salva R5 na pilha.
			CALL	#SWITCH_R				; Puxa rotor escolhido.

			MOV		R7		,R14			; Move para ajustar registros � subrotina.
			MOV.B	0(R8)	,R5				; Move valor de chave relacionado a config
											; para R5.
LOOP_ED:	TST		R5						; Enquanto a chave n�o estiver na config
			JZ		FIM_D					; correta, ele n�o vai para FIM_D.
			DEC		R5
			MOV		#RT_TAM ,R12			; Joga o tamanho do rotor em R12.
			MOV		R14		,R7				; Move o que est� em R14 para R7.
			MOV.B	0(R7)   ,R13			; Guarda o valor do mais � esquerda.
			DEC		R12						; Decrementa para n�o rotacionar mais
											; do que o necess�rio.
LOOP_ID:	TST		R12						; Condi��o de parada, ap�s rotacionar, com
			JZ		DMV_R13					; exce��o do primeiro, todos os valores do rotor.
			DEC		R12

			MOV.B	1(R7)	,0(R7)			; Movimenta valores do rotor para a direita.
			INC		R7						; Movimenta o vetor para pr�xima posi��o a
			JMP		LOOP_ID					; rotacionar.

DMV_R13:	MOV.B	R13		,0(R7)			; Move o primeiro valor para a �ltima
			JMP		LOOP_ED					; posi��o do rotor.

FIM_D:		POP		R5						; Retorna R5 da pilha.
			INC		R8						; Move chave para pr�xima posi��o.
			RET								; Retorna para subrotina Enigma.

;--RRE--------------------------------------------------------------------------

; Faz rota��o dos rotores para esquerda.
RRE:		PUSH	R5						; Salva R5 na pilha.
			CALL	#SWITCH_R				; Puxa rotor escolhido.

			MOV		R7		,R14			; Move para ajustar registros � subrotina.
			MOV.B	0(R8)	,R5				; Move valor de chave relacionado a config
											; para R5.
LOOP_EE:	TST		R5						; Enquanto a chave n�o estiver na config
			JZ		FIM_E					; correta, ele n�o vai para FIM_D.
			DEC		R5
			MOV		#RT_TAM ,R12			; Joga o tamanho do rotor em R12.
			MOV.B	#RT_TAM ,R13			; Move tamanho do vetor para R13.
			ADD		R14		,R13			; Mover� registro para final do vetor.
			DEC		R13						; Ajuste pois passa da posi��o final do vetor.
			MOV.B	@R13	,R7				; Guarda o valor do mais � direita.
			DEC		R12						; Decrementa para n�o rotacionar mais
											; do que o necess�rio.
LOOP_IE:	TST		R12						; Condi��o de parada, ap�s rotacionar, com
			JZ		EMV_R13					; exce��o do �ltimo, todos os valores do rotor.
			DEC		R12

			MOV.B	-1(R13)	,0(R13)			; Movimenta valores do rotor para a esquerda.
			DEC		R13						; Movimenta o vetor para pr�xima posi��o a
			JMP		LOOP_IE					; rotacionar.

EMV_R13:	MOV.B	R7		,0(R13)			; Move o �ltimo valor para a primeira
			JMP		LOOP_EE					; posi��o do rotor.

FIM_E:		POP		R5						; Retorna R5 da pilha.
			INC		R8						; Move chave para pr�xima posi��o.
			RET								; Retorna para subrotina Resete.

;--INDICE-----------------------------------------------------------------------

INDICE:		PUSH	R6						; Salva R6 na pilha.
			PUSH	R5						; Salva R5 na pilha.
			CLR		R5						; Limpa o conte�do de R5.
			MOV		R7		,R6				; Move o resultado do rotor atual para R6.
			CALL	#SWITCH_R				; Puxa rotor escolhido.
			DEC		R8						; Move chave para pr�xima posi��o.

LOOP_II:	CMP.B	@R7+	,R6				; Compara conte�do do rotor com conte�do de
			JEQ		FIM_I					; R6, enquanto n�o forem iguais o loop se
											; mant�m.
			INC		R5						; Obt�m em R5 o �ndice da posi��o em que
			JMP		LOOP_II					; o valor est� no rotor.

FIM_I:		MOV		R5		,R7				; Move o resultado de R5 para R7.
			POP		R5						; Retorna R5 da pilha.
			POP		R6						; Retorna R6 da pilha.
			RET								; Retorna para subrotina Enigma.

;--RAM--------------------------------------------------------------------------
; �rea de dados
 			.data
MSG: 		.byte	"UMA NOITE DESTAS, VINDO DA CIDADE PARA O ENGENHO NOVO,"
			.byte 	" ENCONTREI NO TREM DA CENTRAL UM RAPAZ AQUI DO BAIRRO,"
 			.byte 	" QUE EU CONHECO DE VISTA E DE CHAPEU.",0 ; Don Casmurro
GSM: 		.byte 	"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
 			.byte 	"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
 			.byte 	"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",0
DCF: 		.byte 	"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
 			.byte 	"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
 			.byte 	"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",0

; Rotores com 26 posi��es
ROTORES:
RT1: 		.byte 	10, 18, 0, 7, 3, 2, 4, 8, 14, 17, 5, 22, 20
 			.byte 	25, 23, 1, 24, 13, 21, 19, 11, 6, 12, 15, 9, 16
RT2: 		.byte 	19, 9, 7, 23, 11, 25, 17, 16, 2, 24, 15, 10, 6
 			.byte 	14, 0, 8, 13, 4, 3, 18, 1, 22, 21, 20, 5, 12
RT3: 		.byte 	16, 21, 4, 0, 1, 12, 15, 14, 8, 25, 9, 19, 17
 			.byte 	6, 5, 20, 13, 24, 23, 10, 3, 22, 11, 7, 18, 2
RT4: 		.byte 	25, 4, 21, 22, 17, 14, 12, 8, 5, 15, 23, 6, 2
 			.byte 	18, 10, 16, 13, 3, 19, 20, 0, 24, 11, 7, 9, 1
RT5: 		.byte 	13, 5, 20, 7, 0, 15, 21, 9, 19, 14, 24, 18, 12
 			.byte 	6, 2, 11, 16, 8, 3, 1, 10, 23, 4, 25, 22, 17

; Refletores com 26 posi��es
REFLETORES:
RF1: 		.byte 	7, 17, 5, 19, 15, 2, 9, 0, 14, 6, 18, 16, 25
 			.byte 	23, 8, 4, 11, 1, 10, 3, 22, 24, 20, 13, 21, 12
RF2: 		.byte 	2, 13, 0, 8, 10, 19, 23, 14, 3, 22, 4, 20, 25
 			.byte 	1, 7, 17, 21, 15, 24, 5, 11, 16, 9, 6, 18, 12
RF3: 		.byte 	22, 5, 9, 12, 14, 1, 13, 10, 23, 2, 7, 21, 3
 			.byte 	6, 4, 24, 17, 16, 19, 18, 25, 11, 0, 8, 15, 20

; Copia valores dos rotores
COPIAS:
COPIA_1:	.byte 	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
 			.byte 	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
COPIA_2:	.byte 	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
 			.byte 	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
COPIA_3:	.byte 	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
 			.byte 	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

;-------------------------------------------------------------------------------
; Stack Pointer definition
;-------------------------------------------------------------------------------
            .global __STACK_END
            .sect   .stack
            
;-------------------------------------------------------------------------------
; Interrupt Vectors
;-------------------------------------------------------------------------------
            .sect   ".reset"                ; MSP430 RESET Vector
            .short  RESET
            
