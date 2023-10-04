## How to build
### Linux
#### Debug and Tests
```
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug -Dlibsercli_BUILD_CTESTS=ON -Dlibsercli_BUILD_UTESTS=ON ..
make
```
### Windows
```
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -Dlibsercli_BUILD_CTESTS=ON ..
cmake --build . --config Release
```

## Tests
### Component tests
#### Handshake test
```
./HandshakeTest ./sock
Hello World from HandshakeTest!
Client with ID: 7 connected
Client received data: Hello Client!
Client with ID: 7 sent data: Hello Server!
Successfull handshaking!
Client with ID: 7 diconnected
```
Inet connection
```
./HandshakeTest "127.0.0.1" 12345
Hello World from HandshakeTest!
Client with ID: 7 connected
Client received data: Hello Client!
Client with ID: 7 sent data: Hello Server!
Successfull handshaking!
Client with ID: 7 diconnected
```

#### Interactive test
UNIX socket connection
```
./ServerTest ./sock
Hello World from SocketServerTest!
Client with ID: 5 connected
Total clients: 1
Client with ID: 5 sent data: hi
Client with ID: 5 sent data: ping
Client with ID: 5 sent data: ping
Client with ID: 5 sent data: ping
Client with ID: 5 sent data: ping
Client with ID: 5 sent data: ping
Client with ID: 5 diconnected
Total clients: 0
q:
```
```
./ClientTest ./sock
Hello World from SocketClientTest!
s:hi
Sending...
Sending Done!
s,n5,d1000:ping
Sending...
Sending Done!
q:
```
Inet connection
```
./ServerTest "127.0.0.1" 12345
Hello World from SocketServerTest!
Client with ID: 5 connected
Total clients: 1
Client with ID: 5 sent data: hi
Client with ID: 5 sent data: ping
Client with ID: 5 sent data: ping
Client with ID: 5 sent data: ping
Client with ID: 5 sent data: ping
Client with ID: 5 sent data: ping
s,n3,d100:ping
Sending...
Sending Done!
q:
```
```
./ClientTest "127.0.0.1" 12345
Hello World from SocketClientTest!
s:hi
Sending...
Sending Done!
s,n5,d1000:ping
Sending...
Sending Done!
Server sent data: ping
Server sent data: ping
Server sent data: ping
Server disconnected
q
```

## Troubleshooting
### Helpful tools
#### netstat
Linux:
```
netstat -tuln | grep 12345
tcp        0      0 127.0.0.1:12345         0.0.0.0:*               LISTEN
```
```
netstat -lp | grep handshake
unix  2      [ ACC ]     STREAM     LISTENING     3569003  1064897/HandshakeTe  ./handshake_sock
```
Windows:
```
netstat -an | find "12345"
TCP    127.0.0.1:12345        0.0.0.0:0              LISTENING
```
