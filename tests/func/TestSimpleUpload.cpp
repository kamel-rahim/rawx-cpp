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

#include <gflags/gflags.h>
#include "rawx.hpp"

DEFINE_int32(http_port, 11000, "HTTP port");
DEFINE_string(ip, "localhost", "IP to bind to");
DEFINE_int32(threads, 1, "Number of threads to listen on");


int main(int argc, char * argv[]) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    std::vector<HTTPServer::IPConfig> IPs = {
        {SocketAddress(FLAGS_ip, http_port, true), Protocol::HTTP}
    };

HTTPServerOptions options;
options.threads = static_cast<size_t>(FLAGS_threads);
options.idleTimeout = std::chrono::milliseconds(60000);
options.shutdownOn = {SIGINT, SIGTERM};
options.enableContentCompression = false;
options.handlerFactories = RequestHandlerChain()
        .addThen<RawxHandlerFactory>()
        .build();
options.h2cEnabled = true;

HTTPServer server(std::move(options));
server.bind(IPs);

std::thread t([&] (){
        server.start();
    });

t.join();
return 0;
}

