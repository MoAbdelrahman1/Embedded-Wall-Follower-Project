import socket
import json
import datetime

# --- Configuration ---
HOST = '0.0.0.0'  # Listen on all available network interfaces
PORT = 5000       # Must match SERVER_PORT in config.h

def start_server():
    """
    Starts a TCP server to receive distance data from the STM32 Robot.
    """
    print("========================================")
    print(f"    STM32 Wall Follower TCP Receiver")
    print("========================================")
    print(f"  STEP 1: Connect your laptop to WiFi:")
    print(f"          Name: STM32_Wall_Follower")
    print(f"          Pass: 12345678")
    print(f"  STEP 2: Ensure your Laptop IP is 192.168.4.2")
    print(f"  STEP 3: Running server on port {PORT}...")
    print("========================================\n")
    
    # Create a TCP/IP socket
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        # Allow immediate restart of the script
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        
        try:
            s.bind((HOST, PORT))
            s.listen()
            print(f"[*] Server is up! Waiting for Robot to connect...")
            
            while True:
                conn, addr = s.accept()
                with conn:
                    print(f"\n[+] ROBOT CONNECTED from {addr}")
                    buffer = ""
                    
                    while True:
                        try:
                            data = conn.recv(1024)
                            if not data:
                                print(f"[-] Robot {addr} disconnected.")
                                break
                            
                            # Decode data (expecting JSON)
                            buffer += data.decode('utf-8')
                            
                            # Process complete lines
                            while "\n" in buffer:
                                line, buffer = buffer.split("\n", 1)
                                line = line.strip()
                                if not line: continue
                                    
                                timestamp = datetime.datetime.now().strftime("%H:%M:%S")
                                
                                try:
                                    # Try to parse the distance JSON
                                    payload = json.loads(line)
                                    dist = payload.get('distance', 'N/A')
                                    print(f"[{timestamp}] Distance: {dist:5} cm")
                                except json.JSONDecodeError:
                                    # Fallback for raw text
                                    print(f"[{timestamp}] Raw Data: {line}")
                                    
                        except ConnectionResetError:
                            print(f"[!] Connection reset by Robot.")
                            break
                        except Exception as e:
                            print(f"[!] Receive Error: {e}")
                            break
                            
        except KeyboardInterrupt:
            print("\n[*] Server stopping...")
        except Exception as e:
            print(f"[!] Bind Error: {e}. Is port {PORT} already in use?")

if __name__ == "__main__":
    start_server()