## Compiler
CC=gcc
## Flags
RECEIVER=ubus_receiver
SENDER=sender
CFLAGS=
LDFLAGS=-lubus -lubox -lblobmsg_json -lpthread

.PHONY: all clean receiver

all: $(RECEIVER) $(SENDER)
	
$(RECEIVER): $(RECEIVER).o
	$(CC) $< -o $@ $(LDFLAGS)

$(RECEIVER).o: ubus_receiver.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SENDER): $(SENDER).o
	$(CC) $< -o $@ 

$(SENDER).o: sender.c
	$(CC) -c $< -o $@

clean: 
	rm -rf $(RECEIVER) $(SENDER)

