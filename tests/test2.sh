CLIENT='exe/client'
SOCKET='tests/output/test2/socket'

echo "Starting clients"


$CLIENT -f $SOCKET -p -t 200 -w tests/files/algoritmica/Laboratorio/2_lez -t 200 -w tests/files/algoritmica/Laboratorio/3_lez -t 200 -R -d tests/output/test2/client1 > tests/output/test2/client1.log
$CLIENT -f $SOCKET -p -t 200 -w tests/files/algoritmica/Laboratorio/4_lez -t 200 -R -d tests/output/test2/client2 > tests/output/test2/client2.log
$CLIENT -f $SOCKET -p -t 200 -w tests/files/algoritmica -t 200 -R -d tests/output/test2/client3 > tests/output/test2/client3.log