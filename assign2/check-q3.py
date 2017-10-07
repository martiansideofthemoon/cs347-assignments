import numpy as np
import subprocess
import sys
import time

"""
Sample input specification - (buffer_size, followed by lines of input)
4
1 2 1 4 0
4 0 2 2 1
1 4 2 2 2
3 1 3 2 3
1 2 1 4 0
4 0 2 2 1
1 4 2 2 2
3 1 3 2 3

Sample output specification -
Buffer Items: 7
Overflow Items: 8 9 10 3 4 5

Buffer Items: 5
Overflow Items: 10 17 18 19 20 13 14 15 16 11 12

Buffer Items: 17
Overflow Items: 18 19 20 14 15 16 21 22 23

Buffer Items: 18
Overflow Items: 19 20 26 27 28 29 30 16 21 22 23 24 25

Buffer Items: 18
Overflow Items: 19 20 26 27 28 29 30 16 34 35 36 37 21 22 23 24 25 31 32 33

"""


def parse(output):
    output = output.split('\n')
    results = []
    for i in range(0, len(output) - 1, 3):
        buffer_items = [int(x) for x in output[i].split()[2:]]
        overflow_items = [int(x) for x in output[i + 1].split()[2:]]
        results.append([buffer_items, overflow_items])
    return results


def error(i, e_type):
    print("Error in cycle %d of type %d" % (i, e_type))
    sys.exit()


def test(i, data, buffer_size):
    cycles = len(data)
    command = "./a.out < input_file.txt"
    output = subprocess.check_output(command, shell=True)
    results = parse(output)
    extra = 0
    pipeline = []
    total_produce = 1
    for i in range(cycles):
        result = results[i]
        production = data[i][0] + data[i][1] + data[i][2]
        consumption = data[i][3] + data[i][4]
        extra += production - consumption
        # Check whether length of buffer and overflow is correct
        if extra <= 0:
            if len(result[0]) > 0 or len(result[1]) > 0:
                error(i, 1)
        elif extra > 0 and extra <= buffer_size:
            if len(result[0]) != extra or len(result[1]) > 0:
                error(i, 2)
        else:
            if len(result[0]) != buffer_size or len(result[1]) != (extra - buffer_size):
                error(i, 3)
        # Check whether priorities are correctly implemented
        # If any of previous `pipeline` items in current overflow buffer,
        # all new items must be in overflow buffer. This is sufficient
        # condition.
        if len([j for j in pipeline if j in result[1]]) > 0:
            for k in range(total_produce, production + total_produce):
                if k not in result[1]:
                    error(i, 4)
        pipeline = result[1]
        total_produce += production


BUFFER_RANGE = [1, 11]
CYCLES_RANGE = [5, 55]
RUNS = 5
CASES = 1000

for i in range(CASES):
    np.random.seed(i)
    buffer_size = np.random.randint(BUFFER_RANGE[0], BUFFER_RANGE[1])
    input_file = "%d\n" % buffer_size
    # input_file = ""
    cycles = np.random.randint(CYCLES_RANGE[0], CYCLES_RANGE[1])
    data = []
    production = 0
    consumption = 0
    for j in range(cycles - 1):
        prod1 = np.random.randint(0, 6)
        prod2 = np.random.randint(0, 6)
        prod3 = np.random.randint(0, 6)
        cons1 = np.random.randint(0, 6)
        cons2 = np.random.randint(0, 6)
        production += prod1 + prod2 + prod3
        consumption += cons1 + cons2
        data.append([prod1, prod2, prod3, cons1, cons2])
        input_file += '%d %d %d %d %d\n' % (prod1, prod2, prod3, cons1, cons2)
    if production > consumption:
        data.append([0, 0, 0, production - consumption, 0])
        input_file += "0 0 0 %d 0\n" % (production - consumption)
    elif production < consumption:
        data.append([consumption - production, 0, 0, 0, 0])
        input_file += "%d 0 0 0 0\n" % (consumption - production)

    # with open('input.txt', 'w') as f:
    #     f.write(input_file)
    # with open('input_file.txt', 'w') as f:
    #     f.write(str(buffer_size))
    with open('input_file.txt', 'w') as f:
        f.write(input_file)
    print("Input File %d generated" % i)

    for j in range(RUNS):
        test(i, data, buffer_size)

print("All tests passed!")
