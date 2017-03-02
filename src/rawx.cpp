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
#include <glog/logging.h>
#include <gflags/gflags.h>
#include <proxygen/httpserver/RequestHandlerFactory.h>
#include <proxygen/httpserver/ResponseHandler.h>
#include <proxygen/httpserver/ResponseBuilder.h>
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


DEFINE_string(workingDirectory, "" , "");
DEFINE_string(OIO_NS, "", "Namespace");
DEFINE_string(server_hostname, "", "Hostname of the server");
DEFINE_string(instanceID, "",
              "Unique ID of the service (defaults to the service name)");
DEFINE_string(access_log, "", "");
DEFINE_string(error_log, "", "");

void RawxHandlerFactory::onServerStart(folly::EventBase*) noexcept {
    requestCounter = std::shared_ptr<RequestCounter>(new RequestCounter());
    errorFile.open(FLAGS_error_log , std::ios_base::out);
    accessFile.open(FLAGS_access_log , std::ios_base::out);
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
                                           &accessFile);
            break;
        case HTTPMethod::PUT:
            return new UploadHandler(requestCounter, &errorFile, &accessFile);
            break;
        case HTTPMethod::DELETE:
            return new RemovalHandler(requestCounter, &errorFile, &accessFile);
            break;
        default:
            serviceLog.LogToPrint("INF", "Error Method not recognize");
            return nullptr;
    }
    return nullptr;
}

bool DownloadHandler::headerCheck(proxygen::HTTPMessage *headers) {
    DLOG(INFO) << "DownloadHandler Checking Header";
    std::string tmpheader = headers->getHeaders().rawGet("X-oio-req-id");
    if (tmpheader.empty())
        return false;
    accessLog.RequestID(tmpheader);
    path = headers->getURL();
    DLOG(INFO) << "DownloadHandler Header Req-id Received";
    if (path.empty())
        return false;
    path = path.substr(1, 3) + "/" + path.substr(1);
    DLOG(INFO) << "DownloadHandler Header Path Received";
    tmpheader = headers->getHeaders().rawGet("Range");
    if (!tmpheader.empty()) {
        return download.setRange(tmpheader);
    }
    return true;
}

bool DownloadHandler::GetClientAddr() {
    auto addr = downstream_->getSetupTransportInfo().remoteAddr;
    if (addr.get() != nullptr) {
        std::string clientHostname = addr->getAddressStr();
        clientHostname += addr->getPort();
        accessLog.RemoteClient(clientHostname);
        return true;
    }
    return false;
}

void DownloadHandler::Abort() noexcept {
    ResponseBuilder(downstream_).closeConnection();
}

void DownloadHandler::onRequest(
    std::unique_ptr<proxygen::HTTPMessage> headers) noexcept {
    beginOfRequest = std::time(nullptr);
    DLOG(INFO) << "DownloadHandling";
    requestCounter->incGetHits();
    serviceLog.SetUpBasic(FLAGS_server_hostname, FLAGS_instanceID);
    accessLog.SetUpBasic(FLAGS_server_hostname, FLAGS_instanceID);
    accessLog.RequestType("GET");
    if (!headerCheck(headers.get())) {
        serviceLog.LogToPrint("INF", "Header not conform to the GET request");
        ResponseBuilder(downstream_).status(400, "Bad Request")
                .sendWithEOM();
        requestCounter->incR4xxHits();
        return;
    }
    download.Path(path);
    if (!download.Prepare().Ok()) {
        serviceLog.LogToPrint("INF", "Error with the path to the Chunk");
        accessLog.StatusCode("404");
        ResponseBuilder(downstream_).status(404, "Chunk not found")
                .sendWithEOM();
        requestCounter->incR4xxHits();
        return;
    }
    accessLog.UserID(xattr.getHTTP("container-id"));
    sendHeader();
    sendData();
}

void DownloadHandler::sendHeader() noexcept {
    DLOG(INFO) << "DownloadHandler Sending Header";
    auto namesValues = xattr.HTTPNamesValues();
    ResponseBuilder(downstream_).status(206, "Partial Content");
    for (auto &elem : namesValues) {
        ResponseBuilder(downstream_).header<std::string>(elem.first,
                                                         elem.second);
    }
    requestCounter->incR2xxHits();
    ResponseBuilder(downstream_).send();
}

void DownloadHandler::sendData() noexcept {
    DLOG(INFO) << "DownloadHandler Sending Data";
    std::shared_ptr<Slice> slice;
    while (!download.isEof()) {
        if (!download.Read(slice).Ok()) {
            serviceLog.LogToPrint("INF", "Error reading the chunk");
            Abort();
            return;
        }
        ResponseBuilder(downstream_).body(IOBuf::wrapBuffer(
            slice->data(), (unsigned int)slice->size()));
        ResponseBuilder(downstream_).send();
    }
    ResponseBuilder(downstream_).sendWithEOM();
}

void DownloadHandler::onError(proxygen::ProxygenError err) noexcept {
    DLOG(INFO) << "DownloadHandler Error Happened";
    serviceLog.LogToPrint("INF", getErrorString(err));
}

void DownloadHandler::onBody(std::unique_ptr<folly::IOBuf>) noexcept {
    DLOG(INFO) << "DownloadHandler Body Received";
    serviceLog.LogToPrint("INF", "No data should be send on a GET Request");
}

void DownloadHandler::onUpgrade(proxygen::UpgradeProtocol) noexcept {
    serviceLog.LogToPrint("INF", "Upgrade to h2c are not handled");
}

void DownloadHandler::requestComplete() noexcept {
    endOfRequest = std::time(nullptr);
    accessLog.LogToPrint("INF", xattr.getHTTP("chunk-id"));
}

void DownloadHandler::onEOM() noexcept  {}

bool UploadHandler::headerCheck(proxygen::HTTPMessage *headers) {
    DLOG(INFO) << "UploadHandler Checking Header";
    std::vector<std::string> names {"content-id", "container-id",
                "content-storage-policy", "content-chunk-method",
                "content-path", "chunk-id", "chunk-pos"};
    std::string tmpheader;
    for (auto &elem :  names) {
        tmpheader = headers->getHeaders().rawGet(xattr.HttpPrefix() + elem);
        if (!tmpheader.empty()) {
            xattr.addHTTP(xattr.HttpPrefix()+elem, tmpheader);
        } else {
            return false;
        }
    }
    path = headers->getHeaders().rawGet(xattr.XAttrPrefix()+ "chunk-id");
    path = path.substr(0, 3) + "/" + path;
    return true;
}

int UploadHandler::size() {
    return sizeUploaded;
}

void UploadHandler::Abort() noexcept {
    ResponseBuilder(downstream_).closeConnection();
}

bool UploadHandler::GetClientAddr() {
    auto addr = downstream_->getSetupTransportInfo().remoteAddr;
    if (addr.get() != nullptr) {
        std::string clientHostname = addr->getAddressStr();
        clientHostname += addr->getPort();
        accessLog.RemoteClient(clientHostname);
        return true;
    }
    return false;
}


void UploadHandler::onRequest(std::unique_ptr<proxygen::HTTPMessage> headers)
        noexcept {
    DLOG(INFO) << "UploadHandling";
    serviceLog.SetUpBasic(FLAGS_server_hostname, FLAGS_instanceID);
    accessLog.SetUpBasic(FLAGS_server_hostname, FLAGS_instanceID);
    requestCounter->incPutHits();
    accessLog.RequestType("PUT");
    beginOfRequest = std::time(nullptr);
    headerCheck(headers.get());
    upload.Path(path);
    if (!upload.Prepare().Ok()) {
        DLOG(INFO) << "Error with the path of the file";
        serviceLog.LogToPrint("INF", "Error with the path of file");
        ResponseBuilder(downstream_).status(400, "Bad Request")
                .closeConnection();
        accessLog.StatusCode("400");
        requestCounter->incR4xxHits();
    }
}

void UploadHandler::onBody(std::unique_ptr<folly::IOBuf> body)
        noexcept  {
    DLOG(INFO) << "UploadHandler Receiving Body";
    sizeUploaded += body->length();
    std::shared_ptr<Slice> slice(new FileSlice(body->writableData(),
                                               body->length()));
    upload.Write(slice);
}

void UploadHandler::sendHeader() noexcept {
    DLOG(INFO) << "UploadHandler Sending Header";
    std::vector<std::string> names {"container-id", "content-id",
                "content-path", "content-version", "content-storage-policy",
                "content-chunk-method", "chunk-id", "chunk-hash", "chunk-pos",
                "chunk-size"};
    ResponseBuilder(downstream_).status(201, "Created");
    accessLog.StatusCode("201");
    DLOG(INFO) << "Sending status";
    for (auto &elem : names) {
        ResponseBuilder(downstream_).header<std::string>(
            xattr.HttpPrefix() + elem, xattr.getHTTP(elem));
    }
    ResponseBuilder(downstream_).header<std::string>("Content-Length",
                                                     std::to_string(size()))
            .sendWithEOM();
    requestCounter->incR2xxHits();
    DLOG(INFO) << "Header sent";
}

void UploadHandler::onEOM() noexcept  {
    DLOG(INFO) << "UploadHandler End of Message received";
    upload.Commit();
    accessLog.UserID(xattr.getHTTP("container-id"));
    sendHeader();
}

void UploadHandler::onUpgrade(proxygen::UpgradeProtocol)
        noexcept  {
    serviceLog.LogToPrint("INF", "Upgrade to h2c are not handled");
}

void UploadHandler::requestComplete() noexcept  {
    endOfRequest = std::time(nullptr);
    accessLog.LogToPrint("INF", xattr.getHTTP("chunk-id"));
}

void UploadHandler::onError(proxygen::ProxygenError err) noexcept  {
    serviceLog.LogToPrint("INF", getErrorString(err));
}

void RemovalHandler::Abort() noexcept {
    ResponseBuilder(downstream_).closeConnection();
}

bool RemovalHandler::headerCheck(proxygen::HTTPMessage* headers) {
    path = headers->getURL();
    if (path.empty())
        return false;
    chunk_id = path;
    path = path.substr(0, 3) + "/" + path;
    return true;
}

void RemovalHandler::onRequest(std::unique_ptr<proxygen::HTTPMessage> headers)
        noexcept  {
    serviceLog.SetUpBasic(FLAGS_server_hostname, FLAGS_instanceID);
    accessLog.SetUpBasic(FLAGS_server_hostname, FLAGS_instanceID);
    requestCounter->incDelHits();
    accessLog.RequestType("DELETE");
    beginOfRequest = std::time(nullptr);
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

void RemovalHandler::requestComplete() noexcept  {
    endOfRequest = std::time(nullptr);
    accessLog.LogToPrint("INF", chunk_id);
}

void RemovalHandler::onError(proxygen::ProxygenError err) noexcept {
    serviceLog.LogToPrint("INF", getErrorString(err));
}

void RemovalHandler::onUpgrade(proxygen::UpgradeProtocol) noexcept {
    serviceLog.LogToPrint("INF", "Upgrade to h2c are not handled");
}

bool RemovalHandler::GetClientAddr() {
    auto addr = downstream_->getSetupTransportInfo().remoteAddr;
    if (addr.get() != nullptr) {
        std::string clientHostname = addr->getAddressStr();
        clientHostname += addr->getPort();
        accessLog.RemoteClient(clientHostname);
        return true;
    }
    return false;
}

void StatHandler::onRequest(std::unique_ptr<proxygen::HTTPMessage>)
        noexcept {
    DLOG(INFO) << "StatHandling";
    serviceLog.SetUpBasic(FLAGS_server_hostname, FLAGS_instanceID);
    requestCounter->incStatHits();
    std::string content =  requestCounter->ToText();
    content += "config volume " + FLAGS_workingDirectory + "\n";
    ResponseBuilder(downstream_).status(200, "OK")
            .header("Content-Length", std::to_string(content.length()))
            .body(content)
            .sendWithEOM();
    DLOG(INFO) << "StatHandler Header sent";
}

void StatHandler::onBody(std::unique_ptr<folly::IOBuf>) noexcept {}

void StatHandler::onEOM() noexcept {}

void StatHandler::onUpgrade(proxygen::UpgradeProtocol) noexcept {
    serviceLog.LogToPrint("INF", "Upgrade to h2c are not handled");
}

void StatHandler::requestComplete() noexcept {}

void StatHandler::onError(proxygen::ProxygenError err) noexcept {
    serviceLog.LogToPrint("INF", getErrorString(err));
}

void InfoHandler::onRequest(std::unique_ptr<proxygen::HTTPMessage>)
        noexcept {
    serviceLog.SetUpBasic(FLAGS_server_hostname, FLAGS_instanceID);
    requestCounter->incInfoHits();
    std::string content = "namespace " + FLAGS_OIO_NS + "\n"
            + "path " + FLAGS_workingDirectory + "\n";
    ResponseBuilder(downstream_).status(200, "OK")
            .header("Content-Length", std::to_string(content.length()))
            .body(content)
            .sendWithEOM();
    DLOG(INFO) << "InfoHandler Header sent";
}

void InfoHandler::onBody(std::unique_ptr<folly::IOBuf>) noexcept {}

void InfoHandler::onEOM() noexcept {}

void InfoHandler::onUpgrade(proxygen::UpgradeProtocol) noexcept {
    serviceLog.LogToPrint("INF", "Upgrade to h2c are not handled");
}

void InfoHandler::requestComplete() noexcept {}

void InfoHandler::onError(proxygen::ProxygenError err) noexcept {
    serviceLog.LogToPrint("INF", getErrorString(err));
}
