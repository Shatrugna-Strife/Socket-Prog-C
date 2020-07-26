To compile gcc -o server server.c -lpthread
the same is with client.
To change the Drop rate and pkt size, change the 
macro DROP_RATE and BUFLEN in all the files.
DROP_RATE is in form of percentage.
Input for text file is in name input.txt.
While the output is in output.txt.
Implemented using multithreading. For each packet created a new thread to send and implement timer which has done selective repeat part of the protocol. Two separates relays are implemented using threading.
Thank You.
Korukanti Shatrugna Rao.
