#import modules
import os

#startup variables
cVar = {'i': 'HALP'}
#functions
#integer module
def integ(value):
    if " = " in value:
        name, val = value.split(" = ", 1)
        cVar[str(name)] = int(val)

#string module
def string(value):
    if " = " in value:
        name, val = value.split(" = ", 1)
        cVar[str(name)] = str(val)

#print module
def mprint(value):
    value = value.removeprefix(": ")
    if value in cVar:
        temp = cVar.get(f"{value}")
        print(temp)
    else :
        print(value)         

#module mapping
elements = {'int' : integ,
            'str' : string,
            'print' : mprint}

# example program
program = [
    "int i = 4",
    "str txt = John",
    "print : i",
    "print : txt",
    "print : Hello World"
]

for line in program:
    for key in elements:
        if line.startswith(key):
            elements[key](line[len(key):].strip())