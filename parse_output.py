from collections import defaultdict, Counter
import fileinput

# appends pipe events in the order they are viewed.
d = defaultdict(list)
for line in fileinput.input():
    pipe_id, event = line.split()
    d[pipe_id].append(event)

d.pop('0', None)

vals = [tuple(x) for x in d.values()]
c = Counter(vals)
for event_order, n_occurred in c.items():
    orders = ','.join(event_order)
    print('{:7s} {:>3d}'.format(orders, n_occurred))
