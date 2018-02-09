In this application a file is transfered from client to server.

It uses caeser cipher method to encrpyt the file at client side and decrypt at server side.

Diffie-Hellman technique is used to exchange the keys.

To run
Client:	gcc client.c -lm -o client
		./client filename.txt

Server:	gcc server -o server.c
		./server

After decrypting the file server will save it in plaintext.txt file.				