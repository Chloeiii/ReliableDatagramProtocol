
### CSC361: Computer Communications and Networks  
#### Based on UDP - Reliable Datagram Protocol :stuck_out_tongue_closed_eyes:
----
    1st step compile all the files:
         type: make

    2rd step run sender file:
         type: ./sender 192.168.1.100 8080 10.10.1.100 8080 sent.dat

    3rd step run receivedr file:
         open another terminal in the same directory,
         type: ./receiver 10.10.1.100 8080 received.dat
----
#### Code Design: :flushed:
    I structed a char array(length of 1024) as the header to achieve the transmission of 
    header info and file data between sender and receiver.

    event_types (DAT, ACK ,SYN, FIN, RST), sequence number, ack number, payload size, 
    and window size info are located in header[0] to [83].
    file data are stored in header[100] to [1000].
----
#### Activities (normal case without package loss): :yum:
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
#### Code structure: :imp: 
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
 #### More info: :alien:
     refer to `p2-1.pdf`
    
 ----
 #### TCP VS UDP: :innocent:	
##### General

Both TCP and UDP are protocols used for sending bits of data — known as packets — over the Internet. They both build on top of the Internet protocol. In other words, whether you are sending a packet via TCP or UDP, that packet is sent to an IP address. These packets are treated similarly, as they are forwarded from your computer to intermediary routers and on to the destination.

TCP and UDP are not the only protocols that work on top of IP. However, they are the most widely used. The widely used term “TCP/IP” refers to TCP over IP. UDP over IP could just as well be referred to as “UDP/IP”, although this is not a common term.

##### TCP

TCP stands for Transmission Control Protocol. It is the most commonly used protocol on the Internet.

When you load a web page, your computer sends TCP packets to the web server’s address, asking it to send the web page to you. The web server responds by sending a stream of TCP packets, which your web browser stitches together to form the web page and display it to you. When you click a link, sign in, post a comment, or do anything else, your web browser sends TCP packets to the server and the server sends TCP packets back. TCP is not just one way communication — the remote system sends packets back to acknowledge it is received your packets.

TCP guarantees the recipient will receive the packets in order by numbering them. The recipient sends messages back to the sender saying it received the messages. If the sender does not get a correct response, it will resend the packets to ensure the recipient received them. Packets are also checked for errors. TCP is all about this reliability — packets sent with TCP are tracked so no data is lost or corrupted in transit. This is why file downloads do not become corrupted even if there are network hiccups. Of course, if the recipient is completely offline, your computer will give up and you will see an error message saying it can not communicate with the remote host.

##### UDP

UDP stands for User Datagram Protocol — a datagram is the same thing as a packet of information. The UDP protocol works similarly to TCP, but it throws all the error-checking stuff out. All the back-and-forth communication and deliverability guarantees slow things down.

When using UDP, packets are just sent to the recipient. The sender will not wait to make sure the recipient received the packet — it will just continue sending the next packets. If you are the recipient and you miss some UDP packets, too bad — you can not ask for those packets again. There is no guarantee you are getting all the packets and there is no way to ask for a packet again if you miss it, but losing all this overhead means the computers can communicate more quickly.

UDP is used when speed is desirable and error correction is not necessary. For example, UDP is frequently used for live broadcasts and online games.

Retrieved April 17, 2018, from https://support.holmsecurity.com/hc/en-us/articles/212963869-What-is-the-difference-between-TPC-and-UDP-
