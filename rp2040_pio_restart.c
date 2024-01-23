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

#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"

#include "pio_first_test.pio.h"
#include "pio_second_test.pio.h"

const uint32_t STATUS_GP               = 14;

const uint32_t PIO_FIRST_TEST_TEST_GP  = 15;
const uint32_t PIO_SECOND_TEST_TEST_GP = 16;

/*  16Hz, period is 62.5ms. Measured at 31.3ms with a high signal, then 31.3ms with a low signal (62.6ms) */
/* 100Hz, period is 10.0ms. Measured at  5.0ms with a high signal, then  5.0ms with a low signal (10.0ms) */
/* 200Hz, period is  5.0ms. Measured at  2.5ms with a high signal, then  2.5ms with a low signal ( 5.0ms) */
const uint32_t FIRST_TEST_FREQ         = 200;

const uint32_t LED_PIN                 = PICO_DEFAULT_LED_PIN;

void main( void )
{
  gpio_init( LED_PIN );  gpio_set_dir( LED_PIN, GPIO_OUT );
  gpio_put( LED_PIN, 1 );

  gpio_init( STATUS_GP );  gpio_set_dir( STATUS_GP, GPIO_OUT );
  gpio_put( STATUS_GP, 0 );

  PIO      test_pio             = pio0;


  // Both PIO programs are loaded, both are disabled
  // Both signal GPIOs are at 0

  while( true ) 
  {
    gpio_put( STATUS_GP, 1 );

    int32_t  first_test_sm        = pio_claim_unused_sm(test_pio, true);
    uint32_t first_test_offset    = pio_add_program(test_pio, &pio_first_test_program);
    pio_first_test_program_init(test_pio, first_test_sm, first_test_offset, PIO_FIRST_TEST_TEST_GP);

    pio_sm_set_enabled(test_pio, first_test_sm, true);
    test_pio->txf[first_test_sm] = (clock_get_hz(clk_sys) / (2 * FIRST_TEST_FREQ)) - 3;
    sleep_ms(50);
    pio_sm_set_enabled(test_pio, first_test_sm, false);
    pio_sm_clear_fifos(test_pio, first_test_sm);
    pio_sm_unclaim(test_pio, first_test_sm);
    pio_remove_program(test_pio, &pio_first_test_program, first_test_offset);

    gpio_put( STATUS_GP, 0 );
    sleep_ms(20);
    gpio_put( STATUS_GP, 1 );

    int32_t  second_test_sm       = pio_claim_unused_sm(test_pio, true);
    uint32_t second_test_offset   = pio_add_program(test_pio, &pio_second_test_program);
    pio_second_test_program_init(test_pio, second_test_sm, second_test_offset, PIO_SECOND_TEST_TEST_GP);

    pio_sm_set_enabled(test_pio, second_test_sm, true);
    sleep_ms(50);
    pio_sm_set_enabled(test_pio, second_test_sm, false);
    pio_sm_clear_fifos(test_pio, second_test_sm);
    pio_sm_unclaim(test_pio, first_test_sm);
    pio_remove_program(test_pio, &pio_second_test_program, second_test_offset);

    gpio_put( STATUS_GP, 0 );
    sleep_ms(20);
  }
}


