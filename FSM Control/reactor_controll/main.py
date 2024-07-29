from current_monitoring import CurrentMonitor
from reactor_controller import ReactorController, Reactor
import time
import subprocess

subprocess.Popen(['python3', 'server_DAC.py'])
# Initialize the current monitor
reactor_monitor = CurrentMonitor('****', 'NGP824', 'NGP824', '/int/logging') # Use with rohde and schwarz logging power supply, this section of code can be adjusted to work with other PSU hardware

# Initialize the reactor controller
controller = ReactorController()

R1_treshold = ** #To be manually adjusted based on reactor current state
R2_treshold = ** #To be manually adjusted based on reactor current state

R1_feed_times = {'p1': '172000', 'p2': '28000'}
R2_feed_times = {'p3': '172000', 'p4': '28000'}

# Create reactors with specific channels and thresholds
R1 = Reactor(channel='I1[A]', threshold = (R1_treshold/1000), feed_times= R1_feed_times)
R2 = Reactor(channel='I2[A]', threshold = (R2_treshold/1000), feed_times= R2_feed_times)
# Add more reactors as needed

# Add reactors to the controller
controller.add_reactor(R1)
controller.add_reactor(R2)
# Add more reactors as needed

# Monitoring loop
while True:
    current_values = reactor_monitor.start_monitoring()
    if current_values:
        controller.update_reactors(current_values)
        controller.print_reactor_states()
    else:
        print("No data received or an error occurred.")
    time.sleep(60)  
