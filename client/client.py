
from multiprocessing import Process
import subprocess
import signal
import os

import random
import time

# Handle SIGINT properly
# exit(1) will raise SystemExit, terminating all processes
signal.signal(signal.SIGINT, lambda s, f: exit(1))

IP  = "localhost"
PORT = "1883"
COMMAND = ["mosquitto_pub", "-h", IP, "-p", PORT, "-t", "count", "-m"]
INSTANCES = 50

def run():
    while True:
        # Generate random int [0-9], append to cmd
        rand = random.randint(1, 9)
        cmd = COMMAND + [str(rand)]

        # Sleep for random amount of time
        sleep = random.randint(1, 30)
        print("Process " + str(os.getpid()) + ": sleep "
              + str(sleep) + "s, publish " + str(rand))
        time.sleep(sleep)

        # Run command, if error is returned then exit process
        run = subprocess.run(cmd, capture_output=True, text=True)
        if (run.returncode):
           print("Process " + str(os.getpid()) + ": " + run.stderr, end="")
           return

# Generate multiple instances of script
process_list = [Process(target=run) for _ in range(int(INSTANCES))]

# Spool up instances, then wait for them to finish
for p in process_list:
    p.start()

for p in process_list:
    p.join()

print("Client shutdown")

