#ifndef __LAB4_H__
#define __LAB4_H__

///////////////////////////////////////////////////////////////////////////////
//
// Screen stuff
//
///////////////////////////////////////////////////////////////////////////////
#define TEXT_COLOR      ST7735_16_GREEN
#define BG_COLOR        ST7735_16_BLACK

#define SCREEN_WIDTH    JDT1800_WIDTH
#define SCREEN_HEIGHT   JDT1800_HEIGHT
#define TEXT_WIDTH      6
#define TEXT_HEIGHT     10
#define LEFT_MARGIN     TEXT_WIDTH
#define TOP_MARGIN      TEXT_HEIGHT
// underscore character for cursor
#define CURSOR_CHAR     0x5f

///////////////////////////////////////////////////////////////////////////////
//
// Sw UART
//
///////////////////////////////////////////////////////////////////////////////
#define UART_BAUD          9600
// Bittime = Timer clk/baud = 25000000/9600
#define UART_BITTIME       2604
#define UART_DATA_BITS     8

#define UART_RX_PORT       2
#define UART_RX_PIN        1
#define UART_RX_PIN_BIT    ((1<<UART_RX_PIN))
#define UART_RX_TIMER      LPC_TIM0
#define UART_RX_TIMER_IRQ  TIMER0_IRQn
#define UART_RX_TIMER_PRI  2

#define UART_TX_PORT       2
#define UART_TX_PIN        0
#define UART_TX_PIN_BIT    ((1<<UART_TX_PIN))
#define UART_TX_TIMER      LPC_TIM2
#define UART_TX_TIMER_IRQ  TIMER2_IRQn
#define UART_TX_TIMER_PRI  3

typedef struct
{
   bool receiving;
   uint8_t currentRxBit;
   bool rxDataReady;
   uint8_t rxData;

   bool transmitting;
   uint8_t currentTxBit;
   uint8_t txData;
} SwUart;

///////////////////////////////////////////////////////////////////////////////
//
// Sw SPI
//
///////////////////////////////////////////////////////////////////////////////
// Bittime = Timer clk/spi rate; run SPI at approx 1.5MHz (timer clk is 100MHz/4)
#define LCD_SPI_BITTIME       16
#define LCD_SPI_TX_TIMER      LPC_TIM1
#define LCD_SPI_TX_TIMER_IRQ  TIMER1_IRQn
#define LCD_SPI_TX_TIMER_PRI  2

#define LCD_SPI_MOSI_PIN      (1<<9)
#define LCD_SPI_SCLK_PIN      (1<<7)
#define LCD_SPI_CS_PIN        (1<<18)

typedef enum
{
   STATE_IDLE,
   STATE_TX_CS_LOW,
   STATE_TX_DATA,
   STATE_TX_LAST_CLK,
   STATE_TX_DATA_CLR,
   STATE_TX_CS_HIGH
} SwSPIState;

typedef struct
{
   uint8_t txData;
   SwSPIState state;
   uint8_t currentTxBit;
} SwSPI;

///////////////////////////////////////////////////////////////////////////////
//
// Timer helpers
//
///////////////////////////////////////////////////////////////////////////////
// First match bit in timer registers
#define TIMER_MATCH0_BIT (1<<0)

void SetupTimer(LPC_TIM_TypeDef *timer, uint32_t value, bool repeat);
void EnableTimerIrq(IRQn_Type timerIrq, uint32_t priority);
void DisableTimerIrq(IRQn_Type timerIrq);

#endif //__LAB4_H__
