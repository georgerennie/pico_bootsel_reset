#include "pico/stdlib.h"

// This just blinks the LED, however if bootsel_reset is linked with it, 
// the bootsel button is automatically checked, and if pressed, the pico
// resets to boot mode
int main() {
	const uint LED_PIN = 25;
	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);

	while (true) {
		gpio_put(LED_PIN, 1);
		sleep_ms(250);
		gpio_put(LED_PIN, 0);
		sleep_ms(250);
	}
}
