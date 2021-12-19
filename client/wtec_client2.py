#!/usr/bin/python3

from multiprocessing import Process
import subprocess
import signal
import sys
import os

import argparse
import random
import time

# Handle SIGINT properly
def signal_handler(sig, frame):
    # sys.exit(0) will raise SystemExit exception in the process, which
    # has the same effect of terminating the process
    sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)
parser = argparse.ArgumentParser()
parser.add_argument('-verbose', action='store_true')
parser.add_argument('-ip', action='store')
parser.add_argument('-port', action='store')
parser.add_argument('-processes', action='store')
flags = parser.parse_args()

IP = flags.ip if (flags.ip) else "localhost"
PORT = flags.port if (flags.port) else "1883"
PROCESS_COUNT = flags.processes if (flags.processes) else 50
COMMAND = "mosquitto_pub -h " + str(IP) + " -p " + str(PORT) + " -t counter -m "

def publish_count_data():
    while True:
        # Generate random int (0-9) and set cmd
        rand = random.randint(1, 9)
        cmd = COMMAND + "'" + str(rand) + "'"

        # Sleep for random amount of time
        sleep_time = random.randint(1, 30)
        sleep_time = 5
        print("Process " + str(os.getpid()) + ": sleep " 
              + str(sleep_time) + "s, publish " + str(rand))
        time.sleep(sleep_time)

        # Return from Process if command failed (remove if you want
        # client to attempt to send messages even if server or client 
        # has error)
        run = subprocess.run(cmd, shell=True, capture_output=True, text=True)
        #run = subprocess.run(cmd, shell=True, stdout=subprocess.DEVNULL, capture_output=)
        if (run.returncode):
            print("Process " + str(os.getpid()) + ": " + run.stderr, end="")
            return
            

# Generate multiple instances of script
process_list = [
    Process(target=publish_count_data) for _ in range(int(PROCESS_COUNT))
]

# Spool up instances
for p in process_list:
    p.start()

# Wait for every process to finish
for p in process_list:
    p.join()

print("Client shutdown")
