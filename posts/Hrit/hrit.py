#!/usr/bin/env python3
#-*- coding: utf-8 -*-

### Filename:hrit
## Created by 喵 Alynx
# alynxzhou@gmail.com, http://alynx.xyz/

import os
import time
import argparse

### Deal with the argument.
parser = argparse.ArgumentParser(description="hrit -- Hexo Rename Images Tool.")
parser.add_argument("-f","--forehead",action="store",type=str,help="The string will be added to the begin of the file name.Default is the directory name(dir).")
parser.add_argument("-m","--middle",action="store",default='_',type=str,help="This will be put between the head and the tail.Default '_'")
parser.add_argument("-n","--number",action="store_true",help="If this argument is set,the file name between the head and the last dot will be set as an increasing number.")
#parser.add_argument("dir",action="store",nargs='?',default="./",type=str,help="The directory to work.")
parser.add_argument("dir",action="store",type=str,help="The directory to work.")
args = parser.parse_args()

### Remember the original path.
opath = os.getcwd()

### Change directory.
print("Entering directory %s …"%(args.dir))
try:
    os.chdir(args.dir)
except FileNotFoundError:
    print("ERROR!Can't find directory %s !"%(args.dir))
    print("Quiting…")
    exit()
curdir = os.getcwd()

### Ask.
print("Now it is going to work in ")
print("=====================================================")
print("%s"%(curdir))
print("=====================================================")
print("Please check weither it is the exact directory you want!")
print("Work in other directories can be dangerous and hard to fix(For instance,/,/home,/usr,etc.)!")
answer = input("Continue?[y/N] ")
if answer != 'y' and answer != "yes":
    print("Quiting…")
    os.chdir(opath)
    exit()

print("Wating 5 seconds to regret…")
ilist = ["5…","5 4…","5 4 3…","5 4 3 2…","5 4 3 2 1…"]
for i in ilist:
    print("%s\r"%(i),end="")
    time.sleep(1)
print("5 4 3 2 1…Start!\n")

### Get dir.
if args.forehead == None:
    forehead = curdir.split(os.sep)[-1]
else:
    forehead = args.forehead

### List files.
filelist = os.listdir(curdir)
print("Found files:%s.\n"%(filelist))

### Rename.
if args.number == True:
    ## Number name.
    number = 1
    for file in filelist:
        filel = file.split('.')
        if len(filel) == 1:
            ## Without extand name.
            newname = forehead + args.middle + str(number)
        else:
            ## With extand name.
            newname = forehead + args.middle + str(number) + '.' + filel[-1]

        os.renames(file,newname)
        print("Renamed %s to %s."%(file,newname))
        number = number + 1
else:
    for file in filelist:
        newname = forehead + args.middle + file

        os.renames(file,newname)
        print("Renamed %s to %s."%(file,newname))

### Get new file list.
newlist = os.listdir(curdir)
print("\nNow you can copy the follow hexo tags to your markdown post.\n")
for new in newlist:
    print("{%% asset_img %s %s %%}\n"%(new,new))

### Go back to original path.
os.chdir(opath)

