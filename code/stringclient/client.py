import socket
import threading
import sys

running = 1

def sender_interface(client_socket):
    global running
    message = input()  # take input

    while running:
        if message.lower().strip() == 'bye':
            running = 0
        client_socket.send(message.encode())  # send message
        message = input()  # again take input

def receiver_interface(client_socket):
    global running
    while running:
        data = client_socket.recv(1024).decode()  # receive response

        print('Received from server: ' + data)  # show in terminal


def client_program(host):
    port = 8000  # socket server port number

    client_socket = socket.socket()  # instantiate
    client_socket.connect((host, port))  # connect to the server

    sender_thread = threading.Thread(target=sender_interface, args=[client_socket])
    receiver_thread = threading.Thread(target=receiver_interface, args=[client_socket])

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
    
    client_program(host)