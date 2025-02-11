
/*
	Name: esp8266_spi_shifter.c
	Description: Implements a 74HC595 shifter using HW SPI for ESP8266.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 04-02-2025

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../../cnc.h"
#include "../../../modules/ic74hc595.h"

#if (MCU == MCU_ESP8266)

#ifdef IC74HC595_CUSTOM_SHIFT_IO

#ifndef ITP_SAMPLE_RATE
#define ITP_SAMPLE_RATE (F_STEP_MAX * 2)
#endif

volatile uint8_t ic74hc595_mode;
volatile uint32_t ic74hc595_spi_pins;

DECL_BUFFER(uint32_t, ic74hc595_data, 250); // 1ms data

extern IRAM_ATTR void mcu_update_outputs(void);
extern IRAM_ATTR void mcu_itp_isr(void);

IRAM_ATTR void mcu_spi_isr()
{
	static uint32_t last_io_value;
	uint32_t io_value = last_io_value;
	if (SPIIR & (1 << SPII1))
	{
		mcu_set_output(SPI_CS);
		// SPI1 Interrupt
		SPI1S &= ~(0x1F); // Disable and clear all interrupts on SPI1
		if (!BUFFER_EMPTY(ic74hc595_data))
		{
			BUFFER_DEQUEUE(ic74hc595_data, &io_value);
		}

		// send next value
		SPI1W0 = io_value;
		mcu_clear_output(SPI_CS);
		SPI1S |= SPISTRIE;
		SPI1CMD |= SPIBUSY;
	}
}

void ic74hc595_config_spi(uint32_t samplerate)
{
#ifdef MCU_HAS_SPI
	spi_config_t conf = {0};
	mcu_spi_config(conf, samplerate);
	SPI1U1 = (((IC74HC595_COUNT * 8) - 1) << SPILMOSI) | (((IC74HC595_COUNT * 8) - 1) << SPILMISO);
	mcu_set_output(SPI_CS);
#endif
}

void ic74hc595_shift_init(void)
{
#ifdef MCU_HAS_SPI
	ic74hc595_config_spi(20000000);
	ic74hc595_mode = ITP_STEP_MODE_DEFAULT | ITP_STEP_MODE_SYNC;
#endif
}

IRAM_ATTR void ic74hc595_shift_io_pins(void)
{
	if (SPI1CMD & SPIBUSY)
	{
		return;
	}
	mcu_set_output(SPI_CS);
	SPI1W0 = *((volatile uint32_t *)ic74hc595_io_pins);
	mcu_clear_output(SPI_CS);
	SPI1CMD |= SPIBUSY;
}

// implements the custom step mode function to switch between buffered stepping and realtime stepping
uint8_t itp_set_step_mode(uint8_t mode)
{
	uint8_t last_mode = ic74hc595_mode;
	itp_sync();
	__ATOMIC__
	{
#ifdef USE_I2S_REALTIME_MODE_ONLY
		ic74hc595_mode = (ITP_STEP_MODE_SYNC | ITP_STEP_MODE_REALTIME);
#else
		ic74hc595_mode = (ITP_STEP_MODE_SYNC | mode);
#endif
	}
	cnc_delay_ms(20);
	return last_mode;
}

MCU_CALLBACK void ic74hc595_shift_fill_buffer(void)
{
	uint32_t mode = ic74hc595_mode;
	mcu_set_output(DOUT2);
	while (mode == ITP_STEP_MODE_DEFAULT && !BUFFER_FULL(ic74hc595_data))
	{
		system_soft_wdt_feed();
		mcu_update_outputs();
		BUFFER_ENQUEUE(ic74hc595_data, (uint32_t *)&ic74hc595_io_pins);
	}
	mcu_clear_output(DOUT2);
}

// process
void esp8266_shifter_dotasks()
{

	uint32_t mode = ic74hc595_mode;

	// updates the working mode
	if (mode & ITP_STEP_MODE_SYNC)
	{
		// wait until buffer is flushed
		if (!BUFFER_EMPTY(ic74hc595_data))
		{
			return;
		}

		switch (mode & ~ITP_STEP_MODE_SYNC)
		{
		case ITP_STEP_MODE_DEFAULT:
			proto_feedback("deafult mode");
			// pause itp timer
			// timer1_disable();
			// reconfigure SPI to send data at a fixed rate
			ic74hc595_config_spi(8000000);
			// reassign the ITP timer interrupt
			timer1_attachInterrupt(ic74hc595_shift_fill_buffer);
			// restart the ITP timer at a fixed rate of 2KHz
			timer1_write((APB_CLK_FREQ / 2000));
			timer1_enable(TIM_DIV1, TIM_EDGE, TIM_LOOP);
			break;
		case ITP_STEP_MODE_REALTIME:
		proto_feedback("realtime mode");
			timer1_disable();
			// reconfigure SPI to send data as fast as possible
			ic74hc595_config_spi(20000000UL);
			// reassign the ITP timer interrupt to the timer ITP
			timer1_attachInterrupt(mcu_itp_isr);
			timer1_write((APB_CLK_FREQ / ITP_SAMPLE_RATE));
			timer1_enable(TIM_DIV1, TIM_EDGE, TIM_LOOP);
			// restart ITP timer at the maximum rate
			break;
		}

		// clear sync flag
		ic74hc595_mode &= ~ITP_STEP_MODE_SYNC;
	}
}

#endif
#endif