CLIENT='src/client/client'
SOCKET='tests/output/socket'

echo "Starting clients"

$CLIENT -f $SOCKET -W tests/files/notSoEmptyFile,tests/files/emptyFile -t 200 -R n=1 -d tests/output/client1
$CLIENT -f $SOCKET -w tests/files/can -t 200 -r tests/files/can/you/search/through/folders -R -d tests/output/client2
$CLIENT -f $SOCKET -w tests/files -t 200 -R -d tests/output/client3

echo "Clients ended"