[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/aleph-five/ringslice)

# Introduction

Ringslice is a library for working with slices of ring buffers. It provides a set
of methods for manipulating slices as if they were strings, including searching,
comparing, and getting a substring. Additionally, it allows
for working with slices as if they were simple arrays of bytes, thus allowing
for working with binary data, not just strings.
It is particularly useful in embedded applications where memory is limited,
and you need to work with circular buffers efficiently.

# Motivation

In some applications, receiving data from uart is organized as an interrupt
service routine (ISR) that fills a ring buffer.
The data is then processed in the main loop, where it is often necessary to
search for substrings, compare them, or extract parts of the data.
There are several approaches for this:
1. Copy the data to another buffer and work with it as a string. This approach requires
additional memory for the buffer and processing time for copying data.
2. Search for amount of data in the ring buffer and, if found, copy it
to another buffer onto stack and then work with it as a string, which
requires stack usage and additional processing time for copying data.

Ringslice provides a convenient way to work with such data without copying it
to another buffer, thus saving memory and processing time.
 
# Usage

- Add the files of src directory to your project
- Add the files of config directory to your project (edit [ringslice_config.h](./src/config/ringslice_config.h) if you need)
- Add the `DBC_fault_handler()` function implementation to your project
- Add the [ringslice.c](./src/ringslice.c) source file in your project
- If there is a need, add the [ringslice_scanf.c](./src/ringslice_scanf.c) source file in your project
- Include the header file [ringslice.h](./src/ringslice.h) in your source files where you want to use the library
- Use the provided methods to work with slices of ring buffers

# Documentation

1. Make sure you have doxygen installed on your computer
2. Simply run `doxygen Doxyfile` in [doc](./doc) directory

# Tests

## On host computer

1. Make sure that you have gcc installed on your computer
2. Simply run `make` in [test](./test) directory

## On target platform

1. Add ringslice source files, see [Usage](#Usage), and containing of [test](./test) directory in your test project (excluding [et_host.c](./test/et/et_host.c) file)
2. Implement the `ET_onInit()`, `ET_onPrintChar()` and `ET_onExit()` platform-dependent functions defined in [et_host.c](./test/et/et_host.c) file

# Example of usage

The following primitive example demonstrates how to use the ringslice library
to process data received from a UART in an embedded application.
 
```c
    #include "ringslice.h"
    #include "bsp.h" // Header containing uart related functions,
                     // such as bsp_init(), uart_data_available(), uart_read_byte()
                     // and DBC_fault_handler() implementation
    #include "application.h" // Example application header for application logic,
                             // needed for app_process_registration_status() function

    #define RINGBUFFER_SIZE 256
    static uint8_t ring_buffer[RINGBUFFER_SIZE]; // Example ring buffer
    static volatile int head = 0; // Head index for the ring buffer
    static volatile int tail = 0; // Tail index for the ring buffer

    void uart_isr(void) {
        // Example ISR that fills the ring buffer
        while (uart_data_available()) {
            ring_buffer[head] = uart_read_byte();
            head = (head + 1) % RINGBUFFER_SIZE; // Wrap around
        }
    }

    void process_data(void) {
        // Create a ringslice from the ring buffer
        ringslice_t rs = ringslice_initializer(ring_buffer, RINGBUFFER_SIZE, tail, head);
    
        // Check if the ringslice is empty
        if (ringslice_is_empty(&rs)) {
            return; // Nothing to process
        }
        
        static int processed_bytes = 0; // Counter for processed bytes
        
    
        // Search for an end of line sequence "\r\n"
        ringslice_t found = ringslice_subslice_with_suffix(&rs, processed_bytes, "\r\n");
        if (!ringslice_is_empty(&found)) {
            int arg1, arg2;
            int argc = ringslice_scanf(&found, "+CREG: %d, %d", &arg1, &arg2); // Example of parsing data
            if(argc == 2) {
                // Process the parsed data
                app_process_registration_status(arg1, arg2);
            }
            // Reset processed bytes counter
            processed_bytes = 0;
            // Update tail index after processing
            tail = (tail + ringslice_len(&rs)) % RINGBUFFER_SIZE; // Wrap around
        }
        else {
            processed_bytes = ringslice_len(&rs) - 1; // Update processed bytes counter
                                                      // for optimal next search
        }
    }
    
    int main (void) {
        // Initialize UART and other peripherals
        bsp_init();
    
        // Main loop
        while (1) {
            // Process the data in the ring buffer
            process_data();
            
            // Other application logic...
            app_process();
        }
    }
```
