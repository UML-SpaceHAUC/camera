#!/usr/bin/env python3

import time
import serial
import sys

def camera_sync():
    #SYNC routine for UCAM-III
    counter = 0

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
        ser.write(sync)
        received = ser.read(6)
        try:
            if received[0] == ack[0] and received[1] == ack[1] and received[2] == ack[2]:
                received = ser.read(6)
                if received[0] == sync[0] and received[1] == sync[1]:
                    ser.write(ack)
                    return True
        except:
            pass
        time.sleep(sleep_time)
        sleep_time = sleep_time + 0.001  # ms

    return False

def camera_init_raw():
#	This example will initialize the camera for a RAW snapshot picture,
#	(160 x 120 resolution)

	init = bytearray(12)
	ackinit = bytearray(12)
	received = bytearray(12)
	i = 0
	init[0] = 0xAA
	init[1] = 0x01
	init[2] = 0x00
	init[3] = 0x07     #image format:
                        #03 for 8 bit RAW
                        #08 for RAW 16 bit color, CrYCbY
                        #06 for RAW 16 bit color, 565(RGB)
                        #07 for JPEG

	init[4] = 0x03     #RAW resolution (don't care in JPEG mode)
                        #01 (80x60)
                        #03 (160x120)
                        #09 (128x128)
                        #0B (128x96)

	init[5] = 0x07     #JPEG resolution (don't care in RAW mode)
                        #03 (160x128)
                        #05 (320x240)
                        #07 (640x480)

	ackinit[0] = 0xAA
	ackinit[1] = 0x0E
	ackinit[2] = 0x01

	ser.write(init)
	for i in range(60):
		received = ser.read(6)
		if received[0] == ackinit[0] and received[1] == ackinit[1] and received[2] == ackinit[2]:
			return True
	print("Failed to init\n")
	return False

def camera_set_pckg_size():
	pckg = bytearray(12)
	ack = bytearray(12)
	received = bytearray(12)
	i = 0
	pckg[0] = 0xAA
	pckg[1] = 0x06
	pckg[2] = 0x08
	pckg[3] = 0x00     #pckg size low byte (40 - FF)
	pckg[4] = 0x02     #pckg size high byte (00-02) (MAX is 0200)
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
	snap[2] = 0x00	#snapshot type
			#00 = JPEG
			#01 = RAW
	snap[3] = 0x00  #low skip frame byte (00-FF)
	snap[4] = 0x00  #high skip frame byte (00-FF)

	ack[0] = 0xAA
	ack[1] = 0x0E
	ack[2] = 0x05

	ser.write(snap)

	for i in range(60):
		received = ser.read(6)
		if received[0] == ack[0] and received[1] == ack[1] and received[2] == ack[2]:
			return True
	return False

def camera_get_picture_raw():
    #this function is set up to completely capture and save any
    #of the RAW image setups. JPEG capture must be setup differently.
	get = bytearray(12)
	ack = bytearray(12)
	received = bytearray(12)
	data = bytearray(12)
	image = bytearray(150)
	imgfile = open("image.raw", "wb")  #can change name, open for writing bytes
	i = 0

	received[0] = 0x00
	received[1] = 0x00
	received[2] = 0x00

	get[0] = 0xAA
	get[1] = 0x04
	get[2] = 0x02      #picture type:
                        #01 = snapshot
                        #02 = raw
                        #05 = JPEG

	ack[0] = 0xAA
	ack[1] = 0x0E
	ack[2] = 0x04

	ser.write(get)
	received = ser.read(6)
	received = ser.read(6)     #double reads required for ack?

	if received[0] == ack[0] and received[1] == ack[1] and received[2] == ack[2]:
		received = ser.read(6)    #DATA readout, don't care in this case
		received = ser.read()
		while received:
			imgfile.write(received)
			received = ser.read()       #sends all data at once, so read it until nothing left
		ack[0] = 0xAA
		ack[1] = 0x0E
		ack[2] = 0x0A
		ack[3] = 0x00
		ack[4] = 0x01
		ser.write(ack)                    #MUST ACK OR CAMERA LOOPS
		return True
	return False

def camera_get_picture_jpg():
    #this function is set up to completely capture and save any
    #of the JPEG setups.
	get = bytearray(12)
	ack = bytearray(12)
	received = bytearray(512)
	data = bytearray(12)
	imgfile = open("test.jpg", "wb")  #can change name, open for writing bytes
	print("opened")
	i = 0

	received[0] = 0x00
	received[1] = 0x00
	received[2] = 0x00

	get[0] = 0xAA
	get[1] = 0x04
	get[2] = 0x05      #picture type:
                        #01 = snapshot
                        #02 = raw
                        #05 = JPEG

	ack[0] = 0xAA
	ack[1] = 0x0E
	ack[2] = 0x04

	ser.write(get)
	#received = ser.read(6)
	for i in range(60):
		received = ser.read(6)     #double reads required for ack?
		if received[0] == ack[0] and received[1] == ack[1] and received[2] == ack[2]:
			print("acked for jpg")
			break

	size = 0
	temp = 0
	nump = 0

	data = ser.read(6)    #DATA readout
	if data[0] == 0xAA and data[1] == 0x0A and data[2] == 0x05:
		print("did a data:")
		for value in data:
			print(value)
		size += data[5]
		temp = data[4]
		temp = temp * 256
		size += temp
		temp = data[3]
		temp = temp * 65536
		size += temp

		print("size: ", size)

		nump = size/(512 - 6)
		ack[0] = 0xAA
		ack[1] = 0x0E
		ack[2] = 0x00
		ack[3] = 0x00
		ack[4] = 0x00
		ack[5] = 0x00
		ser.write(ack)

		received = ser.read(512)
		size = received[4]
		temp = received[3]
		temp *= 256
		size += temp
		ack[0] = 0xAA
		ack[1] = 0x0E
		ack[2] = 0x00
		ack[3] = 0x00
		ack[4] = received[0]
		ack[5] = received[1]
		pcknum = 0
		while pcknum < nump:
			if not imgfile.write(received[4:510]):
				print("error reading")
				sys.exit()  # 510-size
			ack[4] = received[0]
			ack[5] = received[1]
			ser.write(ack)
			received = ser.read(512)       #sends all data at once, so read it until nothing left
			pcknum = received[4]
			temp = received[3]
			temp *= 256
			pcknum = temp
            print(pcknum, nump)
							#MUST ACK OR CAMERA LOOPS
		return True
	return False

def camera_set_baud():
	#sets camera baud rate.
	baud = bytearray(12)
	ack = bytearray(12)
	received = bytearray(12)
	i = 0

	baud[0] = 0xAA
	baud[1] = 0x07
	baud[2] = 0x1F     #baud 1st divider
	baud[3] = 0x00     #baud 2nd divider (see datasheet for more)

	ack[0] = 0xAA
	ack[1] = 0x0E
	ack[2] = 0x07

	ser.write(baud)

	for i in range(60):
		received = ser.read(6)
		if received[0] == ack[0] and received[1] == ack[1] and received[2] == ack[2]:
			return True
	return False

def camera_reset():
	#sets camera baud rate.
	reset = bytearray(12)
	ack = bytearray(12)
	received = bytearray(12)
	i = 0

	reset[0] = 0xAA
	reset[1] = 0x08
	reset[2] = 0x00		#reset type:
				#00 = ucam reboot, reset registers and SM
				#01 = no reboot, SM only
	reset[3] = 0x00
	reset[4] = 0x00
	reset[5] = 0xFF		#if FF, special immediate reset.

	ack[0] = 0xAA
	ack[1] = 0x0E
	ack[2] = 0x08

	ser.write(reset)

	#assume it worked? Try to sync.
	return True

def camera_light():
	#sets camera light frequency (hum).
	light = bytearray(12)
	ack = bytearray(12)
	received = bytearray(12)
	i = 0

	light[0] = 0xAA
	light[1] = 0x13
	light[2] = 0x00		#00 = 50hz
				#01 = 60hz

	ack[0] = 0xAA
	ack[1] = 0x0E
	ack[2] = 0x13

	ser.write(light)

	for i in range(60):
		received = ser.read(6)
		if received[0] == ack[0] and received[1] == ack[1] and received[2] == ack[2]:
			return True
	return False

def camera_contrast_bright_exp():
	#sets camera contrast brightness and exposure.
	cbe = bytearray(12)
	ack = bytearray(12)
	received = bytearray(12)
	i = 0

	cbe[0] = 0xAA
	cbe[1] = 0x14
	cbe[2] = 0x02     #contrast (00 (MIN) - 04 (MAX))
	cbe[3] = 0x02     #brightness (00 (MIN) - 04 (MAX))
	cbe[4] = 0x02     #exposure (00 (-2) - 04 (+2))

	ack[0] = 0xAA
	ack[1] = 0x0E
	ack[2] = 0x14

	ser.write(cbe)

	for i in range(60):
		received = ser.read(6)
		if received[0] == ack[0] and received[1] == ack[1] and received[2] == ack[2]:
			return True
	return False

def camera_sleep():
	#sets camera sleep timeout.
	sleep = bytearray(12)
	ack = bytearray(12)
	received = bytearray(12)
	i = 0

	sleep[0] = 0xAA
	sleep[1] = 0x15
	sleep[2] = 0x00		#00-FF (0-255 seconds)
				#0 means disable sleep

	ack[0] = 0xAA
	ack[1] = 0x0E
	ack[2] = 0x15

	ser.write(sleep)

	for i in range(60):
		received = ser.read(6)
		if received[0] == ack[0] and received[1] == ack[1] and received[2] == ack[2]:
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
        if camera_reset():
            print("reset")
            sys.exit()
        else:
            print("uh oh")
            sys.exit()
    time.sleep(2)
    if camera_init_raw():
        print("Init Correct")
    else:
        print("Init Failed")
        sys.exit()
    if camera_set_pckg_size():
        print("pckg size  correct")
    else:
        print("pckg size fail")
        sys.exit()
 #   if camera_snapshot():
 #       print("snap")
 #   else:
 #       print("no snap")
 #       sys.exit()
    time.sleep(2)
    if camera_get_picture_jpg():
        print("got picture")
    else:
        print("no got picture")
        sys.exit()
