# C-Reti1-Exam-Project

### Description

Client-Server Socket TCP Application

### Functionality

The app calculate averange and variance of a range of values.

Client send the values to the server
Server receive the values and send the results

#### How to compile

```bash
gcc -o server server.c
gcc -o client client.c ./list-lib/list.c
```

#### How to use

```bash
./server \[port-number\]
./client \[server-ip\] \[port-number\]
```

See more in Relazione.pdf file