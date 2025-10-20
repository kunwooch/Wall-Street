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
    """
    ----------------------------------INITIALIZATION---------------------------------
    Make sure LDAC is not set to 0 (immediate update) before setting the offset to 0V.
    By default, the offset is 0x1555. 
    If offset is changed to 0x0000 and LDAC is set, all channels will output 6V.
    """

    LDAC_pin = 5
    BUSY_pin = 22
    CLR_pin = 27
    RESET_pin = 4
    ac1 = AD5370_control(LDAC_pin, BUSY_pin, CLR_pin, RESET_pin, port=0, device=0)

    time.sleep(1)
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

    print("----------Writing 0V to all channels----------")
    for ch_idx in range(ac1.channel_num):
        ac1.write_value_volt(ch_idx, 0, immediate=False)
        ac2.write_value_volt(ch_idx, 0, immediate=False)

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

if __name__ == "__main__":
    main()
