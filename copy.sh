 #!/bin/bash
PATH=/usr/local/bin:/usr/bin:/bin:/usr/local/sbin:/usr/sbin:.:/home/cs58/.local/bin:/home/cs58/bin:.

#Use source copy.sh or it will not change directory (It happens in subshell)


#copy yalnix executable
cp yalnix ~/Desktop/

#remove old core dump files
rm -f ~/Desktop/core.*

cd ~/Desktop/

yalnix

