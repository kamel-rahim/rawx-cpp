#!/bin/bash
address=$1
url = /CFA3053F7201F697C0FDFF5BCEB77CE14FDFC0A78EE54E3136841CD6890BEE74;
data = "Test_Upload";
#Simple Upload Test
curl -X PUT \
-d $data\
-H "x-oio-chunk-meta-chunk-id: CFA3053F7201F697C0FDFF5BCEB77CE14FDFC0A78EE54E3136841CD6890BEE74"\
-H "x-oio-chunk-meta-content-storage-policy: SINGLE"\
-H "transfer-encoding: chunked"\
-H "x-oio-chunk-meta-chunk-pos: 0"\
-H "x-oio-chunk-meta-content-id: 45C9A41594490500D73E4AB66A1DD02A"\
-H "x-oio-chunk-meta-content-chunk-method: plain/nb_copy=1"\
-H "x-oio-chunk-meta-content-path: test.txt"\
-H "x-oio-chunk-meta-container-id: CB2D04216603B8274AB831F889EAA4B2656D1EBA45B658712D59C77DAC86E08A"\
-H "x-oio-chunk-meta-content-version: 1488275250661716"\
$address/$url
#TODO Wrong path Upload
curl -X PUT \
-d $data\
-H "x-oio-chunk-meta-chunk-id: 4444453F7201F697C0FDFF5BCEB77CE14FDFC0A78EE54E3136841CD6890BEE4/"\
-H "x-oio-chunk-meta-content-storage-policy: SINGLE"\
-H "transfer-encoding: chunked"\
-H "x-oio-chunk-meta-chunk-pos: 0"\
-H "x-oio-chunk-meta-content-id: 45C9A41594490500D73E4AB66A1DD02A"\
-H "x-oio-chunk-meta-content-chunk-method: plain/nb_copy=1"\
-H "x-oio-chunk-meta-content-path: test.txt"\
-H "x-oio-chunk-meta-container-id: 555554216603B8274AB831F889EAA4B2656D1EBA45B658712D59C77DAC86E08A"\
-H "x-oio-chunk-meta-content-version: 1488275250661716"\
$address/4444453F7201F697C0FDFF5BCEB77CE14FDFC0A78EE54E3136841CD6890BEE4/
#Error Upload Same Header used Test
curl -X PUT \
-d $data\
-H "x-oio-chunk-meta-chunk-id: CFA3053F7201F697C0FDFF5BCEB77CE14FDFC0A78EE54E3136841CD6890BEE74"\
-H "x-oio-chunk-meta-content-storage-policy: SINGLE"\
-H "transfer-encoding: chunked"\
-H "x-oio-chunk-meta-chunk-pos: 0"\
-H "x-oio-chunk-meta-content-id: 45C9A41594490500D73E4AB66A1DD02A"\
-H "x-oio-chunk-meta-content-chunk-method: plain/nb_copy=1"\
-H "x-oio-chunk-meta-content-path: test.txt"\
-H "x-oio-chunk-meta-container-id: CB2D04216603B8274AB831F889EAA4B2656D1EBA45B658712D59C77DAC86E08A"\
-H "x-oio-chunk-meta-content-version: 1488275250661716"\
$address/$url
#Error Upload Wrong Header Test
curl -X PUT TEST\
-d $data\
-H "x-oio-chunk-meta-chunk-id: CFA3053F7201F697C0FDFF5BCEB77CE14FDFC0A78EE54E3136841CD6890BEE74"\
-H "x-oio-chunk-meta-content-storage-policy: SINGLE"\
-H "transfer-encoding: chunked"\
-H "x-oio-chunk-meta-chunk-pos: 0"\
-H "x-oio-chunk-meta-content-path: test.txt"\
-H "x-oio-chunk-meta-container-id: CB2D04216603B8274AB831F889EAA4B2656D1EBA45B658712D59C77DAC86E08A"\
-H "x-oio-chunk-meta-content-version: 1488275250661716"\
$address/$url    
