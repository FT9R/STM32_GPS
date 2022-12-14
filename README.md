# STM32 GPS
There are at least 2 common GPS receivers on the market: NEO-6M and E108 GN02D. Current project deal with both. Let's see the default frames from these receivers:
* NEO-6M default frame
> $GPRMC,,V,,,,,,,,,,N*53                                                     
> $GPVTG,,,,,,,,,N*30                                                          
> $GPGGA,,,,,,0,00,99.99,,,,,,*48                                              
> $GPGSA,A,1,,,,,,,,,,,,,99.99,99.99,99.99*30                                
> $GPGSV,1,1,00*79                                                           
> $GPGLL,,,,,,V,N*64
* E108 GN02D default frame
> $GNGGA,014652.271,,,,,0,00,99.99,,,,,,*48                                    
> $GNGLL,,,,,,V,N*7A                                                           
> $GNGSA,A,1,,,,,,,,,,,,,,,,99.99,99.99,99.99,1*1F                            
> $GNGSA,A,1,,,,,,,,,,,,,,,,99.99,99.99,99.99,4*1A                            
> $GPGSV,1,1,00,0*65                                                          
> $BDGSV,1,1,00,0*74                                                          
> $GNRMC,014652.271,V,,,,,0.000,,,,,N,V*07                                   
> $GNVTG,,,,,,,,,N*2E

## Task of the project
So you can see that the frames are different. In addition, the sentences inside the frame go one after the other without time delays. And then, after ~1sec, another frame follows.
The [`minmea`](https://github.com/kosma/minmea) library is capable of taking a single sentence and parsing it. But in the UART RX buffer, the sentences are arranged sequentially.
The task of this project is to pull the necessary sentence from the UART RX buffer and transfer it to minmea routines 
