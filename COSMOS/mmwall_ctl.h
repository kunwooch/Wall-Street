#include <string>
#include <cstdint>


class mmwall_ctl 
{
    private:
	const char* rpi_ip;
        uint32_t rpi_port;
	const char* host_ip;
        uint32_t host_port;

    public:
	mmwall_ctl();

        mmwall_ctl(const char* rpi_ip_addr, uint32_t rpi_port, const char* host_ip_addr, uint32_t host_port);
        
	void steer(std::string angle);         
      
};
