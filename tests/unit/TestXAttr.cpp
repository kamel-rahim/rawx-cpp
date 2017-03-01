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
#include "utils.hpp"

using utils::XAttr;

class XAttrFixture : public testing::Test {
 public:
    void SetUp() override {}
    void TearDown() override {}
 protected:
    XAttr xattr;
};


// TESTING CONVERSION FROM HTTP TO XATTR NAMES

TEST_F(XAttrFixture, HTTP_2_XAttr_content_container) {
    std::string test_message {"TEST_CONTENT_CONTAINER"};
    std::string xattr_field_name {"content.container"};
    std::string http_field_name {"container-id"};
    xattr.addHTTP(http_field_name, test_message);
    ASSERT_EQ(test_message, xattr.getXAttr(xattr_field_name));
}
TEST_F(XAttrFixture, HTTP_2_XAttr_content_id) {
    std::string test_message {"TEST_CONTENT_ID"};
    std::string xattr_field_name {"content.id"};
    std::string http_field_name {"content-id"};
    xattr.addHTTP(http_field_name, test_message);
    ASSERT_EQ(test_message, xattr.getXAttr(xattr_field_name));
}
TEST_F(XAttrFixture, HTTP_2_XAttr_content_path) {
    std::string test_message {"TEST_CONTENT_PATH"};
    std::string xattr_field_name {"content.path"};
    std::string http_field_name {"content-path"};
    xattr.addHTTP(http_field_name, test_message);
    ASSERT_EQ(test_message, xattr.getXAttr(xattr_field_name));
}
TEST_F(XAttrFixture, HTTP_2_XAttr_content_storage_policy) {
    std::string test_message {"TEST_CONTENT_STORAGE_POLICY"};
    std::string xattr_field_name {"content.storage_policy"};
    std::string http_field_name {"content-storage-policy"};
    xattr.addHTTP(http_field_name, test_message);
    ASSERT_EQ(test_message, xattr.getXAttr(xattr_field_name));
}
TEST_F(XAttrFixture, HTTP_2_XAttr_content_chunk_method) {
    std::string test_message {"TEST_CONTENT_CHUNK_METHOD"};
    std::string xattr_field_name {"content.chunk_method"};
    std::string http_field_name {"content-chunk-method"};
    xattr.addHTTP(http_field_name, test_message);
    ASSERT_EQ(test_message, xattr.getXAttr(xattr_field_name));
}
TEST_F(XAttrFixture, HTTP_2_XAttr_content_metachunk_size) {
    std::string test_message {"TEST_METACHUNK_SIZE"};
    std::string xattr_field_name {"metachunk.size"};
    std::string http_field_name {"metachunk-size"};
    xattr.addHTTP(http_field_name, test_message);
    ASSERT_EQ(test_message, xattr.getXAttr(xattr_field_name));
}
TEST_F(XAttrFixture, HTTP_2_XAttr_content_metachunk_hash) {
    std::string test_message {"TEST_METACHUNK_HASH"};
    std::string xattr_field_name {"metachunk.hash"};
    std::string http_field_name {"metachunk-hash"};
    xattr.addHTTP(http_field_name, test_message);
    ASSERT_EQ(test_message, xattr.getXAttr(xattr_field_name));
}
TEST_F(XAttrFixture, HTTP_2_XAttr_chunk_id) {
    std::string test_message {"TEST_CHUNK_ID"};
    std::string xattr_field_name {"chunk.id"};
    std::string http_field_name {"chunk-id"};
    xattr.addHTTP(http_field_name, test_message);
    ASSERT_EQ(test_message, xattr.getXAttr(xattr_field_name));
}
TEST_F(XAttrFixture, HTTP_2_XAttr_chunk_hash) {
    std::string test_message {"TEST_CHUNK_HASH"};
    std::string xattr_field_name {"chunk.hash"};
    std::string http_field_name {"chunk-hash"};
    xattr.addHTTP(http_field_name, test_message);
    ASSERT_EQ(test_message, xattr.getXAttr(xattr_field_name));
}
TEST_F(XAttrFixture, HTTP_2_XAttr_chunk_position) {
    std::string test_message {"TEST_CHUNK_POSITION"};
    std::string xattr_field_name {"chunk.position"};
    std::string http_field_name {"chunk-pos"};
    xattr.addHTTP(http_field_name, test_message);
    ASSERT_EQ(test_message, xattr.getXAttr(xattr_field_name));
}
TEST_F(XAttrFixture, HTTP_2_XAttr_chunk_size) {
    std::string test_message {"TEST_CHUNK_SIZE"};
    std::string xattr_field_name {"chunk.size"};
    std::string http_field_name {"chunk-size"};
    xattr.addHTTP(http_field_name, test_message);
    ASSERT_EQ(test_message, xattr.getXAttr(xattr_field_name));
}

// TESTING CONVECTION FROM XATTR TO HTTP NAMES

TEST_F(XAttrFixture, XATTR_2_HTTP_content_container) {
    std::string test_message {"TEST_CONTENT_CONTAINER"};
    std::string xattr_field_name {"content.container"};
    std::string http_field_name {"container-id"};
    xattr.addXAttr(xattr_field_name, test_message);
    ASSERT_EQ(test_message, xattr.getHTTP(http_field_name));
}
TEST_F(XAttrFixture, XATTR_2_HTTP_content_id) {
    std::string test_message {"TEST_CONTENT_ID"};
    std::string xattr_field_name {"content.id"};
    std::string http_field_name {"content-id"};
    xattr.addXAttr(xattr_field_name, test_message);
    ASSERT_EQ(test_message, xattr.getHTTP(http_field_name));
}
TEST_F(XAttrFixture, XATTR_2_HTTP_content_path) {
    std::string test_message {"TEST_CONTENT_PATH"};
    std::string xattr_field_name {"content.path"};
    std::string http_field_name {"content-path"};
    xattr.addXAttr(xattr_field_name, test_message);
    ASSERT_EQ(test_message, xattr.getHTTP(http_field_name));
}
TEST_F(XAttrFixture, XATTR_2_HTTP_content_storage_policy) {
    std::string test_message {"TEST_CONTENT_STORAGE_POLICY"};
    std::string xattr_field_name {"content.storage_policy"};
    std::string http_field_name {"content-storage-policy"};
    xattr.addXAttr(xattr_field_name, test_message);
    ASSERT_EQ(test_message, xattr.getHTTP(http_field_name));
}
TEST_F(XAttrFixture, XATTR_2_HTTP_content_chunk_method) {
    std::string test_message {"TEST_CONTENT_CHUNK_METHOD"};
    std::string xattr_field_name {"content.chunk_method"};
    std::string http_field_name {"content-chunk-method"};
    xattr.addXAttr(xattr_field_name, test_message);
    ASSERT_EQ(test_message, xattr.getHTTP(http_field_name));
}
TEST_F(XAttrFixture, XATTR_2_HTTP_content_metachunk_size) {
    std::string test_message {"TEST_METACHUNK_SIZE"};
    std::string xattr_field_name {"metachunk.size"};
    std::string http_field_name {"metachunk-size"};
    xattr.addXAttr(xattr_field_name, test_message);
    ASSERT_EQ(test_message, xattr.getHTTP(http_field_name));
}
TEST_F(XAttrFixture, XATTR_2_HTTP_content_metachunk_hash) {
    std::string test_message {"TEST_METACHUNK_HASH"};
    std::string xattr_field_name {"metachunk.hash"};
    std::string http_field_name {"metachunk-hash"};
    xattr.addXAttr(xattr_field_name, test_message);
    ASSERT_EQ(test_message, xattr.getHTTP(http_field_name));
}
TEST_F(XAttrFixture, XATTR_2_HTTP_chunk_id) {
    std::string test_message {"TEST_CHUNK_ID"};
    std::string xattr_field_name {"chunk.id"};
    std::string http_field_name {"chunk-id"};
    xattr.addXAttr(xattr_field_name, test_message);
    ASSERT_EQ(test_message, xattr.getHTTP(http_field_name));
}
TEST_F(XAttrFixture, XATTR_2_HTTP_chunk_hash) {
    std::string test_message {"TEST_CHUNK_HASH"};
    std::string xattr_field_name {"chunk.hash"};
    std::string http_field_name {"chunk-hash"};
    xattr.addXAttr(xattr_field_name, test_message);
    ASSERT_EQ(test_message, xattr.getHTTP(http_field_name));
}
TEST_F(XAttrFixture, XATTR_2_HTTP_chunk_position) {
    std::string test_message {"TEST_CHUNK_POSITION"};
    std::string xattr_field_name {"chunk.position"};
    std::string http_field_name {"chunk-pos"};
    xattr.addXAttr(xattr_field_name, test_message);
    ASSERT_EQ(test_message, xattr.getHTTP(http_field_name));
}
TEST_F(XAttrFixture, XATTR_2_HTTP_chunk_size) {
    std::string test_message {"TEST_CHUNK_SIZE"};
    std::string xattr_field_name {"chunk.size"};
    std::string http_field_name {"chunk-size"};
    xattr.addXAttr(xattr_field_name, test_message);
    ASSERT_EQ(test_message, xattr.getHTTP(http_field_name));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
