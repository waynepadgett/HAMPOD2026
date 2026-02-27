cd Firmware/
make clean
make
cd ..
cd Software/
make clean
make LEVEL=2
cd ..
cd Firmware/
./firmware.elf &
cd ..
cd Software/
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --log-file=valgrind.log ./Software.elf

