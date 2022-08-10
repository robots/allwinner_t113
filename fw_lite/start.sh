make && make bin || exit -1
./xfel ddr t113-s3
./xfel write 0x40000000 ./FLASH_RUN/mangopi/mangopi.bin
./xfel exec 0x40000000

