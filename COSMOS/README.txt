3/14/2024
================================================================================================================
On srv1-lg1
python3 GNURadioOFDMExample/TX_OFDM_enb1_3840_gain20.py 
On srv2-lg1
python3 GNURadioOFDMExample/TX_OFDM_enb2_3840_gain20.py
On mob4-1
python3 GNURadioOFDMExample/RX_OFDM_mobue_gain10.py


g++ paam.cpp ofdm_tx.cpp ofdm_rx.cpp xytable.cpp gen_trace_two_gnb.cpp -o gen_trace_two_gnb.o -lcurl -pthread


================================================================================================================
Modified the application to collect data from up to 4 gNodeBs

cd /opt/uhd/host/build/examples

On srv1-lg1
./tx_samples_from_file_udp_trigger --args="addr=10.39.6.1,master_clock_rate=122.88e6" --freq 2.99616e9 --rate 3.84e6 --gain 20 --file "/root/GNURadioOFDMExample/tx_samps.dat" --subdev "B:0" --type=float --ipaddr "10.37.1.1" --port 1234
On srv2-lg1
./tx_samples_from_file_udp_trigger --args="addr=10.39.6.2,master_clock_rate=122.88e6" --freq 3.00384e9 --rate 3.84e6 --gain 20 --file "/root/GNURadioOFDMExample/tx_samps.dat" --subdev "B:0" --type=float --ipaddr "10.37.1.2" --port 1234
On mob4-2
./tx_samples_from_file_udp_trigger --args="resource=RIO0,master_clock_rate=184.32e6" --freq 2.99616e9 --rate 3.84e6 --gain 10 --file "/root/GNURadioOFDMExample/tx_samps.dat" --subdev "A:0" --type=float --ipaddr "10.37.21.2" --port 1234
On mob4-3
./tx_samples_from_file_udp_trigger --args="resource=RIO0,master_clock_rate=184.32e6" --freq 3.00384e9 --rate 3.84e6 --gain 10 --file "/root/GNURadioOFDMExample/tx_samps.dat" --subdev "A:0" --type=float --ipaddr "10.37.21.3" --port 1234

On mob4-1
python3 GNURadioOFDMExample/RX_OFDM_mobue_gain10.py


On srv1-lg1
cd paam
set NUM_GNB in gen_trace_four_gnb.cpp to 4 (works with 2 or 4)
g++ paam.cpp ofdm_tx.cpp ofdm_rx.cpp xytable.cpp gen_trace_four_gnb.cpp -o gen_trace_four_gnb.o -lcurl -pthread
