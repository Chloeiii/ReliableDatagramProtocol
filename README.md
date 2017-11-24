
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
    I structed a header using a char array with length 1024 (in sender.c) to
    achieve the transmission of header info and file data between sender and receiver.

    in the header, I stored all the event_types (eg. DAT, ACK ,SYN, FIN, RST) and I use
    the value of '0' or '1' to stand for the existence of each of them. Also, I stored
    sequence number, ack number, payload size, and window size in the header as well.
    the total header info is from header[0] to header[83].
    And after that, I store the file data from header[100] to header[1000], so that each
    time the only packet I'm going to transform is the header,which makes it simple and easy to read.
----
#### Activities
    sender send SYN packet to receiver;
    then receiver send SYN+ACK to sender; 
    then sender send DAT to receiver;
    then receiver send DAT+ACK to sender;
    then sender send DAT to receiver;
    then receiver send DAT+ACK to sender; (repeat doing this)
    ...
    (until the data of file is transfered)
    then sender send FIN to receiver;
    then receiver send FIN+ACK to sender (and quit);
    when sender received the FIN+ACK packet, it quit as well.

    therefore, all the packets with a flag of ACK should be from receiver.
    others should be from sender.
----
#### Code structure
    the sender will be the one to initate the connection by sending the very first SYN packet 
    both of sender and receiver will have a while loop to wait for the activity on socket from the other side
    sender side: 
        if he receive the header from receiver, it will response according to those ACK flags on the header
        if he didn't get any response, he will resent the header to receiver, it could be one of SYN DAT and FIN, whatever
        he send last.
    receiver side: 
        if he receive the header from receiver, it will response according to those flags on the header
        and send SYN packets to sender.
        if he didn't get any response, it will be simple: he just wait. wait for sender try to connect him again.
 ----
 #### More info
     refer to `p2-1.pdf`


