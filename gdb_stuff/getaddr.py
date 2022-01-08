f = open("gdb.output", "r")

addr = []
names = []
sizes = []

s = f.readline()
n = f.readline()
while s!='':
	if names.count(s.split(' ')[1].split('\t')[0][1:-2]) == 0:
		addr.append(s.split(' ')[0])
		names.append(s.split(' ')[1].split('\t')[0][1:-2])
		sizes.append(n.split(' = ')[1].strip())
	s = f.readline()
	n = f.readline()

f.close()

s = ""

for i in range(len(addr)):
	if i >=14:
		s+="{\"" +  names[i] + "\", " + addr[i] + ", " + sizes[i] + ", true}, "
	else:
		s+="{\"" +  names[i] + "\", " + addr[i] + ", " + sizes[i] + ", false}, "
	if names.count(names[i]) > 1:
		print(names[i])
f.close()

print(s)
print(len(addr))
