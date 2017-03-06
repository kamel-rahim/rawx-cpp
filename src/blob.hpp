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

#ifndef SRC_BLOB_HPP_
#define SRC_BLOB_HPP_

#include <openssl/md5.h>
#include <gflags/gflags.h>
#include <memory>
#include <string>
#include <vector>
#include "utils.hpp"

namespace blob {

/**
 * High-level error cause.
 */
enum class Cause {
    OK,
    Already,
    Forbidden,
    NotFound,
    NetworkError,
    ProtocolError,
    Unsupported,
    InternalError
};

enum class TransactionStep {
    Init, Prepared, Done
};

/**
 * Associates a high-level error classs (the Cause) and an explanation message.
 */
class Status {
 protected:
    Cause rc_;
    unsigned int code_;
    std::string msg_;

 public:
    ~Status() {}

    /** Success constructor */
    Status() : rc_{Cause::OK}, code_{0} {}

    /** Build a Status with a cause and a generic message */
    explicit Status(Cause rc) : rc_{rc}, code_{0} {}

    /**
     * Tells if the Status depicts a success (true) or an failure (false)
     * @return true for a success, false for a failure
     */
    inline bool Ok() const { return rc_ == Cause::OK; }

    /**
     * Returns the High-level error cause.
     * @return the underlyinng error cause
     */
    inline Cause Why() const { return rc_; }

    /**
     * Explain what happened, why it failed, etc.
     * The returned pointer validity doesn't exceed the current Status lifetime.
     * @return
     */
    inline const char *Message() const { return msg_.c_str(); }


    inline unsigned int SoftCode() const { return code_; }

    /**
     * Handy method to translate the underlying error cause into a pretty
     * string.
     * @return the textual representation of the underlying Cause
     */
    const char *Name() const;

    std::string Encode() const;
};

std::ostream &operator<<(std::ostream &out, const Status &s);

/**
 * Simple wrapper to initiate a Status depicting a success.
 * Usage:
 *
 * Status successful_method () {
 *   return Ok();
 */
class Ok : public Status {
 public:
    Ok() : Status(Cause::OK) {}
};

/**
 * Simple wrapper to initiate a Status based on the current errno value.
 * Usage:
 *
 * Status failing_method () {
 *   int rc = ::failing_syscall();
 *   if (rc == 0) return Errno(0);
 *   return Errno();
 * }
 */
class Errno : public Status {
 public:
    /**
     * Build a Status with the given errno.
     * @param err an errno value
     * @return a valid Status
     */
    explicit Errno(int err);

    /**
     * Build a Status with the system's errno
     * @return a valid Statuss
     */
    Errno();

    /**
     * Destructor
     */
    ~Errno() {}
};

class Slice {
 public:
    virtual uint8_t * data() = 0;
    virtual int32_t size() = 0;
    virtual void append(uint8_t *data, uint32_t length) = 0;
};

class FileSlice : public Slice {
 public:
    FileSlice() {}
    FileSlice(uint8_t* data, uint64_t length ) {
        append(data, (uint32_t) length);
    }
    uint8_t * data() override {
        return inner.data();
    }
    int32_t size() override {
        return inner.size();
    }
    void append(uint8_t *data, uint32_t length) override {
        inner.insert(inner.end(), data, &data[length]);
    }
 private:
    std::vector<uint8_t> inner;
};

class Upload {
 public:
    virtual Status Prepare() = 0;
    virtual Status Write(std::shared_ptr<Slice> slice) = 0;
    virtual Status Commit() = 0;
    virtual Status Abort() = 0;
};

class Download {
 public:
    virtual Status Prepare() = 0;
    virtual bool isEof() = 0;
    virtual Status Read(std::shared_ptr<Slice> slice) = 0;
    virtual Status Abort() = 0;
};

class Removal {
 public:
    virtual Status Prepare() = 0;
    virtual Status Commit() = 0;
    virtual Status Abort() = 0;
};

class DiskUpload : public Upload {
 public:
    DiskUpload() {
        MD5_Init(&md5Context);
    }
    explicit DiskUpload(std::string path) : path{path} {}
    inline void Path(std::string path) {this->path = path;}
    inline void XAttr(utils::XAttr *xattr) {this->xattr = xattr;}
    Status Prepare() override;
    Status Commit() override;
    Status Write(std::shared_ptr<Slice>) override;
    Status Abort() override;
 private:
    utils::XAttr *xattr {nullptr};
    int fd {-1};
    MD5_CTX md5Context;
    unsigned char md5Hash[MD5_DIGEST_LENGTH];
    int makeParent(std::string path);
    std::string tmpPath;
    std::string path;
    FILE *file {NULL};
    mode_t mode {0755};
};

class DiskDownload : public Download {
 public:
    DiskDownload() {}
    inline void Path(std::string path) {this->path = path;}
    inline void XAttr(utils::XAttr *xattr) {this->xattr = xattr;}
    bool setRange(std::string bytesRange);
    bool setRange(int begin, int end);
    Status Prepare() override;
    bool isEof() override;
    Status Read(std::shared_ptr<Slice>) override;
    Status Abort() override;
 private:
    utils::XAttr *xattr {nullptr};
    int fd {-1};
    int begin {-1};
    int end {-1};
    int read_size {-1};
    std::string path;
    FILE *file {NULL};
};

class DiskRemoval : public Removal {
 public:
    DiskRemoval() {}
    inline void Path(std::string path) {this->path = path;}
    inline void XAttr(utils::XAttr *xattr) {this->xattr = xattr;}
    Status Prepare() override;
    Status Commit() override;
    Status Abort() override;
 private:
    utils::XAttr *xattr {nullptr};
    int fd {-1};
    std::string path;
    FILE *file {NULL};
};
}  // namespace blob

#endif  // SRC_BLOB_HPP_
