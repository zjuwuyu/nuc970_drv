from CommissionCal1R import *
from CommissionCal2R import *
import sys

def get_base_gp():
    base = input("Please input base:")
    gp = input("Please input gp:")
    return int(base), int(gp)


if __name__ == '__main__':
    r_num = input("Please input R number:")

    if r_num == '1':
        base, gp = get_base_gp()
        calculator1R = Calculator1R(base, gp)
        print(calculator1R.populate_data())

    elif r_num == '2':
        base, gp = get_base_gp()
        calculator2R = Calculator2R(base, gp)
        print(calculator2R.populate_data())
    else:
        print("R number should be 1 or 2")
        sys.exit()
