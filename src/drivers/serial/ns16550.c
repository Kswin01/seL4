#include <config.h>
#include <stdint.h>
#include <util.h>
#include <machine/io.h>
#include <plat/machine/devices_gen.h>

/* Registers are all 8 bits in length */
#define UDBL 0x00       /* UART divisor least significant byte register */
#define URBR 0x00       /* UART reciever register buffer */
#define UTHR 0x00       /* UART transmitter holding register */
#define UDMB 0x01       /* UART divisor most significant byte register */
#define UIER 0x01       /* UART interrupt enable register */
#define UAFR 0x02       /* UART alternate function register */
#define UFCR 0x02       /* UART FIFO control register */
#define UIIR 0x02       /* UART interrupt ID register */
#define ULCR 0x03       /* UART line control register */
#define ULSR 0x05       /* UART line status register */
#define USCR 0x07       /* UART scratch register */
#define UDSR 0x10       /* UART DMA status register */

#define UART_ULSR_UTHR_EMPTY    0x20
#define UART_ULSR_RXFIFO_RDR    BIT(7)
#define UART_REG(x) ((volatile uint8_t *)(UART_PPTR + (x)))

#ifdef CONFIG_PRINTING
void uart_drv_putchar(unsigned char c)
{
    while (!(*UART_REG(ULSR) & UART_ULSR_UTHR_EMPTY));
    *UART_REG(UTHR) = c;
}
#endif /* CONFIG_PRINTING */

#ifdef CONFIG_DEBUG_BUILD
unsigned char uart_drv_getchar(void)
{
    while (!(*UART_REG(ULSR) & UART_ULSR_RXFIFO_RDR));
    return *UART_REG(URBR);
}
#endif /* CONFIG_DEBUG_BUILD */
