#include <string>
#include <cstdint>


class ofdm_tx 
{
    private:
	const char* sdr_ip;
        uint32_t tx_port;
	uint32_t num_pkts;
	double interval;

    public:
	ofdm_tx();

        ofdm_tx(const char* ip_addr, uint32_t tx_port, uint32_t num_pkts, double interval);
        
	uint32_t get_num_pkts();

	void send_pkts();         
	void send_pkts_grc();         
      
};
