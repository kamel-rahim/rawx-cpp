#!/bin/bash
main --logtostderr=1 --workingDirectory=`pwd` -OIO_NS="OPENIO"  \
     --access_log="./access_log_test" --error_log="./error_log_test" \
     --server_hostname="oio" --instanceID="OPENIO,test,rawx" &


mkdir CFA
chmod 777 CFA
echo "Test_Download" > CFA/CFA3053F7201F697C0FDFF5BCEB77CE14FDFC0A78EE54E3136841CD6890BEE74
chmod 777 CFA/CFA3053F7201F697C0FDFF5BCEB77CE14FDFC0A78EE54E3136841CD6890BEE74
sleep 1

url="CFA3053F7201F697C0FDFF5BCEB77CE14FDFC0A78EE54E3136841CD6890BEE74"
address=127.0.0.1:11000
range="0-3"
#Simple Download Test
echo "Simple Download"
curl -v -H "X-oio-req-id: Simple-Download" $address/$url 2> tmp_result
grep 'HTTP/1.1 206' tmp_result && echo "Validated" || echo "Simple Download Failed" && rc=1

#Byte Range Download Test
echo "Byte Range Download"
curl -v -H "X-oio-req-id: Byte Range Download" \
-H "Range: bytes=$range" \
$address/$url 2> tmp_result
grep 'HTTP/1.1 206' tmp_result && echo "Validated" || echo "Simple Range Download Failed" && rc=1

#Error Byte Range Test
echo "Error Byte Range"
curl -v -H "X-oio-req-id: Error Byte Range Download" \
-H "Range: bytes=-1-3" \
$address/$url 2> tmp_result
grep 'HTTP/1.1 400' tmp_result && echo "Validated" || echo "Error Range Download Failed" && rc=1
#Error Download Wrong URL Test
echo "Download wrong url" 
curl -v -H "X-oio-req-id: Byte Range Download" \
-H "Range: bytes=$range" \
$address/IMPOSSIBLE 2> tmp_result
grep 'HTTP/1.1 400' tmp_result && echo "Validated" || echo "Error URL Download Failed" && rc=1

#Error Donwload UniqueID not set Test
echo "Download UniqueId not set"
curl -v  $address/$url  2> tmp_result
grep 'HTTP/1.1 400' tmp_result && echo "Validated" || echo "Error Unique ID not set Failed" && rc=1

rm tmp_data
rm -rf ./CFA 
pkill main

exit $rc
