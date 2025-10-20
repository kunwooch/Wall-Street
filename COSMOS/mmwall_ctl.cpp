#include "mmwall_ctl.h"
#include <iostream>
#include <unistd.h>
#include <string>
#include <cstring>
#include <queue>
#include <cstdint>
#include <thread>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

mmwall_ctl::mmwall_ctl(){}

mmwall_ctl::mmwall_ctl(const char* rpi_ip_addr, uint32_t rpi_port, const char* host_ip_addr, uint32_t host_port)
{
    rpi_ip = rpi_ip_addr;
    this->rpi_port = rpi_port;
    host_ip = host_ip_addr;
    this-> host_port = host_port;
}

void mmwall_ctl::steer(std::string angle)
{
    //Create a UDP tx socket
    int tx_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(tx_socket < 0){
        std::cerr << "Error creating socket" << std::endl;	    
    }   

    //Create a UDP rx socket
    int rx_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(rx_socket < 0){
        std::cerr << "Error creating socket" << std::endl;	    
    }    

    //mmwall  address
    struct sockaddr_in mmwall_ctl_addr;
    memset(&mmwall_ctl_addr, 0, sizeof(mmwall_ctl_addr));
    mmwall_ctl_addr.sin_family = AF_INET;
    mmwall_ctl_addr.sin_port = htons(rpi_port);
    mmwall_ctl_addr.sin_addr.s_addr = inet_addr(rpi_ip);

    //host address
    struct sockaddr_in host_addr;
    memset(&host_addr, 0, sizeof(host_addr));
    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(host_port);
    host_addr.sin_addr.s_addr = inet_addr(host_ip);

    //Bind the rx socket to the host address
    if(bind(rx_socket, (struct sockaddr *)&host_addr, sizeof(host_addr)) < 0){
        std::cerr << "Error binding socket" << std::endl;
        close(rx_socket);
    }

    float recv_data[4];
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    uint32_t bytes_recvd;

    //send angle to the mmwall
    uint32_t bytes_sent = sendto(tx_socket, angle.c_str(), angle.length() , 0, (const struct sockaddr*)&mmwall_ctl_addr, sizeof(mmwall_ctl_addr));
    close(tx_socket);

    // wait for the mmwall to send an ack saying done
    bytes_recvd = recvfrom(rx_socket, (char*)recv_data, sizeof(recv_data), 0, (struct sockaddr*)&client_addr, &client_addr_len);

    //std::cout << bytes_recvd;
    if(bytes_recvd == 4)
        close(rx_socket);	    
}
