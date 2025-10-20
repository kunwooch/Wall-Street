#include "ofdm_rx.h"
#include <iostream>
#include <unistd.h>
#include <string>
#include <cstring>
#include <queue>
#include <set>
#include <cstdint>
#include <thread>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


ofdm_rx::ofdm_rx(const char* ip_addr, uint32_t rx_port)
{
    sdr_ip = ip_addr;
    this->rx_port = rx_port;
    rx_socket = -1;
}

void ofdm_rx::recv_pkt_data()
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
            //rx_time_q.push(recv_data[0] + recv_data[1]); Not using rx time yet
	    pkt_num_q.push(recv_data[2]);
	    rsrp_q.push(recv_data[3]);
	}
    }
    	
}


// Reports avg rsrp and packet error rate (only headers). Takes the number of tx packets as input
void ofdm_rx::report_metrics(float* rsrp, uint32_t* rx_num_pkts, float* per, uint32_t tx_num_pkts)
{
    double sum_rsrp = 0;
    int32_t pkt_cnt = 0;
    while(!rsrp_q.empty()){
	//std::cout << rsrp_q.front() << std::endl;    
	sum_rsrp += (double)rsrp_q.front();    
	rsrp_q.pop();
	pkt_num_q.pop();
	pkt_cnt++;
    }
    float avg_rsrp = (float)sum_rsrp/pkt_cnt;
    *rsrp = 10*log10(avg_rsrp);
    *rx_num_pkts = pkt_cnt;
    *per = ((float)(tx_num_pkts-pkt_cnt)/tx_num_pkts)*100;
}
       
// Reports avg rsrp and packet error rate (only headers). Takes the number of tx packets as input
void ofdm_rx::report_metrics(float* rsrp, uint32_t* rx_num_pkts, float* per, uint32_t tx_num_pkts, std::set<uint32_t>& pkt_num_set)
{
    double sum_rsrp = 0;
    int32_t pkt_cnt = 0;
    while(!rsrp_q.empty()){
        // std::cout << rsrp_q.front() << std::endl;
        sum_rsrp += (double)rsrp_q.front();
        rsrp_q.pop();
	//std::cout << pkt_num_q.front() << std::endl;
        pkt_num_set.insert(pkt_num_q.front());
        pkt_num_q.pop();
        pkt_cnt++;
    }
    float avg_rsrp = (float)sum_rsrp/pkt_cnt;
    *rsrp = 10*log10(avg_rsrp);
    *rx_num_pkts = pkt_cnt;
    *per = ((float)(tx_num_pkts-pkt_cnt)/tx_num_pkts)*100;
}

