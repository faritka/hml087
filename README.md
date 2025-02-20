# HML087 emulator using the microprocessor ch32v003 and the library ch32v003fun

The ROM HML087 is used in the BMW E30 coding plug.

This software emulates HML087. 

The communication protocol: when the CS pin is set high by a microprocessor,
the CLOCK pin sends 64 clock signals. On each clock signal going from high to low,
the DATA pin outputs data: 0 or 1.

The data is stored in the array coder for different coding plugs.

Download the library ch32v003fun from https://github.com/cnlohr/ch32v003fun
and set the path to it in Makefile.

Also, select one of the coders in Makefile.

Use a data analyzer to get data for a coder that is not listed. 
   

