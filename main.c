/*
 *
 * Andrea Montefusco 2018
 *
 * Test program for DZT 6001: 80A, Single phase kWh meter, LCD, RS485/Modbus
 *
 * Home page: https://www.dutchmeters.com/index.php/product/dzt-6001/
 * User manual: http://dutchmeters.com/wp-content/uploads/2017/06/DZT-6001-manual.pdf
 * Register reference:  http://dutchmeters.com/wp-content/uploads/2017/04/DZT6001-Modbus.pdf
 *
 * Prerequisite: install libmodbus
 *
 * sudo apt-get install libmodbus-dev
 *
 * Compile and run with:
 *
 * gcc -Wall -I/usr/include/modbus test_dzt6001.c  -lmodbus  -o test_dzt6001 && ./test_dzt6001
 *
 * This program has been slightly modified from:
 *
 * https://electronics.stackexchange.com/questions/136646/libmodbus-trouble-with-reading-from-slave
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>

#include <modbus.h>


int main()
{
  modbus_t *ctx = 0;

  //
  // create a libmodbus context for RTU
  // doesn't check if the serial is really there
  //
  ctx = modbus_new_rtu("/dev/ttyUSB0", 9600, 'N', 8, 1);

  if (ctx == 0) {

    fprintf(stderr, "Unable to create the libmodbus context\n");
    return -1;

  } else {
    struct timeval old_response_timeout;
    struct timeval response_timeout;

    // enable debug
    modbus_set_debug(ctx, true);

    // initialize timeouts with default
    modbus_get_response_timeout(ctx, &old_response_timeout);
    response_timeout = old_response_timeout;

    // set the message and charcater timeout to 2 seconds
    response_timeout.tv_sec = 2;
    modbus_set_response_timeout(ctx, &response_timeout);
    modbus_set_byte_timeout(ctx, &response_timeout);

  }

  // try to connet to the first DZT on the line
  // assume that line address is 1, the default
  // send nothing on the line, just set the address in the context
  if(modbus_set_slave(ctx, 1) == -1) {
    fprintf(stderr, "Didn't connect to slave/n");
    return -1;
  }

  // establish a Modbus connection
  // in a RS-485 context that means the serial interface is opened
  // but nothing is yet sent on the line
  if(modbus_connect(ctx) == -1) {

    fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
    modbus_free(ctx);
    return -1;

  } else {

    int nreg = 0;
    uint16_t tab_reg[64];

    fprintf(stderr, "Connected\n");

    //
    // read all registers in DVT 6001
    // the function uses the Modbus function code 0x03 (read holding registers).
    //
    nreg = modbus_read_registers(ctx,0,0x2c,tab_reg);

    if (nreg == -1) {

       fprintf(stderr, "Error reading registers: %s\n", modbus_strerror(errno));
       modbus_close(ctx);
       modbus_free(ctx);

       return -1;

    } else {
       int i;

       // dump all registers content

       fprintf (stderr, "Register dump:\n");
       for(i=0; i < nreg; i++)
         printf("reg #%d: %d\n", i, tab_reg[i]);


       modbus_close(ctx);
       modbus_free(ctx);

       return 0;
    }
  }
}
