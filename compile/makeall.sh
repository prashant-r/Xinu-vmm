make rebuild
make clean
make
cd ../rdserver
cat /dev/null > backing_store
make clean
make
kill -9 $(pidof rdserver)
./rdserver 30243
sleep 1
