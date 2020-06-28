# -*- coding: utf-8 -*-
import socket as s
print('Student ID : 20175163')
print('Name : jeonghwan Park')
while(True):
    prompt = input("\n> ")
    if prompt == 'quit':
        exit()
    argv = prompt.split(" ")
    argc = len(argv)
    # dl hostname portnum filename
    if(argv[0] != 'dl'):
        print("%s command not found"%argv[0])
        continue

    if(argc != 4):
        print("usage : dl host port path")
        continue

    HOST = argv[1]
    PORT = int(argv[2])
    PATH = argv[3]
    file_name = PATH.split('/')[-1]
    client_socket = s.socket(s.AF_INET, s.SOCK_STREAM)
    try:
        client_socket.connect((HOST,PORT))

    except s.error:
        print("{host}: unknown host".format(host=HOST))
        print("cannot connect to server {host} {port}".format(host=HOST,port=PORT))
        client_socket.close()
        continue

    BUF_SIZE = 1024

    request_msg = '''GET {path} HTTP/1.1\r\nHost: {host}\r\nUser-agent: HW1/1.0\r\nID: 20175163\r\nName: Jeonghwan Park\r\nConnection: close\r\n\r\n'''.format(path=PATH,host=HOST)
    try:
        client_socket.send(request_msg.encode())

    except s.error:
        print("{host}: unkonw host".format(host=HOST))
        print("cannot connect to server {host} {port}".format(host=HOST,port=PORT))
        client_socket.close()
        continue

    last = 0
    curr = 0
    total = 1
    status_line = []
    header_line = {}
    response_msg = client_socket.recv(BUF_SIZE)    
    buff = response_msg.split(b'\r\n\r\n')
    if len(buff) != 1:  # 초기한번만 실행됨
        lines = buff[0].split(b'\r\n')
        status_line = lines[0].split()
        for j in lines[1:]:
            j = j.split(b': ')
            header_line[j[0]] = j[1]
    if status_line[1] != b'200':
        print(b' '.join(status_line[1:]).decode())
        continue
    if b'Content-Length' in header_line:
        total = int(header_line[b'Content-Length'].decode())
        print("Total Size %s bytes"%total)
    with open(file_name, 'wb') as f:
        while response_msg:
            f.write(buff[-1])
            curr += len(buff[-1])
            rate = int(curr/total*100)
            if rate >= last + 10:
                print("Current Downloading {current}/{total} (bytes) {rate}%".format(current=curr,total=total,rate=rate))
                last = rate//10*10
            if rate == 100:
                print("Download Complete: {file}, {current}/{total}".format(file=file_name,current=curr,total=total))
            
            response_msg = client_socket.recv(BUF_SIZE)
            buff = response_msg.split(b'\r\n\r\n')
        client_socket.close()
        f.close()
        