import socket

bd_addr = "00:23:11:01:26:C5"
port = 1

try:
    sock = socket.socket(socket.AF_BLUETOOTH, socket.SOCK_STREAM, socket.BTPROTO_RFCOMM)
    print(f"Connecting to {bd_addr}...")
    sock.connect((bd_addr, port))
    print("Connected! Waiting for data...")

    buffer = "" # Create an empty storage area

    while True:
        try:
            # Receive data and decode it immediately
            chunk = sock.recv(1024).decode('utf-8')
            
            if not chunk:
                break
                
            # Add the new chunk to our storage buffer
            buffer += chunk
            
            # While there is a "New Line" in our buffer, process it
            while '\n' in buffer:
                # Split the buffer: [Complete Message] \n [Leftover for next time]
                line, buffer = buffer.split('\n', 1)
                
                # Print the clean line (strip removes extra spaces/symbols)
                print("Received:", line.strip())
                
        except UnicodeDecodeError:
            # Sometimes a chunk cuts a character in half, ignore that error briefly
            continue

except KeyboardInterrupt:
    print("\nUser stopped script.")
except Exception as e:
    print(f"Error: {e}")
finally:
    sock.close()
    print("Connection Closed")
