/**************************************************
 *
 * Part one of the system initialization code, contains low-level
 * initialization, plain thumb variant.
 *
 * Copyright 2011 IAR Systems. All rights reserved.
 *
 * $Revision: 50291 $
 *
 **************************************************/

;
; The modules in this file are included in the libraries, and may be replaced
; by any user-defined modules that define the PUBLIC symbol _program_start or
; a user defined start symbol.
; To override the cstartup defined in the library, simply add your modified
; version to the workbench project.
;
; The vector table is normally located at address 0.
; When debugging in RAM, it can be located in RAM, aligned to at least 2^6.
; The name "__vector_table" has special meaning for C-SPY:
; it is where the SP start value is found, and the NVIC vector
; table register (VTOR) is initialized to this address if != 0.
;
; Cortex-M version
;


        MODULE  ?cstartup

        ;; Forward declaration of sections.
        SECTION CSTACK:DATA:NOROOT(3)

        SECTION .intvec:CODE:NOROOT(2)

        EXTERN  __iar_program_start
        PUBLIC  __vector_table
        PUBLIC  __vector_table_0x1c
        PUBLIC  __Vectors
        PUBLIC  __Vectors_End
        PUBLIC  __Vectors_Size

        DATA

__vector_table
        DCD     sfe(CSTACK)
        DCD     Reset_Handler
        DCD     NMI_Handler
        DCD     HardFault_Handler
        DCD     MemManage_Handler
        DCD     BusFault_Handler
        DCD     UsageFault_Handler
__vector_table_0x1c
        DCD     0
        DCD     0
        DCD     0
        DCD     0
        DCD     SVC_Handler
        DCD     DebugMon_Handler
        DCD     0
        DCD     PendSV_Handler
        DCD     SysTick_Handler

        ; External Interrupts
        DCD     DAC_IRQHandler          ; 16 D/A Converter
        DCD     M0CORE_IRQHandler       ; 17 M0CORE
        DCD     DMA_IRQHandler          ; 18 General Purpose DMA
        DCD     0                       ; 19 Reserved
        DCD     0                       ; 20 Reserved
        DCD     ETHERNET_IRQHandler     ; 21 Ethernet
        DCD     SDIO_IRQHandler         ; 22 SD/MMC
        DCD     LCD_IRQHandler          ; 23 LCD
        DCD     USB0_IRQHandler         ; 24 USB0
        DCD     USB1_IRQHandler         ; 25 USB1
        DCD     SCT_IRQHandler          ; 26 State Configurable Timer
        DCD     RITIMER_IRQHandler      ; 27 Repetitive Interrupt Timer
        DCD     TIMER0_IRQHandler       ; 28 Timer0
        DCD     TIMER1_IRQHandler       ; 29 Timer1
        DCD     TIMER2_IRQHandler       ; 30 Timer2
        DCD     TIMER3_IRQHandler       ; 31 Timer3
        DCD     MCPWM_IRQHandler        ; 32 Motor Control PWM
        DCD     ADC0_IRQHandler         ; 33 A/D Converter 0
        DCD     I2C0_IRQHandler         ; 34 I2C0
        DCD     I2C1_IRQHandler         ; 35 I2C1
        DCD     SPI_IRQHandler          ; 36 SPI
        DCD     ADC1_IRQHandler         ; 37 A/D Converter 1
        DCD     SSP0_IRQHandler         ; 38 SSP0
        DCD     SSP1_IRQHandler         ; 39 SSP1
        DCD     USART0_IRQHandler       ; 40 USART0
        DCD     UART1_IRQHandler        ; 41 UART1
        DCD     USART2_IRQHandler       ; 42 USART2
        DCD     USART3_IRQHandler       ; 43 USART3
        DCD     I2S0_IRQHandler         ; 44 I2S0
        DCD     I2S1_IRQHandler         ; 45 I2S1
        DCD     SPIFI_IRQHandler        ; 46 SPIFI
        DCD     SGPIO_IRQHandler        ; 47 SGPIO
        DCD     PIN_INT0_IRQHandler     ; 48 PIN_INT0
        DCD     PIN_INT1_IRQHandler     ; 49 PIN_INT1
        DCD     PIN_INT2_IRQHandler     ; 50 PIN_INT2
        DCD     PIN_INT3_IRQHandler     ; 51 PIN_INT3
        DCD     PIN_INT4_IRQHandler     ; 52 PIN_INT4
        DCD     PIN_INT5_IRQHandler     ; 53 PIN_INT5
        DCD     PIN_INT6_IRQHandler     ; 54 PIN_INT6
        DCD     PIN_INT7_IRQHandler     ; 55 PIN_INT7
        DCD     GINT0_IRQHandler        ; 56 GINT0
        DCD     GINT1_IRQHandler        ; 57 GINT1
        DCD     EVENTROUTER_IRQHandler  ; 58 Event Router
        DCD     C_CAN1_IRQHandler       ; 59 CCAN1
        DCD     0
        DCD     0
        DCD     ATIMER_IRQHandler       ; 62 Alarm Timer
        DCD     RTC_IRQHandler          ; 63 RTC
        DCD     0
        DCD     WWDT_IRQHandler         ; 65 WWDT
        DCD     0
        DCD     C_CAN0_IRQHandler       ; 67 CCAN0
        DCD     QEI_IRQHandler          ; 68 QEI
__Vectors_End

__Vectors       EQU   __vector_table
__Vectors_Size  EQU   __Vectors_End - __Vectors

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Default interrupt handlers.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        THUMB

        PUBWEAK Reset_Handler
        SECTION .text:CODE:REORDER(2)
Reset_Handler
        LDR     R0, =__iar_program_start
        BX      R0

        PUBWEAK NMI_Handler
        PUBWEAK HardFault_Handler
        PUBWEAK MemManage_Handler
        PUBWEAK BusFault_Handler
        PUBWEAK UsageFault_Handler
        PUBWEAK SVC_Handler
        PUBWEAK DebugMon_Handler
        PUBWEAK PendSV_Handler
        PUBWEAK SysTick_Handler
        PUBWEAK DAC_IRQHandler
        PUBWEAK M0CORE_IRQHandler
        PUBWEAK DMA_IRQHandler
        PUBWEAK ETHERNET_IRQHandler
        PUBWEAK SDIO_IRQHandler
        PUBWEAK LCD_IRQHandler
        PUBWEAK USB0_IRQHandler
        PUBWEAK USB1_IRQHandler
        PUBWEAK SCT_IRQHandler
        PUBWEAK RITIMER_IRQHandler
        PUBWEAK TIMER0_IRQHandler
        PUBWEAK TIMER1_IRQHandler
        PUBWEAK TIMER2_IRQHandler
        PUBWEAK TIMER3_IRQHandler
        PUBWEAK MCPWM_IRQHandler
        PUBWEAK ADC0_IRQHandler
        PUBWEAK I2C0_IRQHandler
        PUBWEAK I2C1_IRQHandler
        PUBWEAK SPI_IRQHandler
        PUBWEAK ADC1_IRQHandler
        PUBWEAK SSP0_IRQHandler
        PUBWEAK SSP1_IRQHandler
        PUBWEAK USART0_IRQHandler
        PUBWEAK UART1_IRQHandler
        PUBWEAK USART2_IRQHandler
        PUBWEAK USART3_IRQHandler
        PUBWEAK I2S0_IRQHandler
        PUBWEAK I2S1_IRQHandler
        PUBWEAK SPIFI_IRQHandler
        PUBWEAK SGPIO_IRQHandler
        PUBWEAK PIN_INT0_IRQHandler
        PUBWEAK PIN_INT1_IRQHandler
        PUBWEAK PIN_INT2_IRQHandler
        PUBWEAK PIN_INT3_IRQHandler
        PUBWEAK PIN_INT4_IRQHandler
        PUBWEAK PIN_INT5_IRQHandler
        PUBWEAK PIN_INT6_IRQHandler
        PUBWEAK PIN_INT7_IRQHandler
        PUBWEAK GINT0_IRQHandler
        PUBWEAK GINT1_IRQHandler
        PUBWEAK EVENTROUTER_IRQHandler
        PUBWEAK C_CAN1_IRQHandler
        PUBWEAK ATIMER_IRQHandler
        PUBWEAK RTC_IRQHandler
        PUBWEAK WWDT_IRQHandler
        PUBWEAK C_CAN0_IRQHandler
        PUBWEAK QEI_IRQHandler
        SECTION .text:CODE:REORDER(1)
NMI_Handler
        B NMI_Handler
SVC_Handler
        B SVC_Handler
DebugMon_Handler
        B DebugMon_Handler
PendSV_Handler
        B PendSV_Handler
SysTick_Handler
        B SysTick_Handler
HardFault_Handler
        B HardFault_Handler
MemManage_Handler
        B MemManage_Handler
BusFault_Handler
        B BusFault_Handler
UsageFault_Handler
DAC_IRQHandler
M0CORE_IRQHandler
DMA_IRQHandler
ETHERNET_IRQHandler
SDIO_IRQHandler
LCD_IRQHandler
USB0_IRQHandler
USB1_IRQHandler
SCT_IRQHandler
RITIMER_IRQHandler
TIMER0_IRQHandler
TIMER1_IRQHandler
TIMER2_IRQHandler
TIMER3_IRQHandler
MCPWM_IRQHandler
ADC0_IRQHandler
I2C0_IRQHandler
I2C1_IRQHandler
SPI_IRQHandler
ADC1_IRQHandler
SSP0_IRQHandler
SSP1_IRQHandler
USART0_IRQHandler
UART1_IRQHandler
USART2_IRQHandler
USART3_IRQHandler
I2S0_IRQHandler
I2S1_IRQHandler
SPIFI_IRQHandler
SGPIO_IRQHandler
PIN_INT0_IRQHandler
PIN_INT1_IRQHandler
PIN_INT2_IRQHandler
PIN_INT3_IRQHandler
PIN_INT4_IRQHandler
PIN_INT5_IRQHandler
PIN_INT6_IRQHandler
PIN_INT7_IRQHandler
GINT0_IRQHandler
GINT1_IRQHandler
EVENTROUTER_IRQHandler
C_CAN1_IRQHandler
ATIMER_IRQHandler
RTC_IRQHandler
WWDT_IRQHandler
C_CAN0_IRQHandler
QEI_IRQHandler
Default_IRQHandler
        B Default_IRQHandler

/* CRP Section - not needed for flashless devices */

;;;        SECTION .crp:CODE:ROOT(2)
;;;        DATA
/* Code Read Protection
NO_ISP  0x4E697370 -  Prevents sampling of pin PIO0_1 for entering ISP mode
CRP1    0x12345678 - Write to RAM command cannot access RAM below 0x10000300.
                   - Copy RAM to flash command can not write to Sector 0.
                   - Erase command can erase Sector 0 only when all sectors
                     are selected for erase.
                   - Compare command is disabled.
                   - Read Memory command is disabled.
CRP2    0x87654321 - Read Memory is disabled.
                   - Write to RAM is disabled.
                   - "Go" command is disabled.
                   - Copy RAM to flash is disabled.
                   - Compare is disabled.
CRP3    0x43218765 - Access to chip via the SWD pins is disabled. ISP entry
                     by pulling PIO0_1 LOW is disabled if a valid user code is
                     present in flash sector 0.
Caution: If CRP3 is selected, no future factory testing can be
performed on the device.
*/
;;;     DCD     0xFFFFFFFF
;;;

        END
