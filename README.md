## Table of Contents
+ [IPK2024-Projekt1](#ipk2024-projekt1)
+ [Introduction](#introduction)
  - [Protocol](#protocol)
    - [TCP (Transmission Control Protocol)](#tcp)
    - [UDP (User Datagram Protocol)](#udp)
+ [Implementation](#implementation)
  - [Modules](#modules)
    - [Data Structures](#data-structures)
    - [Main Program](#main-program)
    - [Client Program](#client-program)
+ [Decision Making](#decision-making)
  - [Returning Error Code on ERR Sent by Server](#returning-error-code-on-err-sent-by-server)
  - [Validate User Input](#validate-user-input)
+ [Testing](#testing)
  - [Unit Tests](#unit-tests)
  - [Dynamic Testing](#dynamic-testing)
    - [Scenarios](#scenarios)
+ [Note](#note)
+ [Bibliography](#bibliography)

## IPK2024-Projekt1 <a id="ipk2024-projekt1"></a>

This project implements a chat client for the `IPK24-CHAT` protocol, a protocol on application layer which relies on UDP/TCP protocols. The protocol's specification can be found [here](https://git.fit.vutbr.cz/NESFIT/IPK-Projects-2024/src/branch/master/Project%201). 

- **Language**: C
- **Author**: Le Duy Nguyen (xnguye27)
- **OS**: Linux
- **GIT**: https://git.fit.vutbr.cz/xnguye27/IPK2024-Projekt1

## Introduction <a id="introduction"></a>

A networking protocol is a set of rules and conventions that govern communication between devices within a network. It defines how data is transmitted, received, and processed, ensuring seamless interaction between different systems. This project will utilize two of the most famous protocols currently operating on the internet: UDP and TCP.

#### TCP (Transmission Control Protocol) <a id="tcp"></a>
TCP is a protocol that operates on the transport layer of the OSI model. It provides reliable, orderly, and error-checked delivery of data between applications. TCP ensures that data packets are delivered in the correct order and without errors by establishing a connection between two parties before data is transferred.

Key features of TCP include:

- **Reliability**: TCP ensures that data packets are delivered in the correct order and without errors.
- **Connection-oriented**: TCP establishes a connection between the sender and receiver before data transfer begins to ensure that both parties are ready to send and receive data.
- **Flow control**: TCP implements flow control mechanisms to prevent data overflow and ensure efficient transmission.

#### UDP (User Datagram Protocol) <a id="udp"></a>
UDP is a connectionless protocol that operates on the transport layer of the OSI model. It provides a way for applications to send datagrams to one another. UDP is known for its simplicity and low overhead, making it suitable for applications where speed and efficiency are critical. However, UDP does not guarantee delivery or order of packets, and it does not provide error checking or flow control mechanisms.

Key features of UDP include:

- **Connectionless**: UDP does not require a connection to be established before data transfer.
- **Unreliable delivery**: UDP does not handle packet loss or data correction.
- **Low overhead**: UDP has minimal overhead compared to TCP, making it more efficient for applications that require fast data transmission and can tolerate some packet loss.

## Implementation <a id="implementation"></a>
The implementation can be located in the `src/` directory, and it can be compiled using `make`.

The program is organized into modules, each responsible for handling specific tasks, which enhances the program's structure and facilitates testing.

### Modules <a id="modules"></a>
Each module comprises a header file (.h) and an implementation file (.c).

- **bytes**: Manages a byte slice, enabling operations like trimming, appending, and skipping bytes.
    - The program utilizes a byte statically allocated with a maximum capacity of 1500 characters. This limitation is intentional, aligning with the protocol's design to prevent data fragmentation during transport, as the protocol aims to keep packet sizes within the 1500-byte threshold.
- **trie**: An optimized data structure for matching byte sequence prefixes with a cost of memory space.
- **bit_field**: A memory-efficient structure for storing numbers and verifying their existence quickly.
- **args**: Parses command-line arguments.
- **error**: Handles errors within the program.
- **connection**: Defines the interface for client-server communication, abstracting away the underlying protocol details.
    - The interface consists of 4 main functions:
        - `connect(Connection *)`: Establishes a connection with the server if necessary.
        - `send(Connection *, Payload)`: Sends a payload to the server.
        - `receive(Connection *) -> Payload`: Retrieves a payload from the server.
        - `disconnect(Connection *)`: Terminates the connection if needed.
- **tcp**: Implements TCP-based communication using the `Connection` interface.
    - Utilizes the *trie* for efficient payload type determination.
    - Sending payload type `CONFIRM` does not do anything.
- **udp**: Implements UDP-based communication using the `Connection` interface.
    - The Client is responsible for sending `CONFIRM` on the received data.
- **commands**: Parses user input into a structured `Command` format.
    - Utilizes the *trie* to quickly identify command types.
- **input**: Provides functionality to read input line by line from stdin.
- **payload**: Defines a universal structure for communication payloads, facilitating easy interpretation regardless of the underlying protocol.
- **time**: Offers functions for time-related operations, used primarily for timeout handling during UDP communication.

#### Main Program <a id="main-program"></a>
The core program logic resides in `main.c`, `client.c|h`, and `server.c|h`. Currently, `server.c|h` act as a placeholder for forthcoming project components.

The main function is responsible for parsing command-line arguments as per the specification and subsequently delegates further handling to the client.

#### Client Program <a id="client-program"></a>
+ **Initialization**: The client initializes various components. Failure in any of these components leads to immediate program termination.
    - Initiates the connection.
    - Sets up command handling from user input (stdin).
    - Registers a signal handler for `SIGINT`, ensuring graceful termination by sending a BYE message if not already in the Start state.
    - Initializes a bit field for tracking processed message IDs.
        - This field is redundant in TCP mode but remains for simplicity and avoids potential issues.
    - Creates two `epoll` file descriptors, one for socket monitoring and the other for monitoring both socket and stdin.
    - Sets the client's initial state to Start.

+ **Client Loop**: The client remains in this loop until its state transitions to End, with additional UDP-specific waiting for confirmation of the last payload.

    The loop performs the following actions:
    + Polls the socket and stdin.
        - In UDP mode, adjusts the polling timeout based on program arguments if the previous payload hasn't been confirmed.
        - During the Auth state or unconfirmed payload, only socket polling occurs; user input processing resumes once these conditions are no longer met.
    + Handles polling errors, typically caused by signal interruption, by sending BYE to the server and terminating the program.
    + Manages timeouts by attempting retransmission of the last payload. Exceeding the specified number of retransmissions (default is 3) results in program termination without sending BYE, assuming server unavailability.
    + Processes epoll descriptors:
        - Socket: Receives payloads, processes data, and updates client state accordingly.
            - Errors trigger transmission of an Err payload, transitioning the client to the Error state.
        - User input (stdin): Reads input, parses commands, processes data per specification, and updates client state.
            - Errors result in error messages to stderr and continuation of the loop.
    + If the state is Error and the Err payload is confirmed, sends BYE to the server and transitions the client to the End state.

+ **Clean Up**: Performs necessary cleanup of initialized data before program termination.

#### Additional Commands
In addition to the set of commands specified in the project specification, this project implements 2 additional commands to enhance the chatting experience:
- **exit**: Similar to sending a SIGINT signal by pressing ctrl-c, but provides a clearer indication to the user.
- **clear**: Clears the terminal screen for improved readability.

## Decision Making <a id="decision-making"></a>
During the development process, several key decisions were made to guide the implementation of the program.

### Not Returning Error Code on ERR Sent by Server <a id="returning-error-code-on-err-sent-by-server"></a>
When an `ERR` is received from the server, it is recommended that the client accepts it, responds with a `BYE` message, and terminates the program with a **return code of 0**. This approach indicates that the issue is with the external service and not with the client program.

### Validate User Input <a id="validate-user-input"></a>
The principle of **"Never trust user input"** is fundamental in software development, especially in client-server applications where security is paramount. Validating user inputs on the server side is essential to mitigate vulnerabilities and ensure system integrity.

On the client side, three approaches to validating user input were considered, each with its own implications:

- **No validation**: The client ignores user input entirely, deferring validation to the server.
    - *Pros*: Changes in the protocol do not necessitate client updates.
    - *Cons*: Places a heavy burden on the server, leaving it vulnerable to availability issues such as denial-of-service (DoS) attacks.

- **Full validation**: User input is rigorously validated both on the client and server sides.
    - *Pros*: Allows the client to reject invalid input promptly, enhancing user experience and reducing server bandwidth usage, potentially enabling it to handle more clients simultaneously.
    - *Cons*: Requires client updates alongside server changes, which may pose challenges if users are unwilling to update their applications.

- **Partial validation**: The client validates only certain aspects of user input that are unlikely to change frequently.
    - *Pros*: Strikes a balance between minimizing client updates and preventing server overload with nonsensical payloads.
    - *Cons*: Determining which parts of the protocol are future-proof can be challenging.

For this project, the decision was made to **fully validate** user inputs for three primary reasons:
- Many large-scale applications, such as the Apache server, adopt this approach, demonstrating its effectiveness.
- The reference server used for testing imposes rate limits, making prompt validation beneficial.
- The implementation can be easily adapted to partial or no validation if necessary.

## Testing <a id="testing"></a>
Testing plays a significant role in the development process, ensuring the correctness and functionality of the program's building blocks.

The program was tested on my system with the following specifications:
- CPU: Intel i5 1240P
- OS: Manjaro
- Kernel version: 6.6

All outputs aligned with their expected results at the time of testing.

### Unit Tests <a id="unit-tests"></a>
Unit tests are conducted using the [`greatest` testing framework](https://github.com/silentbicycle/greatest) by Scott Vokes, validating various components of the chat client's functionality. 

These tests, located in the `/test` directory, cover modules with functions independent of others, such as data structures, UDP/TCP de/serialization, command parsing, and argument parsing. 

They can be executed using the `make test` command.

### Dynamic Testing <a id="dynamic-testing"></a>
Dynamic testing involves observing the program's behavior while it is running. 

Initially, `netcat` and `Wireshark` are used to monitor data transmissions between the client and server. 

Once communication stability is achieved, signifying correctness relying on the type of data the client should send to the server at a given time/state, 
a small script is employed to simulate communication between client and server. This script defines multiple test scenarios and checks the input/output of the client to validate program correctness. 

Testing is conducted using White Box Testing, which means that the internal structure is known, allowing inspection of the program's inner state at any given time.

#### Scenarios <a id="scenarios"></a>
These scenarios are verified for both TCP and UDP protocols. In UDP, tests include `CONFIRM` payload from/to the server.

| Scenario | Input | Expected Output |
|----------|-------|--------|
| Join before AUTH | - Client sends JOIN | - Error message |
| Invalid command | - /randomCommand | - Error message |
| Correct AUTH | - Client sends valid AUTH | - Receive REPLY with OK |
| Incorrect AUTH | - Client sends invalid AUTH | - Receive REPLY with NOK<br>- Client's state changes to NotAuth |
| Incorrect AUTH then correct AUTH | - Client sends invalid AUTH<br>- Client sends valid AUTH | - Receive REPLY with NOK<br>- Client's state changes to NotAuth<br>- Receive REPLY with OK<br>- Client's state changes to `Open` |
| Multiple AUTH | - Client sends valid AUTH<br>- Client sends valid AUTH | - Receive REPLY with OK<br>- Client's state changes to `Open`<br>- Error message on the next AUTH command |
| Message before AUTH | - Client sends message | - Error message |
| Exit before AUTH | - SIGINT | - Client terminates without sending BYE to the server |
| Exit after incorrect AUTH | - Client sends incorrect AUTH<br>- Server reply with NOK<br>- SIGINT | - Client send BYE and then terminates |
| Error before REPLY | - Client sends AUTH<br>- Server sends ERR | - Send `BYE` to the server and then terminate the program |

Assuming the user has authorized and the client is in the `Open` state:

| Scenario | Input | Expected Output |
|----------|-------|--------|
| Send message | - Client sends random message | - Output message to stdout |
| Receive message | - Client receives message from server | - Output message to stdout |
| Signal handle 1 | - SIGINT | - Send BYE then terminate the program |
| Signal handle 2 | - EOF (ctrl-d) | - Send BYE then terminate the program |
| Error 1 | - Server sends ERR | - Send BYE then terminate the program |
| Error 2 | - Server sends invalid data | - Send ERR<br>- Send BYE then terminate the program |
| Error 3 | - Server sends invalid data | - Send ERR<br>- Send BYE then terminate the program |
| JOIN | - Client sends JOIN<br>- Client sends message | - Output message to stdout |
| Rename | - Client sends message<br>- Use rename command<br>- Client sends message | - Output 2 messages with different display names |

Scenarios specific to UDP protocol:

| Scenario | Input | Expected Output |
|----------|-------|--------|
| Dynamic Port | - Send /auth<br>- Receive CONFIRM on different port<br>- Send message | - Server receives payload on the dynamic port sent CONFIRM of the AUTH |
| Timeout | - Client sends payload<br>- Server does not react | - Attempt re-send n times based on the program argument<br>- Terminate the program if exceeding the number of retransmissions |
| Receive duplicated payload | - Server sends 2 or more payloads with the same ID | - Send CONFIRM to all of them<br>- Only output the first one |
| Receive duplicated CONFIRM | - Server sends 2 or more CONFIRM of the same ID | - Ignore from the second CONFIRM |

## Note <a id="note"></a>
This document was done with the help of [Deepl Write](https://www.deepl.com/write).

## Bibliography <a id="bibliography"></a>
[IAL]
Prof. Ing.Jan M Honzík, CSc.
ALGORITMY IAL Studijní opora. 
2014.
[cited 2024-03-31]

[RFC9293] 
Eddy, W. 
Transmission Control Protocol (TCP) [online]. 
August 2022. 
[cited 2024-03-31]. 
DOI: 10.17487/RFC9293. 
Available at: https://datatracker.ietf.org/doc/html/rfc9293

[RFC768] 
J. Postel.
User Datagram Protocol [online]. 
August 1980. 
[cited 2024-03-31]. 
DOI: 10.17487/RFC0768.
Available at: https://datatracker.ietf.org/doc/html/rfc768

[Testing]
Mathur, Aditya P.
Foundations of Software Testing. 
Pearson Education India. p. 18. 
2007. 
[cited 2024-04-01]
ISBN 978-81-317-1660-1.
Avaiable at: https://books.google.cz/books?id=yU-rTcurys8C
