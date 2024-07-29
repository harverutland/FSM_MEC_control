import ftplib
import pandas as pd
import io
import time

class CurrentMonitor:
    def __init__(self, ftp_address, username, password, directory):
        self.ftp_address = ftp_address
        self.username = username
        self.password = password
        self.directory = directory

    def start_monitoring(self):
        while True:
            try:
                with ftplib.FTP(self.ftp_address) as ftp:
                    ftp.login(self.username, self.password)
                    ftp.cwd(self.directory)
                    ftp.set_pasv(True)
                    print("Directory listing:")
                    ftp.dir()

                    filenames = ftp.nlst()

                    csv_file = self.find_csv(filenames)
                    if csv_file:
                        last_row = self.retrieve_csv_data(ftp, csv_file)
                        current_draws = self.process_current_draw(last_row)
                        return current_draws
                    else:
                        print("No CSV file found in the directory.")
                        return None
            except ftplib.all_errors as e:
                print(f"FTP error: {e}")
                return None
            time.sleep(60)

    def find_csv(self, filenames):
        return next((filename for filename in filenames if filename.lower().endswith('.csv')), None)


    def retrieve_csv_data(self, ftp, csv_file):
        with io.BytesIO() as file_data:
            ftp.retrbinary(f'RETR {csv_file}', file_data.write)
            file_data.seek(0)
            df = pd.read_csv(file_data, skiprows=15)
            return df.iloc[-1]

    def process_current_draw(self, last_row):
        current_keys = [key for key in last_row.keys() if key.startswith('I')]
        return {key: last_row[key] for key in current_keys}
