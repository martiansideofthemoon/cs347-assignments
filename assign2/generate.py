import numpy as np

districts = 5
entries = 100000

for j in range(10):
    output = ""
    for k in range(districts):
        output += str(np.random.randint(1, 1001)) + "\n"
        output += '\n'.join(["%s %s" % (np.random.randint(1, 21), np.random.randint(1, 6)) for i in range(entries)])
        output += '\n\n'
    with open("%d.txt" % j, 'w') as f:
        f.write(output)
