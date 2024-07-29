from flask import Flask, request
import csv
import time
import json
import os

app = Flask(__name__)

json_file_path = 'feed_times.json'

# Function to read data from the JSON file
def read_json_file(file_path):
    if os.path.exists(file_path):
        with open(file_path, 'r') as file:
            return json.load(file)
    else:
        print('Could not find .json file')
        return None

# Function to write data to a CSV file
def write_to_csv(data):
    try:
        with open('data_log.csv', 'a', newline='') as file:
            writer = csv.writer(file)
            writer.writerow([time.strftime("%Y-%m-%d %H:%M:%S")] + [json.dumps(data)])
            # print("Data written to CSV:", json.dumps(data))
    except Exception as e:
        print("Error writing to CSV:", e)

# Route to provide data to ESP32
@app.route('/getdata', methods=['GET'])
def provide_data():
    data_packet = read_json_file(json_file_path)
    print('data packed is:', data_packet)
    if data_packet:
        # Filter out key-value pairs where the value is not numeric or is 0 or less
        filtered_data_packet = {key: value for key, value in data_packet.items() 
                                if value.isnumeric() and int(value) > 0}

        print('Sending data packet:', filtered_data_packet)
        write_to_csv(filtered_data_packet)  # Assuming the definition of this function exists
        return filtered_data_packet
    else:
        print("No data packet available.")
        return "No data packet available."




@app.route('/data', methods=['POST'])
def receive_data():
    global expected_response_received
    data = request.form.to_dict()
    if data:
        print("Data received from ESP32:", data)

        # Check if the 'response' field exists and is not empty
        if 'response' in data and data['response']:
            try:
                response_data = json.loads(data['response'])
                # Check if the received data matches the expected response criteria
                if response_data.get('dev') == 'ESP' and any(int(v) > 0 for k, v in response_data.items() if k.startswith('remainingTime')):
                    expected_response_received = True
                    # Update feed_times.json
                    update_feed_times(response_data)
                if response_data.get('dev') == 'ESP_relay':
                    expected_response_received = True
                    # Print the JSON contents
                    print("Received JSON contents:", response_data)
                
           
            except json.JSONDecodeError as e:
                print("Error decoding JSON response:", e)
        write_to_csv(data)  # Assuming the definition of this function exists
        return "Data received and logged"
    else:
        return "No data received"

def update_feed_times(response_data):
    try:
        with open('feed_times.json', 'r') as file:
            feed_times = json.load(file)

        # Define a mapping from response_data keys to feed_times keys
        key_mapping = {
            'remainingTimeA': 'p1',
            'remainingTimeB': 'p2',
            'remainingTimeC': 'p3',
            'remainingTimeD': 'p4'
        }

        # Iterate through the keys in the response_data
        for key in response_data.keys():
            if key.startswith('remainingTime') and int(response_data[key]) > 0:
                feed_key = key_mapping[key]
                if feed_key in feed_times:
                    feed_times[feed_key] = "0"

        # Write the updated feed_times back to the file
        with open('feed_times.json', 'w') as file:
            json.dump(feed_times, file)

    except Exception as e:
        print("Error updating feed times:", e)



@app.route('/')
def index():
    return "Hello from the Flask Server!"

if __name__ == '__main__':
    app.run(host='0.0.0.0', debug=True, port=5000)
