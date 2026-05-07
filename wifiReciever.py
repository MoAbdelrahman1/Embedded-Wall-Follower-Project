import socket
import json
import datetime

# --- Configuration ---
# Make sure this matches your STM32's config.h
HOST = '0.0.0.0'  # Listen on all network interfaces
PORT = 5000       # Must match SERVER_PORT

def start_server():
    """
    Starts a TCP server to receive data from the ESP-01S module.
    Expects JSON data in the format: {"distance": value}
    """
    print("========================================")
    print(f"    STM32 Wall Follower TCP Receiver")
    print(f"    Listening on port: {PORT}")
    print("========================================")
    
    # Create a TCP/IP socket
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        # Set socket options to allow immediate restart
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        
        try:
            s.bind((HOST, PORT))
            s.listen()
            print(f"[*] Server is up! Waiting for ESP-01S to connect...")
            
            while True:
                conn, addr = s.accept()
                with conn:
                    print(f"\n[+] Connection established from {addr}")
                    
                    # Buffer for incomplete messages
                    buffer = ""
                    
                    while True:
                        try:
                            data = conn.recv(1024)
                            if not data:
                                print(f"[-] Client {addr} disconnected.")
                                break
                            
                            # Decode and add to buffer
                            buffer += data.decode('utf-8')
                            
                            # Process all complete lines in buffer
                            while "\n" in buffer:
                                line, buffer = buffer.split("\n", 1)
                                line = line.strip()
                                
                                if not line:
                                    continue
                                    
                                timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                                
                                try:
                                    # Parse JSON payload
                                    payload = json.loads(line)
                                    distance = payload.get('distance', 'Unknown')
                                    print(f"[{timestamp}] Distance: {distance:5} cm")
                                except json.JSONDecodeError:
                                    # Handle non-JSON or partial data
                                    print(f"[{timestamp}] Raw Data: {line}")
                                    
                        except ConnectionResetError:
                            print(f"[!] Connection reset by {addr}")
                            break
                        except Exception as e:
                            print(f"[!] Error receiving data: {e}")
                            break
                            
        except KeyboardInterrupt:
            print("\n[*] Server stopping...")
        except Exception as e:
            print(f"[!] Bind Error: {e}. Is port {PORT} already in use?")

if __name__ == "__main__":
    start_server()
