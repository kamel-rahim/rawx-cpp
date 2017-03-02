#!/bin/bash
address=$1
url=$2
#Simple Download Test
curl -v -H "X-oio-req-id : Simple Download" $address/$url
#Byte Range Download Test
curl -v -H "X-oio-req-id : Byte Range Download"\
-H "Range : bytes$range"\
$address/$url
#Error Byte Range Test
curl -v -H "X-oio-req-id : Error Byte Range Download"\
-H "Range : bytes-1-3"\
$address/$url
#Error Download Wrong URL Test
curl -v -H "X-oio-req-id : Byte Range Download"\
-H "Range : bytes$range"\
$address/IMPOSSIBLE
#Error Donwload UniqueID not set Test
curl -v -H $address/$url
