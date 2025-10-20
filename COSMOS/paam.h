// paam.h

#include<string>
#include<curl/curl.h>


class paam
{
    public:	
	std::string name;
        CURL* curl;
	std::string main_url;

	paam();
	paam(std::string name);
	void init_beam_table();
	void enable(std::string ic, int num_elements, std::string txrx, char pol, long cfo); 
	void configure(std::string ic, int num_elements, std::string txrx, char pol, int theta, int phi); 
	void steer(int theta, int phi);
	void switch_beam_index(int index);
	void disable();
};
