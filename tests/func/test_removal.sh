address=$1
$url = /CFA3053F7201F697C0FDFF5BCEB77CE14FDFC0A78EE54E3136841CD6890BEE74;
#Simple Removal Test
curl -X DELETE \
$address/$url
#Wrong path Removal Test
curl -X DELETE \
$address/"Does not exist"
