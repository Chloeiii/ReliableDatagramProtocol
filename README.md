
### CSC361: Computer Communications and Networks  
#### Programming Assignment2 - Reliable Datagram Program :stuck_out_tongue_closed_eyes:
----
    1st step compile all the files:
         type: make

    2rd step run sender file:
         type: ./sender 192.168.1.100 8080 10.10.1.100 8080 sent.dat

    3rd step run receivedr file:
         open another terminal in the same directory,
         type: ./receiver 10.10.1.100 8080 received.dat
----
#### Code Design:
    I structed a char array(length of 1024) as the header to achieve the transmission of 
    header info and file data between sender and receiver.

    event_types (DAT, ACK ,SYN, FIN, RST), sequence number, ack number, payload size, 
    and window size info are located in header[0] to [83].
    file data are stored in header[100] to [1000].
----
#### Activities (normal case without package loss)
    sender send SYN packet to receiver;
    receiver send SYN+ACK back to sender; 
    sender send DAT to receiver;
    receiver send DAT+ACK back to sender;
    sender send DAT to receiver;
    receiver send DAT+ACK to sender; (repeat)
    ...
    (until the data of file is transfered)
    sender send FIN to receiver;
    receiver send FIN+ACK to sender (and quit);
    when sender received the FIN+ACK packet, it quit as well.

    therefore, all the packets with a flag of ACK should be from receiver.
    others should be from sender.
----
#### Code structure
    the sender will be the one to initate the connection by sending the very first SYN packet. 
    both sender and receiver have a while loop to wait for the activity on socket from the other side
    sender side: 
        if he receive the header from receiver, it will response according to ACK flags on the header
        if he didn't get any response, he will resent the header to receiver, it could be one of SYN DAT and FIN, whatever
        he send last.
    receiver side: 
        if he receive the header from receiver, it will response according to those flags on the header
        and send SYN packets to sender.
        if he didn't get any response, it will be simple: he just wait. wait for sender try to connect him again.
 ----
 #### More info
     refer to `p2-1.pdf`
