/// 4/4/2024
//  Generate RSRP trace at the UE (mob4-1) from 4 gNbs while the UE is equipped with the mmwall
//  UE PAAM beam is always pointing to the wall(theta 0)
//  At each location, gNb and the mmwall search for the best orientation
//
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
#include "mmwall_ctl.h"

#define NUM_GNB 3

std::ofstream g_logfile[NUM_GNB];
std::ofstream g_rsrpfile[NUM_GNB];
std::ofstream g_maxrsrpfile;


void find_opt_beams_surface(paam* tx, mmwall_ctl* rx, ofdm_tx* pkt_tx, ofdm_rx* pkt_rx, size_t* tx_indices, int tx_theta_list[], uint32_t tx_angle_step, uint32_t wall_angle_step, float* max_rsrp)
{
        size_t local_tx_indices[2];
        local_tx_indices[0] = *(tx_indices);
        local_tx_indices[1] = *(tx_indices + 1);

        // Setting both tx and rx phi to 0
        int tx_phi = 0;
        int rx_phi = 0;
        int opt_tx_theta[2], opt_wall_theta[2];
        float rsrp = -100000;
        uint32_t rx_pkts = 0;
        float per = 0;
	int tx_theta1 = 0;
	int tx_theta2 = 0;

        for(size_t i = 0; i < 2; i++){
            g_logfile[local_tx_indices[i]] << "TX Theta  WallRXTheta    RSRP      RX_PKTS     PER" << std::endl;
            g_logfile[local_tx_indices[i]] << "==============================================" << std::endl;
            opt_tx_theta[i] = -360;
            opt_wall_theta[i] = -360;
        }


        //Sweep tx theta
        for(int tx_theta = -20; tx_theta <= 20; tx_theta+=tx_angle_step){
	    tx_theta1 = tx_theta_list[local_tx_indices[0]] + tx_theta; 
	    tx_theta2 = tx_theta_list[local_tx_indices[1]] + tx_theta;
            (tx + local_tx_indices[0])->steer(tx_theta1, tx_phi);
            (tx + local_tx_indices[1])->steer(tx_theta2, tx_phi);
            //Sweept rx theta
            for(int wall_theta = -70; wall_theta <= 70; wall_theta+=wall_angle_step){
                rx->steer(std::to_string(wall_theta));
                g_logfile[local_tx_indices[0]] << "  " << std::setw(4) << tx_theta1 << "   " << "    " << std::setw(4) << wall_theta << "   ";
                g_logfile[local_tx_indices[1]] << "  " << std::setw(4) << tx_theta2 << "   " << "    " << std::setw(4) << wall_theta << "   ";
                (pkt_tx + local_tx_indices[0])-> send_pkts();
                (pkt_tx + local_tx_indices[1])-> send_pkts();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));

                for(size_t i = 0; i < 2; i++){
                    (pkt_rx + i)->report_metrics(&rsrp, &rx_pkts, &per, (pkt_tx + local_tx_indices[i])->get_num_pkts());
                    g_logfile[local_tx_indices[i]] << std::fixed << std::setprecision(6) << std::setw(10)<< rsrp << "  " << std::setw(5) << rx_pkts << "   "<< std::setprecision(3) << std::setw(6) << per << std::endl;
                    if(*(max_rsrp + local_tx_indices[i]) < rsrp){
                        *(max_rsrp + local_tx_indices[i]) = rsrp;
                         opt_tx_theta[i] = tx_theta_list[local_tx_indices[i]] + tx_theta;
                         opt_wall_theta[i] = wall_theta;
                    }
                }

            }
        }

        for(size_t i=0; i < 2; i++){
            g_logfile[local_tx_indices[i]] << std::endl << "Opt TX theta " << opt_tx_theta[i] << " Opt WallRX theta " << opt_wall_theta[i] << " RSRP :" << *(max_rsrp + local_tx_indices[i]) << std::endl;
        }

        for(size_t i=0; i < 2; i++){
            g_rsrpfile[local_tx_indices[i]] << std::setw(6) << opt_tx_theta[i] << "        " << std::setw(6) << opt_wall_theta[i] << "        " << *(max_rsrp + local_tx_indices[i]) << std::endl;
        }
}



int main(int argc, char* argv[])
{
    int tx_angle_step_size = 60;
    int wall_angle_step_size = 70;	
    if(argc < 3){
	std::cout << "Please enter tx_angle_step_size and wall_angle_step_size" << std::endl;
	return 0;
    }else{
	tx_angle_step_size = std::stoi(std::string(argv[1]));    
	wall_angle_step_size = std::stoi(std::string(argv[2]));    
    }


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
    mmwall_ctl rpi1("10.37.23.1", 1234, "10.37.1.1", 3001);


    // Enable the PAAMs 
    long cfo[4] = {250000, 0, 0, 0};
    for(size_t i = 0; i < NUM_GNB; i++){
	paam_tx[i].enable("all", 16, "tx", 'v', cfo[i]);
    }
    paam_rx.enable("all", 16, "rx", 'v', 0);
    paam_rx.steer(0,0);

    // Create ofdm_tx objects to send packets  
    // Packets are sent to OFDM tx grc on port 1234. 
    // No.of packets = 50 gap between packets = 0.6ms

    ofdm_tx pkt_tx[NUM_GNB];
    std::string pkt_tx_ip[3] = {"10.37.1.1", "10.37.1.2", "10.37.21.3"};
    for(size_t i = 0; i < NUM_GNB; i++){
	pkt_tx[i] = ofdm_tx(pkt_tx_ip[i].c_str(), 1234, 50, 0.6);    
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

	    int tx_theta_list[NUM_GNB];

	    // Get traces from UE and 1 target gNb at a time
            for(int gNb_index = 0; gNb_index < NUM_GNB; gNb_index++){
                int tx_theta = 0;
                std::cout << "Enter target gNb tx angle : " << std::endl;
                std::cin >> tx_theta;
		if(tx_theta + 20 > 60){	
		     std::cout << "This angle is out of the range. The angle is set to 40 " << std::endl;
                     tx_theta = 40;
		}
		if(tx_theta - 20 < -60){
		     std::cout << "This angle is out of the range. The angle is set to -40 " << std::endl;
		     tx_theta = -40;
		}

		tx_theta_list[gNb_index] = tx_theta;	
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
	    size_t tx_indices[2] = {0,1};

	    //find_opt_beams_surface(paam_tx array, mmwall_ctl obj, pkt_tx array, pkt_rx array, tx_index array, tx_angle_step, rx_angle_step, max_rsrp array)
            find_opt_beams_surface(paam_tx, &rpi1, pkt_tx, pkt_rx, tx_indices, tx_theta_list, tx_angle_step_size, wall_angle_step_size, max_rsrp);

	    if(NUM_GNB > 2){
	        // Tx from gNodeBs 2 and 3
                tx_indices[0] = 2;
                tx_indices[1] = 3;
                find_opt_beams_surface(paam_tx, &rpi1, pkt_tx, pkt_rx, tx_indices, tx_theta_list, tx_angle_step_size, wall_angle_step_size, max_rsrp);
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
    
    for(size_t i = 0; i < NUM_GNB; i++){
	paam_tx[i].disable();
	g_logfile[i].close();
	g_rsrpfile[i].close();
    }
    g_maxrsrpfile.close();
    return 0;	
}

