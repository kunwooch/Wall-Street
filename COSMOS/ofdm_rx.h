#include <string>
#include <queue>
#include <set>
#include <cstdint>


class ofdm_rx 
{
    private:
	const char* sdr_ip;
        uint32_t rx_port;
	int rx_socket;
	std::queue<float> rx_time_q;
	std::queue<uint32_t> pkt_num_q;
	std::queue<float> rsrp_q;

    public:
        ofdm_rx(const char* ip_addr, uint32_t rx_port);
        void recv_pkt_data();
	void report_metrics(float* rsrp, uint32_t* rx_num_pkts, float* per, uint32_t tx_num_pkts);
	void report_metrics(float* rsrp, uint32_t* rx_num_pkts, float* per, uint32_t tx_num_pkts, std::set<uint32_t>& pkt_num_set);
      
};
