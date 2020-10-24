## DESCRIPTION

Implementation of the TCP protocol in C language, the client connect to the server and ask for a file, the server sends it to him

## USAGE

To compile : 
```bash
gcc -Wall -lm -o ServerProject ServerProject.c
gcc -Wall -lm -o client_UDP client_UDP.c
```

To run :

```bash
./ServerProject <connection port> <file transfert port>
./client_UDP <server IP @> <connection port>
  ```
