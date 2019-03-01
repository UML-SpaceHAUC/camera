#!/usr/bin/env python3

import time
import serial
import sys
# this import can convert .raw to .jpeg, but need to install
# from PIL import Image

def camera_sync():
#    ser = serial. Serial(
 #           port='/dev/serial0',
  #          baudrate = 115200,
   #         parity=serial.PARITY_NONE,
    #        stopbits=serial.STOPBITS_ONE,
     #       bytesize=serial.EIGHTBITS,
      #      timeout=0.5
    #)

    counter = 0

    #received = bytearray(12)

    sync = bytearray(12)
    ack = bytearray(12)
    received = bytearray(12)
    sync[0] = 0xAA
    sync[1] = 0x0D

    ack[0] = 0xAA
    ack[1] = 0x0E
    ack[2] = 0x0D

    sleep_time = 0.005  # ms
    for i in range(60):
        #print("Run {0}".format(i))
        ser.write(sync)
        #ser.write(0xAA0D)
        #ser.write(0x0000)
        #ser.write(0x0000)
        received = ser.read(6)
#        print(received)
        try: 
            if received[0] == ack[0] and received[1] == ack[1] and received[2] == ack[2]:
        #        print("Connection Established to Camera")
                received = ser.read(6)
                if received[0] == sync[0] and received[1] == sync[1]:
                    ser.write(ack)
#                    print(received[0])
 #                   print(received[1])
  #                  print(received)
                    return True # exit for now, but continue from here in future
        except:
            pass
        time.sleep(sleep_time)
        sleep_time = sleep_time + 0.001  # ms

    return False

def camera_init_raw():
#	ser = serial. Serial(
#		port = '/dev/serial0',
#		baudrate = 115200,
#		parity = serial.PARITY_NONE,
#		stopbits = serial.STOPBITS_ONE,
#		bytesize = serial.EIGHTBITS,
#		timeout = 0.5
#	)
#	This example will initialize the camera for a JPEG snapshot picture,
#	(640 x 480 resolution)

	init = bytearray(12)
	ackinit = bytearray(12)
	received = bytearray(12)
	i = 0
	init[0] = 0xAA
	init[1] = 0x01
	init[2] = 0x00
	init[3] = 0x06
	init[4] = 0x03
	init[5] = 0x07

	ackinit[0] = 0xAA
	ackinit[1] = 0x0E
	ackinit[2] = 0x01
	
	ser.write(init)
	for i in range(60):
		received = ser.read(6)
#	print(received)
		if received[0] == ackinit[0] and received[1] == ackinit[1] and received[2] == ackinit[2]:
			return True
	print("Failed to init\n")
#		print(received)
	return False

def camera_set_pckg_size():
	pckg = bytearray(12)
	ack = bytearray(12)
	received = bytearray(12)
	i = 0
	pckg[0] = 0xAA
	pckg[1] = 0x06
	pckg[2] = 0x08
	pckg[3] = 0x00
	pckg[4] = 0x02
	pckg[5] = 0x00

	ack[0] = 0xAA
	ack[1] = 0x0E
	ack[2] = 0x06

	ser.write(pckg)

	for i in range(60):
		received = ser.read(6)
		if received[0] == ack[0] and received[1] == ack[1] and received[2] == ack[2]:
			return True
	print("bad pckg sz")
	return False

def camera_snapshot():
	snap = bytearray(12)
	ack = bytearray(12)
	received = bytearray(12)
	i = 0

	snap[0] = 0xAA
	snap[1] = 0x05
	snap[2] = 0x01

	ack[0] = 0xAA
	ack[1] = 0x0E
	ack[2] = 0x05

	ser.write(snap)
	
	for i in range(60):
		received = ser.read(6)
		if received[0] == ack[0] and received[1] == ack[1] and received[2] == ack[2]:
			return True
	return False

def camera_get_picture():
	get = bytearray(12)
	ack = bytearray(12)
	received = bytearray(12)
	data = bytearray(12)
	image = bytearray(150)
	imgfile = open("image.raw", "wb")
	i = 0
	
	received[0] = 0x00
	received[1] = 0x00
	received[2] = 0x00

	get[0] = 0xAA
	get[1] = 0x04
	get[2] = 0x02

	ack[0] = 0xAA
	ack[1] = 0x0E
	ack[2] = 0x04

	ser.write(get)
#	for i in range(60):	
	received = ser.read(6)
	received = ser.read(6)
#	print(received)
	print("\n")

#	received = ser.read(6)
#	print("\n")
#	print(received)
#	data = ser.read(12)
#	print(data)
	if received[0] == ack[0] and received[1] == ack[1] and received[2] == ack[2]:
		received = ser.read(6)
#		print(received[3])
#		print(received[4])
#		print(received[5])
#		image = ser.read(38400)
#		imgfile.write(image)
 #               return True
		received = ser.read()
		while received:
			imgfile.write(received)
			received = ser.read()
		ack[0] = 0xAA
		ack[1] = 0x0E
		ack[2] = 0x0A
		ack[3] = 0x00
		ack[4] = 0x01
		ser.write(ack)
		return True
	return False

if __name__ == "__main__":
    ser = serial. Serial(
	port = '/dev/serial0',
	baudrate = 115200,
	parity = serial.PARITY_NONE,
	stopbits = serial.STOPBITS_ONE,
	bytesize = serial.EIGHTBITS,
	timeout = 0.5
    )
    if camera_sync():
        print("Connection Established")
    else:
        print("Connection Failed")
    time.sleep(2)
    if camera_init_raw():
        print("Init Correct")
    else:
        print("Init Failed")
#    if camera_set_pckg_size():
#        print("pckg size  correct")
#    else:
#        print("pckg size fail")
    if camera_snapshot():
        print("snap")
    else:
        print("no snap")
    time.sleep(2)
    if camera_get_picture():
        print("got picture")
    else:
        print("no got picture")
