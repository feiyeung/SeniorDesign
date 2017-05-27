#!/usr/bin/env python
import sys, re
r = re.compile('^([0-9\.\- ]+),[0-9\.\- ]+,[0-9\.\- ]+')
last_ind = -1
count = 0
data_count = 0
for line in sys.stdin:
    line = line.strip('\n')
    hit = r.match(line)
    count += 1
    if hit:
        data_count += 1
        this_ind = float(hit.group(1).strip())
        diff = (this_ind - last_ind)
        if (diff == 1) or (diff == -255 and this_ind == 0):
            pass
        else:
            print(' not continuous at #%7.d,\tdiff %3.d,\tthis_ind %3.d,\t"%s"' % (count, diff, this_ind, line))
        last_ind = this_ind
    else:
        print('no data at #%d: %s' % (count, line))
print ('%d lines of data' % data_count)