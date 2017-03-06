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

#include "rawx.hpp"
// #include <zlib.h>
#include <glog/logging.h>
#include <gflags/gflags.h>
#include <proxygen/httpserver/RequestHandlerFactory.h>
#include <proxygen/httpserver/ResponseHandler.h>
#include <proxygen/httpserver/ResponseBuilder.h>
// #include <proxygen/lib/utils/ZlibStreamCompressor.h>
#include <folly/io/IOBuf.h>
#include <folly/io/async/EventBase.h>
#include <vector>
#include <iostream>

#include "utils.hpp"
#include "blob.hpp"

using folly::IOBuf;
using rawx::RawxHandlerFactory;
using rawx::DownloadHandler;
using rawx::UploadHandler;
using rawx::RemovalHandler;
using rawx::StatHandler;
using rawx::InfoHandler;
using proxygen::RequestHandler;
using proxygen::HTTPMethod;
using proxygen::ResponseBuilder;
using proxygen::HTTPMessage;
using proxygen::HTTPHeaderCode;
using blob::Slice;
using blob::FileSlice;
using utils::RequestCounter;


DEFINE_string(workingDirectory, "", "");
DEFINE_string(OIO_NS, "", "Namespace");
DEFINE_string(server_hostname, "", "Hostname of the server");
DEFINE_string(instanceID, "",
              "Unique ID of the service (defaults to the service name)");
DEFINE_string(access_log, "", "");
DEFINE_string(error_log, "", "");

void RawxHandlerFactory::onServerStart(folly::EventBase*) noexcept {
    requestCounter = std::shared_ptr<RequestCounter>(new RequestCounter());
    errorFile.open(FLAGS_error_log, std::ios_base::out);
    accessFile.open(FLAGS_access_log, std::ios_base::out);
}

void RawxHandlerFactory::onServerStop() noexcept {}

RequestHandler* RawxHandlerFactory::onRequest(RequestHandler*,
                                              HTTPMessage* msg) noexcept {
    auto method = msg->getMethod();
    if (!method) {
        serviceLog.LogToPrint("INF", "Error no Method detected");
        return nullptr;
    }
    switch (*method) {
        case HTTPMethod::GET:
            if (msg->getURL() == "/stat")
                return new StatHandler(requestCounter, &errorFile);
            else if (msg->getURL() == "/info")
                return new InfoHandler(requestCounter, &errorFile);
            else
                return new DownloadHandler(requestCounter, &errorFile,
                                           &accessFile, localServer_);
            break;
        case HTTPMethod::PUT:
            return new UploadHandler(requestCounter, &errorFile, &accessFile,
                                     localServer_);
            break;
        case HTTPMethod::DELETE:
            return new RemovalHandler(requestCounter, &errorFile, &accessFile,
                                      localServer_);
            break;
        default:
            serviceLog.LogToPrint("INF", "Error Method not recognize");
            return nullptr;
    }
    return nullptr;
}

bool DownloadHandler::headerCheck(proxygen::HTTPMessage *headers) {
    std::string tmpheader = headers->getHeaders().rawGet("X-oio-req-id");
    if (tmpheader.empty())
        return false;
    accessLog.RequestID(tmpheader);
    path = headers->getURL();
    if (path.empty() || path.length() < 3)
        return false;
    path = path.substr(1, 3) + "/" + path.substr(1);
    tmpheader = headers->getHeaders().rawGet("Range");
    if (!tmpheader.empty()) {
        return download.setRange(tmpheader);
    }
    return true;
}

void DownloadHandler::Abort() noexcept {
    ResponseBuilder(downstream_).closeConnection();
}

void DownloadHandler::onRequest(
    std::unique_ptr<proxygen::HTTPMessage> headers) noexcept {
    beginOfRequest = std::chrono::steady_clock::now();
    requestCounter->incGetHits();
    serviceLog.SetUpBasic(FLAGS_server_hostname, FLAGS_instanceID);
    accessLog.SetUpBasic(FLAGS_server_hostname, FLAGS_instanceID);
    accessLog.RequestType("GET");
    accessLog.RemoteClient(headers->getClientAddress().getAddressStr() + ":" +
                           std::to_string(headers->getClientAddress()
                                          .getPort()));
    if (!headerCheck(headers.get())) {
        serviceLog.LogToPrint("INF", "Header not conform to the GET request");
        accessLog.StatusCode("400");
        ResponseBuilder(downstream_).status(400, "Bad Request")
                .sendWithEOM();
        requestCounter->incR4xxHits();
        return;
    }
    download.Path(path);
    download.XAttr(&xattr);
    if (!download.Prepare().Ok()) {
        serviceLog.LogToPrint("INF", "Error with the path to the Chunk");
        accessLog.StatusCode("404");
        ResponseBuilder(downstream_).status(404, "Chunk not found")
                .sendWithEOM();
        requestCounter->incR4xxHits();
        return;
    }
    accessLog.UserID(xattr.getHTTP("container-id"));
    accessLog.Message(xattr.getHTTP("chunk-id"));
    sendHeader();
    sendData();
}

void DownloadHandler::sendHeader() noexcept {
    auto namesValues = xattr.HTTPNamesValues();
    // FIXME(KR) There is a problem with ResponseBuilder.
    // If we don't chain function it can have status not set, even when use
    // just before header() so we need to unroll the loop
    ResponseBuilder(downstream_).status(206, "Partial Content")
            .header<std::string>(xattr.HttpPrefix() + "container-id",
                                 xattr.getHTTP("container-id"))
            .header<std::string>(xattr.HttpPrefix() + "content-id",
                                 xattr.getHTTP("content-id"))
            .header<std::string>(xattr.HttpPrefix() + "content-path",
                                 xattr.getHTTP("content-path"))
            .header<std::string>(xattr.HttpPrefix() + "content-version",
                                 xattr.getHTTP("content-version"))
            .header<std::string>(xattr.HttpPrefix() + "content-storage-policy",
                                 xattr.getHTTP("content-storage-policy"))
            .header<std::string>(xattr.HttpPrefix() + "content-chunk-method",
                                 xattr.getHTTP("content-chunk-method"))
            .header<std::string>(xattr.HttpPrefix() + "chunk-id",
                                 xattr.getHTTP("chunk-id"))
            .header<std::string>(xattr.HttpPrefix() + "chunk-hash",
                                 xattr.getHTTP("chunk-hash"))
            .header<std::string>(xattr.HttpPrefix() + "chunk-pos",
                                 xattr.getHTTP("chunk-pos"))
            .header<std::string>(xattr.HttpPrefix() + "chunk-size",
                                 xattr.getHTTP("chunk-size"))
            .send();
    requestCounter->incR2xxHits();
}

void DownloadHandler::sendData() noexcept {
    std::shared_ptr<Slice> slice(new FileSlice());
    while (!download.isEof()) {
        if (!download.Read(slice).Ok()) {
            serviceLog.LogToPrint("INF", "Error reading the chunk");
            Abort();
            return;
        }
        std::unique_ptr<IOBuf> uniIOB =  std::unique_ptr<IOBuf>(
            new IOBuf(IOBuf::CopyBufferOp::COPY_BUFFER,
                      slice->data(), slice->size()));
        ResponseBuilder(downstream_).body(std::move(uniIOB))
                .send();
    }
    ResponseBuilder(downstream_).sendWithEOM();
    accessLog.StatusCode("206");
}

void DownloadHandler::onError(proxygen::ProxygenError err) noexcept {
    serviceLog.LogToPrint("INF", getErrorString(err));
}

void DownloadHandler::onBody(std::unique_ptr<folly::IOBuf>) noexcept {
    serviceLog.LogToPrint("INF", "No data should be send on a GET Request");
}

void DownloadHandler::onUpgrade(proxygen::UpgradeProtocol) noexcept {
    serviceLog.LogToPrint("INF", "Upgrade to h2c are not handled");
}

void DownloadHandler::requestComplete() noexcept {
    endOfRequest = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>
            (endOfRequest- beginOfRequest).count();
    accessLog.ResponseTime(std::to_string(duration));
    requestCounter->incGetTime(duration);
    accessLog.LogToPrint("INF", xattr.getHTTP("chunk-id"));
}

void DownloadHandler::onEOM() noexcept  {}

bool UploadHandler::headerCheck(proxygen::HTTPMessage *headers) {
    std::vector<std::string> names {"content-id", "container-id",
                "content-storage-policy", "content-chunk-method",
                "content-path", "chunk-id", "chunk-pos"};
    std::string tmpheader;
    for (auto &elem :  names) {
        tmpheader = headers->getHeaders().rawGet(xattr.HttpPrefix() + elem);
        if (!tmpheader.empty()) {
            xattr.addHTTP(elem, tmpheader);
        } else {
            return false;
        }
    }
    path = headers->getURL();
    path = path.substr(1, 3) + "/" + path.substr(1);
    accessLog.UserID(headers->getHeaders().rawGet(xattr.HttpPrefix() +
                                                  "container-id"));
    accessLog.RequestID(headers->getHeaders().rawGet(xattr.HttpPrefix() +
                                                     "chunk-id"));
    return true;
}

int UploadHandler::size() {
    return sizeUploaded;
}

void UploadHandler::Abort() noexcept {
    ResponseBuilder(downstream_).status(500, "Internal Server Error")
            .sendWithEOM();
    accessLog.StatusCode("500");
}

void UploadHandler::onRequest(std::unique_ptr<proxygen::HTTPMessage> headers)
        noexcept {
    serviceLog.SetUpBasic(FLAGS_server_hostname, FLAGS_instanceID);
    accessLog.SetUpBasic(FLAGS_server_hostname, FLAGS_instanceID);
    requestCounter->incPutHits();
    accessLog.RequestType("PUT");
    accessLog.RemoteClient(headers->getClientAddress().getAddressStr() + ":" +
                           std::to_string(headers->getClientAddress()
                                          .getPort()));
    beginOfRequest = std::chrono::steady_clock::now();
    if (!headerCheck(headers.get())) {
        serviceLog.LogToPrint("INF", "Error with Header received");
        ResponseBuilder(downstream_).status(400, "Bad Request")
                .sendWithEOM();
        accessLog.StatusCode("400");
        requestCounter->incR4xxHits();
        errorHappened = true;
        return;
    }
    upload.Path(path);
    upload.XAttr(&xattr);
    if (!upload.Prepare().Ok()) {
        serviceLog.LogToPrint("INF", "Error with the path of file");
        ResponseBuilder(downstream_).status(400, "Bad Request")
                .sendWithEOM();
        accessLog.StatusCode("400");
        requestCounter->incR4xxHits();
        errorHappened = true;
    }
}

void UploadHandler::onBody(std::unique_ptr<folly::IOBuf> body)
        noexcept  {
    if (errorHappened)
        return;
    sizeUploaded += body->length();
    // std::unique_ptr<IOBuf> uniIOB = zlibStreamCompressor.compress(body.get()); //NOLINT
    std::shared_ptr<Slice> slice(new FileSlice(body->writableData(),
                                               body->length()));
    upload.Write(slice);
}

void UploadHandler::sendHeader() noexcept {
    // FIXME(KR) There is a problem with ResponseBuilder.
    // If we don't chain function it can have status not set even when the
    // function is used just before so we need to unroll the loop
    ResponseBuilder(downstream_).status(201, "Created")
            .header<std::string>(xattr.HttpPrefix() + "container-id",
                                 xattr.getHTTP("container-id"))
            .header<std::string>(xattr.HttpPrefix() + "content-id",
                                 xattr.getHTTP("content-id"))
            .header<std::string>(xattr.HttpPrefix() + "content-path",
                                 xattr.getHTTP("content-path"))
            .header<std::string>(xattr.HttpPrefix() + "content-version",
                                 xattr.getHTTP("content-version"))
            .header<std::string>(xattr.HttpPrefix() + "content-storage-policy",
                                 xattr.getHTTP("content-storage-policy"))
            .header<std::string>(xattr.HttpPrefix() + "content-chunk-method",
                                 xattr.getHTTP("content-chunk-method"))
            .header<std::string>(xattr.HttpPrefix() + "chunk-id",
                                 xattr.getHTTP("chunk-id"))
            .header<std::string>(xattr.HttpPrefix() + "chunk-hash",
                                 xattr.getHTTP("chunk-hash"))
            .header<std::string>(xattr.HttpPrefix() + "chunk-pos",
                                 xattr.getHTTP("chunk-pos"))
            .header<std::string>(xattr.HttpPrefix() + "chunk-size",
                                 xattr.getHTTP("chunk-size"))
            .header<std::string>("Content-Length", std::to_string(size()))
            .sendWithEOM();
    accessLog.StatusCode("201");
    requestCounter->incR2xxHits();
}

void UploadHandler::onEOM() noexcept  {
    if (errorHappened)
        return;
    upload.Commit();
    sendHeader();
}

void UploadHandler::onUpgrade(proxygen::UpgradeProtocol)
        noexcept  {
    serviceLog.LogToPrint("INF", "Upgrade to h2c are not handled");
}

void UploadHandler::requestComplete() noexcept  {
    endOfRequest = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>
            (endOfRequest- beginOfRequest).count();
    accessLog.ResponseTime(std::to_string(duration));
    requestCounter->incPutTime(duration);
    accessLog.ResponseSize("0");
    accessLog.LogToPrint("INF", xattr.getHTTP("chunk-id"));
}

void UploadHandler::onError(proxygen::ProxygenError err) noexcept  {
    serviceLog.LogToPrint("INF", getErrorString(err));
}

void RemovalHandler::Abort() noexcept {
    ResponseBuilder(downstream_).closeConnection();
}

bool RemovalHandler::headerCheck(proxygen::HTTPMessage* headers) {
    accessLog.RemoteClient(headers->getClientAddress().getAddressStr() + ":" +
                           std::to_string(headers->getClientAddress()
                                          .getPort()));
    path = headers->getURL();
    if (path.empty())
        return false;
    chunk_id = path;
    path = path.substr(1, 3) + "/" + path.substr(1);
    return true;
}

void RemovalHandler::onRequest(std::unique_ptr<proxygen::HTTPMessage> headers)
        noexcept  {
    serviceLog.SetUpBasic(FLAGS_server_hostname, FLAGS_instanceID);
    accessLog.SetUpBasic(FLAGS_server_hostname, FLAGS_instanceID);
    requestCounter->incDelHits();
    accessLog.RequestType("DELETE");
    beginOfRequest = std::chrono::steady_clock::now();
    if (!headerCheck(headers.get())) {
        ResponseBuilder(downstream_).status(400, "Bad Request").sendWithEOM();
        accessLog.StatusCode("400");
        requestCounter->incR4xxHits();
        return;
    }
    removal.Path(path);
    if (!removal.Prepare().Ok()) {
        ResponseBuilder(downstream_).status(404, "Chunk not found")
                .sendWithEOM();
        accessLog.StatusCode("404");
        requestCounter->incR404Hits();
        return;
    }
}

void RemovalHandler::onBody(std::unique_ptr<folly::IOBuf>)
        noexcept  {
    serviceLog.LogToPrint("INF", "No data should be send on a GET Request");
}

void RemovalHandler::onEOM() noexcept  {
    removal.Commit();
    ResponseBuilder(downstream_).status(204, "No Content");
    accessLog.StatusCode("204");
    ResponseBuilder(downstream_).sendWithEOM();
    requestCounter->incR2xxHits();
}

void RemovalHandler::requestComplete() noexcept {
    endOfRequest = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>
            (endOfRequest- beginOfRequest).count();
    accessLog.ResponseTime(std::to_string(duration));
    requestCounter->incDelTime(duration);
    accessLog.LogToPrint("INF", chunk_id);
}

void RemovalHandler::onError(proxygen::ProxygenError err) noexcept {
    serviceLog.LogToPrint("INF", getErrorString(err));
}

void RemovalHandler::onUpgrade(proxygen::UpgradeProtocol) noexcept {
    serviceLog.LogToPrint("INF", "Upgrade to h2c are not handled");
}

void StatHandler::onRequest(std::unique_ptr<proxygen::HTTPMessage>)
        noexcept {
    beginOfRequest = std::chrono::steady_clock::now();
    serviceLog.SetUpBasic(FLAGS_server_hostname, FLAGS_instanceID);
    requestCounter->incStatHits();
    std::string content =  requestCounter->ToText();
    content += "config volume " + FLAGS_workingDirectory + "\n";
    ResponseBuilder(downstream_).status(200, "OK")
            .header("Content-Length", std::to_string(content.length()))
            .body(content)
            .sendWithEOM();
}

void StatHandler::onBody(std::unique_ptr<folly::IOBuf>) noexcept {}

void StatHandler::onEOM() noexcept {}

void StatHandler::onUpgrade(proxygen::UpgradeProtocol) noexcept {
    serviceLog.LogToPrint("INF", "Upgrade to h2c are not handled");
}

void StatHandler::requestComplete() noexcept {
    endOfRequest = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>
            (endOfRequest- beginOfRequest).count();
    requestCounter->incStatTime(duration);}

void StatHandler::onError(proxygen::ProxygenError err) noexcept {
    serviceLog.LogToPrint("INF", getErrorString(err));
}

void InfoHandler::onRequest(std::unique_ptr<proxygen::HTTPMessage>)
        noexcept {
    beginOfRequest = std::chrono::steady_clock::now();
    serviceLog.SetUpBasic(FLAGS_server_hostname, FLAGS_instanceID);
    requestCounter->incInfoHits();
    std::string content = "namespace " + FLAGS_OIO_NS + "\n"
            + "path " + FLAGS_workingDirectory + "\n";
    ResponseBuilder(downstream_).status(200, "OK")
            .header("Content-Length", std::to_string(content.length()))
            .body(content)
            .sendWithEOM();
}

void InfoHandler::onBody(std::unique_ptr<folly::IOBuf>) noexcept {}

void InfoHandler::onEOM() noexcept {}

void InfoHandler::onUpgrade(proxygen::UpgradeProtocol) noexcept {
    serviceLog.LogToPrint("INF", "Upgrade to h2c are not handled");
}

void InfoHandler::requestComplete() noexcept {
    endOfRequest = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>
            (endOfRequest- beginOfRequest).count();
    requestCounter->incInfoTime(duration);}

void InfoHandler::onError(proxygen::ProxygenError err) noexcept {
    serviceLog.LogToPrint("INF", getErrorString(err));
}
