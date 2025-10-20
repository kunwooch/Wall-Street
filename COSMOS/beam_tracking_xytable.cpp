/// Testing the paam class

#include<iostream>
#include<fstream>
#include<string>
#include<thread>
#include<iomanip>
#include<cmath>
#include<math.h>
#include "paam.h"
#include "ofdm_tx.h"
#include "ofdm_rx.h"
#include "xytable.h"


std::ofstream g_logfile;


float get_rsrp(ofdm_tx* pkt_tx, ofdm_rx* pkt_rx)
{
    pkt_tx->send_pkts();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    float avg_rsrp = 0;
    uint32_t rx_pkts = 0;
    float per = 0;
    pkt_rx->report_metrics(&avg_rsrp, &rx_pkts, &per, pkt_tx->get_num_pkts());    
    g_logfile << std::fixed << std::setprecision(6) << std::setw(10)<< avg_rsrp << "  " << std::setw(5) << rx_pkts << "   "<< std::setprecision(3) << std::setw(6) << per << std::endl;
    return avg_rsrp;
}

void find_opt_beam(paam* tx, paam*rx, ofdm_tx* pkt_tx, ofdm_rx* pkt_rx, uint32_t tx_angle_step, uint32_t rx_angle_step)
{
	g_logfile << "TX Theta RX Theta    RSRP      RX_PKTS     PER" << std::endl;
        g_logfile << "===============================================" << std::endl;	

	// Setting both tx and rx phi to 0
	int tx_phi = 0;
	int rx_phi = 0;
	float max_rsrp = -100000;
	int opt_tx_theta = 0;
	int opt_rx_theta = 0;

        //get_rsrp(pkt_tx, pkt_rx);
	//Sweep tx theta
	for(int tx_theta = -60; tx_theta <= 60; tx_theta+=tx_angle_step){
	    tx->steer(tx_theta, tx_phi);
            //Sweept rx theta	    
	    for(int rx_theta = -60; rx_theta <= 60; rx_theta+=rx_angle_step){
		rx->steer(rx_theta, rx_phi);
                g_logfile << "  " << std::setw(4) << tx_theta << "   " << "  " << std::setw(4) << rx_theta << "   ";	
		float rsrp = get_rsrp(pkt_tx, pkt_rx);
	        if(max_rsrp < rsrp){
		    max_rsrp = rsrp;
	            opt_tx_theta = tx_theta;
	            opt_rx_theta = rx_theta;	    
		}
	    }
	}
	tx->steer(opt_tx_theta, tx_phi);
	rx->steer(opt_rx_theta, rx_phi);
	g_logfile << std::endl << "Opt TX theta " << opt_tx_theta << " Opt RX theta " << opt_rx_theta << " RSRP :" << max_rsrp << std::endl;
	std::cout << "Opt TX theta " << opt_tx_theta << " Opt RX theta " << opt_rx_theta << std::endl;
}

void find_opt_tx_beam(paam* tx, paam*rx, ofdm_tx* pkt_tx, ofdm_rx* pkt_rx, uint32_t tx_angle_step)
{
        g_logfile << "TX Theta   RSRP      RX_PKTS     PER" << std::endl;
        g_logfile << "===============================================" << std::endl;

        // Setting both tx and rx phi to 0
        int tx_phi = 0;
        float max_rsrp = -100000;
        int opt_tx_theta = 0;

        rx->steer(0,0);

        //Sweep tx theta
        for(int tx_theta = -60; tx_theta <= 60; tx_theta+=tx_angle_step){
            tx->steer(tx_theta, tx_phi);
            g_logfile << "  " << std::setw(4) << tx_theta << "      ";
            float rsrp = get_rsrp(pkt_tx, pkt_rx);
            if(max_rsrp < rsrp){
                max_rsrp = rsrp;
                opt_tx_theta = tx_theta;
            }
        }
        tx->steer(opt_tx_theta, tx_phi);
        g_logfile << std::endl << "Opt TX theta " << opt_tx_theta << " RSRP :" << max_rsrp << std::endl;
	std::cout << "Opt TX theta " << opt_tx_theta << " RSRP :" << max_rsrp << std::endl;
}

void find_opt_rx_beam(paam* tx, paam*rx, ofdm_tx* pkt_tx, ofdm_rx* pkt_rx, uint32_t rx_angle_step)
{
	std::cout << std::endl << "Beam search..... " << std::endl;
        g_logfile << "RX Theta   RSRP      RX_PKTS     PER" << std::endl;
        g_logfile << "===============================================" << std::endl;

        // Setting both tx and rx phi to 0
        int rx_phi = 0;
        float max_rsrp = -100000;
        int opt_rx_theta = 0;

        tx->steer(0,0);

        //Sweep rx theta
        for(int rx_theta = -60; rx_theta <= 60; rx_theta+=rx_angle_step){
            rx->steer(rx_theta, rx_phi);
            g_logfile << "  " << std::setw(4) << rx_theta << "      ";
            float rsrp = get_rsrp(pkt_tx, pkt_rx);
            if(max_rsrp < rsrp){
                max_rsrp = rsrp;
                opt_rx_theta = rx_theta;
            }
        }
        rx->steer(opt_rx_theta, rx_phi);
        g_logfile << std::endl << "Opt RX theta " << opt_rx_theta << " RSRP :" << max_rsrp << std::endl;
        std::cout << "RX theta " << opt_rx_theta <<  std::endl;
}




int main()
{
    std::chrono::time_point<std::chrono::system_clock> start, end;   	
    start = std::chrono::system_clock::now();
    //Open Log file
    g_logfile.open("trace_log.txt");	

    // Create paam objects for PAAMs 	
    paam paam_tx("rfdev4-in1.sb1.cosmos-lab.org");
    paam paam_rx("rfdev4-in2.sb1.cosmos-lab.org");
    //paam paam_rx("rfdev-mob4-1.sb1.cosmos-lab.org");
    // Enable the PAAMs - in tx mode, in rx mode
    paam_rx.enable("all", 16, "rx", 'h', 0);
    paam_tx.enable("all", 16, "tx", 'h', 250000 );


    // Create ofdm_tx object to send packets  
    // Packets are sent to OFDM tx grc on port 1234. 
    // No.of packets = 50 gap between packets = 0.6ms
    ofdm_tx pkt_tx("10.37.1.1", 1234, 50, 0.6);
    
   
    // Create ofdm_rx object for receiving packets
    // Packets are received, processed and rsrp data is sent to :2001
    ofdm_rx pkt_rx("10.37.1.1", 2001);

    // Create xytable object for the xytable in in1 corner
    xytable xytable_rx("xytable2.sb1.cosmos-lab.org");
  
    // Start a thread to recv rsrp data 
    std::thread rx_thread([&pkt_rx]() { pkt_rx.recv_pkt_data(); });



    //-----------------------------------------------------------
    // Generate RSRP trace by moving the UE on the xytable.
    // Populate an array with xy table positions
    
    /* std::vector<xytable_pos> xytable_path;
    xytable_path.push_back(xytable_pos(20, 200, 0));
    xytable_path.push_back(xytable_pos(200, 200, 10));
    xytable_path.push_back(xytable_pos(400, 300, 22));
    //xytable_path.push_back(xytable_pos(500, 300, 15));
    xytable_path.push_back(xytable_pos(600, 300, 0));
    xytable_path.push_back(xytable_pos(800, 300, 45));
    xytable_path.push_back(xytable_pos(1000, 300, 0));
    xytable_path.push_back(xytable_pos(1200, 200, 20));*/
    //------------------------------------------------------------

    std::ofstream rsrpfile;
    rsrpfile.open("rsrp.txt");

    xytable_rx.move(xytable_pos(50,100, 0));
    //find_opt_tx_beam(&paam_tx, &paam_rx, &pkt_tx, &pkt_rx, 10);
    //find_opt_rx_beam(&paam_tx, &paam_rx, &pkt_tx, &pkt_rx, 10);
    find_opt_beam(&paam_tx, &paam_rx, &pkt_tx, &pkt_rx, 10, 10);
    float rsrp = get_rsrp(&pkt_tx, &pkt_rx);
    std::cout<< "RSRP : " << rsrp << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    int rx_angle[26] = {5, 8, 10, 13, 15, 13, 10, 6, 1, -2, -5, -8, -11, -14, -17, -13, -10, -8, -6, -4, 0, 3, 5, 8, 10, 12}; 
    while(1){
        for(size_t i = 1; i <= 25; i++)
        {
            xytable_rx.move(xytable_pos(i*50,100,rx_angle[i]));
	    float new_rsrp =  get_rsrp(&pkt_tx, &pkt_rx);
	    if ((new_rsrp < rsrp - 3)||(isnan(new_rsrp))){
	        find_opt_rx_beam(&paam_tx, &paam_rx, &pkt_tx, &pkt_rx, 10);
	        new_rsrp =  get_rsrp(&pkt_tx, &pkt_rx);
	    }else{
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));	   	
	    }

            std::cout<< "RSRP : " << new_rsrp << std::endl;
        }
        for(size_t i = 24; i >= 1; i--)
        {
           xytable_rx.move(xytable_pos(i*50,100,rx_angle[i]));
           float new_rsrp =  get_rsrp(&pkt_tx, &pkt_rx);
           if ((new_rsrp < rsrp - 3)||(isnan(new_rsrp))){
               find_opt_rx_beam(&paam_tx, &paam_rx, &pkt_tx, &pkt_rx, 10);
               new_rsrp =  get_rsrp(&pkt_tx, &pkt_rx);
           }else{
               std::this_thread::sleep_for(std::chrono::milliseconds(1000));
           }

           std::cout<< "RSRP : " << new_rsrp << std::endl;
	}

    }
    
    //xytable_rx.move(xytable_pos(50,100,0));	    

    /*for(int i = 0; i < xytable_path.size(); i++)
    {
	xytable_pos current = xytable_path[i];    
	xytable_rx.move(current);
        g_logfile << "XY table position X:" << current.x << " Y:" << current.y << " angle:" << current.angle << std::endl;	
        std::this_thread::sleep_for(std::chrono::milliseconds(4000));
        find_opt_beam(&paam_tx, &paam_rx, &pkt_tx, &pkt_rx, 60, 60);
	rsrpfile << get_rsrp(&pkt_tx, &pkt_rx) << std::endl;
	for(int j = 1; j <= 5; j++){
	    xytable_rx.move(xytable_pos(current.x,current.y,current.angle+j));	
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            g_logfile << "XY table position X:" << current.x << " Y:" << current.y << " angle:" << current.angle+j << std::endl;	
	    rsrpfile << get_rsrp(&pkt_tx, &pkt_rx) << std::endl;
	}

    }*/

    //end = std::chrono::system_clock::now();
    //std::chrono::duration<double> elapsed_seconds = end - start;
    //std::cout << "Experiment Time " << elapsed_seconds.count() << std::endl;
    //std::cout << "Num data points " << xytable_path.size()*6 << std::endl;
    
    /*std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    find_opt_beam(&rfdev_mob4_1, &rfdev4_in2, &mob4_1, 50, 0.6, 15, 15, "rsrp_mob4_1_rfdev4_in2.txt");
    
    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "Time " << elapsed_seconds.count() << std::endl;*/

    paam_tx.disable();
    paam_rx.disable();

    rsrpfile.close();
    g_logfile.close();
    return 0;	
}

