# Fingerprint Access Control System

## Description

This project represents a fingerprint-based access control system. It uses fingerprint sensors, GPIO for managing buttons and LEDs, and interacts with a database and external services via cURL.

## Requirements

Before starting, ensure that you have the following libraries installed:

- `libgpiod-dev`
- `sqlite3`
- `libcurl`
- `cjson`
To install dependencies on Debian/Ubuntu:

```bash
sudo apt-get install -y libgpiod-dev sqlite3 libcurl4-openssl-dev libcjson-dev
```
## Project Compilation

The project uses a Makefile for compilation. It supports two build modes:

- **Debug**: Includes debugging information and logs output using printf.
- **Release**: Logs messages using syslog with LOG_ERR level.

**Compiling in Debug Mode**

```bash
make debug
```
**Compiling in Release Mode**

```bash
make release
```

**Cleaning the Build**

To clean up all built files:
```bash
make clean
```
**Running the Project**

After compilation, the executable will be located in *./build/out/.* You can run it with:

```bash
./build/out/fingerprint
```

## Features

- **Fingerprint scanning** for entry and exit.
- **New employee registration** with unique identifiers.
- **Database update** with access and employee registration information.
- **LCD display interface** for showing information.
- **Data submission** about new employees to CRM via HTTP requests.

## Project Structure

### Directory Organization:

- `./Inc/`: Header files containing function declarations.
- `./Src/`: Source code files implementing the system functionality.
- `config.conf`: Configuration file for system parameters.

### Header Files (`./Inc/`)

- `GPIO.h`: Functions for GPIO operations.
- `UART.h`: Functions for UART operations.
- `I2C.h`: Functions for I2C operations.
- `DataBase.h`: Functions for database operations.
- `lcd20x4_i2c.h`: Functions for controlling LCD display via I2C.
- `threads.h`: Functions for thread operations.
- `config.h`: Configuration parameters.
- `syslog_util.h`: Functions for syslog operations.
- `defines.h`: Definitions and constants.
- `curl_client.h`: Functions for managing cURL operations.
- `file_utils.h`: Utility functions for file operations.
- `FP_delete.h`: Functions for deleting fingerprints.
- `FP_enrolling.h`: Functions for enrolling new fingerprints.
- `FP_find_finger.h`: Functions for finding and verifying fingerprints.
- `keypad.h`: Functions for handling keypad input.
- `packet.h`: Functions for managing network packets.
- `signal_handlers.h`: Functions for handling signals.
  
### Source Files (`./Src/`)

- `main.c`: Main program file, initializes system, creates threads, and handles button events.
- `GPIO.c`: Implementation of GPIO functions.
- `UART.c`: Implementation of UART functions.
- `I2C.c`: Implementation of I2C functions.
- `DataBase.c`: Implementation of database functions.
- `lcd20x4_i2c.c`: Implementation of LCD display functions.
- `threads.c`: Implementation of thread functions.
- `config.c`: Implementation of configuration reading and handling.
- `syslog_util.c`: Implementation of syslog utility functions.
- `utils.c`: Utility functions used across the project.
- `curl_client.c`: Implementation of cURL client functions.
- `file_utils.c`: Implementation of file utility functions.
- `FP_delete.c`: Implementation of fingerprint deletion functions.
- `FP_enrolling.c`: Implementation of fingerprint enrollment functions.
- `FP_find_finger.c`: Implementation of fingerprint searching functions.
- `keypad.c`: Implementation of keypad handling functions.
- `packet.c`: Implementation of network packet management functions.
- `signal_handlers.c`: Implementation of signal handling functions.

## Configuration

The system configuration is managed using the `config.conf` file. This file allows you to specify various parameters required for the system to function correctly. 

## Usage

### Buttons and Their Functions

The system reacts to button presses as follows:
- **IN Button**: Records entry into the system.
- **OUT Button**: Records exit from the system.
- **NEW Button**: Registers a new employee.

### System Workflow

When pressing the buttons, the system will:

1. **Scan the Fingerprint**:
   - The system scans the fingerprint using the fingerprint sensor.

2. **Process the Result and Update the Database**:
   - For recognized fingerprints, the system updates the database with the action (entry or exit).
   - If registering a new employee, it adds their details to the database.

### How to Work with the SQLite Database

1. Start the SQLite CLI:

   ```sql
   sqlite3 employee_attendance.db
   ```

2. View the Tables:

   ```sql
   .tables
   ```

3. Query the Data:

   ```sql
   SELECT * FROM attendance;
   ```

4. Exit the SQLite CLI:

   ```sql
   .exit
   ```

3. **Display the Appropriate Message on the LCD**:
   - Success or failure messages and other relevant information are displayed on the LCD.

4. **Interact with External Services**:
   - For new employee registrations, the system sends data to an external CRM or server.
   - It handles retries and logs any errors encountered during this process.

5. **Error Indication**:
   - If there is a connection error (e.g., during new employee registration), the red LED will turn on to indicate a failure.

### Setting Up as a Daemon

To run the project as a background service (daemon) in Linux, follow the steps below:

- **Copy the executable** to */bin* so it can be executed from anywhere:
```bash
cp fingerprint /bin
```
- **Create a systemd service file**:
```bash
sudo nano /etc/systemd/system/fingerprint.service
```
- **Edit the service file** to look like this:
```ini
[Unit]
Description=Fingerprint Management System
After=network.target

[Service]
ExecStart=/bin/fingerprint
Restart=always
User=root
Group=root

[Install]
WantedBy=multi-user.target
```
- **Reload systemd** to apply changes:
```bash
sudo systemctl daemon-reload
```
- **Start the service**:
```bash
sudo systemctl start fingerprint
```
- **Enable the service** to start automatically at boot:
```bash 
sudo systemctl enable fingerprint
```
- **Check the status** of the service:
```bash
sudo systemctl status fingerprint
```
### Managing the Daemon

- **Stop the service**:
```bash
sudo systemctl stop fingerprint
```
- **Reload systemd** after making changes:
```bash
sudo systemctl daemon-reload
```
- **Reboot the system** to test the service startup:
```bash
sudo reboot
```

### Debugging and Logging

For debugging and monitoring the system status:

- **Syslog**: All errors and important events are logged using syslog. These logs are available in the system journal and can be used for debugging and monitoring.
- **Error Indications**: Visual feedback (e.g., red LED) is provided to indicate connection errors or other issues to assist in troubleshooting.
   
   to view log entries   
   ```bash
    journalctl _COMM=fingerprint
   ``` 
   to clear the log 
   ```bash      
   sudo journalctl --rotate
   sudo journalctl --vacuum-time=1s
   ```

## Contact
If you have any questions, feel free to reach out via email: yaelbriskin@gmail.com
