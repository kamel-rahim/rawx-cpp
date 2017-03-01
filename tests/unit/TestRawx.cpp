/**
 * This file is part of the OpenIO client libraries
 * Copyright (C) 2017 OpenIO SAS
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 */



#include <glog/logging.h>
#include <gflags/gflags.h>
#include <gtest/gtest.h>
#include <typeinfo>
#include "rawx.hpp"

using rawx::RawxHandlerFactory;
using rawx::DownloadHandler;
using rawx::UploadHandler;
using rawx::RemovalHandler;
using proxygen::RequestHandler;
using proxygen::HTTPMessage;
using proxygen::HTTPMethod;
using proxygen::HTTPHeaders;

class RawxHandlerFactoryFixture : public testing::Test {
 public:
    void SetUp() override {}
    void TearDown() override {}
 protected:
    RawxHandlerFactory handlerFactory;
};

class DownloadHandlerFixture : public testing::Test {
 public:
    void SetUp() override {}
    void TearDown() override {}
 protected:
    DownloadHandler downloadHandler;
};

class UploadHandlerFixture : public testing::Test {
 public:
    void SetUp() override {}
    void TearDown() override {}
 protected:
    UploadHandler uploadHandler;
};

class RemovalHandlerFixture : public testing::Test {
 public:
    void SetUp() override {}
    void TearDown() override {}
 protected:
    RemovalHandler removalHandler;
};

// Testing RawxHandlerFactory
TEST_F(RawxHandlerFactoryFixture, GET_Header_return_DownloadHandler) {
    HTTPMessage msg;
    msg.setMethod(HTTPMethod::GET);
    RequestHandler * handler = handlerFactory.onRequest(nullptr, &msg);
    ASSERT_TRUE(typeid(DownloadHandler).hash_code()
                ==  typeid(*handler).hash_code());
}

TEST_F(RawxHandlerFactoryFixture, PUT_Header_return_UploadHandler) {
    HTTPMessage msg;
    msg.setMethod(HTTPMethod::PUT);
    RequestHandler * handler = handlerFactory.onRequest(nullptr, &msg);
    ASSERT_TRUE(typeid(UploadHandler).hash_code()
                ==  typeid(*handler).hash_code());
}

TEST_F(RawxHandlerFactoryFixture, DELETE_Header_return_RemovalHandler) {
    HTTPMessage msg;
    msg.setMethod(HTTPMethod::DELETE);
    RequestHandler * handler = handlerFactory.onRequest(nullptr, &msg);
    ASSERT_TRUE(typeid(RemovalHandler).hash_code()
                ==  typeid(*handler).hash_code());
}

// Testing DownloadHandler
TEST_F(DownloadHandlerFixture, CheckAllValueHere) {
    HTTPMessage msg;
    HTTPHeaders headers = msg.getHeaders();
// GET //09C7861345B1834C36C7493A2322A3D4C0237FADB407E68155F51E5066922972
    // HTTP/1.1
// Host: 127.0.0.1:6010
// Accept-Encoding: identity
// Range: bytes=0-13
// X-oio-req-id: 524364949D2352A824071FBBFF7D24FC
}

TEST_F(DownloadHandlerFixture, CheckMissingValue) {
    HTTPMessage msg;
    HTTPHeaders headers = msg.getHeaders();
}

// Testing UploadHandler
TEST_F(UploadHandlerFixture, CheckAllValueHere) {
    HTTPMessage msg;
    HTTPHeaders &headers = msg.getHeaders();
    //    PUT //09C7861345B1834C36C7493A2322A3D4C0237FADB407E68155F51E5066922972
    // HTTP/1.1
    //            Host: 127.0.0.1:6010
    headers.rawAdd("Accept-Encoding", "identity");
    headers.rawAdd("x-oio-chunk-meta-chunk-id",
                   "09C7861345B1834C36C7493A2322A3D4C"
                   "0237FADB407E68155F51E5066922972");
    headers.rawAdd("x-oio-chunk-meta-content-storage-policy", "SINGLE");
    headers.rawAdd("transfer-encoding", "chunked");
    headers.rawAdd("x-oio-chunk-meta-chunk-pos", "0");
    headers.rawAdd("x-oio-chunk-meta-content-id",
                   "B2A1AC8E32490500F89F561826502BC8");
    headers.rawAdd("x-oio-chunk-meta-content-chunk-method", "plain/nb_copy=1");
    headers.rawAdd("x-oio-chunk-meta-content-path", "test.txt");
    headers.rawAdd("x-oio-chunk-meta-container-id",
                   "CB2D04216603B8274AB831F889EAA4B265"
                   "6D1EBA45B658712D59C77DAC86E08A");
    headers.rawAdd("x-oio-chunk-meta-content-version", "1487856374424018");
    ASSERT_TRUE(uploadHandler.headerCheck(&msg));
}

// TEST_F(UploadHandlerFixture, CheckMissingValue) {
//     // TODO(KR)
// }

// // Testing RemovalHandler
// TEST_F(RemovalHandlerFixture, CheckAllValueHere) {
//     // TODO(KR)
// }

// TEST_F(RemovalHandlerFixture, CheckMissingValue) {
//     // TODO(KR)
// }

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
