# STM32 GPS
There are at least 2 common GPS receivers on the market: NEO-6M and E108 GN02D. Current project deal with both. Let's see the default frames from these receivers:
* NEO-6M default frame
> $GNGGA,001643.272,,,,,0,00,99.99,,,,,,*4F                                     
> $GNGLL,,,,,,V,N*7A                                                           
> $GNGSA,A,1,,,,,,,,,,,,,,,,99.99,99.99,99.99,1*1F                            
> $GNGSA,A,1,,,,,,,,,,,,,,,,99.99,99.99,99.99,4*1A                            
> $GPGSV,1,1,00,0*65                                                          
> $BDGSV,1,1,00,0*74                                                         
> $GNRMC,001643.272,V,,,,,0.000,,,,,N,V*00                                   
> $GNVTG,,,,,,,,,N*2E
* E108 GN02D default frame
> $GNGGA,001643.272,,,,,0,00,99.99,,,,,,*4F                                     
> $GNGLL,,,,,,V,N*7A                                                           
> $GNGSA,A,1,,,,,,,,,,,,,,,,99.99,99.99,99.99,1*1F                            
> $GNGSA,A,1,,,,,,,,,,,,,,,,99.99,99.99,99.99,4*1A                            
> $GPGSV,1,1,00,0*65                                                          
> $BDGSV,1,1,00,0*74                                                         
> $GNRMC,001643.272,V,,,,,0.000,,,,,N,V*00                                   
> $GNVTG,,,,,,,,,N*2E

So you can see that the frames are different. In addition, the sentences inside the frame go one after the other without time delays. And then, after ~1sec, another frame follows.
The minmea library is capable of taking a single sentence and parsing it. But in the UART RX buffer, the sentences are arranged sequentially.
The task of this project is to pull the necessary sentence from the UART RX buffer and transfer it to minmea 
