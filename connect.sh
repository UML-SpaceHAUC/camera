#!/usr/bin/expect -f
spawn ssh -p 8823 pi@spacehauc.ddns.net
expect "assword:"
send "raspberry\r"
interact
