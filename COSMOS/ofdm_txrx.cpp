#include "ofdm_txrx.h"
#include <iostream>
#include <unistd.h>
#include <string>
#include <cstring>
#include <queue>
#include <cstdint>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


ofdm_txrx::ofdm_txrx(const char* ip_addr, uint32_t tx_port, uint32_t rx_port, uint32_t data_capacity)
{
    sdr_ip = ip_addr;
    this->tx_port = tx_port;
    this->rx_port = rx_port;
    //rx_time.reserve(data_capacity);
    //pkt_num.reserve(data_capacity);
    //rsrp.reserve(data_capacity);
    rx_socket = -1;
}

void ofdm_txrx::recv_pkt_data()
{
    	
    //Create a UDP socket
    rx_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(rx_socket < 0){
    std::cerr << "Error creating socket" << std::endl;	   
    } 

    //Server address
    struct sockaddr_in ofdm_rx_addr;
    memset(&ofdm_rx_addr, 0, sizeof(ofdm_rx_addr));
    ofdm_rx_addr.sin_family = AF_INET;
    ofdm_rx_addr.sin_port = htons(rx_port);
    ofdm_rx_addr.sin_addr.s_addr = inet_addr(sdr_ip);

    //Bind the socket to the server address
    if(bind(rx_socket, (struct sockaddr *)&ofdm_rx_addr, sizeof(ofdm_rx_addr)) < 0){
	std::cerr << "Error binding socket" << std::endl;     
	close(rx_socket);
    }

    
    float recv_data[4];
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    uint32_t bytes_recvd;
    while(true){
        bytes_recvd = recvfrom(rx_socket, (char*)recv_data, sizeof(recv_data), 0, (struct sockaddr*)&client_addr, &client_addr_len); 
	if(recv_data[0] >= 0){
	    //std::cout << recv_data[0] << " " << recv_data[1] << " " << recv_data[2] << " " << recv_data[3]<< std::endl;
            rx_time_q.push(recv_data[0] + recv_data[1]);
	    pkt_num_q.push(recv_data[2]);
	    rsrp_q.push(recv_data[3]);
	}
    }
    	
}

// interval in milliseconds
void ofdm_txrx::send_pkts(uint32_t num_pkts, double interval)
{
    std::chrono::duration<double, std::milli> gap(interval);	
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
//    std::string message = "Hello from the C";  
 
    uint32_t bytes_sent = 0;
    uint32_t error_pkts = 0;
    for(int i = 0; i < num_pkts; i++){
        bytes_sent = sendto(tx_socket, message.c_str(), message.size(), 0, (const struct sockaddr*)&ofdm_tx_addr, sizeof(ofdm_tx_addr));    
        if(bytes_sent < 0){
            std::cerr << "Error sending message" << std::endl;
	    error_pkts++;
	}
	std::this_thread::sleep_for(gap);
    }
    //std::cout << "Sent " << (num_pkts-error_pkts) << " packets to the OFDM transmitter" << std::endl;
    close(tx_socket);
}

// Reports avg rsrp and packet error rate (only headers). Takes the number of tx packets as input
void ofdm_txrx::report_metrics(float* rsrp, float* per, uint32_t tx_num_pkts)
{
    double sum_rsrp = 0;
    int32_t pkt_cnt = 0;
    while(!rsrp_q.empty()){
	//std::cout << rsrp_q.front() << std::endl;    
	sum_rsrp += (double)rsrp_q.front();    
	rsrp_q.pop();
	pkt_cnt++;
    }
    *rsrp = (float)sum_rsrp/pkt_cnt;
    *per = ((float)(tx_num_pkts-pkt_cnt)/tx_num_pkts)*100;
}
        
