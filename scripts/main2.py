# import OS module
import os
 
# Get the list of all files and directories
libc = input("Directory on libc: ")
path = "../src/libc/" + libc
dir_list = os.listdir(path)
 
print("Files and directories in '", path, "' :")
 
# prints all files
#print(dir_list[3])
i = 0
while True:
    try:
        f = dir_list[i]
        fo = f[:-1]
        print("$(OBJ)/" + fo + "o")
        i += 1
    except IndexError:
        break
