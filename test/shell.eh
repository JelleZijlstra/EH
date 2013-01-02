#!/usr/bin/ehi

put(shell "echo 3")

cmd = "ls " + argv->0

put(shell(cmd))
