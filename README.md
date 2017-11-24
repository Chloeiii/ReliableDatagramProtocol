
### CSC361: Computer Communications and Networks  
#### Programming Assignment2 - Reliable Datagram Program :stuck_out_tongue_closed_eyes:

-----
#### Introduction  

    So far, you have implemented a Simple Web Server following the Hyper Text Transfer Protocol
    (HTTP) in the first programming assignment (P1). Good job! But very soon you have found
    that the simple web server is not always working reliably, due to the fact that it just relies on the
    unreliable User Datagram Protocol (UDP). For example, if the HTTP request or response messages
    are larger than what a normal UDP packet can accommodate, multiple UDP packets have to be
    sent over the network. These packets may get lost, duplicated, reordered or corrupted along the
    way, and neither the sender nor the receiver has the capability to recover them. In addition, a fast
    sender can send packets much faster than how much the receiver can handle and may potentially
    overflow the receiver’s buffer and cause packet loss, even if the packets have arrived at the receiver.
    Further, the sender should be able to indicate to the receiver the end of the request or response
    messages, so the receiver does not need to wait any further for more packets.
    Therefore, you need to add some flow control and error control capabilities, as well as the start
    and finish indicators, to UDP, leading to a “new” protocol that we call CSC361 “Reliable Datagram
    Protocol” or RDP 1. In this programming assignment, you will complete the design of the CSC361
    RDP protocol and implement it using the datagram (DGRAM) socket Application Programming
    Interface (API) with UDP. The goal is to transfer a text file from a sender to a receiver through
    the Linksys router in ECS360 that emulates various network impairments such as packet loss,
    duplication, reordering and corruption in a real network.
    In this assignment, only the assignment requirements and basic design are provided, and you
    have the freedom to create your own detailed design and implement it. Be creative, but you should
    be able to justify your own design choices.
----
#### Schedule
    Date                        Lab Lecture and Tutorial                           Tasks Milestones
    Feb 20/21/22/23         P2 Spec, RDP header, Connection Mgmt (CM)            RDP header done
    Feb 27/28/Mar 1/2       Flow Control (FC) and Error Control (EC)             CM done, FC half-way
    Mar 6/7/8/9             network impairments and NAT                          FC done, first due; EC half-way
    Mar 13/14/15/16         feedback, debug and test                             CM, FC and EC all done
    Mar 20/21/22/23                 demo                                         all done and tested;  second due
----
#### Requirements



