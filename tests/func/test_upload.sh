#!/bin/bash
export PATH=~/rawx-cpp/src:$PATH
main --logtostderr=1 --workingDirectory=`pwd` -OIO_NS="OPENIO"  \
     --access_log="./access_log_test" --error_log="./error_log_test" \
     --server_hostname="oio" --instanceID="OPENIO,test,rawx" &

sleep 1
address="localhost:11000"
url="CFA3053F7201F697C0FDFF5BCEB77CE14FDFC0A78EE54E3136841CD6890BEE74"
data="Test_Upload"
rc=0
#Simple Upload Test
echo "Simple Upload Test"
curl -v -X PUT  \
-d $data \
-H "x-oio-chunk-meta-chunk-id: CFA3053F7201F697C0FDFF5BCEB77CE14FDFC0A78EE54E3136841CD6890BEE74" \
-H "x-oio-chunk-meta-content-storage-policy: SINGLE" \
-H "transfer-encoding: chunked" \
-H "x-oio-chunk-meta-chunk-pos: 0" \
-H "x-oio-chunk-meta-content-id: 45C9A41594490500D73E4AB66A1DD02A" \
-H "x-oio-chunk-meta-content-chunk-method: plain/nb_copy=1" \
-H "x-oio-chunk-meta-content-path: test.txt" \
-H "x-oio-chunk-meta-container-id: CB2D04216603B8274AB831F889EAA4B2656D1EBA45B658712D59C77DAC86E08A" \
-H "x-oio-chunk-meta-content-version: 1488275250661716" \
     $address/$url  2> tmp_result
grep 'HTTP/1.1 201 Created' tmp_result && echo "Validated" || echo "Simple Upload Failed" && rc=1

#Error Upload Same Header used Test
echo "Upload Same Header used Test"
curl -v -X PUT  \
-d $data \
-H "x-oio-chunk-meta-chunk-id: CFA3053F7201F697C0FDFF5BCEB77CE14FDFC0A78EE54E3136841CD6890BEE74" \
-H "x-oio-chunk-meta-content-storage-policy: SINGLE" \
-H "transfer-encoding: chunked" \
-H "x-oio-chunk-meta-chunk-pos: 0" \
-H "x-oio-chunk-meta-content-id: 45C9A41594490500D73E4AB66A1DD02A" \
-H "x-oio-chunk-meta-content-chunk-method: plain/nb_copy=1" \
-H "x-oio-chunk-meta-content-path: test.txt" \
-H "x-oio-chunk-meta-container-id: CB2D04216603B8274AB831F889EAA4B2656D1EBA45B658712D59C77DAC86E08A" \
-H "x-oio-chunk-meta-content-version: 1488275250661716" \
     $address/$url  2> tmp_result
grep 'HTTP/1.1 400 Bad Request' tmp_result && echo "Validated" || (echo "Upload Same Header Test Failed" && rc=1)
rm -rf `pwd`/CFA 

#TODO Wrong path Upload
echo "Wrong path Upload"
curl -v -X PUT  \
-d $data \
-H "x-oio-chunk-meta-chunk-id: 4444453F7201F697C0FDFF5BCEB77CE14FDFC0A78EE54E3136841CD6890BEE4/" \
-H "x-oio-chunk-meta-content-storage-policy: SINGLE" \
-H "transfer-encoding: chunked" \
-H "x-oio-chunk-meta-chunk-pos: 0" \
-H "x-oio-chunk-meta-content-id: 45C9A41594490500D73E4AB66A1DD02A" \
-H "x-oio-chunk-meta-content-chunk-method: plain/nb_copy=1" \
-H "x-oio-chunk-meta-content-path: test.txt" \
-H "x-oio-chunk-meta-container-id: 555554216603B8274AB831F889EAA4B2656D1EBA45B658712D59C77DAC86E08A" \
-H "x-oio-chunk-meta-content-version: 1488275250661716" \
     $address/4444453F7201F697C0FDFF5BCEB77CE14FDFC0A78EE54E3136841CD6890BEE4/  2> tmp_result

grep 'HTTP/1.1 400 Bad Request' tmp_result && echo "Validated" || (echo "Wrong path Upload Failed" && rc=1)
rm -rf `pwd`/444

#Error Upload Wrong Header Test
echo "Upload Wrong Header Test"
curl -v -X PUT \
-d $data \
-H "x-oio-chunk-meta-chunk-id: CFA3053F7201F697C0FDFF5BCEB77CE14FDFC0A78EE54E3136841CD6890BEE74" \
-H "x-oio-chunk-meta-content-storage-policy: SINGLE" \
-H "transfer-encoding: chunked" \
-H "x-oio-chunk-meta-chunk-pos: 0" \
-H "x-oio-chunk-meta-content-path: test.txt" \
-H "x-oio-chunk-meta-container-id: CB2D04216603B8274AB831F889EAA4B2656D1EBA45B658712D59C77DAC86E08A" \
-H "x-oio-chunk-meta-content-version: 1488275250661716" \
$address/TEST 2> tmp_result 
    grep 'HTTP/1.1 400 Bad Request' tmp_result && echo "Validated" || (echo "Upload Wrong Header Failed" && rc=1)
rm -rf `pwd`/CFA   
rm -rf `pwd`/CFA `pwd`/444
rm tmp_result
rm access_log_test error_log_test
pkill main

exit $rc
