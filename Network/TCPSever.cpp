#include "../Network/TCPServer.hpp"
#include "../Network/Packet.hpp"
#include "../Network/Encode/Deserializer.hpp"
#include<iostream>

bool TCPServer::Start(){
    // gttting the server socketFD
    serverFD = socket(AF_INET , SOCK_STREAM , 0); //domain- AF_INET (IPV4) , type- SOCK_STREAM(TCP) , protocl- 0 (for any prototcl)

    if(serverFD < 0){
        // if failed to get then return .. not able to start server
        return false;
    }

    int opt =1;

    setsockopt(serverFD, SOL_SOCKET , SO_REUSEADDR ,&opt,  sizeof(opt));

    sockaddr_in addr {};

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    // binding to the socket with IPV$ , host to network short , sizeof(addr) t bind with
    if(bind(serverFD , (sockaddr*)&addr, sizeof(addr)) < 0){ // not bind, bind failed
        // if not bind then close the server .. cause no use of server if not bind to port
        std::cerr << "bind failed: " << strerror(errno)
              << " (errno = " << errno << ")\n";
        close(serverFD);

        return false;
    }

    // listening on serverFD , and 128 is the connections to what is get queeud on listing
    if(listen(serverFD, 128) < 0){ // listrening faled
        // close if not listeing 
        close(serverFD);
        return false;
    }
    running = true;

    // running the accept loop after all setup
    acceptThread = std::thread(&TCPServer::Accept, this);
    return true;
}

// accption loop accpting the incoming clients 
void TCPServer::Accept(){
    while(running){
        sockaddr_in clientAddr {};
        socklen_t len = sizeof(clientAddr);

        // accepting the connection coming frmo socketFD , and storeing that info 
        int client = accept(serverFD, (sockaddr*)&clientAddr , &len);
        // i flcien is not foind then skip and start wijt other client coming from
        if(client <0){
            continue;
        }

        // client loop , pushingn back to clientThread the client strcuture  will updagre to pooling later on
        clientThread.emplace_back(&TCPServer::ClientLoop, this, client);
    }
}

// client loops
void TCPServer::ClientLoop(int clientFd){
    while(running){
        PacketHeader header;
        size_t n = recv(clientFd, &header, sizeof(header), MSG_WAITALL);

        // if nothing recived break loop
        if(n <= 0){
            break;
        }
        // converting from networ byte order to host byte order(litte endian)
        header.length = ntohl(header.length);

        // using vetor of char as raw byte container
        std::vector<char> body(header.length);

        n = recv(clientFd, body.data(), body.size(), MSG_WAITALL);

        if(n <= 0) break;
        // parse 
        Command cmd = _deserializer.Deserialize(header,body);
        
        // create command
        dispatcher.submit(std::move(cmd));
    }
    close(clientFd);
}

void TCPServer::Stop(){
    if(!running){
        return;
    }

    running = false;

    // shutdown & close
    shutdown(serverFD , SHUT_RDWR);
    close(serverFD);

    if(acceptThread.joinable()){
        acceptThread.join();
    }
    for(auto& t : clientThread){
        if(t.joinable()){
            t.join();
        }
    }

}