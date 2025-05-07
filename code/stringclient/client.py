import socket
import threading
import sys
import signal
import select

running = 1

#ctrl+c handling
def stop_program(signum, frame):
    global running
    running = 0
    #client_socket.shutdown(socket.SHUT_WR)
    print("detected ctrl+c, stopping program...")

def sender_interface(client_socket):
    global running
    try:
        message = input()  # again take input
    except (KeyboardInterrupt, EOFError):
        stop_program(0,0)

    while running:
        if message.lower().strip() == 'bye':
            running = 0
        client_socket.send(message.encode())  # send message
        try:
            message = input()  # again take input
        except (KeyboardInterrupt, EOFError):
            stop_program(0,0)
    print("ended sender thread!")

def receiver_interface(client_socket):
    global running
    client_socket.settimeout(1)
    while running:
        try:
            data = client_socket.recv(1024)  # receive response
            data = data.decode()
            print('Received from server: ' + data)  # show in terminal
        except socket.timeout:
            continue
    print("ended receiver thread!")


def client_program(host):
    port = 8000  # socket server port number

    client_socket = socket.socket()  # instantiate
    client_socket.connect((host, port))  # connect to the server

    sender_thread = threading.Thread(target=sender_interface, args=[client_socket])
    receiver_thread = threading.Thread(target=receiver_interface, args=[client_socket])
    receiver_thread.daemon=True

    for thethread in [sender_thread, receiver_thread]:
        thethread.start()

    for thethread in [receiver_thread, sender_thread]:
        thethread.join()

    client_socket.close()  # close the connection


if __name__ == '__main__':
    if len(sys.argv) == 2:
        host = sys.argv[1]
    else:
        host = "ENTER.DEFAULT.HOST_IP.HERE"

    #ctrl+c handling    
    signal.signal(signal.SIGINT, stop_program)
    
    print(f"connecting to host {host}")
    client_program(host)