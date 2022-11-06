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
        fo = f[:-2]
        print("$(OBJ)/"+ fo + ".o" + " : $(LIBC)/" + libc + "/" + f)
        a = ''' ]\n"'''
        #print('''    @printf "[ $(SRC)/''' + libc + "/" + f + a)
        print("    $(CC) $(CC_FLAGS) -c $(LIBC)/" + libc + "/" + f + " -o $(OBJ)/" + fo + ".o")
        print("\n")
        i +=1
    except IndexError:
        break
