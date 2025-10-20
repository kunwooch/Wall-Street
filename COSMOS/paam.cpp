// paam.cpp

#include <iostream>
#include <string>
#include <curl/curl.h>
#include "paam.h"


size_t paam_write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
   return size * nmemb;
}


paam::paam(){}

paam::paam(std::string name)
{
    this->name = name;	
    this->curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, paam_write_data);
    //main_url = "http://"+name+":3000/array_mgmt/";     For later when the service is running on the PAAM
    this->main_url = "http://am1.orbit-lab.org:5054/array_mgmt/";
}

void paam::init_beam_table(){}

void paam::enable(std::string ic, int num_elements, std::string txrx, char pol, long cfo)
{
    std::string url = main_url + "connect?dev_name=" +name+ "&ics=" +ic+ "&num_elements=" +std::to_string(num_elements)+ "&txrx=" +txrx+ "&pol="+pol+ "&cfo=" +std::to_string(cfo);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    CURLcode res = curl_easy_perform(curl);


    url = main_url + "enable?dev_name=" +name+ "&ics=" +ic+ "&num_elements=" +std::to_string(num_elements)+ "&txrx=" +txrx+ "&pol="+pol+ "&cfo=" +std::to_string(cfo);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    res = curl_easy_perform(curl);
    //std::cout << name << std::endl;
    //std::cout << url << " " << res;
}


void paam::configure(std::string ic, int num_elements, std::string txrx, char pol, int theta, int phi)
{
    std::string url = main_url + "configure?dev_name=" +name+ "&ics=" +ic+ "&num_elements=" +std::to_string(num_elements)+ "&txrx=" +txrx+ "&pol="+pol+ "&theta="+std::to_string(theta)+ "&phi="+std::to_string(phi)+ "&cfo=0";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    CURLcode res = curl_easy_perform(curl);
    //std::cout << name << std::endl;
    //std::cout << url << " " << res;
}

void paam::steer(int theta, int phi){

    std::string url = main_url + "steer?dev_name=" +name+ "&theta=" +std::to_string(theta)+ "&phi="+std::to_string(phi);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    CURLcode res = curl_easy_perform(curl);

}
void paam::switch_beam_index(int index){}

void paam::disable()
{
    	
    std::string url = main_url + "cleanup?dev_name=" +name;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    CURLcode res = curl_easy_perform(curl);

    url = main_url + "disconnect?dev_name=" +name;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    res = curl_easy_perform(curl);
}

