Deadline: 19.06.2022
# FreeRtos-Transceiver

A modular C++ library to simplify the use of FreeRtos inter-task communication. 

# Features

- Passing data over the queue  
  - Sending data to every possible task
  - Simultaneous transmission of x different datatypes (over the same queue)
  - Simultaneous transmission of x different datatypes to y different queues
  
- Receiving data over the queue  
  - Receiving data sent by any task in the system

- Formatted representation regarding information about the connection with other tasks  
  - Amount of communication partners
  - Name of communication partners
  - Amount of tx-lines connected (outgoing data)
  - Amount of rx-lines connected (incoming data)
  - Optional:
    - Data/Bytes received
    - Current rx and tx buffer informations
    - ...

- A single queue can be used by multiple tasks
  - Adressing the source by supplying the target-taskhandle
  - 1...n transmitter of data

- Queue manipulation (only on tx line possible)  
  - Check if datatype x available on queue y
  - Removing an element  
    - Removing the x. element from queue y
    - Removing the oldest element x from queue y
    - Flushing the queue
    - ...

- Task notification functionality  
  - TBA

- Threadsafe

# Supported Devices

Mainly in development for the ESP32 microcontroller. Portability planned.
