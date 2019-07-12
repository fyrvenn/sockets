# sockets
Execute next command in terminal:
```
$ gcc -o receiver receiver.c -lubus -lubox -lblobmsg_json -lpthread
```
To test program run in different terminals
```
$ sudo ubusd
$ ubus listen
$ sudo ./receiver
```
