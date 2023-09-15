# LeetsChat
A minimal implementation of socket-based chatroom(mostly built for learning how epoll and select work).

# Usage
+ Build this project
+ Server side: run `./server <Server port>` to start server on a specific port.
```
./server 12345
```
+ Client side: run `./client <Server IP> <Server port>` to connect to server`
```
./client 127.0.0.1 12345
```
+ Send messages in client, and everyone in chatroom should receive it.
