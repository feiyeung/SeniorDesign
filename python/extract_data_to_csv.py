#!/usr/bin/env python
import sys, re
r = re.compile('^([0-9\.\- ]+),[0-9\.\- ]+,[0-9\.\- ]+')
for line in sys.stdin:
    if r.match(line):
        sys.stdout.write(line)