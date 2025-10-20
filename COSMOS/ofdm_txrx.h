#include <string>
#include <queue>
#include <cstdint>


class ofdm_txrx 
{
    private:
	const char* sdr_ip;
        uint32_t tx_port;
        uint32_t rx_port;
	int rx_socket;
	std::queue<float> rx_time_q;
	std::queue<uint32_t> pkt_num_q;
	std::queue<float> rsrp_q;

    public:
        ofdm_txrx(const char* ip_addr, uint32_t tx_port, uint32_t rx_port, uint32_t data_capacity);
        void recv_pkt_data();
        void send_pkts(uint32_t num_pkts, double interval);         
        void report_metrics(float* rsrp, float* per, uint32_t tx_num_pkts);
      
};
