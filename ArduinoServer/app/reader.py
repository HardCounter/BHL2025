import socket
import json
from .database import SessionLocal
from .models import Reading


def start_reading(bd_addr="00:23:11:01:26:C5", port=1):
    db = SessionLocal()
    try:
        sock = socket.socket(socket.AF_BLUETOOTH, socket.SOCK_STREAM, socket.BTPROTO_RFCOMM)
        print(f"Connecting to {bd_addr}...")
        sock.connect((bd_addr, port))
        print("Connected! Waiting for data...")

        buffer = ""
        json_buffer = ""
        in_message = False

        while True:
            try:
                # Receive data and decode it immediately
                chunk = sock.recv(1024).decode('utf-8')

                if not chunk:
                    break

                buffer += chunk

                while '\n' in buffer:
                    line, buffer = buffer.split('\n', 1)
                    line = line.strip()
                    print("Received:", line)

                    if line == '[':
                        in_message = True
                        json_buffer = ""
                    elif line == ']':
                        in_message = False
                        if json_buffer:
                            # remove trailing comma
                            if json_buffer.endswith(','):
                                json_buffer = json_buffer[:-1]
                            
                            full_json = "{" + json_buffer + "}"
                            
                            try:
                                data = json.loads(full_json)
                                db_reading = Reading(**data)
                                db.add(db_reading)
                                db.commit()
                                db.refresh(db_reading)
                                print("Saved to database:", data)
                            except json.JSONDecodeError as e:
                                print(f"Error decoding JSON: {e}")
                                print(f"Invalid JSON string: {full_json}")
                        json_buffer = ""
                    elif in_message:
                        json_buffer += line
                        
            except UnicodeDecodeError:
                continue

    except KeyboardInterrupt:
        print("\nUser stopped script.")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        sock.close()
        db.close()
        print("Connection Closed")

if __name__ == '__main__':
    start_reading()
