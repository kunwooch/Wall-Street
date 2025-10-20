#include "ofdm_tx.h"
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

ofdm_tx::ofdm_tx(){}

ofdm_tx::ofdm_tx(const char* ip_addr, uint32_t tx_port, uint32_t num_pkts, double interval)
{
    sdr_ip = ip_addr;
    this->tx_port = tx_port;
    this->num_pkts = num_pkts;
    this->interval = interval;
}

uint32_t ofdm_tx::get_num_pkts()
{
    return this->num_pkts;	
}

void ofdm_tx::send_pkts()
{
    //std::chrono::duration<double, std::milli> gap(this->interval);	
    //Create a UDP socket
    int tx_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(tx_socket < 0){
        std::cerr << "Error creating socket" << std::endl;	    
    }    

    //Server address
    struct sockaddr_in ofdm_tx_addr;
    memset(&ofdm_tx_addr, 0, sizeof(ofdm_tx_addr));
    ofdm_tx_addr.sin_family = AF_INET;
    ofdm_tx_addr.sin_port = htons(tx_port);
    ofdm_tx_addr.sin_addr.s_addr = inet_addr(sdr_ip);

     //std::string message = "Hello from the C++ program!!!!Hello from the C++ program!!!!Hello from the C++ program!!!!Hello from the";  
    //std::string message = "Hello from the C++ program!!!!Hello from the C++ program!!!!Hello from the C++ program!!!!Hello from the C++ program!!!!Hello from the C++ program!!!!Hello from the C++ program!!!!Hello from the C++ program!!!!Hello from the C++ program!!!!";  
 
    /*uint32_t bytes_sent = 0;
    uint32_t error_pkts = 0;
    for(int i = 0; i < this->num_pkts; i++){
        bytes_sent = sendto(tx_socket, message.c_str(), message.size(), 0, (const struct sockaddr*)&ofdm_tx_addr, sizeof(ofdm_tx_addr));    
        if(bytes_sent < 0){
            std::cerr << "Error sending message" << std::endl;
	    error_pkts++;
	}
	std::this_thread::sleep_for(gap);
    }*/
    //std::cout << "Sent " << (num_pkts-error_pkts) << " packets to the OFDM transmitter" << std::endl;

    float send_data[2];
    send_data[0] = (float)this->num_pkts;
    send_data[1] = (float)this->interval;
    uint32_t bytes_sent = sendto(tx_socket, send_data, sizeof(send_data) , 0, (const struct sockaddr*)&ofdm_tx_addr, sizeof(ofdm_tx_addr));
    close(tx_socket);
}

void ofdm_tx::send_pkts_grc()
{
    std::chrono::duration<double, std::milli> gap(this->interval);
    //Create a UDP socket
    int tx_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(tx_socket < 0){
        std::cerr << "Error creating socket" << std::endl;
    }

    //Server address
    struct sockaddr_in ofdm_tx_addr;
    memset(&ofdm_tx_addr, 0, sizeof(ofdm_tx_addr));
    ofdm_tx_addr.sin_family = AF_INET;
    ofdm_tx_addr.sin_port = htons(tx_port);
    ofdm_tx_addr.sin_addr.s_addr = inet_addr(sdr_ip);

    std::string message = "Hello from the C++ program!!!!Hello from the C++ program!!!!Hello from the C++ program!!!!Hello from the";
    //std::string message = "Hello from the C++ program!!!!Hello from the C++ program!!!!Hello from the C++ program!!!!Hello from the C++ program!!!!Hello from the C++ program!!!!Hello from the C++ program!!!!Hello from the C++ program!!!!Hello from the C++ program!!!!";

    uint32_t bytes_sent = 0;
    uint32_t error_pkts = 0;
    for(int i = 0; i < this->num_pkts; i++){
        bytes_sent = sendto(tx_socket, message.c_str(), message.size(), 0, (const struct sockaddr*)&ofdm_tx_addr, sizeof(ofdm_tx_addr));
        if(bytes_sent < 0){
            std::cerr << "Error sending message" << std::endl;
            error_pkts++;
        }
        std::this_thread::sleep_for(gap);
    }
    //std::cout << "Sent " << (num_pkts-error_pkts) << " packets to the OFDM transmitter" << std::endl;

    //float send_data[2];
    //send_data[0] = (float)this->num_pkts;
    //send_data[1] = (float)this->interval;
    //uint32_t bytes_sent = sendto(tx_socket, send_data, sizeof(send_data) , 0, (const struct sockaddr*)&ofdm_tx_addr, sizeof(ofdm_tx_addr));
    close(tx_socket);
}

