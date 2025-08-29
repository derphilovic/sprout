#import modules
import os

#startup variables
cVar = {}
#functions
def integ(value):
    if " = " in value:
        name, val = value.split(" = ", 1)
        cVar[str(name)] = int(val)

def string(value):
    if " = " in value:
        name, val = value.split(" = ", 1)
        cVar[str(name)] = str(val)

elements = {'int' : integ,
            'str' : string}

a = "hoj = test"
string(a)