
import socket
import sys
from PIL import ImageGrab
from PIL import Image
import numpy as np
from time import sleep
from mss import mss
import random
import time
shape = (0, 0, 1920,1080)
mon = {"top": shape[0], "left": shape[1], "width": shape[2]-shape[1], "height": shape[3]-shape[0]}

def capture_screenshot():
    # Capture entire screen
    with mss() as sct:
        #monitor = sct.monitors[1]
        sct_img = sct.grab(mon)
        # Convert to PIL/Pillow Image
        return np.asarray(sct_img)
# Create a TCP/IP socket
# while(True):
# 	image = ImageGrab.grab()
# 	w, h = image.size
# 	print(w)
# 	print(h)
# 	im = np.array(image)
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# number of LEDs to an inch
LEDsInch = 1.52

# length of sides
sideLength = 28
topLength = 43

# number of LEDs
sideLEDs = 42
topLEDs = 64

# number of zones
sideNumZones = 7
topNumZones = 11

# print(int(1080/(sideLength*LEDsInch/sideNumZones)))

#Connect the socket to the port where the server is listening
server_address = ('192.168.0.18', 23)
print('connecting to %s port %s' % server_address)
#image = ImageGrab.grab()
sock.connect(server_address)
while(True):	
	start_time = time.time()
	image = ImageGrab.grab()
	# image = Image.open(r'C:\Users\adhik\Pictures\Untitled.jpg'))
	

	im = np.array(image) # BGR
	w, h = image.size
	
	# number of pixels to an inch
	pixPerInch = 33

	# LED lists :
	leftLEDs = []
	rightLEDs = []
	topLEDs = []

	# Calculate the average values per zone on sides
	i_prev = 0
	i_max = 1080
	for i in range(134,1080, 135):
	    # calculate average color of left/right zone
	    leftZone = np.mean(np.mean(im[i_max-135:i_max,:147,:], axis=0), axis=0).astype(int)
	    rightZone = np.mean(np.mean(im[i_prev:i,1773:,:], axis=0), axis=0).astype(int)
	    
	    # append the average value using the proper format for LED strip command
	    leftLEDs.append(str(leftZone[0]) + "r" + str(leftZone[1]) + "g" + str(leftZone[2]) + "b")
	    rightLEDs.append(str(rightZone[0]) + "r" + str(rightZone[1]) + "g" + str(rightZone[2]) + "b")
	    
	    # update the previous i value
	    i_prev = i
	    i_max -= 135
	    
	# Calculate the average values per zone on top/bottom
	j_prev = 10
	for j in range(147,1920, 147):
	    # calculate average color of top/bottom zone
	    topZone = np.mean(np.mean(im[:135,j_prev:j,:], axis=0), axis=0).astype(int)
	    
	    # append the average value using the proper format LED strip command
	    topLEDs.append(str(topZone[0]) + "r" + str(topZone[1]) + "g" + str(topZone[2]) + "b")
	    j_prev = j

	# create the command for a given number of lights :
	commands = ""
	ind = 0
	for k in range(0, len(leftLEDs)):
	    for l in range(5):
	        commands+=(leftLEDs[k] + str(ind) + "i")
	        ind += 1
	        

	ind += 5

	for m in topLEDs:
	    for l in range(6):
	        commands+=(m + str(ind) + "i")
	        ind += 1

	ind += 7
	        
	for n in rightLEDs:
	    for l in range(5):
	        commands+=(n + str(ind) + "i")
	        ind += 1

	
	commands+='x'
	sock.send(commands.encode('utf8'))
	print("--- %s seconds ---" % (time.time() - start_time))
	
