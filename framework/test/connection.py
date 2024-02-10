import socket
from threading import Thread


def main():
    print('Starting test')
    print('Initializing threads...')
    threads = []
    for i in range (100):
        thread = Thread(target=client_cycle)
        thread.start();
        threads.append(thread)
    print('Test started')
    threads[0].join()
    print('Test powering down!')

def client_cycle():
    while 1==1:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect(('127.0.0.1', 12555));
            print('Connected')
            s.sendall(b'getval|mod|0')
            data = ''
            while not r'\\\\_!end_!\\\\' in data:
                data += str(s.recv(4096))
            print(data)

if __name__=='__main__':
    main()
