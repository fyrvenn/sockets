# Ubus Receiver service
The service:
* receive network packets via a RAW socket from the selected network interface
* filters them according to the following simultaneous conditions
  * direction (only the received packets)
  * source ip address (user defined)
  * IP-proto (UDP protocol only)
* collects the following statistics for the given filter:
  * number of received packets
  * sum of received bytes
* sends the collected statistics on ubus with the preassigned period.
Target OS - Linux Debian 7 or newer version.

## Usage
Run in different terminals following commands:
```
$ sudo ubusd
```
```
$ ubus listen
```
```
$ ./ubus_receiver --time=TIME --ip=IP_ADDRESS --interface=INTERFACE...
```
The result ot statistics counting will be located in terminal with `ubus listen`.

## External Dependencies

*GCC compiler

Libraries:
* libubus
* libubox
* pthread

## Author
Katerina Budnikova
