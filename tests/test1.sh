CLIENT='src/client/client'
SOCKET='tests/output/socket'

echo "Starting clients"

$CLIENT -f $SOCKET -p -t 200 -W tests/files/notSoEmptyFile,tests/files/emptyFile -t 200 -R n=1 -d tests/output/client1 > tests/output/client1.log
$CLIENT -f $SOCKET -p -t 200 -w tests/files/can -t 200 -r tests/files/can/you/search/through/folders -R -d tests/output/client2 > tests/output/client2.log
$CLIENT -f $SOCKET -p -t 200 -w tests/files,n=10 -t 200 -R -d tests/output/client3 > tests/output/client3.log