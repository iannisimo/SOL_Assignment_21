CLIENT='exe/client'
SOCKET='tests/output/socket'

echo "Starting clients"


$CLIENT -f $SOCKET -p -t 200 -w tests/files/algoritmica/Laboratorio/2_lez -t 200 -w tests/files/algoritmica/Laboratorio/3_lez -t 200 -R -d tests/output/client1 > tests/output/client1.log
$CLIENT -f $SOCKET -p -t 200 -w tests/files/algoritmica/Laboratorio/4_lez -t 200 -R -d tests/output/client2 > tests/output/client2.log
$CLIENT -f $SOCKET -p -t 200 -w tests/files/algoritmica -t 200 -R -d tests/output/client3 > tests/output/client3.log