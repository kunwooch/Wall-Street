#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

int main(){

    std::unordered_map<std::string, int*> a;
    int** angles = new int*;


    std::ifstream angle_file;
    std::string line;
    std::string loc, tx_theta, rx_theta;
    angle_file.open("enb0_angle_info.txt");
    size_t cnt = 0;
    while(std::getline(angle_file, line)){
	    std::cout << line << std::endl;
	    std::stringstream linestr(line);
	    linestr >> loc;
	    linestr >> tx_theta;
	    linestr >> rx_theta;

            std::cout << "** " << loc << " " << tx_theta << " " << rx_theta << std::endl;

	    *(angles + cnt) = new int[2];
	    angles[cnt][0] = std::stoi(tx_theta);
	    angles[cnt][1] = std::stoi(rx_theta);
	    a.insert(std::pair<std::string, int*> (loc, angles[cnt])); 
	    cnt++;
    }    
    //std::unordered_map<std::string, int[3]> a = { {"0_0_45", {-36, 15, 12}}, {"0_1_45", {-36, 10, 12}}, {"0_2_45", {-3, 1, 2}}};	
    //std::unordered_map<std::string, int> a = { {"0_0_45", -36}, {"0_1_45", -3}, {"0_2_45", -2} };	
    //int angle[3] = {1, -10, 20};
    //int angle1[3] = {15, -100, 2};
    //std::unordered_map<std::string, int*> a = { {"0_0_45", angle}, {"0_1_45", angle}, {"0_2_45", angle1} };	

   
    for(const std::pair<std::string, int*>& n : a){
	std::cout << "Key: " << n.first << " ";
	int* tmp = n.second;
        std::cout << tmp[0] << " " << tmp[1];
        std::cout << std::endl;	
        	
    }
}

