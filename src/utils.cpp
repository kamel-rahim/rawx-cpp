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

#include <pthread.h>
#include <gflags/gflags.h>
#include <attr/xattr.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctime>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include "utils.hpp"

using utils::XAttr;
using utils::AccessLog;
using utils::ServiceLog;
using utils::RequestCounter;

void AccessLog::SetUpBasic(std::string hostname, std::string instanceID) {
    this->hostname = hostname;
    this->instanceID = instanceID;
    this->PID = ::getpid();
    this->threadID = pthread_self();
    this->logType = "access";
}

void ServiceLog::SetUpBasic(std::string hostname, std::string instanceID) {
    this->hostname = hostname;
    this->instanceID = instanceID;
    this->PID = ::getpid();
    this->threadID = pthread_self();
    this->logType = "log";
}

void ServiceLog::LogToPrint(std::string level, std::string message) {
    std::time_t stime = std::time(nullptr);
    std::tm *ltime = std::localtime(&stime);
    char time_print[30];
    std::strftime(time_print, 30, "%b %d %H:%M:%S", ltime);
    this->timestamp = std::string(time_print);
    this->level = level;
    this->message = message;
    *logFileStream << timestamp << " " << hostname << " " <<  instanceID
                   << " " << PID << " " << threadID << " " << logType << " "
                   << level << " " << message << std::endl;
}

void AccessLog::LogToPrint(std::string level, std::string message) {
    std::time_t stime = std::time(nullptr);
    std::tm *ltime = std::localtime(&stime);
    char time_print[30];
    std::strftime(time_print, 30, "%b %d %H:%M:%S", ltime);
    this->timestamp = std::string(time_print);
    this->level = level;
    this->message = message;
    *logFileStream << timestamp << " " << hostname << " " << instanceID << " "
                   << PID << " " << threadID << " " << logType << " " << level
                   << " " << localServer << " " << remoteClient << " "
                   << requestType << " " << statusCode << " " << responseTime
                   << " " << responseSize << " " << userID << " " << requestID
                   << " " << message << std::endl;
}


bool XAttr::retrieveXAttr(int fd) {
    char buffer[sizeBuffer]; // NOLINT
    int size = sizeBuffer;
    for (auto &elem : attributes) {
        if (fgetxattr(fd, (xattrPrefix + elem.xattrName).c_str(), buffer,
                      size) != 0) {
            // TODO(KR): check the error (LOG)
        } else {
            elem.value = std::string(buffer, size);
        }
        size = sizeBuffer;
    }
    return true;
}

bool XAttr::writeXAttr(int fd) {
    for (auto &elem : attributes) {
        if (elem.value.empty())
            continue;
        if (fsetxattr(fd, (xattrPrefix + elem.xattrName).c_str(),
                      elem.value.c_str(), elem.value.size(), XATTR_CREATE)
            != 0) {
            // TODO(KR): check the error (LOG)
        }
    }
    return true;
}

std::vector<std::pair<std::string, std::string>> XAttr::XAttrNamesValues() {
    std::vector<std::pair<std::string, std::string>> namesValues;
    for (auto &elem : attributes) {
        namesValues.push_back(std::pair<std::string, std::string>
                              (xattrPrefix + elem.xattrName, elem.value));
    }
    return namesValues;
}

std::vector<std::pair<std::string, std::string>> XAttr::HTTPNamesValues() {
    std::vector<std::pair<std::string, std::string>> namesValues;
    for (auto &elem : attributes) {
        namesValues.push_back(std::pair<std::string, std::string>
                              (httpPrefix + elem.httpName, elem.value));
    }
    return namesValues;
}

bool XAttr::addHTTP(std::string name, std::string value) {
    for (auto &elem : attributes) {
        if (elem.httpName == name) {
            elem.value = value;
            return true;
        }
    }
    return false;
}

bool XAttr::addXAttr(std::string name, std::string value) {
    for (auto &elem : attributes) {
        if (elem.xattrName == name) {
            elem.value = value;
            return true;
        }
    }
    return false;
}

std::string XAttr::getHTTP(std::string name) {
    for (auto &elem : attributes) {
        if (elem.httpName == name) {
            return elem.value;
        }
    }
    return std::string();
}

std::string XAttr::getXAttr(std::string name) {
    for (auto &elem : attributes) {
        if (elem.xattrName == name) {
            return elem.value;
        }
    }
    return std::string();
}

void RequestCounter::incPutTime(unsigned int time) {
    rcMutex.lock();
    putTime += time;
    rcMutex.unlock();
}
void RequestCounter::incGetTime(unsigned int time) {
    rcMutex.lock();
    getTime += time;
    rcMutex.unlock();
}
void RequestCounter::incDelTime(unsigned int time) {
    rcMutex.lock();
    delTime += time;
    rcMutex.unlock();
}
void RequestCounter::incStatTime(unsigned int time) {
    rcMutex.lock();
    statTime += time;
    rcMutex.unlock();
}
void RequestCounter::incinfoTime(unsigned int time) {
    rcMutex.lock();
    infoTime += time;
    rcMutex.unlock();
}
void RequestCounter::incRawTime(unsigned int time) {
    rcMutex.lock();
    rawTime += time;
    rcMutex.unlock();
}
void RequestCounter::incOtherTime(unsigned int time) {
    rcMutex.lock();
    otherTime += time;
    rcMutex.unlock();
}

void RequestCounter::incPutHits() {
    rcMutex.lock();
    putHits++;
    rcMutex.unlock();
}
void RequestCounter::incGetHits() {
    rcMutex.lock();
    getHits++;
    rcMutex.unlock();
}
void RequestCounter::incDelHits() {
    rcMutex.lock();
    delHits++;
    rcMutex.unlock();
}
void RequestCounter::incStatHits() {
    rcMutex.lock();
    statHits++;
    rcMutex.unlock();
}
void RequestCounter::incInfoHits() {
    rcMutex.lock();
    infoHits++;
    rcMutex.unlock();
}
void RequestCounter::incRawHits() {
    rcMutex.lock();
    rawHits++;
    rcMutex.unlock();
}
void RequestCounter::incR2xxHits() {
    rcMutex.lock();
    r2xxHits++;
    rcMutex.unlock();
}
void RequestCounter::incR4xxHits() {
    rcMutex.lock();
    r4xxHits++;
    rcMutex.unlock();
}
void RequestCounter::incR5xxHits() {
    rcMutex.lock();
    r5xxHits++;
    rcMutex.unlock();
}
void RequestCounter::incOtherHits() {
    rcMutex.lock();
    otherHits++;
    rcMutex.unlock();
}
void RequestCounter::incR403Hits() {
    rcMutex.lock();
    r403Hits++;
    rcMutex.unlock();
}
void RequestCounter::incR404Hits() {
    rcMutex.lock();
    r404Hits++;
    rcMutex.unlock();
}
void RequestCounter::incBread(unsigned count) {
    rcMutex.lock();
    bread += count;
    rcMutex.unlock();
}
void RequestCounter::incBwritten(unsigned count) {
    rcMutex.lock();
    bwritten += count;
    rcMutex.unlock();
}

std::string RequestCounter::ToText() {
    std::stringstream ss;
    rcMutex.lock();
    ss << "counter req.time " << (putTime + getTime + delTime + statTime +
                                  rawTime + otherTime) << std::endl;
    ss << "counter req.time.put " << putTime << std::endl;
    ss << "counter req.time.get " << getTime << std::endl;
    ss << "counter req.time.del " << delTime << std::endl;
    ss << "counter req.time.stat " << statTime << std::endl;
    ss << "counter req.time.info " << infoTime << std::endl;
    ss << "counter req.time.raw " << rawTime << std::endl;
    ss << "counter req.time.other " << otherTime << std::endl;
    ss << "counter req.hits.put " << putHits << std::endl;
    ss << "counter req.hits.get " << getHits << std::endl;
    ss << "counter req.hits.del " << delHits << std::endl;
    ss << "counter req.hits.stat " << statHits << std::endl;
    ss << "counter req.hits.info " << infoHits << std::endl;
    ss << "counter req.hits.raw " << rawHits << std::endl;
    ss << "counter req.hits.2xx " << r2xxHits << std::endl;
    ss << "counter req.hits.4xx " << r4xxHits << std::endl;
    ss << "counter req.hits.5xx " << r5xxHits << std::endl;
    ss << "counter req.hits.other " << otherHits << std::endl;
    ss << "counter req.hits.403 " << r403Hits << std::endl;
    ss << "counter req.hits.404 " << r404Hits << std::endl;
    //
    ss << "counter req.hits.bread " << bread << std::endl;
    ss << "counter req.hits.bwritten " << bwritten << std::endl;
    rcMutex.unlock();

    return ss.str();
}
