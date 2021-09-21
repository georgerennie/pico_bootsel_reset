// SPDX-FileCopyrightText: 2021 George Rennie
// SPDX-License-Identifier: BSD-3-Clause

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/sync.h"
#include "hardware/structs/ioqspi.h"
#include "hardware/structs/sio.h"
#include "pico/bootrom.h"

// Allow user override of the LED mask and check time (in ms)
#ifndef USB_BOOT_LED_ACTIVITY_MASK
#define USB_BOOT_LED_ACTIVITY_MASK 0
#endif
#ifndef BOOTSEL_RESET_PERIOD_MS
#define BOOTSEL_RESET_PERIOD_MS 100
#endif

// This function taken from pico-examples/src/picoboard/button/button.c
// Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
// SPDX-License-Identifier: BSD-3-Clause
//
// Picoboard has a button attached to the flash CS pin, which the bootrom
// checks, and jumps straight to the USB bootcode if the button is pressed
// (pulling flash CS low). We can check this pin in by jumping to some code in
// SRAM (so that the XIP interface is not required), floating the flash CS
// pin, and observing whether it is pulled low.
//
// This doesn't work if others are trying to access flash at the same time,
// e.g. XIP streamer, or the other core.

static bool __no_inline_not_in_flash_func(get_bootsel_button)() {
	const uint CS_PIN_INDEX = 1;

	// Must disable interrupts, as interrupt handlers may be in flash, and we
	// are about to temporarily disable flash access!
	uint32_t flags = save_and_disable_interrupts();

	// Set chip select to Hi-Z
	hw_write_masked(
		&ioqspi_hw->io[CS_PIN_INDEX].ctrl,
		GPIO_OVERRIDE_LOW << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
		IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS
	);

	// Note we can't call into any sleep functions in flash right now
	for (volatile int i = 0; i < 500; ++i);

	// The HI GPIO registers in SIO can observe and control the 6 QSPI pins.
	// Note the button pulls the pin *low* when pressed.
	bool button_state = !(sio_hw->gpio_hi_in & (1u << CS_PIN_INDEX));

	// Need to restore the state of chip select, else we are going to have a
	// bad time when we return to code in flash!
	hw_write_masked(
		&ioqspi_hw->io[CS_PIN_INDEX].ctrl,
		GPIO_OVERRIDE_NORMAL << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
		IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS
	);

	restore_interrupts(flags);

	return button_state;
}

static bool repeating_timer_callback(struct repeating_timer *t) {
	if (get_bootsel_button()) {
		reset_usb_boot(USB_BOOT_LED_ACTIVITY_MASK, 0);
	}
	return true;
}

static struct repeating_timer timer;
static void __attribute__((constructor)) bootsel_reset() {
	add_repeating_timer_ms(
		BOOTSEL_RESET_PERIOD_MS,
		repeating_timer_callback,
		NULL,
		&timer
	);
}

