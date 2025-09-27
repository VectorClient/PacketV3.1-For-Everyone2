import json

with open('offsets.txt', 'r') as r, open('output.txt', 'w') as w:
    output = '{'
    count = 0
    for line in r.read().splitlines():
        pair = line.split(' ')
        output += '{' + pair[0] + ', 0x' + pair[1] + '},'
        count += 1
        if count % 10 == 0:
            output += '\n'
    output = output[:-1] + '}'
    w.write(output)
