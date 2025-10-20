/// 3/10/2024 
// Record rsrp at the UE from 2 gNbs simultaneously using different frequency bands
// transmit from mob4-1(10.37.21.1), mob4-2(10.37.21.2). Receive from srv1-lg1 

#include<iostream>
#include<fstream>
#include<string>
#include<thread>
#include<iomanip>
#include "paam.h"
#include "ofdm_txrx.h"

void find_opt_beam(paam* tx, paam*rx, ofdm_txrx* pkt_ctrl, uint32_t tx_num_pkts, double tx_pkt_interval, uint32_t angle_step, std::string filename)
{
	// open file to write rsrp and per while sweeping tx and rx beams
	std::ofstream rsrpfile;
	rsrpfile.open(filename);
	rsrpfile << "TX Theta RX Theta    RSRP    PER" << std::endl;
        rsrpfile << "=======================================" << std::endl;	

	// Setting both tx and rx phi to 0
	uint32_t tx_phi = 0;
	uint32_t rx_phi = 0;
	//Sweep tx theta
	for(int tx_theta = -60; tx_theta <= 60; tx_theta+=angle_step){
	    tx->steer(tx_theta, tx_phi);
            //Sweept rx theta	    
	    for(int rx_theta = -60; rx_theta <= 60; rx_theta+=angle_step){
		rx->steer(rx_theta, rx_phi);
	        //send pkts with the current alignment	
		pkt_ctrl->send_pkts(tx_num_pkts,tx_pkt_interval);
	        std::this_thread::sleep_for(std::chrono::milliseconds(500));
	        float avg_rsrp = 0;
                float per = 0;
		//report and record metrics
	 	pkt_ctrl->report_metrics(&avg_rsrp, &per, tx_num_pkts);
                rsrpfile << "  " << std::setw(4) << tx_theta << "   " << "  " << std::setw(4) << rx_theta << "   " << std::fixed << std::setprecision(8) << avg_rsrp << "  " << std::setprecision(7) << per << std::endl;	
	    }
	}
}


int main()
{
    // Create paam objects for PAAMs in mob4-1 and mob4-2	
    paam rfdev_mob4_1("rfdev-mob4-1.sb1.cosmos-lab.org");
    paam rfdev_mob4_2("rfdev-mob4-2.sb1.cosmos-lab.org");
    rfdev_mob4_1.enable("all", 16, "tx", 'h');
    rfdev_mob4_2.enable("all", 16, "rx", 'h');


    // Create ofdm_txrx object to send packets to mob4-1 and recv data from mob4-2
    // Packets are sent to OFDM tx grc on mob4-1:1234. rsrp data is sent from OFDM rx grc on mob4-2 to mob4-1:2001
    ofdm_txrx mob4_1("10.37.21.1", 1234, 2001, 2000);
    
    // Start a thread to recv rsrp data 
    std::thread rx_thread([&mob4_1]() { mob4_1.recv_pkt_data(); });

    find_opt_beam(&rfdev_mob4_1, &rfdev_mob4_2, &mob4_1, 200, 0.8, 5, "rsrp_mob4_1_mob4_2.txt");

    rfdev_mob4_1.disable();
    rfdev_mob4_2.disable();














    // Set tx paam to theta and phi 0
    //rfdev_mob4_1.configure("all", 16, "tx", 'h', 0, 0);
    //uint32_t tx_num_pkts = 1000;
    //uint32_t tx_pkt_interval = 2; //msec
    // Vary receive theta and collect rsrp
    //for(int theta = -60; theta <= 60; theta += 5){ 
    //    rfdev_mob4_2.configure("all", 16, "rx", 'h', theta, 0);
	//wait for the service to return
      //  std::this_thread::sleep_for(std::chrono::seconds(1));
	//send packets at 2ms interval
//	mob4_1.send_pkts(tx_num_pkts, tx_pkt_interval);
  //      std::this_thread::sleep_for(std::chrono::seconds(2));
    //    float avg_rsrp = 0;
//	float per = 0;
  //      mob4_1.report_metrics(&avg_rsrp, &per, tx_num_pkts);
    //    std::cout << "RX theta " << theta << " Avg RSRP " << avg_rsrp << " PER " << per << std::endl;
   // }

    return 0;	
}

