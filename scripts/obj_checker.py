
path = '../static/models/tooth/N5.obj'

count = 0
vt = 0
vtlist = set()

container = []

with open(path, 'r') as ifile:
    while (line := ifile.readline()):
        xx = line.split()
        if len(xx) == 0:
            continue
        if xx[0] == 'v':
            vt += 1
        if xx[0] != 'f':
            continue
        xx = xx[1:]
        xx.sort()
        e1 = (xx[0], xx[1])
        e2 = (xx[0], xx[2])
        e3 = (xx[1], xx[2])
        vtlist.add(xx[0])
        vtlist.add(xx[1])
        vtlist.add(xx[2])
        if e1 not in container:
            container.append(e1)
            count += 1
        if e2 not in container:
            container.append(e2)
            count += 1
        if e3 not in container:
            container.append(e3)
            count += 1

print(count)
print(vt)

vtlist = list(vtlist)
vtlist.sort()

c = 0
invalid_vt = []
for i in range(1, vt + 1):
    if str(i) not in vtlist:
        invalid_vt.append(i)
        c += 1

print('invalid vertices:', c)
print('invalid vertices:', len(invalid_vt))

print('real valid vertices:', vt - c)

if c == 0:
    exit(0)

print('validating')


ifile = open('../static/models/tooth/N5.obj', 'r')
ofile = open('../static/models/tooth/valid.obj', 'w')


vtidx = 1
x = 0
while (line := ifile.readline()):
    xx = line.split()
    if len(xx) == 0:
        continue
    if xx[0] == 'v':
        if vtidx in invalid_vt:
            vtidx += 1
            continue
        ofile.write(line)
        x += 1
        vtidx += 1
    if xx[0] == 'f':
        if int(xx[1]) in valid


ifile.close()
ofile.close()

