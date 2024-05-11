# Commanding and Telemetry Server
### Objective
Building a simple commanding and telemetry server using C++20, TCP, and Python (pytest).

### Server:
- Written in C++20
- Using TCP sockets (Linux C socket API) to communicate with the client
- Multithreaded:
    - Commanding thread: listen for packets, deserialize packet into op-code (identifying code of the command, as defined by the command definition file) and payload (arguments of the command)
    - Commanding execution threads: run the corresponding function to the command on a separate thread
        - For now, the commands will be modifying the state of the "spacecraft", which will be a thread-safe key-value store (an `std::unordered_map` with a mutex, for now) 
    - Telemetry thread: periodically send the data points from the state of the "spacecraft" over the TCP connection to the client

### Client:
- Written in Python
- Connect to telemetry and commanding sockets
- Can manually send commands and read telemetry
- Use Pytest to run an automated test suite
