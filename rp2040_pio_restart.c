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

/*
 * This is an exercise in setting up, then tearing down and restarting PIO
 * programs on the RP2040. The objective is to set up and set running a PIO
 * program (called "first test"), let it run for a while, then tear it down,
 * reclaim all resources, and then set up and run another program (called
 * "second test"). Repeat indefinitely.
 *
 * I was struggling to get a project working which used a battery of PIO based
 * tests. It was behaving very oddly, and I tracked the issue down using this
 * test code. (The issue was the lack of the injected "JMP 0" when the new
 * program was loaded - see below.)
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "pico/stdlib.h"

#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"

/*
 * The 2 PIO programs. It doesn't really matter what they are for this example.
 * As it happens, I've taken the pio/blink example from the Pico development
 * archive and called that "first test", and the pio/squarewave example and
 * called that "second test". They're set up so I can see their outputs on
 * the scope.
 */
#include "pio_first_test.pio.h"
#include "pio_second_test.pio.h"

/* Outputs - scope these pins */
const uint32_t PIO_FIRST_TEST_TEST_GP  = 15;
const uint32_t PIO_SECOND_TEST_TEST_GP = 16;

/* A third GPIO, toggled to indicate when a test is running */
const uint32_t STATUS_GP               = 14;

/* 200Hz, period is 5.0ms. Measured at 2.5ms with a high signal, then 2.5ms with a low signal (exactly 5.0ms) */
const uint32_t FIRST_TEST_FREQ         = 200;

const uint32_t LED_PIN                 = PICO_DEFAULT_LED_PIN;

void main( void )
{
  /* LED shows we're running */
  gpio_init( LED_PIN );  gpio_set_dir( LED_PIN, GPIO_OUT );
  gpio_put( LED_PIN, 1 );

  /* Configure status indicator GPIO */
  gpio_init( STATUS_GP );  gpio_set_dir( STATUS_GP, GPIO_OUT );
  gpio_put( STATUS_GP, 0 );

  while( true ) 
  {
    PIO test_pio = pio0;

    gpio_put( STATUS_GP, 1 );

    /*
     * Claim a state machine, add the first test program and initialise it.
     * The API call pio_sm_init() does all of this, I think. Rather than use that,
     * I'm spelling it all out just to be clear
     */
    int32_t  first_test_sm        = pio_claim_unused_sm(test_pio, true);
    uint32_t first_test_offset    = pio_add_program(test_pio, &pio_first_test_program);
    pio_first_test_program_init(test_pio, first_test_sm, first_test_offset, PIO_FIRST_TEST_TEST_GP);

    /*
     * According to this post:
     *  https://forums.raspberrypi.com/viewtopic.php?t=341316#p2045387
     * it can be necessary to reset the PIO's program counter. Simply resetting and
     * restarting the state machine doesn't do a JMP 0 (or wherever the PIO program
     * starts) so if a state machine is stopped and a new program loaded into it,
     * that program will start at an arbitrary location in the PIO code.
     */
    pio_sm_exec(test_pio, first_test_sm, pio_encode_jmp(first_test_offset));

    /* This test needs to be given a frequency, so set it running and give it the value */
    pio_sm_set_enabled(test_pio, first_test_sm, true);
    test_pio->txf[first_test_sm] = (clock_get_hz(clk_sys) / (2 * FIRST_TEST_FREQ)) - 3;

    /* Let it run a while */
    sleep_ms(50);

    /* Tear down: disable it, clear the fifos, unclaim the state machine and remove the program */
    pio_sm_set_enabled(test_pio, first_test_sm, false);
    pio_sm_clear_fifos(test_pio, first_test_sm);
    pio_sm_unclaim(test_pio, first_test_sm);
    pio_remove_program(test_pio, &pio_first_test_program, first_test_offset);

    /* Not sure about this, the init code does it. Unneeded for this example, might be for something else */
    pio_sm_clkdiv_restart(test_pio, first_test_sm);



    /* Another pause so the scope can show the gap */
    gpio_put( STATUS_GP, 0 );
    sleep_ms(20);
    gpio_put( STATUS_GP, 1 );



    /* Same again - claim the state machine, load and initialise the second test program */
    int32_t  second_test_sm       = pio_claim_unused_sm(test_pio, true);
    uint32_t second_test_offset   = pio_add_program(test_pio, &pio_second_test_program);
    pio_second_test_program_init(test_pio, second_test_sm, second_test_offset, PIO_SECOND_TEST_TEST_GP);

    pio_sm_exec(test_pio, second_test_sm, pio_encode_jmp(second_test_offset));

    /* No need to poke this test program, just start it and let it run */
    pio_sm_set_enabled(test_pio, second_test_sm, true);
    sleep_ms(50);

    /* Tear down and clean up as before */
    pio_sm_set_enabled(test_pio, second_test_sm, false);
    pio_sm_clear_fifos(test_pio, second_test_sm);
    pio_sm_unclaim(test_pio, second_test_sm);
    pio_remove_program(test_pio, &pio_second_test_program, second_test_offset);
    pio_sm_clkdiv_restart(test_pio, second_test_sm);

    gpio_put( STATUS_GP, 0 );
    sleep_ms(20);
  }
}


