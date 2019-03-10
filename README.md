# Distance-Vector-Protocol
Implemented a simplified version of the Distance Vector routing protocol over simulated routers.

# Developer
 * Bilin Shi
 * Wenqi Li

# Description
A router’s operations are divided into two dimensions: Data Plane and Control Plane

## Control Plane
The distance vector protocol is implemented to run on top of the hosts (behaving as routers) on a pre-defined port number, which is referred as ***router-port***. The following commands have been implemented.
 * AUTHOR
 * INIT: Contains the data required to initialize a router.
 * ROUTING-TABLE: The controller uses this to request the current routing/forwarding table from a given router.
 * UPDATE: The controller uses this to change/update the link cost between between the router receiving this message and a neighboring router.
 * CRASH: The controller uses this to simulate a crash (unexpected failure) on a router.
 * SENDFILE: The controller uses this to initiate a file transfer from the router receiving this message to any other router in the network.
 * SENDFILE-STATS: The controller uses this to get statistics from the routers involved in a file transfer about the routed data packets.
 * LAST-DATA-PACKET: The controller uses this to get a copy of the last data packet that was sent/routed/received through the router.
 * PENULTIMATE-DATA-PACKET: The controller uses this to get a copy of the second last data packet that was sent/routed/received through the router.
 ## Data Plane
On receiving the SENDFILE control message, the sending (source) router will need to read the file from the disk and packetize it using the specific packet format. The destination (sink) router will need to reconstruct the file back from the received data packets and write it to disk.
