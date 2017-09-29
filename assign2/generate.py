import numpy as np

districts = 20
constituencies = 5
files = 10
parties = 20

for j in range(files):
    output = ""
    for k in range(districts):
        output += str(j * files + k) + "\n"
        winners = [0] * parties
        for i in range(constituencies):
            winners[np.random.randint(0, parties)] += 1
        for i in range(parties):
            if winners[i] == 0:
                continue
            output += str(i + 1) + " " + str(winners[i]) + "\n"
        output += '\n'
    with open("%d.txt" % (j + 1), 'w') as f:
        f.write(output)
