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

#ifndef SRC_RAWX_HPP_
#define SRC_RAWX_HPP_
#include <fstream>
#include <string>
#include <proxygen/httpserver/RequestHandlerFactory.h> // NOLINT
#include "utils.hpp"
#include "blob.hpp"

namespace rawx {

class RawxHandlerFactory : public proxygen::RequestHandlerFactory {
 public:
    void onServerStart(folly::EventBase* evb) noexcept override;
    void onServerStop() noexcept override;
    proxygen::RequestHandler* onRequest(proxygen::RequestHandler*,
                                        proxygen::HTTPMessage*)
            noexcept override;

 private:
    std::shared_ptr<utils::RequestCounter> requestCounter;
    utils::AccessLog accessLog;
    utils::ServiceLog serviceLog;
    std::fstream errorFile;
    std::fstream accessFile;
};

class DownloadHandler : public proxygen::RequestHandler {
 public:
    DownloadHandler() {}
    explicit DownloadHandler(std::shared_ptr<utils::RequestCounter> rc,
                             std::fstream *errFile, std::fstream *accFile)
            : requestCounter {rc}, errorFile {errFile}, accessFile {accFile} {}
    void sendData() noexcept;
    void sendHeader() noexcept;
    bool GetClientAddr();
    bool headerCheck(proxygen::HTTPMessage *headers);
    void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers)
            noexcept override;
    void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;
    void onEOM() noexcept override;
    void onUpgrade(proxygen::UpgradeProtocol proto) noexcept override;
    void requestComplete() noexcept override;
    void onError(proxygen::ProxygenError err) noexcept override;
    void Abort() noexcept;

 private:
    std::shared_ptr<utils::RequestCounter> requestCounter;
    time_t beginOfRequest;
    time_t endOfRequest;
    utils::AccessLog accessLog;
    utils::ServiceLog serviceLog;
    blob::DiskDownload download;
    utils::XAttr xattr;
    std::string path;
    std::fstream *errorFile;
    std::fstream *accessFile;
};

class UploadHandler : public proxygen::RequestHandler {
 public:
    UploadHandler() {}
    explicit UploadHandler(std::shared_ptr<utils::RequestCounter> rc,
                           std::fstream *errFile, std::fstream *accFile)
            : requestCounter {rc}, errorFile {errFile}, accessFile {accFile} {}
    int size();
    void sendHeader() noexcept;
    bool GetClientAddr();
    bool headerCheck(proxygen::HTTPMessage *headers);
    void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers)
            noexcept override;
    void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;
    void onEOM() noexcept override;
    void onUpgrade(proxygen::UpgradeProtocol proto) noexcept override;
    void requestComplete() noexcept override;
    void onError(proxygen::ProxygenError err) noexcept override;
    void Abort() noexcept;

 private:
    std::shared_ptr<utils::RequestCounter> requestCounter;
    time_t beginOfRequest;
    time_t endOfRequest;
    utils::AccessLog accessLog;
    utils::ServiceLog serviceLog;
    int sizeUploaded {0};
    blob::DiskUpload upload;
    utils::XAttr xattr;
    std::string path;
    std::fstream *errorFile;
    std::fstream *accessFile;
};

class RemovalHandler : public proxygen::RequestHandler {
 public:
    RemovalHandler() {}
    explicit RemovalHandler(std::shared_ptr<utils::RequestCounter> rc,
                            std::fstream *errFile, std::fstream *accFile)
            : requestCounter {rc}, errorFile {errFile}, accessFile {accFile} {}
    void sendHeader();
    bool GetClientAddr();
    bool headerCheck(proxygen::HTTPMessage *headers);
    void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers)
            noexcept override;
    void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;
    void onEOM() noexcept override;
    void onUpgrade(proxygen::UpgradeProtocol proto) noexcept override;
    void requestComplete() noexcept override;
    void onError(proxygen::ProxygenError err) noexcept override;
    void Abort() noexcept;

 private:
    std::shared_ptr<utils::RequestCounter> requestCounter;
    time_t beginOfRequest;
    time_t endOfRequest;
    std::string chunk_id;
    utils::AccessLog accessLog;
    utils::ServiceLog serviceLog;
    blob::DiskRemoval removal;
    std::string path;
    std::fstream *errorFile;
    std::fstream *accessFile;
};

class StatHandler : public proxygen::RequestHandler {
 public:
    StatHandler() {}
    explicit StatHandler(std::shared_ptr<utils::RequestCounter> rc,
                         std::fstream *errFile)
            :requestCounter {rc}, errorFile {errFile} {}
    void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers)
            noexcept override;
    void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;
    void onEOM() noexcept override;
    void onUpgrade(proxygen::UpgradeProtocol proto) noexcept override;
    void requestComplete() noexcept override;
    void onError(proxygen::ProxygenError err) noexcept override;

 private:
    std::shared_ptr<utils::RequestCounter> requestCounter;
    utils::ServiceLog serviceLog;
    std::string workingDirectory;
    std::fstream *errorFile;
};

class InfoHandler : public proxygen::RequestHandler {
 public:
    InfoHandler() {}
    explicit InfoHandler(std::shared_ptr<utils::RequestCounter> rc,
                         std::fstream *errFile)
            :requestCounter {rc}, serviceLog {errFile} {}
    void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers)
            noexcept override;
    void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;
    void onEOM() noexcept override;
    void onUpgrade(proxygen::UpgradeProtocol proto) noexcept override;
    void requestComplete() noexcept override;
    void onError(proxygen::ProxygenError err) noexcept override;

 private:
    std::string OIONamespace;
    std::string workingDirectory;
    std::shared_ptr<utils::RequestCounter> requestCounter;
    utils::ServiceLog serviceLog;
};

}  // namespace rawx

#endif  // SRC_RAWX_HPP_
