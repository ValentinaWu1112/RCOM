# Cisco Router Configuration

## 1 
* Analyse the configuration file for a Cisco router

## 2
 
* Router name: gnu-rtr1

* Ethernet Ports available and of what type:
  - interface FastEthernet0/0 
  - interface FastEthernet0/1

* Configured IP addresses and netmask of ports:
  - interface FastEthernet0/0 : ip address 172.16.30.1 255.255.255.0
  - interface FastEthernet0/1 : ip address 172.16.254.45 255.255.255.0

* Configured routes :
  - ip route 0.0.0.0 0.0.0.0 172.16.254.1
  - ip route 172.16.40.0 255.255.255.0 172.16.30.2

## 3

* The interface connected to the Internet: 
  - interface FastEthernet0/1 
  - porque 'ip nat outside'

* 1 Ip adresses are available
  - 'ip nat pool ovrld 172.16.254.45 172.16.254.45 prefix-length 24' that's mean:  defines a NAT pool named ovrld with a range of a single IP 

* The router is useing overloading 
  - 'ip nat pool **ovrld** 172.16.254.45 172.16.254.45 prefix-length 24'


# Linux Routing

Comandos para parte do Linux Routing:
 <ip route show> ou <route -n> ; 
 <route del default gw ip_da_default_gw> ;    <route add -net 104.17.113.188 gw ip_da_gateway_que_encontraram netmask 255.255.255.0> ; 
 <route add -net 9.9.9.9 gw ip_da_gateway_que_encontraram netmask 255.255.255.0>

