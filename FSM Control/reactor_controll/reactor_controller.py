import json
import time
import os
import datetime


class Reactor:
    def __init__(self, channel, threshold, feed_times):
        self.channel = channel
        self.current = 0
        self.threshold = threshold
        self.state = "Startup"
        self.feed_times = feed_times
        self.config_file_path = f'{channel}_config.json'
        self.last_recovery_time = None
        self.load_reactor_config()

    def update_current(self, current):
        self.current = current
        self.update_state()

    def update_state(self):
        now = datetime.datetime.now()

        # remaining_time = 1.5 - ((now - self.last_recovery_time).total_seconds() / 3600)
        # print(f"Previous feed time for reactor {self.channel} was at {self.last_recovery_time}. {remaining_time:.2f} more hours before a transition to 'Starved' state will be enabled.")
        
        try:
            remaining_time = 1.5 - ((now - self.last_recovery_time).total_seconds() / 3600)
            print(f"Previous feed time for reactor {self.channel} was at {self.last_recovery_time}. {remaining_time:.2f} more hours before a transition to 'Starved' state will be enabled.")
        except TypeError:
            # This will catch if self.last_recovery_time is None or not a datetime object
            print(f"No recovery time logged for reactor {self.channel}.")


        print(' finding state-------')
        print('state is', self.state)

        if self.state == "Startup":
            if self.current > self.threshold:
                self.state = "Fed"
            else:
                self.state = "Starved"
        elif self.state == "Starved":
            self.feed_reactor()
            self.state = "Recovery"
            self.last_recovery_time = now
            self.save_reactor_config()

        elif self.state == "Recovery":
            if self.current > self.threshold:
                self.state = "Fed"
            else:
                self.state = "Recovery"
                # if self.last_recovery_time:
                #     remaining_time = 1.5 - ((now - self.last_recovery_time).total_seconds() / 3600)
                #     print(f"Current below threshold, but must wait {remaining_time:.2f} more hours before transitioning to 'Starved' state.")
        elif self.state == "Fed":
            # print('recovery time is', remaining_time)
            if self.current < self.threshold:
               
                if self.last_recovery_time is None or ((now - self.last_recovery_time).total_seconds() / 3600 > 1.5):
                    self.state = "Starved"
                else:
                    print(f"Current below threshold, but must wait {remaining_time:.2f} more hours before transitioning to 'Starved' state.")

    def __str__(self):
        return f"Reactor {self.channel}: Current = {self.current}, State = {self.state}"

    def feed_reactor(self):
        json_file_path = 'feed_times.json'
        try:
            with open(json_file_path, 'r') as file:
                data_packet = json.load(file)
        except (FileNotFoundError, json.JSONDecodeError):
            data_packet = {'dev': 'PC', 'p1': '0', 'p2': '0', 'p3': '0', 'p4': '0'}

        # Directly assign without strftime if feed_times already contains string representations
        for phase, time in self.feed_times.items():
            data_packet[phase] = time  # Assuming time is already a string

        with open(json_file_path, 'w') as file:
            json.dump(data_packet, file)

    def save_reactor_config(self):
        config = {
            'last_recovery_time': self.last_recovery_time.strftime("%Y-%m-%d %H:%M:%S") if self.last_recovery_time else None,
            'feed_times': self.feed_times,
            'state': self.state,
        }
        with open(self.config_file_path, 'w') as file:
            json.dump(config, file)

    def load_reactor_config(self):
        if os.path.exists(self.config_file_path):
            with open(self.config_file_path, 'r') as file:
                config = json.load(file)
                self.last_recovery_time = datetime.datetime.strptime(config.get('last_recovery_time'), "%Y-%m-%d %H:%M:%S") if config.get('last_recovery_time') else None
                self.feed_times = config.get('feed_times', self.feed_times)
                self.state = config.get('state', self.state)









# class Reactor:
#     def __init__(self, channel, threshold, feed_times):
#         self.channel = channel
#         self.current = 0
#         self.threshold = threshold
#         self.state = "Startup"
#         self.feed_times = feed_times
#         self.config_file_path = f'{channel}_config.json'  # Unique config file for each reactor
#         self.load_reactor_config()  # Load reactor configuration including last recovery time

#     def update_current(self, current):
#         """Updates the current value and checks if a state transition is needed."""
#         self.current = current
#         self.update_state()

#     def update_state(self):
#         """Updates the reactor state based on the current value and time since the last recovery."""
#         if self.state == "Startup":
#             # Apply similar logic for Startup state as for Fed state
#             if self.current > self.threshold:
#                 self.state = "Fed"
#             else:
#                 self.state = "Starved"



#         elif self.state == "Starved":
#             self.feed_reactor()
#             self.state = "Recovery"
#             self.last_recovery_time = time.time()  # Record time at state change to Recovery
#             self.save_reactor_config()  # Save configuration, including the last recovery time

#         elif self.state == "Recovery":
#             if self.current > self.threshold:
#                 self.state = "Fed"
#             else:
#                 self.state = "Recovery"
#                 remaining_time = 1.5 - ((time.time() - self.last_recovery_time) / 3600)
#                 print(f"Current below threshold, but must wait {remaining_time:.2f} more hours before transitioning to 'Starved' state.")

#         elif self.state == "Fed":
#             if self.current < self.threshold:
#                 if self.last_recovery_time is None or (time.time() - self.last_recovery_time) / 3600 > 1.5:
#                     self.state = "Starved"
#                 else:
#                     remaining_time = 1.5 - ((time.time() - self.last_recovery_time) / 3600)
#                     print(f"Current below threshold, but must wait {remaining_time:.2f} more hours before transitioning to 'Starved' state.")
#             else:
#                 self.state = "Fed"

#     def __str__(self):
#         """Provides a string representation of the reactor's current state."""
#         return f"Reactor {self.channel}: Current = {self.current}, State = {self.state}"
    
#     def feed_reactor(self):
#         """Handles feeding the reactor and updating feed times in a JSON file."""
#         json_file_path = 'feed_times.json'

#         try:
#             with open(json_file_path, 'r') as file:
#                 data_packet = json.load(file)
#         except (FileNotFoundError, json.JSONDecodeError):
#             data_packet = {'dev': 'PC', 'p1': '0', 'p2': '0', 'p3': '0', 'p4': '0'}

#         # Update feed times based on current reactor configuration
#         data_packet['p1'] = self.feed_times.get('p1', data_packet.get('p1', '0'))
#         data_packet['p2'] = self.feed_times.get('p2', data_packet.get('p2', '0'))
#         data_packet['p3'] = self.feed_times.get('p3', data_packet.get('p3', '0'))
#         data_packet['p4'] = self.feed_times.get('p4', data_packet.get('p4', '0'))

#         with open(json_file_path, 'w') as file:
#             json.dump(data_packet, file)


#     def save_reactor_config(self):
#         """Saves reactor-specific configuration, including the last recovery time."""
#         config = {
#             'last_recovery_time': self.last_recovery_time,
#             'feed_times': self.feed_times,
#             'state': self.state,
#         }
#         with open(self.config_file_path, 'w') as file:
#             json.dump(config, file)

#     def load_reactor_config(self):
#         """Loads reactor-specific configuration, including the last recovery time."""
#         if os.path.exists(self.config_file_path):
#             with open(self.config_file_path, 'r') as file:
#                 config = json.load(file)
#                 self.last_recovery_time = config.get('last_recovery_time')
#                 self.feed_times = config.get('feed_times', self.feed_times)
#                 self.state = config.get('state', self.state)
#         else:
#             self.last_recovery_time = None




class ReactorController:
    def __init__(self):
        self.reactors = {}

    def add_reactor(self, reactor):
        self.reactors[reactor.channel] = reactor

    def update_reactors(self, current_draws):
        for channel, current in current_draws.items():
            if channel in self.reactors:
                self.reactors[channel].update_current(current)

    def print_reactor_states(self):
        for reactor in self.reactors.values():
            print(reactor)
