"""
    Control of the AD5370 EVAL board using a Raspberry Pi 3.
    Uses SPI and some GPIO pins.
    Created: 2016-05-12
    Author: Filip Lindau、

    Modified by Chenghao Feng
    Changes:
    Compatible with python 3.7 or newer
    Compatible with Raspberry Pi 4

    Modified by Kun Woo Cho
    Changes:
    Offseting to 0V
    Compatible with two DACs and two SPIs (RPI provides up to two SPIs)
"""

import spidev
import time
import RPi.GPIO as gpio
import struct
import numpy as np
import h5py
import scipy.io
import socket

class AD5370_control(object):
    def __init__(self, ldac, busy, clr, reset, port=0, device=0):
        self.V_ref = 5.00
        self.offsets = []
        self.gains = []
        self.voltages = []
        self.V_max = []
        self.V_min = []

        self.channel_num = 40
        # default value
        for o in range(self.channel_num):
            self.offsets.append(0x0)
            self.gains.append(1)
            self.V_max.append(4*self.V_ref*(65535-4*self.offsets[0])/2**16)
            self.V_min.append(4*self.V_ref*(-4*self.offsets[0])/2**16)
            self.voltages.append(0.0)

        self.LDAC_pin = ldac
        self.BUSY_pin = busy
        self.CLR_pin = clr
        self.RESET_pin = reset

        self.spi = None
        self.spi_port = port       # There is one spi port exposed on the raspberry 40 pin connector
        self.spi_device = device     # There are two devices available through the spio cs pins

        self.clear_state = True
        self.setup_gpio()
        self.setup_spi()

    def setup_gpio(self):
        """
        Setup gpio pins on the raspberry. We use BCM naming.
        :return:
        """
        gpio.setmode(gpio.BCM)
        gpio.setup(self.LDAC_pin, gpio.OUT)
        gpio.setup(self.RESET_pin, gpio.OUT)
        gpio.setup(self.CLR_pin, gpio.OUT)
        gpio.setup(self.BUSY_pin, gpio.IN)

        self.reset()
        self.clear(False)

    def setup_spi(self):
        """
        Setup SPI interface on the raspberry using the spidev module.
        :return:
        """
        self.spi = spidev.SpiDev()
        # openning /dev/spidev<port>.<device>
        self.spi.open(self.spi_port, self.spi_device)
        self.spi.mode = 0b10        # Something something polarity clock

    def close(self):
        self.spi.close()
        gpio.cleanup()

    def reset(self):
        print("----------Resetting DACs (Low --> High)----------")
        gpio.output(self.RESET_pin, 0)
        if pin_status(0, gpio.input(self.RESET_pin)):
            return 1
        time.sleep(0.001)
        gpio.output(self.RESET_pin, 1)
        if pin_status(1, gpio.input(self.RESET_pin)):
            return 1
        return 0

    def clear(self, enable=None):
        """
        Puts the device in CLEAR state (all outputs to ground).
        :param enable: True = Device in CLEAR state
         False = Device released from CLEAR
         None (default) = Toggle CLEAR state
        :return:
        """
        print("----------Clearing DACs----------")
        if enable is None:
            if self.clear_state is True:
                enable = False
            else:
                enable = True
        if enable is True:
            gpio.output(self.CLR_pin, 0)
            self.clear_state = True
        else:
            gpio.output(self.CLR_pin, 1)
            self.clear_state = False

    def load_dac(self, keep=False):
        """
        Updates DAC outputs by pulsing LDAC low. If keep=True the LDAC
        print("DAC1 SPI Xfer input:  ", write_list)
        remains low, causing immediate updates when new values are written
         to the DAC.
        :param keep: False (default) = LDAC is pulsed causing written values to update the outputs.
        If new values are written after this, they are withheld until a new call to load_dac is made.
        True = LDAC is kept low, causing new values to immediately be output
        :return:
        """
        gpio.output(self.LDAC_pin, 0)
        if pin_status(0, gpio.input(self.LDAC_pin)):
            return 1
        if keep is False:
            time.sleep(0.001)
            gpio.output(self.LDAC_pin, 1)
            if pin_status(1, gpio.input(self.LDAC_pin)):
                return 1
        return 0

    def write_value_volt(self, output, value, immediate=True):
        """
        Write DAC value to specific output pin
        :param output: Pin to output. 0-39
        :param value:  Voltage to write. Range -6.67 - 13.3 V for default offset and 5 V reference
        :param immediate: True = Output voltage to pin immediately (default)
                          False= Wait for call to load_dac
        :return:
        """
        x = 0b11000000
        if not isinstance(output, int):
            raise TypeError('Output must be integer')
        if output < 0 or output > 39:
            raise ValueError('Output must be in range 0-39')
        if value < self.V_min[output] or value > self.V_max[output]:
            raise ValueError(''.join(('Value must be in range ', str(self.V_min[output]), '-', str(self.V_max[output]))))
        # gr = output / 8     # Group number
        # ch = output % 8     # Channel number
        a = output + 8
        # self.voltages[output] = value
        dac = int(value * 2**16 / (4 * self.V_ref) + self.offsets[output] * 4)
        d = struct.pack('<H', dac)
        write_list = [x+a, d[1], d[0]]
        self.spi.xfer(write_list)
        if immediate is True:
            self.load_dac()

    def write_offset_int(self, value):
        """
        Write DAC offset value to specific output pin
        :param output: Pin to output. 0-39
        :param value:  Value to write. 0-16384
        :return:
        """
        if not isinstance(value, int):
            raise TypeError('Value must be integer')
        if value < 0 or value > 16384:
            raise ValueError('Value must be in range 0-65535')
        d = struct.pack('<H', value)
        # group 0 - channel 1 to 8
        write_list = [2, d[1], d[0]]
        ret = self.spi.xfer(write_list)
        # print("DAC1 SPI Xfer input:  ", write_list)
        # print("DAC1 SPI xfer output: ", ret)
        if sum(write_list) != sum(ret):
            print("ERROR:::SPI xfer error occurred")
            return 1
        # group 1 - channel 9 to 40
        write_list = [3, d[1], d[0]]
        ret = self.spi.xfer(write_list)
        # print("DAC2 SPI Xfer input:  ", write_list)
        # print("DAC2 SPI Xfer output: ", ret)
        if sum(write_list) != sum(ret):
            print("ERROR:::SPI xfer error occured")
            return 1
        for idx in range(self.channel_num):
            self.offsets[idx] = 0x0
            self.V_max[idx] = 4*self.V_ref*(65535-4*self.offsets[0])/2**16
            self.V_min[idx] = 4*self.V_ref*(-4*self.offsets[0])/2**16
        return 0

    def write_offset_volt(self, value):
        """
        Write DAC offset value to specific output pin
        :param output: Pin to output. 0-39
        :param value:  Voltage offset to write. Range 0 - 20 V for 5 V reference
        :return:
        """
        print("DAC1 SPI Xfer input:  ", write_list)
        """
        if not isinstance(value, int):
        print("DAC1 SPI Xfer input:  ", write_list)
            raise TypeError('Value must be integer')
        """
        print("DAC1 SPI Xfer input:  ", write_list)
        if value < 0 or value > 20:
            raise ValueError('Value must be in range 0-20')
        dac = int(value * 2**16 / (4 * self.V_ref))
        d = struct.pack('<H', dac)
        # group 0
        write_list = [2, d[1], d[0]]
        self.spi.xfer(write_list)
        #self.load_dac()
        # group 1
        write_list = [3, d[1], d[0]]
        self.spi.xfer(write_list)
        #self.load_dac()

    def write_gain(self, output, value):
        """
        Write DAC gain value to specific output pin
        :param output: Pin to output. 0-39
        :param value:  Value to write. 0-65535
        :return:
        """
        print(d)
        if not isinstance(output, int):
            raise TypeError('Output must be integer')
        if output < 0 or output > 39:
            raise ValueError('Output must be in range 0-39')
        if not isinstance(value, int):
            raise TypeError('Value must be integer')
        if value < 0 or value > 65535:
            raise ValueError('Value must be in range 0-65535')
        x = 0b01000000
        a = output
        d = struct.pack('<H', value)
        write_list = [x + a, d[1], d[0]]
        self.spi.xfer(write_list)
        self.load_dac()

    def write_function(self, function, value):
        """
        Write special function
        :param function: 6 bit function code
        :param value:  Value to write. 0-65535
        :return:
        """
        if not isinstance(function, int):
            raise TypeError('Function must be integer')
        if function < 0 or function > 64:
            raise ValueError('Function must be in range 0-63')
        if not isinstance(value, int):
            raise TypeError('Value must be integer')
        if value < 0 or value > 65535:
            raise ValueError('Value must be in range 0-65535')
        x = 0b00000000
        a = function
        d = struct.pack('<H', value)
        write_list = [x + a, d[1], d[0]]
        self.spi.xfer(write_list)

def pin_status(command, state):
    if state:
        print('on')
    else:
        print('off')
    if command == state:
        return 0
    else:
        "ERROR:::PIN ERROR occurred"
        return 1

def main():
    gpio.setwarnings(False)
    gnb1, gnb2, ue, direction = input("gNB 1 index [0-2], gNB 2 index [0-2], ue num [1-2], and direction [0 for positive, 1 for negative]: ").split()
    
    print("gNB 1: ", gnb1)
    print("gNB 2: ", gnb2)
    print("UE: ", ue)
    if int(direction) == 1:
        print("Angles in negative (ie. reverse)")
        filename = 'mbb_enb' + gnb1 + '_enb' + gnb2 + '_ue' + ue + "_reverse.mat"
    else:
        print("Angles in positive")
        filename = 'mbb_enb' + gnb1 + '_enb' + gnb2 + '_ue' + ue + ".mat"
    

    print("Reading " + filename) 

    """
    ----------------------------------INITIALIZATION---------------------------------
    Make sure LDAC is not set to 0 (immediate update) before setting the offset to 0V.
    By default, the offset is 0x1555. 
    If offset is changed to 0x0000 and LDAC is set, all channels will output 6V.
    """
    if direction:
        f = scipy.io.loadmat('/home/kwcho/Desktop/controller/DACData/' + filename)
    else:
        f = scipy.io.loadmat('/home/kwcho/Desktop/controller/DACData/' + filename)
 
    ue_list = f['ue_list']
    um_list = f['um_list']
    Tx_angle1 = f['Tx_angle1']
    Tx_angle2 = f['Tx_angle2']
    eNB_angle = f['eNB_angle']
    
    dac1_e = range(0, 39, 2)
    dac1_m = range(1, 40, 2)
    dac2_e = range(0, 37, 2)
    dac2_m = range(1, 38, 2)

    #broken_e = [0, 7, 16, 26, 36]
    broken_e = []
    broken_m = []

    LDAC_pin = 5
    BUSY_pin = 22
    CLR_pin = 27
    RESET_pin = 4
    ac1 = AD5370_control(LDAC_pin, BUSY_pin, CLR_pin, RESET_pin, port=0, device=0)
    time.sleep(0.2)

    LDAC_pin = 23
    BUSY_pin = 25
    CLR_pin = 24
    RESET_pin = 26
    ac2 = AD5370_control(LDAC_pin, BUSY_pin, CLR_pin, RESET_pin, port=1, device=0)

    print("----------Set LDAC High (no immediate change of voltages)----------")
    time.sleep(0.001)
    gpio.output(ac1.LDAC_pin, 1)
    state = gpio.input(ac1.LDAC_pin)
    err = pin_status(1, gpio.input(ac1.LDAC_pin))
    if err > 0:
        ac1.clear()
        ac2.clear()
        return 
    time.sleep(0.001)
    gpio.output(ac2.LDAC_pin, 1)
    err = pin_status(1, gpio.input(ac2.LDAC_pin))
    if err > 0:
        ac1.clear()
        ac2.clear()
        return
    time.sleep(0.2)
    
    """
    time.sleep(0.001)
    err = ac1.reset()
    if err > 0:
        ac1.clear()
        ac2.clear()
        return
    time.sleep(0.001)
    err = ac2.reset()
    if err > 0:
        ac1.clear()
        ac2.clear()
        return
    time.sleep(1)
    """

    print("----------Set OFFSET to 0V----------")
    time.sleep(0.001)
    err = ac1.write_offset_int(0)
    if err > 0:

        ac1.clear()
        ac2.clear()
        return
    time.sleep(0.001)
    ret = ac2.write_offset_int(0)
    if err > 0:
        ac1.clear()
        ac2.clear()
        return
    time.sleep(0.2)


    print("----------Writing 0V to all channels----------")
    for ch_idx in range(ac1.channel_num):
        ac1.write_value_volt(ch_idx, 0, immediate=False)
        ac2.write_value_volt(ch_idx, 0, immediate=False)
    time.sleep(0.2)


    print("Set LDAC Low (immediate updates when new values are written)")
    time.sleep(0.001)
    err = ac1.load_dac(keep=True)
    if err > 0:
        ac1.clear()
        ac2.clear()
        return
    time.sleep(0.001)
    err = ac2.load_dac(keep=True)
    if err > 0:
        ac1.clear()
        ac2.clear()
        return

    time.sleep(0.2)
    
    RPI_IP = "10.37.23.1"
    
    userInput = 0
    inSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
    inSocket.bind((RPI_IP, 1234))
    
    while userInput != "exit":
        # print("Listening on port 1234 for input - a steering angle or exit: ")
        [userInputBytes, sender] = inSocket.recvfrom(4)
        # print(userInputBytes)
        userInput = str(userInputBytes, encoding="utf-8") 

        if userInput == "exit":
            # print("Setting all channels to 0V...") 
            # for ch_idx in range(ac1.channel_num):
            #    ac1.write_value_volt(ch_idx, 0, immediate=False)
            #    ac2.write_value_volt(ch_idx, 0, immediate=False)
            print("Exiting the program...")

        elif userInput == "0V":
            print("Setting all channels to 0V...")
            for ch_idx in range(ac1.channel_num):
                ac1.write_value_volt(ch_idx, 0, immediate=False)
                ac2.write_value_volt(ch_idx, 0, immediate=False)
        
        else:
            try:
                input_loc = int(userInput)
            except ValueError:
                print("Not an integer!")
                continue
            else:
                if input_loc >= 0 and input_loc <= 29:
                    print("Input location: %2d" % input_loc)

                    start = time.time()

                    volt_idx = np.where(f['loc'].flatten() == input_loc)[0][0]
                    UE_angle1 = Tx_angle1[volt_idx]
                    UE_angle2 = Tx_angle2[volt_idx]
                    eNB_angle1 = eNB_angle[volt_idx, 0]
                    eNB_angle2 = eNB_angle[volt_idx, 1]

                    print("gNB 1 angle: %2d, gNB 2 angle: %2d, UE angle 1: %2d, UE angle 2: %2d" %  (eNB_angle1, eNB_angle2, UE_angle1, UE_angle2))
                    
                    # Reverse the voltage list because the test 1 to 7 is in the order of working->broken boards
                    # but the edge of working board does not take up the whole 40 pins (FR4 control board was designed this way)
                    # We group adjacent two ribs to speed up the steering so we index by 2

                    ue = ue_list[volt_idx, ::-2].round(2)
                    um = um_list[volt_idx, ::-2].round(2)
                    
                    # print("UE: ", ue)
                    # print("UM: ", um)
                    
                    ue = np.append(ue,ue[-1])
                    um = np.append(um,um[-1])

                    # Read the lookup table and apply voltage levels
                    # start = time.time()
                    '''
                    for ch_idx in range(ac1.channel_num):
                        ac1.write_value_volt(ch_idx, 0, immediate=False)
                        ac2.write_value_volt(ch_idx, 0, immediate=False)

                    '''
                    for idx in range(len(dac1_e)):
                        if dac1_e[idx] not in broken_e:
                            ue_val = ue[idx]
                            um_val = um[idx]
                            if ue_val >= 0 and ue_val < 16 and um_val >= 0 and um_val < 16:
                                #print('dac1_e: %2d, val: %6sV || dac1_m: %2d, val: %6sV' % (dac1_e[idx], ue_val, dac1_m[idx], um_val))
                                ac1.write_value_volt(dac1_e[idx], ue_val, immediate=False)
                                ac1.write_value_volt(dac1_m[idx], um_val, immediate=False)
                            else:
                                print('Negative volt detected')
                        else:
                            print('dac2_e: %2d is broken board' % dac1_e[idx])

                    for idx in range(len(dac2_e)):
                        if dac2_e[idx]+ ac1.channel_num not in broken_e:
                            ue_val = ue[idx+len(dac1_e)]
                            um_val = um[idx+len(dac1_m)]
                            if ue_val >= 0 and ue_val < 16 and um_val >= 0 and um_val < 16:
                                #print('dac2_e: %2d, val: %6sV || dac2_m: %2d, val: %6sV' % (dac2_e[idx]+ac1.channel_num, ue_val, dac2_m[idx]+ac1.channel_num, um_val))
                                ac2.write_value_volt(dac2_e[idx], ue_val, immediate=False)
                                ac2.write_value_volt(dac2_m[idx], um_val, immediate=False)
                            else:
                                print('Negative volt detected')
                        else:
                            print('dac2_e: %2d is broken board' % dac2_e[idx])
                    
                    end = time.time()
                    print("Execution time", (end-start)* 1000, "miliseconds")
                    
                    time.sleep(0.04)
                    inSocket.sendto(str.encode("Done"),(sender[0],3001))

            
    #ac1.clear()
    #ac2.clear()
    #ac1.close()
    #ac2.close()

if __name__ == "__main__":
    main()
