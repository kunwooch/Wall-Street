// xytable.cpp

#include <iostream>
#include <string>
#include <curl/curl.h>
#include "xytable.h"


size_t xytable_write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
   return size * nmemb;
}

xytable_pos::xytable_pos(uint32_t x, uint32_t y, int angle)
{
    this->x = x;
    this->y = y;
    this->angle = angle;
}

xytable::xytable(std::string name)
{
    this->name = name;	
    this->curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, xytable_write_data);
    this->main_url = "http://am1.orbit-lab.org:5054/xy_table/";
}

void xytable::move(xytable_pos position)
{
    std::string url = main_url + "move_to?name=" +name + "&x=" +std::to_string(position.x) + "&y=" +std::to_string(position.y) + "&angle="+std::to_string(position.angle);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    CURLcode res = curl_easy_perform(curl);

}
