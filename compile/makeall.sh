make rebuild
make clean
make
cd ../rdserver
make clean
make
kill -9 $(pidof rdserver)
./rdserver 30241 & 
sleep 1