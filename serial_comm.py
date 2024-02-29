import serial
import matplotlib.pyplot as plt

port = '/dev/ttyUSB0'  # Use the correct port for your setup
baud_rate = 230400  # Set the baud rate to match your microcontroller's configuration

# Initialize lists to store x, y1 and y2 coordinates
x_coords = []
y1_coords = []
y2_coords = []

try:
    # Open the serial port with the specified baud rate
    with serial.Serial(port, baud_rate, timeout=1) as mySerial:
        print(f"Opened port {port} for reading...")
        
        # Continuously read data from the serial port
        while len(x_coords) < 200:
            if mySerial.in_waiting > 0:
                data_read = mySerial.readline().decode('utf-8').rstrip()
                print(f"Data read: {data_read}")

                # Parse the incoming data as coordinates
                x, y1, y2 = map(float, data_read.split())  # Assumes data is in "x y1 y2" format

                # Add the new coordinates to the lists
                x_coords.append(x)
                y1_coords.append(y1)
                y2_coords.append(y2)

    # Plot the data
                
    print(x_coords)
    print(y1_coords)
    print(y2_coords)

    plt.figure()
    plt.plot(x_coords, y1_coords, label='ADC Value')
    plt.plot(x_coords, y2_coords, label='Reference')
    plt.xlabel('Sample Number (at 100 Hz)')
    plt.ylabel('Brightness (ADC counts)')
    plt.title('Brightness vs. Sample Number')
    plt.legend()
    plt.show()

except serial.SerialException as e:
    print(f"Error opening serial port: {e}")

except KeyboardInterrupt:
    print("Program terminated by user")