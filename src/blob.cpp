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

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <openssl/md5.h>
#include <glog/logging.h>
#include <gflags/gflags.h>
#include <cstdio>
#include <cerrno>
#include <fstream>
#include "blob.hpp"

using blob::Status;
using blob::Cause;
using blob::DiskUpload;
using blob::DiskDownload;
using blob::DiskRemoval;

DEFINE_int32(read_buffer_size, 65536, "");

int DiskUpload::makeParent(std::string path) {
    std::size_t found = 0;
    while (true) {
        found = path.find('/', found);
        if (found == 0)
            return 0;
        if (found == std::string::npos)
            return 0;
        auto parent = path.substr(0, found);
        found++;
        int rc = mkdir(parent.c_str(), mode);
        if (rc != 0 && errno != EEXIST)
            return errno;
    }
}

/**
 * Check if the file not already existed
 * Create the path if not already created
 * Open the file for writing
 */
Status DiskUpload::Prepare() {
    makeParent(path);
    struct stat sb;
    if (stat(path.c_str(), &sb) == 0)
        return Status(Cause::InternalError);
    file = fopen(path.c_str(), "w");
    if (file == NULL) {
        return Status(Cause::InternalError);
    }
    fd = fileno(file);
    return Status();
}

/**
 * Finalise the write (flush) and push it to the non temporary place
 */
Status DiskUpload::Commit() {
    MD5_Final(md5Hash, &md5Context);
    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        ss << (unsigned int)  md5Hash[i];
    }
    xattr->addXAttr("chunk.hash", ss.str());
    xattr->writeXAttr(fd);
    if (fflush(file) != 0)
        return Status(Cause::InternalError);
    return Status();
}

/**
 * Write the data to the file, the Write may be buffered
 */
Status DiskUpload::Write(std::shared_ptr<Slice> slice) {
    int sizeSent = 0;
    size_t rc;
    do {
        MD5_Update(&md5Context, slice->data(), slice->size());
        rc = fwrite(slice->data() - sizeSent, sizeof(char),
                    slice->size() - sizeSent, file);
        sizeSent += rc;
    }while (sizeSent != slice->size());
    return Status();
}

/**
 * Stop the writing and delete the temporary file
 */
Status DiskUpload::Abort() {
    if (file != NULL)
        fclose(file);
    return Status();
}

/**
 * Open the file at the right place
 * and read the xattr attribute
 */
Status DiskDownload::Prepare() {
    file = fopen(path.c_str(), "r");
    if (file == NULL)
        return Status(Cause::InternalError);
    if (begin >= 0) {
        if (fseek(file, begin, SEEK_SET) != 0)
            return Status(Cause::InternalError);
    } else {
        read_size = FLAGS_read_buffer_size;
    }
    fd = fileno(file);
    xattr->retrieveXAttr(fd);
    return Status();
}

/**
 * Check if there still are data to read
 */
bool DiskDownload::isEof() {
    return feof(file) || end == 0;
}

bool DiskDownload::setRange(int begin, int end) {
    if ( begin < 0 || end < 0 || begin > end)
        return false;
    this->begin = begin;
    this->end = end;
    read_size = end - begin;
    return true;
}

bool DiskDownload::setRange(std::string bytesRange) {
    int begin, end;
    int rc = sscanf(bytesRange.c_str(), "bytes=%d-%d", &begin, &end);
    if (rc < 2)
        return false;
    return setRange(begin, end);
}



/**
 * Read the data and fill the Slice
 */
Status DiskDownload::Read(std::shared_ptr<Slice> slice) {
    uint8_t buffer[FLAGS_read_buffer_size];
    int tmp_read = fread(buffer, sizeof(char), read_size, file);
    if (tmp_read == 0 && ferror(file)) {
        perror("DiskDownload fread");
        return Status(Cause::InternalError);
    }
    if (end > -1) {
        end -= tmp_read;
        if (read_size > end) {
            read_size = end;
        }
    }
    slice->append(buffer, tmp_read);
    return Status();
}

/**
 * Stop reading and close the  file handler
 */
Status DiskDownload::Abort() {
    return Status();
}

/**
 * Search if the file exist and that we can delete it
 */
Status DiskRemoval::Prepare() {
    struct stat sb;
    if (stat(path.c_str(), &sb) != 0)
        return Status(Cause::InternalError);
    return Status();
}

/**
 * Remove the file
 */
Status DiskRemoval::Commit() {
    if (remove(path.c_str()))
        return Status(Cause::InternalError);
    return Status();
}
/**
 * Close the file handler
 */
Status DiskRemoval::Abort() {
    return Status();
}
