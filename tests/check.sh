[ ! -d "tests/output/$1" ] && echo "tests/output/$1 not found." && exit

VE=`cat tests/output/$1/v_client?.log tests/output/$1/valgrind.log | awk -v sum=0 -F'SUMMARY: |errors' '/ERROR SUMMARY/{sum += $2} END {print sum}'`
W=`cat tests/output/$1/client?.log tests/output/$1/server.log | awk -v sum=0 '/Warning: /{sum += 1} END {print sum}'`
E=`cat tests/output/$1/client?.log tests/output/$1/server.log | awk -v sum=0 '/Something went wrong, exiting.../{sum += 1} END {print sum}'`

if [[ -z $2 ]]; then
    echo "# warnings: $W";
    echo "# errors: $E";
    echo "# valgrind_errors: $VE";
else
    echo $((E+VE));
fi