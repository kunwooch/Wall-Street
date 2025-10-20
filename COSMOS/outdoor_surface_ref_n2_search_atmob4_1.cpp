/// 8/26/2024
//  Generate RSRP trace at the UE (mob4-1) from 4 gNbs while the UE is equipped with the mmwall
//  UE PAAM beam is always pointing to the wall(theta 0)
//  At each location, gNb and the mmwall search for the best orientation
//
#include<iostream>
#include<fstream>
#include<string>
#include<unordered_map>
#include<thread>
#include<iomanip>
#include<math.h>
#include "paam.h"
#include "ofdm_tx.h"
#include "ofdm_rx.h"
#include "xytable.h"
#include "mmwall_ctl.h"

#define NUM_GNB 3

std::ofstream g_logfile[NUM_GNB];
std::ofstream g_rsrpfile[NUM_GNB];
std::ofstream g_maxrsrpfile;

std::unordered_map<std::string, int*> g_angle_map;

void parse_data_file(std::string file_enb0, std::string file_enb1, std::string file_enb2)
{
    std::ifstream angle_file0, angle_file1, angle_file2;
    std::string line0, line1, line2;
    std::string loc, tx_theta, rx_theta, wall_theta;
    angle_file0.open(file_enb0);
    angle_file1.open(file_enb1);
    angle_file2.open(file_enb2);

    size_t num_data = 0;
    while(std::getline(angle_file0, line0)){
           //std::cout << line << std::endl;
           num_data++;
    };
    //std::cout << num_data << std::endl;
    angle_file0.close();
    angle_file1.close();
    angle_file2.close();

    angle_file0.open(file_enb0);
    angle_file1.open(file_enb1);
    angle_file2.open(file_enb2);

    int** angles = new int*[num_data];
    for(size_t i = 0 ; i < num_data; i++){
        angles[i] = new int[6];

        std::getline(angle_file0, line0);
        std::stringstream linestr0(line0);
        linestr0 >> loc;
        linestr0 >> tx_theta;
        linestr0 >> wall_theta;

        angles[i][0] = std::stoi(tx_theta);
        angles[i][1] = std::stoi(wall_theta);
        
	std::getline(angle_file1, line1);
        std::stringstream linestr1(line1);
        linestr1 >> loc;
        linestr1 >> tx_theta;
        linestr1 >> wall_theta;

        angles[i][2] = std::stoi(tx_theta);
        angles[i][3] = std::stoi(wall_theta);

	std::getline(angle_file2, line2);
        std::stringstream linestr2(line2);
        linestr2 >> loc;
        linestr2 >> tx_theta;
        linestr2 >> wall_theta;

        angles[i][4] = std::stoi(tx_theta);
        angles[i][5] = std::stoi(wall_theta);


        g_angle_map.insert(std::pair<std::string, int*> (loc, angles[i]));
    }

}

void find_opt_beams_surface(paam* tx, paam* rx, mmwall_ctl* wall, ofdm_tx* pkt_tx, ofdm_rx* pkt_rx, size_t* tx_indices, uint32_t tx_num, int tx_theta_list[], int rx_phi, int rx_angle_step, int rx_angle_range, int wall_angle_step, float* max_rsrp)
{
	std::cout << "Rx Phi: " << rx_phi << std::endl;
        size_t local_tx_indices[tx_num];
        local_tx_indices[0] = *(tx_indices);
	if(tx_num > 1){
        	local_tx_indices[1] = *(tx_indices + 1);
	}
        // Setting both tx and rx phi to 0
        int tx_phi = 0;
        int opt_tx_theta[tx_num], opt_rx_theta[tx_num], opt_wall_theta[tx_num];
        float rsrp1 = -100000;
        uint32_t rx_pkts = 0;
        float per = 0;

	int tx_theta_final = 0;

        for(size_t i = 0; i < tx_num; i++){
            g_logfile[local_tx_indices[i]] << "Tx Theta  Rx Theta  Wall Theta  RSRP  RX_PKTS  PER" << std::endl;
            g_logfile[local_tx_indices[i]] << "==================================================" << std::endl;
            opt_tx_theta[i] = tx_theta_list[local_tx_indices[i]];
	    opt_rx_theta[i] = -360;
            opt_wall_theta[i] = -360;
        }

	// Fix tx angle from lookup table and sweep Rx and mmWall
        for(size_t i = 0; i < tx_num; i++){
        	(tx + local_tx_indices[i])->steer(tx_theta_list[local_tx_indices[i]], tx_phi);
        }

        for(int rx_theta = -rx_angle_range; rx_theta <= rx_angle_range; rx_theta+=rx_angle_step){
             rx->steer(rx_theta, rx_phi);		     
	
	     for(int wall_theta = -70; wall_theta <= 70; wall_theta += wall_angle_step){
		wall->steer(std::to_string(wall_theta));

		for(size_t i = 0; i < tx_num; i++){
		    g_logfile[local_tx_indices[i]] << "  " << std::setw(4) << tx_theta_list[local_tx_indices[i]] << "   " << "  " << std::setw(4) << rx_theta << "   " << "  " << std::setw(4) << wall_theta << "   ";
		    (pkt_tx + local_tx_indices[i])-> send_pkts();
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		for(size_t i = 0; i < tx_num; i++){
		    (pkt_rx + i)->report_metrics(&rsrp1, &rx_pkts, &per, (pkt_tx + local_tx_indices[i])->get_num_pkts());
		    g_logfile[local_tx_indices[i]] << std::fixed << std::setprecision(6) << std::setw(10)<< rsrp1 << "  " << std::setw(5) << rx_pkts << "   "<< std::setprecision(3) << std::setw(6) << per << std::endl;
		    
		    if(*(max_rsrp + local_tx_indices[i]) < rsrp1){
			*(max_rsrp + local_tx_indices[i]) = rsrp1;
			 opt_tx_theta[i] = tx_theta_list[local_tx_indices[i]];
			 opt_rx_theta[i] = rx_theta;
			 opt_wall_theta[i] = wall_theta;
		    }
	        }
	    }
	}

        for(size_t i=0; i < tx_num; i++){
            g_logfile[local_tx_indices[i]] << std::endl << "Search: Opt TX theta " << opt_tx_theta[i] << " Opt Rx theta " << opt_rx_theta[i] << " Opt WallRX theta " << opt_wall_theta[i] << " RSRP :" << *(max_rsrp + local_tx_indices[i]) << std::endl;
        }

        for(size_t i=0; i < tx_num; i++){
            g_rsrpfile[local_tx_indices[i]] << std::setw(6) << opt_tx_theta[i] << "        " <<  std::setw(6) << opt_rx_theta[i] << "        " << std::setw(6) << opt_wall_theta[i] << "        " << *(max_rsrp + local_tx_indices[i]) << std::endl;
        }
}



int main(int argc, char* argv[])
{
    int gnb1 = 0;
    int gnb2 = 1;
    int ue_angle_step_size = 20;
    int ue_angle_range = 20;
    int wall_angle_step_size = 70;
    int rx_phi = 0;
    
    if(argc < 7){
	std::cout << "Please enter gnb 1 idx, gnb 2 idx, ue_angle_range, ue_angle_step_size, ue_phi, and wall_angle_step" << std::endl;
	return 0;
    }else{
	gnb1 = std::stoi(std::string(argv[1]));
	gnb2 = std::stoi(std::string(argv[2]));
        ue_angle_range = std::stoi(std::string(argv[3]));
	ue_angle_step_size = std::stoi(std::string(argv[4]));
	rx_phi = std::stoi(std::string(argv[5]));	
	wall_angle_step_size = std::stoi(std::string(argv[6]));    
    }

    std::string angle_file0 = "lookup/surface_enb0_ue1.txt";
    std::string angle_file1 = "lookup/surface_enb1_ue1.txt";
    std::string angle_file2 = "lookup/surface_enb2_ue1.txt";
    parse_data_file(angle_file0, angle_file1, angle_file2);

    //Open Log files
    for(size_t i = 0; i < NUM_GNB; i++){
        g_logfile[i].open("trace_log"+std::to_string(i)+".txt");	
    }

    // Create paam objects for PAAMs 
    std::string paam_tx_names[3] = {"rfdev4-in1.sb1.cosmos-lab.org", "rfdev4-in2.sb1.cosmos-lab.org", "rfdev-mob4-3.sb1.cosmos-lab.org"};
    paam paam_tx[NUM_GNB];
    for(size_t i = 0; i < NUM_GNB; i++){
	paam_tx[i] = paam(paam_tx_names[i]);
    }

    // mob4-1 is the receiver
    paam paam_rx("rfdev-mob4-1.sb1.cosmos-lab.org");

    // Create mmwall_ctl object 
    // mmWall IP: 10.37.23.1  || host IP (mob4-1): 10.37.21.1
    mmwall_ctl rpi1("10.37.23.1", 1234, "10.37.21.1", 3001);


    // Enable the PAAMs 
    long cfo[4] = {250000, 0, 0, 0};
    for(size_t i = 0; i < NUM_GNB; i++){
	paam_tx[i].enable("all", 16, "tx", 'v', cfo[i]);
    }
    paam_rx.enable("all", 16, "rx", 'v', 0);

    // Rx angle set to 0 degree
    // paam_rx.steer(0,0);

    // Create ofdm_tx objects to send packets  
    // Packets are sent to OFDM tx grc on port 1234. 
    // No.of packets = 50 gap between packets = 0.6ms
    ofdm_tx pkt_tx[NUM_GNB];
    std::string pkt_tx_ip[3] = {"10.37.1.1", "10.37.1.2", "10.37.21.3"};
    for(size_t i = 0; i < NUM_GNB; i++){
	pkt_tx[i] = ofdm_tx(pkt_tx_ip[i].c_str(), 1234, 20, 0.6);    
    }
    
   
    // Create ofdm_rx object for receiving packets
    // Packets are received, processed and rsrp data is sent to 2001, 2002
    // mob4-1 is the controller
    ofdm_rx pkt_rx[2] = {{"10.37.21.1", 2001}, {"10.37.21.1", 2002}};

  
    // Start threads to recv rsrp data
    std::thread rx1_thread([&pkt_rx]() { pkt_rx[0].recv_pkt_data(); });
    std::thread rx2_thread([&pkt_rx]() { pkt_rx[1].recv_pkt_data(); });


    //std::ofstream rsrpfile[NUM_GNB];
    for(size_t i = 0; i < NUM_GNB; i++){
        g_rsrpfile[i].open("rsrp"+std::to_string(i)+".txt");
    }
    g_maxrsrpfile.open("maxrsrp.txt");
    float max_rsrp[NUM_GNB];
    char more_data;
    std::string loc;

    for(size_t i = 0; i < NUM_GNB; i++){
        g_rsrpfile[i] << "***********************************************************************************************************" << std::endl;
        g_rsrpfile[i] << "Rx Phi " << rx_phi << "   RX location Info     Opt TX Theta     Opt RX Theta    Opt Wall Theta     Max RSRP" << std::endl;
        g_rsrpfile[i] << "**********************************************************************************************************" << std::endl;
        }

    while(1){
	//more_data = 'n';    
	std::cout << "Do you want to collect more data? [y/n] :" ;
        std::cin >> more_data;
    	if(more_data == 'n'){
	    break;
	}
	if(more_data == 'y'){
            std::cout << "Enter info about the rx location etc " << std::endl;
	    std::cin >> loc;
	    for(size_t i = 0; i < NUM_GNB; i++){
                g_logfile[i] << "**************************************************" << std::endl;
                g_logfile[i] << loc << std::endl;
                g_logfile[i] << "**************************************************" << std::endl;
	    }

	    int tx_theta_list[NUM_GNB];

	    // Get traces from UE and 1 target gNb at a time
            for(int gNb_index = 0; gNb_index < NUM_GNB; gNb_index++){

		int* loc_angle;
            	int tx_theta, wall_theta;
            	auto loc_angle_pair = g_angle_map.find(loc);

            	if(loc_angle_pair != g_angle_map.end()){
                	loc_angle = loc_angle_pair->second;
            	}
            	else{
                	std::cout << "Can't find location data " << loc << std::endl;
                	break;
            	}

            	tx_theta = *(loc_angle + (gNb_index * 2));
            	wall_theta = *(loc_angle + (gNb_index * 2) + 1);

		std::cout << "gNB index: " << gNb_index << " , gNB Theta: " << tx_theta << " , Wall Theta: " << wall_theta << std::endl;

		tx_theta_list[gNb_index] = tx_theta;	
            }

	    
	    for(size_t i = 0; i < NUM_GNB; i++){
                g_rsrpfile[i] << std::setw(20) << loc <<"               "; 
	    }

	    // Reset max_rsrp for each data point
	    for(size_t i = 0; i < NUM_GNB; i++){
                max_rsrp[i] = -100000;
            }

	    std::chrono::time_point<std::chrono::system_clock> start, end;
            start = std::chrono::system_clock::now();
    
	    // Get traces from all the gNodesBs 2 at a time. Tx from gNodeBs 0 and 1 first 
	    size_t tx_indices[2];
	    tx_indices[0] = gnb1;
	    tx_indices[1] = gnb2;

            find_opt_beams_surface(paam_tx, &paam_rx, &rpi1, pkt_tx, pkt_rx, tx_indices, 2, tx_theta_list, rx_phi, ue_angle_step_size, ue_angle_range, wall_angle_step_size, max_rsrp);

	     /*
	     // Tx from gNodeBs 2 and 3
             tx_indices[0] = 2;
             tx_indices[1] = 3;

	    if (NUM_GNB == 3){
                find_opt_beams_surface(paam_tx, &paam_rx, &rpi1, pkt_tx, pkt_rx, tx_indices, 1, tx_theta_list, rx_phi, tx_angle_step_size, tx_angle_range, ue_angle_step_size, ue_angle_range, wall_angle_step_size, max_rsrp); 
	    }
            */

	    g_maxrsrpfile << std::setw(20) << loc << "               ";
	    for(size_t i = 0; i < NUM_GNB; i++){
		g_maxrsrpfile <<  std::fixed << std::setprecision(6) << std::setw(10)<< max_rsrp[i] << "  "; 
	    }
	    g_maxrsrpfile << std::endl;
   

	    end = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_seconds = end - start;
            std::cout << "Time " << elapsed_seconds.count() << std::endl;
        } 
    }

    std::cout << "Terminating the program..." << std::endl;

    paam_rx.steer(0, 0);
    paam_rx.disable();
    for(size_t i = 0; i < NUM_GNB; i++){
        paam_tx[i].steer(0, 0);
        paam_tx[i].disable();
        g_logfile[i].close();
        g_rsrpfile[i].close();
    }

    g_maxrsrpfile.close();
    return 0;
}

