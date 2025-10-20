#include "paam.h"
#include <string.h>
#include <vector.h>


class paam_ofdm : public paam
{
    private:
        string sdr_ip;
        uint32_t tx_port;
        uint32_t rx_port;
        std::vector<float> rx_time;
        std::vector<uint32_t> pkt_num;
        std::vector<float> rsrp;

    public:
        paam_ofdm(std::string name, std::string ip_addr, uint32_t tx_port, uint32_t rx_port, uint32_t data_capacity);
        void collect_data();
        void send_pkts(uint32_t num_pkts);         
        void report_metrics(float time, float* rsrp, uint32_t num_pkts);
      
}
