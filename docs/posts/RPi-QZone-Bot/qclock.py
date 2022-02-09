#!/usr/bin/env python3
#-*- coding: utf-8 -*-

### Filename: qclock.py
## Created by 喵 Alynx
# alynxzhou@gmail.com, http://alynx.xyz/

import os
import time
import qqlib
from qqlib import qzone

QQ_NUM = 1234567890
QQ_PASSWD = "MyPassWord."

qz = qzone.QZone(QQ_NUM, QQ_PASSWD)

try:
    qz.login()
except qqlib.NeedVerifyCode as nvc:
    # Get a verify code.
    verifier = nvc.verifier
    with open("verify.jpg", "wb") as verify_jpg:
        verify_jpg.write(verifier.image)
    print("Saved verify photo to verify.jpg, please open it by yourself.")
    vcode = input("Enter verify code here: ")
    try:
        # Test verify code.
        kw = verifier.verify(vcode)
    except qqlib.VerifyCodeError as vce:
        os.remove("verify.jpg")
        raise vce("Wrong verify code!")
    else:
        # Finish login.
        qz.login()
        os.remove("verify.jpg")

print("Login finished.")

unit = "°C"

def get_temp():
    # Get internal CPU temperature.
    with open("/sys/class/thermal/thermal_zone0/temp") as temp_open:
        temp = float(temp_open.read())/1000
    return temp

# Loop to get time and feed to qzone.
try:
    while True:
        tm = time.localtime()
        if tm.tm_min == 0:
            qz.feed("树莓派自动报时：\n现在是%d年%d月%d日 %d时整。\nCPU温度为%.2f%s。"%(tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, get_temp(), unit))
            print("Feeded at %d-%.2d-%.d %.2d:%.2d."%(tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min))
        elif tm.tm_min == 30:
            qz.feed("树莓派自动报时：\n现在是%d年%d月%d日 %d时%d分。\nCPU温度为%.2f%s。"%(tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, get_temp(), unit))
            print("Feeded at %d-%.2d-%.d %.2d:%.2d."%(tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min))
        # Use 60 to keep only one feed in the minute.
        time.sleep(60)
except KeyboardInterrupt:
    exit()
