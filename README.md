# Networks-Laboratory-CS39006-2023

Computer Networks Laboratory (Spring 2023) course assignments and implementations.

## Course Information
- **Course Code**: CS39006
- **Semester**: Spring 2023
- **Department**: Computer Science and Engineering
- **Institute**: Indian Institute of Technology Kharagpur

## Table of Contents
- [Assignment 1: Time Synchronization](#assignment-1)
- [Assignment 2: Socket Programming](#assignment-2)
- [Assignment 3: Simple Chat Client-Server](#assignment-3)
- [Assignment 4: File Transfer Using Sockets](#assignment-4)
- [Assignment 5: Message Oriented TCP](#assignment-5)
- [Assignment 6: Raw Sockets](#assignment-6)

## Assignment Details

### Assignment 1
**Time Synchronization**
- Implementation of Berkeley Algorithm
- Clock synchronization between multiple systems
- Technologies: C, Socket Programming
- [View Code](./Assignment%201/)

### Assignment 2
**Socket Programming**

- Implementation of a simple client-server communication using TCP/IP sockets
- The client and server communicate by exchanging messages through a socket connection
- Technologies: C, Socket Programming
- [View Code](./Assignment%202/)

### Assignment 3
**Simple Chat Client-Server**
- Implementation of a simplified load balancer with two computation servers and multiple clients
- The load balancer periodically monitors server loads and allocates client requests to the server with the least load
- Servers generate a dummy load and respond to client service requests via the load balancer
- Technologies: C, Socket Programming
- [View Code](./Assignment%203/)

### Assignment 4
**File Transfer Using Sockets**
- Implementation of a reliable communication protocol (MRP) over an unreliable link using UDP sockets
- The protocol ensures message delivery but does not guarantee in-order or exactly-once delivery
- The implementation includes functions for creating, binding, sending, receiving, and closing MRP sockets
- The project involves creating a static library for MRP sockets and testing it with user programs
- Technologies: C, UDP Sockets, Multithreading
- [View Code](./Assignment%204/)

### Assignment 5
**Message Oriented TCP**
- Implementation of a message-oriented TCP protocol using standard TCP connections
- The protocol ensures that messages sent with a single send call are received by a single receive call, maintaining reliability and FIFO order
- The implementation includes functions for creating, binding, listening, accepting, connecting, sending, receiving, and closing MyTCP sockets
- The project involves creating a library for MyTCP sockets and testing it with user programs
- Technologies: C, TCP Sockets, Multithreading
- [View Code](./Assignment%205/)

### Assignment 6
**Raw Sockets**
- Implementation of a program `PingNetInfo` to find the route and estimate the latency and bandwidth of each link in the path to a given site
- The program sends ICMP packets to probe the network and measures the round-trip time (RTT) for different packet sizes
- The program discovers intermediate nodes similar to the traceroute tool and estimates link latency and bandwidth
- Handles various network conditions such as packet loss, out-of-order responses, and non-responsive nodes
- Technologies: C, Raw Sockets, ICMP, Network Programming
- [View Code](./Assignment%206/)

## Setup Instructions
```bash
# Clone repository
git clone https://github.com/hs094/Networks-Laboratory-CS39006-2023.git
cd Networks-Laboratory-CS39006-2023

# Each assignment has its own build instructions
# Generally for C programs:
gcc filename.c -o output
./output