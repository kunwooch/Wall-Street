#include "paam_ofdm.h"
#include <string.h>
#include <vector.h>

paam_ofdm::paam_ofdm(std::string name, std::string ip_addr, uint32_t tx_port, uint32_t rx_port, uint32_t data_capacity)
{
    sdr_ip = ip_addr;
    this->tx_port = tx_port;
    this->rx_port = rx_port;
    rx_time.reserve(data_capacity);
    pkt_num.reserve(data_capacity);
    rsrp.reserve(data_capacity);
}

void paam_ofdm::collect_data(){}

void paam_ofdm::send_pkts(){}

void paam_ofdm::report_metrics(float time, float*rsrp, uint32_t num_pkts){}
        
