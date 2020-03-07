import os
import sys


def file_reader(file_name):
    lst_1 = list()
    lst_2 = list()
    with open(file_name) as f:
        lst = f.read().split("\n")
    for j in range(len(lst)):
        if j % 2 == 0:
            lst_1.append(float(lst[j]))
        else:
            lst_2.append(float(lst[j]))
    return lst_1, lst_2


if __name__ == "__main__":
    try:
        n = int(sys.argv[1])
        conf_file = sys.argv[2]
        for i in range(n):
            os.system(os.path.abspath(f"./cmake-build-debug/file_reading.exe {conf_file}"))
        res_lst, time_lst = file_reader("out.txt")
        print(res_lst, time_lst)
        m = min(time_lst)
        mis = 0.01
        flag = 0
        for j in range(len(res_lst)):
            for k in range(j + 1, len(res_lst)):
                if abs(res_lst[j] - res_lst[k]) > mis:
                    flag += 1
        if flag == 0:
            print("Results are the same")
        else:
            print("Results are different")
        print(m)
    except IndexError:
        print("There is no conf file or number of trails")
