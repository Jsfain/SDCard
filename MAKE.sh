clear

#directory to store build/compiled files
buildDir=../build

#directory for source files
sourceDir=source

#make build directory if it doesn't exist
mkdir -p -v $buildDir


t=0.25
# -g = debug, -Os = Optimize Size
Compile=(avr-gcc -Wall -g -Os -mmcu=atmega1280 -c -o)
Link=(avr-gcc -Wall -g -mmcu=atmega1280 -o)
IHex=(avr-objcopy -j .text -j .data -O ihex)


echo -e ">> COMPILE: "${Compile[@]}" "$buildDir"/test.o " $sourceDir"/test.c"
"${Compile[@]}" $buildDir/test.o $sourceDir/test.c
status=$?
sleep $t
if [ $status -gt 0 ]
then
    echo -e "error compiling TEST.C"
    echo -e "program exiting with code $status"
    exit $status
else
    echo -e "Compiling TEST.C successful"
fi



 echo -e "\n\r>> COMPILE: "${Compile[@]}" "$buildDir"/prints.o" $sourceDir"/prints.c"
"${Compile[@]}" $buildDir/prints.o $sourceDir/prints.c
status=$?
sleep $t
if [ $status -gt 0 ]
then
    echo -e "error compiling PRINTS.C"
    echo -e "program exiting with code $status"
    exit $status
else
    echo -e "Compiling PRINTS.C successful"
fi



echo -e "\n\r>> COMPILE: "${Compile[@]}" "$buildDir"/usart.o "$sourceDir"/usart.c"
"${Compile[@]}" $buildDir/usart.o $sourceDir/usart.c
status=$?
sleep $t
if [ $status -gt 0 ]
then
    echo -e "error compiling USART.C"
    echo -e "program exiting with code $status"
    exit $status
else
    echo -e "Compiling USART.C successful"
fi



echo -e "\n\r>> COMPILE: "${Compile[@]}" "$buildDir"/sd_spi.o" $sourceDir"/sd_spi.c"
"${Compile[@]}" $buildDir/sd_spi.o $sourceDir/sd_spi.c
status=$?
sleep $t
if [ $status -gt 0 ]
then
    echo -e "error compiling SD_SPI.C"
    echo -e "program exiting with code $status"
    exit $status
else
    echo -e "Compiling SD_SPI.C successful"
fi


echo -e "\n\r>> COMPILE: "${Compile[@]}" "$buildDir"/sd_misc.o" $sourceDir"/sd_misc.c"
"${Compile[@]}" $buildDir/sd_misc.o $sourceDir/sd_misc.c
status=$?
sleep $t
if [ $status -gt 0 ]
then
    echo -e "error compiling SD_MISC.C"
    echo -e "program exiting with code $status"
    exit $status
else
    echo -e "Compiling SD_MISC.C successful"
fi



echo -e "\n\r>> COMPILE: "${Compile[@]}" "$buildDir"/spi.o "$sourceDir"/spi.c"
"${Compile[@]}" $buildDir/spi.o $sourceDir/spi.c
status=$?
sleep $t
if [ $status -gt 0 ]
then
    echo -e "error compiling SPI.C"
    echo -e "program exiting with code $status"
    exit $status
else
    echo -e "Compiling SPI.C successful"
fi



echo -e "\n\r>> LINK: "${Link[@]}" "$buildDir"/test.elf "$buildDir"/test.o  "$buildDir"/spi.o "$buildDir"/sd_spi.o "$buildDir"/sd_misc.o "$buildDir"/usart.o "$buildDir"/prints.o "
"${Link[@]}" $buildDir/test.elf $buildDir/test.o $buildDir/spi.o $buildDir/sd_spi.o $buildDir/sd_misc.o $buildDir/usart.o $buildDir/prints.o
status=$?
sleep $t
if [ $status -gt 0 ]
then
    echo -e "error during linking"
    echo -e "program exiting with code $status"
    exit $status
else
    echo -e "Linking successful. Output in TEST.ELF"
fi



echo -e "\n\r>> GENERATE INTEL HEX File: "${IHex[@]}" "$buildDir"/test.elf "$buildDir"/test.hex"
"${IHex[@]}" $buildDir/test.elf $buildDir/test.hex
status=$?
sleep $t
if [ $status -gt 0 ]
then
    echo -e "error generating HEX file"
    echo -e "program exiting with code $status"
    exit $status
else
    echo -e "HEX file successfully generated. Output in TEST.HEX"
fi



echo -e "\n\r>> DOWNLOAD HEX FILE TO AVR"
echo "avrdude -p atmega1280 -c dragon_jtag -U flash:w:test.hex:i -P usb"
avrdude -p atmega1280 -c dragon_jtag -U flash:w:$buildDir/test.hex:i -P usb
status=$?
sleep $t
if [ $status -gt 0 ]
then
    echo -e "error during download of HEX file to AVR"
    echo -e "program exiting with code $status"
    exit $status
else
    echo -e "Program successfully downloaded to AVR"
fi

