import random

def get_number():
    r = random.random()
    if r < 0.1:
        return random.randint(-1, 1)
    elif r < 0.3:
        return random.randint(-0xffffffff, +0xffffffff)
    else:
        return random.randint(
            -1000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000,
            +1000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000)

def get_op():
    return random.choice('+-*/%')

for i in range(10000):
    a = get_number()
    b = get_number()
    op = get_op()
    c = "NaN"
    try:
        if (op == '+'):
            c = a + b
        if (op == '-'):
            c = a - b
        if (op == '*'):
            c = a * b
        if (op == '/'):
            c = a / b
        if (op == '%'):
            c = a % b
    except:
        c = "NaN"

    print('{0}\t{1}\t{2}\t{3}'.format(a, op, b, c));
