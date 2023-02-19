import os
import time

# process name
name="socks5_server"
# interval(second) to print
interval=10

def execute(cmd):  
    r = os.popen(cmd)  
    text = r.read()  
    r.close()  
    return text 
def gettime():
    date=execute("date")
    arr=date.split(" ")
    return arr[3]

def getpid(name:str)->str:
    output=execute("ps -ef | grep \""+name+"\"")
    arr=output.split(" ")
    return arr[6]

pid=getpid(name)
print("pid: ",pid)
input()
cmd_get_opened_file_num="lsof -p "+pid +" |wc -l"
print("cmd: ",cmd_get_opened_file_num)
while True:
    now=gettime()
    msg=execute(cmd_get_opened_file_num)
    print(now,": ",msg)
    time.sleep(interval)
    