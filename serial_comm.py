import serial
import matplotlib.pyplot as plt

port = '/dev/ttyUSB0'  # Use the correct port for your setup
baud_rate = 230400  # Set the baud rate to match your microcontroller's configuration

try:
    with serial.Serial(port, baud_rate, timeout=None) as mySerial:  # timeout=None makes read block until data is received
        print(f"Opened port {port} for reading...")

        # Wait for data to be received
        print("Waiting for data...")
        while mySerial.in_waiting == 0:
            pass  # Do nothing, just wait until data is available

        # Read one set of data from the serial port
        data_read = mySerial.readline().decode('utf-8').rstrip()
        print(f"Data read: {data_read}")

        # Parse the incoming data as coordinates
        x, y = map(int, data_read.split(','))  # Assumes data is in "x,y" format

        # Plot the data using matplotlib
        plt.plot(x, y, 'ro')  # Plot the point as a red circle
        plt.xlabel('X axis')
        plt.ylabel('Y axis')
        plt.title('Plotting a Single Data Point')
        plt.show()

except serial.SerialException as e:
    print(f"Error opening serial port: {e}")
except KeyboardInterrupt:
    print("Program terminated by user")
except ValueError:
    print("Received data in an unexpected format. Make sure the data is two integers separated by a comma.")
