1. Memcached now has 4 modes: no replication, replication with isis service, replication with isis library, replication with raw tcp
    a. no replication: normal start with original commandline
    b. isis service: ./memcached -N nodenumber -K myrank -H shardsize -q localisisport
    c. isis library: ./memcached -N nodenumber -K myrank -H shardsize
    d. raw tcp:      ./memcached -N nodenumber -K myrank -H shardsize -T

2. Related functions:
    a. isis service: 
        (i): sending replication: complete_nread_ascii() (memcached.c)
        (ii): reading reply: isis_readcb() (memcached.c)
        (iii): isis service: In project IsisServer, IsisServer.cs
    b. isis library:
        (i): sending replication: complete_nread_ascii() (memcached.c) calls isis_send() (monoisis.c)  
        (ii): isis library: In monoisis.c
    c. raw tcp: 
        (i): sending replication: complete_nread_ascii() (memcached.c)
        (ii): reading reply: member_readcb() (memcached.c)

