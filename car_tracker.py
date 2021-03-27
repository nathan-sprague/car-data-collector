# sudo apt-get -y install can-utils libsocketcan2 libsocketcan-dev

import requests
import os
import time
from threading import Thread


importedCan = False
try:
    import can
    importedCan = True
except:
    print("can't import can")


def logTemp():
    while True:
        r = requests.get('http://192.168.4.1')
        if r.status_code == 200:
            print('Success!')
            print("content " + str(r.content))
            with open("tempData.txt", 'a+') as fileHandle:
                fileHandle.write(str(time.time()) + "," + str(r.content) + "\n")
                fileHandle.close()

        elif r.status_code == 404:
            print('Not Found.')
        time.sleep(1)


def setupCan(canSpeed):
    print('Bring up CAN0....')
    os.system("sudo /sbin/ip link set can0 up type can bitrate " + str(canSpeed))
    time.sleep(0.1)
    try:
        bus = can.interface.Bus(channel='can0', bustype='socketcan_native')
        return bus
    except OSError:
        print('Cannot find PiCAN board.')
    return False

def receiveMessage():
    global bus
    while True:
        message = bus.recv() # Wait until a message is received.
        
        # c = '{0:f} {1:x} {2:x} '.format(message.timestamp, message.arbitration_id, message.dlc)
        s = str(message.timestamp)
        s += ", " + message.arbitration_id
   
        for i in range(message.dlc):
           s += str(hex(message.data[i]))
        print(s)

        with open("canData.txt", 'a+') as fileHandle:
            fileHandle.write(str(s) + "\n")
            fileHandle.close()
    # return message

def endCan():
    os.system("sudo /sbin/ip link set can0 down")
    print("can ended")



if importedCan:
    bus = setupCan(500000)
    if bus != False:
	    thread1 = Thread(target=receiveMessage)
	    thread2 = Thread(target=logTemp)

	    thread1.start()
	    thread2.start()
	    time.sleep(1)

	    thread1.join()
	    thread2.join()
