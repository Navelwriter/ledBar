#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"


#define FIRST_GPIO 2
#define BUTTON1_GPIO (FIRST_GPIO+10)
#define BUTTON2_GPIO (FIRST_GPIO+11)
#define ADC1_GPIO 26


bool state;
const int delayTime = 75;
bool button1Press = false;
bool button2Press = false;

// This array converts a number 0-9 to a bit pattern to send to the GPIOs
int bits[11] = {
  0x0,  // 0
  0x1,  // 1
  0x3,  // 2
  0x7,  // 3
  0xF,  // 4
  0x1F,  // 5
  0x3F,  // 6
  0x7F,  // 7
  0xFF,  // 8
  0x1FF,  // 9
  0x3FF,  // 10
};
static char event_str[128];

void gpio_event_string(char *buf, uint32_t events);
int map(int s, int a1, int a2, int b1, int b2);

int map(int s, int a1, int a2, int b1, int b2) {
  return b1 + (s - a1) * (b2 - b1) / (a2 - a1);
}

void setupGPIO(void){
  for (int gpio = FIRST_GPIO; gpio < FIRST_GPIO + 11; gpio++) {
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_OUT);
    // Our bitmap above has a bit set where we need an LED on, BUT, we are pulling low to light
    // so invert our output
  }
  adc_init();
  adc_gpio_init(ADC1_GPIO);
  adc_select_input(0);
}


void gpio_callback(uint gpio, uint32_t events) {
  if (gpio == 12) {
    button1Press = true;
  }
  else if (gpio == 13) {
    button2Press = true;
  }

}

static const char *gpio_irq_str[] = {
  "LEVEL_LOW",  // 0x1
  "LEVEL_HIGH", // 0x2
  "EDGE_FALL",  // 0x4
  "EDGE_RISE"   // 0x8
};

void gpio_event_string(char *buf, uint32_t events) {
  for (uint i = 0; i < 4; i++) {
    uint mask = (1 << i);
    if (events & mask) {
      // Copy this event string into the user string
      const char *event_str = gpio_irq_str[i];
      while (*event_str != '\0') {
        *buf++ = *event_str++;
      }
      events &= ~mask;

      // If more events add ", "
      if (events) {
        *buf++ = ',';
        *buf++ = ' ';
      }
    }
  }
  *buf++ = '\0';
}

int main() {
  stdio_init_all();
  printf("Hello, 7segment - press button to count down!\n");

  setupGPIO();
  // gpio_set_irq_enabled_with_callback(12, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
  // gpio_set_irq_enabled_with_callback(13, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

  // We are using the button to pull down to 0v when pressed, so ensure that when
  // unpressed, it uses internal pull ups. Otherwise when unpressed, the input will
  // be floating.

  int val = 0;
  int mem = 0;
  while (true) {
    // Count upwards or downwards depending on button input
    // We are pulling down on switch active, so invert the get to make
    // a press count downwards
    uint16_t result = adc_read();
    if (result >= 0 && result <= 0xfff) {
      val = map(result, 0, 0xfff, 0, 10); //Just map the analog input from range 0..10
    }

    // We are starting with GPIO 2, our bitmap starts at bit 0 so shift to start at 2.
    // Set all our GPIOs in one go!
    // If something else is using GPIO, we might want to use gpio_put_masked()
    int32_t mask = bits[val] << FIRST_GPIO;
    gpio_set_mask(mask);
    sleep_ms(50);
    gpio_clr_mask(mask);
  }

  int incrementValue(int val){
    
  }

  return 0;
}