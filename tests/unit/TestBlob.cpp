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

#include <gtest/gtest.h>
#include <gflags/gflags.h>
#include "blob.hpp"
#include "utils.hpp"

using blob::Status;
using blob::Cause;
using blob::DiskUpload;
using blob::DiskDownload;
using blob::DiskRemoval;
using utils::XAttr;

class DiskUploadFixture : public testing::Test {
 public:
    void SetUp() override {
        xattr = new XAttr();
        upload.XAttr(xattr);
    }
    void TearDown() override {
        upload.Abort();
        delete xattr;
    }
 protected:
    DiskUpload upload;
    XAttr *xattr;
};

class DiskDownloadFixture : public testing::Test {
 public:
    void SetUp() override {
        xattr = new XAttr();
        download.XAttr(xattr);
    }
    void TearDown() override {
        download.Abort();
        delete xattr;
    }
 protected:
    DiskDownload download;
    XAttr *xattr;
};

class DiskRemovalFixture : public testing::Test {
 public:
    void SetUp() override {
        xattr = new XAttr();
        removal.XAttr(xattr);
    }
    void TearDown() override {
        removal.Abort();
        delete xattr;
    }
 protected:
    XAttr *xattr;
    DiskRemoval removal;
};
// TEST DISKUPLOAD

TEST_F(DiskUploadFixture, GoodPathCreate) {
    std::string path {"./goodpath"};
    upload.Path(path);
    ASSERT_TRUE(upload.Prepare().Ok());
}

TEST_F(DiskUploadFixture, BadPathError) {
    std::string path {"./badpath/"};
    upload.Path(path);
    ASSERT_FALSE(upload.Prepare().Ok());
}

TEST_F(DiskUploadFixture, EmptyPathError) {
    std::string path {""};
    upload.Path(path);
    ASSERT_FALSE(upload.Prepare().Ok());
}

TEST_F(DiskUploadFixture, makeParentOnGoodPath) {
    std::string path {"./goodpathdir/good/path/good"};
    upload.Path(path);
    ASSERT_TRUE(upload.Prepare().Ok());
}

// TEST DISKDOWNLOAD

TEST_F(DiskDownloadFixture, GoodPathOpen) {
    std::string path {"./goodpath"};
    download.Path(path);
    ASSERT_TRUE(download.Prepare().Ok());
}

TEST_F(DiskDownloadFixture, BadPathError) {
    std::string path {"./badpath/azeza"};
    download.Path(path);
    ASSERT_FALSE(download.Prepare().Ok());
}

TEST_F(DiskDownloadFixture, EmptyPathError) {
    std::string path {""};
    download.Path(path);
    ASSERT_FALSE(download.Prepare().Ok());
}

// TEST DISKREMOVAL

TEST_F(DiskRemovalFixture, GoodPathExist) {
    std::string path {"./goodpath"};
    removal.Path(path);
    ASSERT_TRUE(removal.Prepare().Ok());
}

TEST_F(DiskRemovalFixture, BadPathError) {
    std::string path {"./badpath/aze"};
    removal.Path(path);
    ASSERT_FALSE(removal.Prepare().Ok());
}

TEST_F(DiskRemovalFixture, EmptyPathError) {
    std::string path {""};
    removal.Path(path);
    ASSERT_FALSE(removal.Prepare().Ok());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
