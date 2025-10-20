/// Testing the paam class

#include<iostream>
#include<fstream>
#include<string>
#include<thread>
#include<iomanip>
#include<math.h>
#include "paam.h"
#include "ofdm_tx.h"
#include "ofdm_rx.h"
#include "xytable.h"

#define NUM_GNB 3

std::ofstream g_logfile[NUM_GNB];
std::ofstream g_rsrpfile[NUM_GNB];
std::ofstream g_maxrsrpfile;


void find_opt_beams(paam* tx, paam* rx, ofdm_tx* pkt_tx, ofdm_rx* pkt_rx, size_t* tx_indices, uint32_t tx_num, uint32_t tx_angle_step, uint32_t rx_angle_step, float* max_rsrp)
{
	size_t local_tx_indices[tx_num];
	local_tx_indices[0] = *(tx_indices);
	if(tx_num > 1){
	     local_tx_indices[1] = *(tx_indices + 1);
	}
	// Setting both tx and rx phi to 0
        int tx_phi = 0;
        int rx_phi = 0;
	int opt_tx_theta[tx_num], opt_rx_theta[tx_num];
	float rsrp = -100000;
	uint32_t rx_pkts = 0;
	float per = 0;

	for(size_t i = 0; i < tx_num; i++){
	    g_logfile[local_tx_indices[i]] << "TX Theta RX Theta    RSRP      RX_PKTS     PER" << std::endl;
	    g_logfile[local_tx_indices[i]] << "==============================================" << std::endl;
	    opt_tx_theta[i] = -360;
	    opt_rx_theta[i] = -360;
	}


	//Sweep tx theta
	for(int tx_theta = -60; tx_theta <= 60; tx_theta+=tx_angle_step){
	    for(size_t i = 0; i < tx_num; i++){	
	    	(tx + local_tx_indices[i])->steer(tx_theta, tx_phi);
	    }
	    //Sweept rx theta	    
	    for(int rx_theta = -60; rx_theta <= 60; rx_theta+=rx_angle_step){
		rx->steer(rx_theta, rx_phi);

		for(size_t i = 0; i < tx_num; i++){
                    g_logfile[local_tx_indices[i]] << "  " << std::setw(4) << tx_theta << "   " << "  " << std::setw(4) << rx_theta << "   ";	
                    (pkt_tx + local_tx_indices[i])-> send_pkts();
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

                for(size_t i = 0; i < tx_num; i++){
                    (pkt_rx + i)->report_metrics(&rsrp, &rx_pkts, &per, (pkt_tx + local_tx_indices[i])->get_num_pkts());    
                    g_logfile[local_tx_indices[i]] << std::fixed << std::setprecision(6) << std::setw(10)<< rsrp << "  " << std::setw(5) << rx_pkts << "   "<< std::setprecision(3) << std::setw(6) << per << std::endl;
                    if(*(max_rsrp + local_tx_indices[i]) < rsrp){
                        *(max_rsrp + local_tx_indices[i]) = rsrp;
                         opt_tx_theta[i] = tx_theta;
                         opt_rx_theta[i] = rx_theta;
    		    }
                }    
		
	    }
	}

	for(size_t i=0; i < tx_num; i++){
            g_logfile[local_tx_indices[i]] << std::endl << "Opt TX theta " << opt_tx_theta[i] << " Opt RX theta " << opt_rx_theta[i] << " RSRP :" << *(max_rsrp + local_tx_indices[i]) << std::endl;
	}

	for(size_t i=0; i < tx_num; i++){
            g_rsrpfile[local_tx_indices[i]] << std::setw(6) << opt_tx_theta[i] << "        " << std::setw(6) << opt_rx_theta[i] << "        " << *(max_rsrp + local_tx_indices[i]) << std::endl;
	}
}


int main(int argc, char* argv[])
{   
    int tx_angle_step_size = 60;
    int rx_angle_step_size = 60;
    int rx_ue = 1;
    if(argc < 4){
        std::cout << "Please enter [gnb_angle_step] [ue_angle_step] [ue_mob] (e.g., 4 15 1)" << std::endl;
        return 0;
    }else{
        tx_angle_step_size = std::stoi(std::string(argv[1]));
        rx_angle_step_size = std::stoi(std::string(argv[2]));
	rx_ue = std::stoi(std::string(argv[3]));
    }
	
    //Open Log files
    for(size_t i = 0; i < NUM_GNB; i++){
        g_logfile[i].open("trace_log"+std::to_string(i)+"_ue"+std::to_string(rx_ue)+".txt");	
    }

    // Create paam objects for gNB PAAMs (srv1, srv2, mob4-3) 
    std::string paam_tx_names[3] = {"rfdev4-in1.sb1.cosmos-lab.org", "rfdev4-in2.sb1.cosmos-lab.org", "rfdev-mob4-3.sb1.cosmos-lab.org"};
 
    paam paam_tx[NUM_GNB];
    for(size_t i = 0; i < NUM_GNB; i++){
	paam_tx[i] = paam(paam_tx_names[i]);
    }

    std::string paam_rx_name;
    if(rx_ue == 1){
    	// mob4-1 is the receiver
	paam_rx_name = "rfdev-mob4-1.sb1.cosmos-lab.org";
    }
    else{
	// mob4-2 is the receiver
	paam_rx_name = "rfdev-mob4-2.sb1.cosmos-lab.org";
    }
    paam paam_rx(paam_rx_name);

    // Enable the PAAMs 
    long cfo[4] = {250000, 0, 0, 0};
    for(size_t i = 0; i < NUM_GNB; i++){
	paam_tx[i].enable("all", 16, "tx", 'v', cfo[i]);
    }
    paam_rx.enable("all", 16, "rx", 'v', 0);


    // Create ofdm_tx objects to send packets  
    // Packets are sent to OFDM tx grc on port 1234. 
    // No.of packets = 50 gap between packets = 0.6ms

    ofdm_tx pkt_tx[NUM_GNB];
    // ip corresponding to tx
    std::string pkt_tx_ip[3] = {"10.37.1.1", "10.37.1.2", "10.37.21.3"}; 
    
    for(size_t i = 0; i < NUM_GNB; i++){
	pkt_tx[i] = ofdm_tx(pkt_tx_ip[i].c_str(), 1234, 20, 0.6);    
    }
    
    // Create ofdm_rx object for receiving packets
    // Packets are received, processed and rsrp data is sent to 2001, 2002
    // currently mob4-1 is the controller
    ofdm_rx pkt_rx[2] = {{"10.37.21.1", 2001}, {"10.37.21.1", 2002}};

  
    // Start threads to recv rsrp data
    std::thread rx1_thread([&pkt_rx]() { pkt_rx[0].recv_pkt_data(); });
    std::thread rx2_thread([&pkt_rx]() { pkt_rx[1].recv_pkt_data(); });


    for(size_t i = 0; i < NUM_GNB; i++){
        g_rsrpfile[i].open("rsrp"+std::to_string(i)+"_ue"+std::to_string(rx_ue)+".txt");
    }
    g_maxrsrpfile.open("maxrsrp_ue"+std::to_string(rx_ue)+".txt");
    float max_rsrp[NUM_GNB];
    char more_data;
    std::string rx_info;

    for(size_t i = 0; i < NUM_GNB; i++){
        g_rsrpfile[i] << "***************************************************************************" << std::endl;
        g_rsrpfile[i] << "RX location Information         Opt TX Theta     Opt RX Theta     Max RSRP" << std::endl;
        g_rsrpfile[i] << "***************************************************************************" << std::endl;
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
	    std::cin >> rx_info;
	    for(size_t i = 0; i < NUM_GNB; i++){
                g_logfile[i] << "**************************************************" << std::endl;
                g_logfile[i] << rx_info << std::endl;
                g_logfile[i] << "**************************************************" << std::endl;
	    }

	    
	    for(size_t i = 0; i < NUM_GNB; i++){
                g_rsrpfile[i] << std::setw(20) << rx_info <<"               "; 
	    }

	    // Reset max_rsrp for each data point
	    for(size_t i = 0; i < NUM_GNB; i++){
                max_rsrp[i] = -100000;
            }

	    std::chrono::time_point<std::chrono::system_clock> start, end;
            start = std::chrono::system_clock::now();
    
	    // Get traces from all the gNodesBs 2 at a time. Tx from gNodeBs 0 and 1 first 
	    size_t tx_indices[2];

	    tx_indices[0] = 0;
            tx_indices[1] = 1;
            find_opt_beams(paam_tx, &paam_rx, pkt_tx, pkt_rx, tx_indices, 2, tx_angle_step_size, rx_angle_step_size, max_rsrp);
	
	    if(NUM_GNB == 3){
	        // Tx from gNodeBs 2 and 3
                tx_indices[0] = 2;
                tx_indices[1] = 3;

                find_opt_beams(paam_tx, &paam_rx, pkt_tx, pkt_rx, tx_indices, 1, tx_angle_step_size, rx_angle_step_size, max_rsrp);
	    }
	    else if(NUM_GNB == 4){
	        // Tx from gNodeBs 2 and 3
                tx_indices[0] = 2;
                tx_indices[1] = 3;

                find_opt_beams(paam_tx, &paam_rx, pkt_tx, pkt_rx, tx_indices, 2, tx_angle_step_size, rx_angle_step_size, max_rsrp);
	    }

	    g_maxrsrpfile << std::setw(20) << rx_info <<"               ";
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

