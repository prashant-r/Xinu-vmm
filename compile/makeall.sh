make rebuild
make clean
make
cd ../rdserver
make clean
make
kill -9 $(pidof rdserver)
./rdserver 30245 &
sleep 1