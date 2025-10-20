/// Testing the paam class

#include<iostream>
#include<fstream>
#include<string>
#include<thread>
#include<unordered_map>
#include<queue>
#include<iomanip>
#include<math.h>
#include "paam.h"
#include "ofdm_tx.h"
#include "ofdm_rx.h"
#include "mmwall_ctl.h"

std::ofstream g_logfile;
std::ofstream g_rsrpfile;
std::unordered_map<std::string, int*> g_angle_map;
std::queue<std::string> g_path;

void parse_data_file(std::string file)
{
    std::ifstream angle_file;
    std::string line;
    std::string loc, tx_theta, rx_theta, wall_theta;
    angle_file.open(file);
    size_t num_data = 0;
    while(std::getline(angle_file, line)){
           //std::cout << line << std::endl;
           num_data++;
    };
    //std::cout << num_data << std::endl;
    angle_file.close();

    int** angles = new int*[num_data];
    angle_file.open(file);
    for(size_t i = 0 ; i < num_data; i++){
        std::getline(angle_file, line);
        std::stringstream linestr(line);
        linestr >> loc;
        linestr >> tx_theta;
        //linestr >> rx_theta;
	linestr >> wall_theta;


        angles[i] = new int[2];
        angles[i][0] = std::stoi(tx_theta);
        //angles[i][1] = std::stoi(rx_theta);
	angles[i][1] = std::stoi(wall_theta);
        g_angle_map.insert(std::pair<std::string, int*> (loc, angles[i]));
    }
	
}

void parse_path_file(std::string file)
{
    std::ifstream path_file;
    path_file.open(file);
    std::string line;
    while(std::getline(path_file, line)){
        g_path.push(line);
    };
}

float get_rsrp(ofdm_tx* pkt_tx, ofdm_rx* pkt_rx)
{
    pkt_tx->send_pkts();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    float avg_rsrp = 0;
    uint32_t rx_pkts = 0;
    float per = 0;
    pkt_rx->report_metrics(&avg_rsrp, &rx_pkts, &per, pkt_tx->get_num_pkts());
    g_logfile << std::fixed << std::setprecision(6) << std::setw(10)<< avg_rsrp << "  " << std::setw(5) << rx_pkts << "   "<< std::setprecision(3) << std::setw(6) << per << std::endl;
    return avg_rsrp;
}


int main(int argc, char* argv[])
{   
    int gnb_index = 0;
    int ue_index = 1;
    double speed = 12;
    std::string path_file;
    if(argc < 4){
	std::cout << "Please enter gNB index [0-2], UE index[1-2], second per meter, and the path file:" ;
    }else{
 	gnb_index = std::stoi(std::string(argv[1]));
	ue_index = std::stoi(std::string(argv[2])); 
	speed = std::stod(std::string(argv[3]));
	path_file = std::string(argv[4]);
    }    

    std::string angle_file = "lookup/surface_enb" + std::string(argv[1])  + "_ue" + std::string(argv[2]) + ".txt";
    parse_data_file(angle_file);
    parse_path_file(path_file);


    //Open Log file
    g_logfile.open("trace_log"+std::to_string(gnb_index)+"_ue"+std::to_string(ue_index) + ".txt");	
    

    // Create paam objects for PAAMs 
    std::string paam_tx_names[3] = {"rfdev4-in1.sb1.cosmos-lab.org", "rfdev4-in2.sb1.cosmos-lab.org", "rfdev-mob4-3.sb1.cosmos-lab.org"};
    paam paam_tx(paam_tx_names[gnb_index]);

    std::string paam_rx_name;
    
    /*
    if(ue_index == 1){
    	// mob4-1 is the receiver
	paam_rx_name = "rfdev-mob4-1.sb1.cosmos-lab.org";
    }
    else{
	// mob4-2 is the receiver
	paam_rx_name = "rfdev-mob4-2.sb1.cosmos-lab.org";
    }
    */

    paam_rx_name = "rfdev-mob4-1.sb1.cosmos-lab.org";
    paam paam_rx(paam_rx_name);


    // Enable the PAAMs 
    long cfo[4] = {250000, 0, 0, 0};
    paam_tx.enable("all", 16, "tx", 'v', cfo[gnb_index]);
    paam_rx.enable("all", 16, "rx", 'v', 0);

    // Create mmwall_ctl object 
    // mmWall IP: 10.37.23.1  || host IP (mob4-1): 10.37.21.1
    mmwall_ctl rpi1("10.37.23.1", 1234, "10.37.21.1", 3001);

    // Create ofdm_tx objects to send packets  
    // Packets are sent to OFDM tx grc on port 1234. 
    // No.of packets = 20, gap between packets = 0.6ms

    ofdm_tx pkt_tx;
    std::string pkt_tx_ip[3] = {"10.37.1.1", "10.37.1.2", "10.37.21.3"};
    pkt_tx = ofdm_tx(pkt_tx_ip[gnb_index].c_str(), 1234, 20, 0.6);    
   
    // Create ofdm_rx object for receiving packets
    // Packets are received, processed and rsrp data is sent to 2001, 2002
    ofdm_rx pkt_rx[2] = {{"10.37.21.1", 2001}, {"10.37.21.1", 2002}};

  
    // Start threads to recv rsrp data
    std::thread rx1_thread([&pkt_rx]() { pkt_rx[0].recv_pkt_data(); });
    std::thread rx2_thread([&pkt_rx]() { pkt_rx[1].recv_pkt_data(); });


    g_rsrpfile.open("rsrp"+std::to_string(gnb_index)+"_ue"+std::to_string(ue_index) + ".txt");
    

    g_logfile << "********************************************************************************" << std::endl;
    g_logfile << "Path file: "<< path_file << " Angle file: " << angle_file << std::endl;
    g_logfile << "********************************************************************************" << std::endl << std::endl;
    g_logfile << "Time       Loc         gNb Theta   UE Theta   Wall Theta    RSRP   NumPkts  PER" << std::endl; 
    g_logfile << "********************************************************************************" << std::endl;

    g_rsrpfile << "*******************************************************************************" << std::endl;
    g_rsrpfile << "Path file: "<< path_file << " Angle file: " << angle_file << std::endl;
    g_rsrpfile << "*******************************************************************************" << std::endl;

    char start;
    std::cout << "Start experiment [y/n]: " ;
    std::cin >> start;

    std::string loc = g_path.front();
    g_path.pop();

    if(start != 'y'){
        //return 0;
    }else{
	std::chrono::time_point<std::chrono::system_clock> start, timer, end;
        start = std::chrono::system_clock::now();
	timer = start;

	while(1){
    	    end = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_seconds = end - start;
            std::chrono::duration<double> timer_seconds = end - timer;

	    if(timer_seconds.count() > speed){
		timer = std::chrono::system_clock::now();
		loc = g_path.front();
                g_path.pop();
	    }		

	    int* loc_angle;
            int enb_theta, ue_theta, wall_theta;
	    auto loc_angle_pair = g_angle_map.find(loc);
	    
	    if(loc_angle_pair != g_angle_map.end()){
 	        loc_angle = loc_angle_pair->second;
	    }
	    else{
		std::cout << "Can't find location data" << loc << std::endl;
		break;
	    }
	    
	    g_logfile << std::setw(7) << double(elapsed_seconds.count()) << std::setw(7) << loc << "    "; 		
	    
	    enb_theta = *loc_angle;
	    //ue_theta = *(loc_angle + 1);
	    ue_theta = 0;
	    wall_theta = *(loc_angle + 1);

	    paam_tx.steer(enb_theta, 0);    
	    paam_rx.steer(ue_theta, 0);
    	    rpi1.steer(std::to_string(wall_theta));  

	    g_logfile << std::setw(7) << int(enb_theta) << "    " << std::setw(7)<< int(ue_theta) << "    " << std::setw(7) << int(wall_theta) << "    ";

	    float rsrp = get_rsrp(&pkt_tx, pkt_rx);
	    g_rsrpfile << std::setw(7) << rsrp << std::endl;
       }
    }

    std::cout << "Terminating the program..." << std::endl;

    paam_tx.steer(0,0);
    paam_tx.disable();
    paam_rx.steer(0,0);
    paam_rx.disable();
    g_logfile.close();
    g_rsrpfile.close();
    return 0;	
}

