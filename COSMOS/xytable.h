// xytable.h

#include<string>
#include<curl/curl.h>


class xytable_pos
{
    public:
        uint32_t x;
        uint32_t y;
	int angle;

	xytable_pos(uint32_t x, uint32_t y, int angle);     	
};

class xytable
{
    public:	
	std::string name;
        CURL* curl;
	std::string main_url;

	xytable(std::string name);
	void move(xytable_pos position);
};
