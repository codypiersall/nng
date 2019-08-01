from collections import defaultdict, Counter
import fileinput

d = defaultdict(list)
for line in fileinput.input():
    pipe_id, event = line.split()
    d[pipe_id].append(event)

d.pop('0', None)

vals = [tuple(x) for x in d.values()]
c = Counter(vals)
print(c)
