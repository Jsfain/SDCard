clear

#directory to store build/compiled files
buildDir=obj

#directory for source files
sourceDir=source

#make build directory if it doesn't exist
mkdir -p -v $buildDir


t=0.25
# -g = debug, -Os = Optimize Size
Compile=(avr-gcc -Wall -g -Os -mmcu=atmega1280 -c -o)
Link=(avr-gcc -Wall -g -mmcu=atmega1280 -o)
IHex=(avr-objcopy -j .text -j .data -O ihex)


echo -e ">> COMPILE: "${Compile[@]}" "$buildDir"/usart.o " $sourceDir"/usart.c"
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


echo -e "\n\r>> COMPILE: "${Compile[@]}" "$buildDir"/prints.o "$sourceDir"/prints.c"
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
