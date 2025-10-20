/// Testing the paam class

#include<iostream>
#include<fstream>
#include<string>
#include<thread>
#include<set>
#include<unordered_map>
#include<iomanip>
#include<math.h>
#include "paam.h"
#include "ofdm_tx.h"
#include "ofdm_rx.h"
#include "mmwall_ctl.h"


std::ofstream g_logfile;
std::unordered_map<std::string, int*> g_angle_map[2];
std::queue<std::string> g_path;

void parse_data_file(uint32_t enb_index, std::string file)
{
    std::ifstream angle_file;
    std::string line;
    std::string loc, tx_theta, rx_theta;
    angle_file.open(file);
    size_t num_data = 0;
    while(std::getline(angle_file, line)){
            //std::cout << line << std::endl;
            num_data ++;
    };
    angle_file.close();

    int** angles = new int*[num_data];
    angle_file.open(file);
    for(size_t i = 0 ; i < num_data; i++){
        std::getline(angle_file, line);
        std::stringstream linestr(line);
        linestr >> loc;
        linestr >> tx_theta;
        linestr >> rx_theta;


        angles[i] = new int[2];
        angles[i][0] = std::stoi(tx_theta);
        angles[i][1] = std::stoi(rx_theta);
        g_angle_map[enb_index].insert(std::pair<std::string, int*> (loc, angles[i]));
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


void get_combined_rsrp(ofdm_tx* pkt_tx, ofdm_rx* pkt_rx)
{
    float rsrp = -100000;
    uint32_t rx_pkts1, rx_pkts2 = 0;
    float per = 0;	

    (pkt_tx)-> send_pkts();
    (pkt_tx + 1)-> send_pkts();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::set<uint32_t> tx1_pkt_num_set = std::set<uint32_t>();
    std::set<uint32_t> tx2_pkt_num_set = std::set<uint32_t>();
    std::set<uint32_t> com_pkt_num_set = std::set<uint32_t>();

    uint32_t tx_pkts1 = (pkt_tx)->get_num_pkts();
    (pkt_rx)->report_metrics(&rsrp, &rx_pkts1, &per, tx_pkts1, tx1_pkt_num_set);
    g_logfile<< std::fixed << std::setprecision(6) << std::setw(10)<< rsrp << "  " << std::setw(5) << rx_pkts1 << "       "<< std::setprecision(3) << std::setw(6) << (int) per << "   ";
    
    uint32_t tx_pkts2 = (pkt_tx + 1)->get_num_pkts();
    (pkt_rx + 1)->report_metrics(&rsrp, &rx_pkts2, &per, tx_pkts2, tx2_pkt_num_set);
    g_logfile<< std::fixed << std::setprecision(6) << std::setw(10)<< rsrp << "  " << std::setw(5) << rx_pkts2 << "       "<< std::setprecision(3) << std::setw(6) << (int) per << "   ";

    com_pkt_num_set = tx1_pkt_num_set;
    com_pkt_num_set.insert(tx2_pkt_num_set.begin(), tx2_pkt_num_set.end());

    std::cout << "PKTS1: " << tx1_pkt_num_set.size() << " PKTS2: " << tx2_pkt_num_set.size() << " COMBINED: " << com_pkt_num_set.size()<< std::endl;
    g_logfile << std::setw(7)<< com_pkt_num_set.size() << "       " << (tx_pkts1 - com_pkt_num_set.size())*100/tx_pkts1 << std::endl;	
}

int main(int argc, char* argv[])
{   
    int gnb1_index = 0;
    int gnb2_index = 1;
    int num_tx_pkts = 50;
    int ue_index = 1;
    double speed = 12;
    std::string path_file;
    if(argc < 7){
        std::cout << "Please enter gNB1 index [0-2], gNB2 index [0-2], ue mob [1-2], speed, num_tx_pkts, and the path file" << std::endl;
        return 0;
    }else{
	gnb1_index = std::stoi(std::string(argv[1]));
	gnb2_index = std::stoi(std::string(argv[2]));
	ue_index = std::stoi(std::string(argv[3]));
	speed = std::stod(std::string(argv[4]));
        num_tx_pkts = std::stoi(std::string(argv[5]));
        path_file = std::string(argv[6]);
    }
	
    //Open Log file
    g_logfile.open("trace_log_gNB1_" + std::to_string(gnb1_index) + "_gNB2_" + std::to_string(gnb2_index) + "_ue" + std::to_string(ue_index) + ".txt");	
    //Read path file
    parse_path_file(path_file);
    //Parse angle files
    std::string angle_file1 = "lookup/baseline_enb" + std::to_string(gnb1_index) + "_ue" + std::to_string(ue_index) + ".txt";
    std::string angle_file2 = "lookup/baseline_enb" + std::to_string(gnb2_index) + "_ue" + std::to_string(ue_index) + ".txt";

    // std::cout << angle_file1 << " " << angle_file2 << std::endl;

    parse_data_file(0, angle_file1);
    parse_data_file(1, angle_file2);

    // Create paam objects for PAAMs 
    std::string paam_tx_names[3] = {"rfdev4-in1.sb1.cosmos-lab.org", "rfdev4-in2.sb1.cosmos-lab.org", "rfdev-mob4-3.sb1.cosmos-lab.org"};
    paam paam_tx[2];

    paam_tx[0] = paam(paam_tx_names[gnb1_index]);
    paam_tx[1] = paam(paam_tx_names[gnb2_index]);

    // mob4-3 is the receiver
    paam paam_rx("rfdev-mob4-1.sb1.cosmos-lab.org");


    // Enable the PAAMs 
    long cfo[2] = {250000, 0};
    for(size_t i = 0; i < 2; i++){
	paam_tx[i].enable("all", 16, "tx", 'v', cfo[i]);
    }
    paam_rx.enable("all", 16, "rx", 'v', 0);


    //Creat mmwall object
    //mmwall_ctl rpi1("10.37.23.1", 1234, "10.37.1.1", 3001);
    
    // Create ofdm_tx objects to send packets  
    // Packets are sent to OFDM tx grc on port 1234. 
    // No.of packets = 50 gap between packets = 0.6ms

    ofdm_tx pkt_tx[2];
    std::string pkt_tx_ip[3] = {"10.37.1.1", "10.37.1.2", "10.37.21.3"};
    pkt_tx[0] = ofdm_tx(pkt_tx_ip[gnb1_index].c_str(), 1234, num_tx_pkts, 0.6);    
    pkt_tx[1] = ofdm_tx(pkt_tx_ip[gnb2_index].c_str(), 1234, num_tx_pkts, 0.6);

   
    // Create ofdm_rx object for receiving packets
    // Packets are received, processed and rsrp data is sent to 2001, 2002
    ofdm_rx pkt_rx[2] = {{"10.37.21.1", 2001}, {"10.37.21.1", 2002}};

  
    // Start threads to recv rsrp data
    std::thread rx1_thread([&pkt_rx]() { pkt_rx[0].recv_pkt_data(); });
    std::thread rx2_thread([&pkt_rx]() { pkt_rx[1].recv_pkt_data(); });


    char more_data;
    std::string rx_info;
    std::string rx_info_file;

    g_logfile << "*************************************************************************************************************" << std::endl;
    g_logfile << "Path file: "<< path_file << " Angle file0: " << angle_file1 << " Angle file1: " << angle_file2 << std::endl;
    g_logfile << "*************************************************************************************************************" << std::endl << std::endl;	
    g_logfile << "Time        Loc   TX1Theta    TX2Theta    RxTheta    RSRP1    RX_PKTS1      PER1      RSRP2    RX_PKTS2    PER2  COM_RX_PKTS  COM_PER" << std::endl;
    g_logfile << "================================================================================================================================================" << std::endl;

    char start;
    std::cout << "Start experiment [y/n]: " ;
    std::cin >> start;

    std::string loc = g_path.front();
    g_path.pop();

    // UE points to the wall
    //paam_rx.steer(0,0);
    
    if(start != 'y'){
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

	    g_logfile << std::setw(7) << double(elapsed_seconds.count()) << std::setw(7) << loc << std::setw(7);

            // Steer both the gNbs using angles from downlink_transmit files
            int enb_theta, ue_theta = 0;
            for(size_t i = 0; i < 2; i++){
	        int* loc_angle;
                auto loc_angle_pair = g_angle_map[i].find(loc);

                if(loc_angle_pair != g_angle_map[i].end()){
                    loc_angle = loc_angle_pair->second;
                }
		else{
                    std::cout << "Can't find location data" << loc << std::endl;
	 	    std::cout << "Terminating the program..." << std::endl;
		    
		    paam_rx.steer(0, 0);
		    paam_rx.disable();
		    g_logfile.close();
		    
		    for(size_t i = 0; i < 2; i++){
			paam_tx[i].steer(0, 0);
			paam_tx[i].disable();
		    }
		    return 0;
                }

	        enb_theta = *loc_angle;
                paam_tx[i].steer(enb_theta, 0);

		if(i == 1){
		    ue_theta = *(loc_angle + 1);
		    paam_rx.steer(ue_theta, 0);
		}
		g_logfile << std::setw(7)<< int(enb_theta) << "    ";
	    }

	    g_logfile << std::setw(7)<< int(ue_theta) << "    ";

            get_combined_rsrp(pkt_tx, pkt_rx);

	}
    }
    
    std::cout << "Terminating the program..." << std::endl;
    
    paam_rx.steer(0, 0);
    paam_rx.disable();
    g_logfile.close();
    for(size_t i = 0; i < 2; i++){
        paam_tx[i].steer(0, 0);
        paam_tx[i].disable();
    }
    return 0;
}

