#include "wiznet_spi.h"

#ifndef WIZNET_DRIVER
#define WIZNET_DRIVER W5500
#define _WIZCHIP_ WIZNET_DRIVER
#endif

#define WIZNET_HW_SPI 1
#define WIZNET_SW_SPI 2

#ifndef WIZNET_BUS
#define WIZNET_BUS WIZNET_HW_SPI
#endif

#ifndef WIZNET_CS
#define WIZNET_CS SPI_CS
#endif

#if (WIZNET_BUS == WIZNET_HW_SPI)
HARDSPI(wiz_spi_port, 14000000UL, 0, mcu_spi_port);
#define WIZNET_SPI &wiz_spi_port
#endif

/**
 * 
 * SPI callback functions
 * 
 */

void wiznet_init(void){
	io_set_output(WIZNET_CS);
	spi_config_t conf = {0};
	softspi_config(WIZNET_SPI, conf, 14000000UL);
}

void wiznet_critical_section_enter(void)
{
	mcu_disable_global_isr();
}

void wiznet_critical_section_exit(void)
{
	mcu_enable_global_isr();
}

void wiznet_cs_select(void)
{
	io_clear_output(WIZNET_CS);
	softspi_start(WIZNET_SPI);
}

void wiznet_cs_deselect(void)
{
	softspi_stop(WIZNET_SPI);
	io_set_output(WIZNET_CS);
}

uint8_t wiznet_getc(void)
{
	return softspi_xmit(WIZNET_SPI, 0xFF);
}

void wiznet_putc(uint8_t c)
{
	softspi_xmit(WIZNET_SPI, c);
}

void wiznet_read(uint8_t *buff, uint16_t len)
{
	while (len--)
	{
		*buff = softspi_xmit(WIZNET_SPI, 0xFF);
		buff++;
	}
}

void wiznet_write(uint8_t *buff, uint16_t len)
{
	while (len--)
	{
		softspi_xmit(WIZNET_SPI, *buff);
		buff++;
	}
}