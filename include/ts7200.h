#pragma once

/*
 * ts7200.h - definitions describing the ts7200 peripheral registers
 *
 * Specific to the TS-7200 ARM evaluation board
 *
 */

#define	TIMER1_BASE	0x80810000
#define	TIMER2_BASE	0x80810020
#define	TIMER3_BASE	0x80810080

#define	LDR_OFFSET	0x00000000	// 16/32 bits, RW
#define	VAL_OFFSET	0x00000004	// 16/32 bits, RO
#define CRTL_OFFSET	0x00000008	// 3 bits, RW
	#define	ENABLE_MASK	0x00000080
	#define	MODE_MASK	0x00000040
	#define	CLKSEL_MASK	0x00000008
#define CLR_OFFSET	0x0000000c	// no data, WO

#define VIC1 0x800B0000
#define VIC2 0x800C0000
	#define IRQSTATUS_OFFSET 0x0 	// one bit for each int source
	#define RAWINTR_OFFSET 0x08		// not masked raw intr input
	#define	INTSELECT_OFFSET 0x0c 	// 0 IRQ, 1: FIQ
	#define INTENABLE_OFFSET 0x10	// 0 is masked, 1 is enabled
	#define INTENCLR_OFFSET 0x14 	// WO, clears bits in INTENABLE
	#define SOFTINT_OFFSET 0x18		// asserts software interrupt
	#define SOFTINTCLR_OFFSET 0x1c	// WO, clears bits in SOFTINT
	#define PROTECTION_OFFSET 0x20 // 0 means allow user mode access
	#define VECTADDR_OFFSET 0x30 	// vectored irq. do not use

// interrupt source #
#define INT_MASK(irq) (1 << (irq & 0x1f))

#define TC1UI 4
#define UART1RXINTR1 23
#define UART1TXINTR1 24
#define UART2RXINTR1 25
#define UART2TXINTR1 26
#define INT_UART1 52
#define INT_UART2 54

#define EVENT_TIMER1 0
#define EVENT_UART1RX 1
#define EVENT_UART1TX 2
#define EVENT_UART2RX 3
#define EVENT_UART2TX 4
#define EVENT_UART1 5
#define EVENT_UART2 6


#define NUM_IRQS 64

#define LED_ADDRESS	0x80840020
	#define LED_NONE	0x0
	#define LED_GREEN	0x1
	#define LED_RED		0x2
	#define LED_BOTH	0x3

#define COM1	0
#define COM2	1

#define IRDA_BASE	0x808b0000
#define UART1_BASE	0x808c0000
#define UART2_BASE	0x808d0000

// All the below registers for UART1
// First nine registers (up to Ox28) for UART 2

#define UART_DATA_OFFSET	0x0	// low 8 bits
	#define DATA_MASK	0xff
#define UART_RSR_OFFSET		0x4	// low 4 bits
	#define FE_MASK		0x1
	#define PE_MASK		0x2
	#define BE_MASK		0x4
	#define OE_MASK		0x8
#define UART_LCRH_OFFSET	0x8	// low 7 bits
	#define BRK_MASK	0x1
	#define PEN_MASK	0x2	// parity enable
	#define EPS_MASK	0x4	// even parity
	#define STP2_MASK	0x8	// 2 stop bits
	#define FEN_MASK	0x10	// fifo
	#define WLEN_MASK	0x60	// word length
#define UART_LCRM_OFFSET	0xc	// low 8 bits
	#define BRDH_MASK	0xff	// MSB of baud rate divisor
#define UART_LCRL_OFFSET	0x10	// low 8 bits
	#define BRDL_MASK	0xff	// LSB of baud rate divisor
#define UART_CTLR_OFFSET	0x14	// low 8 bits
	#define UARTEN_MASK	0x1
	#define MSIEN_MASK	0x8	// modem status int
	#define RIEN_MASK	0x10	// receive int
	#define TIEN_MASK	0x20	// transmit int
	#define RTIEN_MASK	0x40	// receive timeout int
	#define LBEN_MASK	0x80	// loopback
#define UART_FLAG_OFFSET	0x18	// low 8 bits
	#define CTS_MASK	0x1
	#define DCD_MASK	0x2
	#define DSR_MASK	0x4
	#define TXBUSY_MASK	0x8
	#define RXFE_MASK	0x10	// Receive buffer empty
	#define TXFF_MASK	0x20	// Transmit buffer full
	#define RXFF_MASK	0x40	// Receive buffer full
	#define TXFE_MASK	0x80	// Transmit buffer empty
#define UART_INTR_OFFSET	0x1c
	#define UARTRTINTR (1<<3) // timeout, unused
	#define UARTTXINTR (1<<2)
	#define UARTRXINTR (1<<1)
	#define UARTMSINTR (1<<0)
#define UART_DMAR_OFFSET	0x28

// Specific to UART1

#define UART_MDMCTL_OFFSET	0x100
#define UART_MDMSTS_OFFSET	0x104
#define UART_HDLCCTL_OFFSET	0x20c
#define UART_HDLCAMV_OFFSET	0x210
#define UART_HDLCAM_OFFSET	0x214
#define UART_HDLCRIB_OFFSET	0x218
#define UART_HDLCSTS_OFFSET	0x21c
