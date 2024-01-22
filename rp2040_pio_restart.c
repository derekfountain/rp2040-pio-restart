/*

MIT License

Copyright (c) 2024 Derek Fountain

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/stdlib.h"

#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"

#include "pio1.pio.h"
#include "pio2.pio.h"

const uint32_t PIO1_TEST_GP  = 15;
const uint32_t PIO2_TEST_GP  = 16;

const uint LED_PIN = PICO_DEFAULT_LED_PIN;

void main( void )
{
  gpio_init( PIO1_TEST_GP );  gpio_set_dir( PIO1_TEST_GP, GPIO_OUT );
  gpio_put( PIO1_TEST_GP, 0 );
  gpio_init( PIO2_TEST_GP );  gpio_set_dir( PIO2_TEST_GP, GPIO_OUT );
  gpio_put( PIO2_TEST_GP, 0 );

  gpio_init( LED_PIN );  gpio_set_dir( LED_PIN, GPIO_OUT );
  gpio_put( LED_PIN, 1 );

  stdio_init_all();

  int  pio1_sm   = pio_claim_unused_sm(pio0, true);
  uint offset    = pio_add_program(pio0, &pio1_program);
  pio1_program_init(pio0, pio1_sm, offset, PIO1_TEST_GP);

  pio_sm_set_enabled(pio0, pio1_sm, true);

  int  pio2_sm   = pio_claim_unused_sm(pio1, true);
       offset    = pio_add_program(pio1, &pio2_program);
  pio2_program_init(pio1, pio2_sm, offset, PIO2_TEST_GP);

  pio_sm_set_enabled(pio1, pio2_sm, true);

  pio0->txf[pio1_sm] = (clock_get_hz(clk_sys) / (2 * 4)) - 3;
  pio1->txf[pio2_sm] = (clock_get_hz(clk_sys) / (2 * 8)) - 3;

  while( true ) 
  {
    printf("Tick\n\n");

    sleep_ms(1000);
  }
}
