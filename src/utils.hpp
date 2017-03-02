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

#ifndef SRC_UTILS_HPP_
#define SRC_UTILS_HPP_

#include <fstream>
#include <mutex> //NOLINT
#include <utility>
#include <vector>
#include <string>
#include <gflags/gflags.h> //NOLINT



namespace utils {


/**
 * This structure contains the fields to log access to services
 */
class AccessLog {
 public:
    AccessLog() {}
    explicit AccessLog(std::fstream *fileStream)
            : logFileStream {fileStream} {}
    void SetUpBasic(std::string hostname, std::string instanceID);
    void LogToPrint(std::string level, std::string message);

    inline void RemoteClient(std::string remoteClient) {
        this->remoteClient = remoteClient;
    }
    inline void StatusCode(std::string statusCode) {
        this->statusCode = statusCode;
    }
    inline void RequestType(std::string requestType) {
        this->requestType = requestType;
    }
    inline void ResponseTime(std::string responseTime) {
        this->responseTime = responseTime;
    }
    inline void ResponseSize(std::string responseSize) {
        this->responseSize = responseSize;
    }
    inline void UserID(std::string userID) {
        this->userID = userID;
    }
    inline void RequestID(std::string requestID) {
        this->requestID = requestID;
    }
    inline void LocalServer(std::string localServer) {
        this->localServer = localServer;
    }
    inline void Hostname(std::string hostname) {
        this->hostname = hostname;
    }

 private:
    std::string timestamp;
    std::string hostname;
    std::string instanceID;
    std::string PID;
    std::string threadID;
    std::string logType;
    std::string level;
    std::string localServer;
    std::string remoteClient;
    std::string requestType;
    std::string statusCode;
    std::string responseTime;
    std::string responseSize;
    std::string userID {"-"};
    std::string requestID {"-"};
    std::string message;
    std::fstream *logFileStream;
};

/**
 * This structure contains the fields to log  information about the state of
 * the service, or errors the service encountered
 */
struct ServiceLog {
 public:
    ServiceLog() {}
    explicit ServiceLog(std::fstream *fileStream)
            : logFileStream {fileStream} {}
    void SetUpBasic(std::string hostname, std::string instanceID);
    void LogToPrint(std::string level, std::string message);
 private:
    std::string timestamp;
    std::string hostname;
    std::string instanceID;
    std::string PID;
    std::string threadID;
    std::string logType;
    std::string level;
    std::string message;
    std::fstream *logFileStream;
};

/**
 * This class permit to retrieve Extended Attribute and convert them to
 * OIO HTTP Headers format and vice versa
 */
struct XAttrTuple{
    std::string xattrName;
    std::string httpName;
    std::string value;
};

class XAttr {
 public:
    XAttr() {}
    bool retrieveXAttr(int fd);
    bool writeXAttr(int fd);
    bool addHTTP(std::string name, std::string value);
    bool addXAttr(std::string name, std::string value);
    std::string getHTTP(std::string name);
    std::string getXAttr(std::string name);
    inline std::string HttpPrefix() {return httpPrefix;}
    inline std::string XAttrPrefix() {return xattrPrefix;}
    std::vector<std::pair<std::string, std::string>> XAttrNamesValues();
    std::vector<std::pair<std::string, std::string>> HTTPNamesValues();

 private:
    std::vector<XAttrTuple> attributes {
        {"content.container", "container-id", ""},
        {"content.id", "content-id", ""},
        {"content.path", "content-path", ""},
        {"content.version", "content-version", ""},
        {"content.storage_policy", "content-storage-policy", ""},
        {"content.chunk_method", "content-chunk-method", ""},
        {"metachunk.size", "metachunk-size", ""},
        {"metachunk.hash", "metachunk-hash", ""},
        {"chunk.id", "chunk-id", ""},
        {"chunk.hash", "chunk-hash", ""},
        {"chunk.position", "chunk-pos", ""},
        {"chunk.size", "chunk-size", ""}
    };
    std::string httpPrefix {"X-oio-chunk-meta-"};
    std::string xattrPrefix {"user.grid."};
    size_t sizeBuffer;
};

class RequestCounter {
 public:
    void incPutTime(unsigned int time);
    void incGetTime(unsigned int time);
    void incDelTime(unsigned int time);
    void incStatTime(unsigned int time);
    void incinfoTime(unsigned int time);
    void incRawTime(unsigned int time);
    void incOtherTime(unsigned int time);
    void incPutHits();
    void incGetHits();
    void incDelHits();
    void incStatHits();
    void incInfoHits();
    void incRawHits();
    void incR2xxHits();
    void incR4xxHits();
    void incR5xxHits();
    void incOtherHits();
    void incR403Hits();
    void incR404Hits();
    void incBread(unsigned count);
    void incBwritten(unsigned count);
    std::string ToText();

 private:
    //
    unsigned int putTime;
    unsigned int getTime;
    unsigned int delTime;
    unsigned int statTime;
    unsigned int infoTime;
    unsigned int rawTime;
    unsigned int otherTime;
    //
    unsigned int putHits;
    unsigned int getHits;
    unsigned int delHits;
    unsigned int statHits;
    unsigned int infoHits;
    unsigned int rawHits;
    unsigned int r2xxHits;
    unsigned int r4xxHits;
    unsigned int r5xxHits;
    unsigned int otherHits;
    unsigned int r403Hits;
    unsigned int r404Hits;
    //
    unsigned int bread;
    unsigned int bwritten;
    //
    std::mutex rcMutex;
};

}  // namespace utils

#endif  // SRC_UTILS_HPP_
