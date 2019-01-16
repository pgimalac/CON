#!/bin/sh
make re

case $1 in 
    serv    ) ./con server;;
    host    ) ./con host $2;;
    client  ) ./con client $2;;
    *       ) ./con local;;
esac

# ./con server # SERVER
# ./con st # HOTE
# ./con client # CLIENT

# On peut préciser addresse ip.