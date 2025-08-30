#import modules
import os

#startup variables
cVar = {'i': 'HALP'}
#functions
#integer module
def integ(value):
    if " = " in value:
        name, val = value.split(" = ", 1)
        if "." in value:
            cVar[str(name)] = float(val)
        else:
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

#math module
def math(expr):
    # remove spaces
    expr = expr.replace(" ", "")
    
    # Addition
    if "+" in expr:
        parts = expr.split("+")
        total = 0
        for p in parts:
            if p in cVar:
                total += float(cVar[p])
            else:
                total += float(p)
        return total
    
    # Subtraction
    if "-" in expr:
        parts = expr.split("-")
        total = float(cVar.get(parts[0], parts[0]))  # first term
        for p in parts[1:]:
            if p in cVar:
                total -= float(cVar[p])
            else:
                total -= float(p)
        return total
    
    # Multiplication
    if "*" in expr:
        parts = expr.split("*")
        total = 1
        for p in parts:
            if p in cVar:
                total *= float(cVar[p])
            else:
                total *= float(p)
        return total
    
    # Division
    if "/" in expr:
        parts = expr.split("/")
        total = float(cVar.get(parts[0], parts[0]))
        for p in parts[1:]:
            if p in cVar:
                total /= float(cVar[p])
            else:
                total /= float(p)
        return total

    # If it's just a number or variable
    if expr in cVar:
        return float(cVar[expr])
    return float(expr)

#module mapping
elements = {'int' : integ,
            'str' : string,
            'print' : mprint}

# load Sprout program
file = "test.spt" #input("Document to open: ") 

try:
    with open(file, "r", encoding="utf-8") as program:
        for raw_line in program:
            line = raw_line.strip()
            
            # skip empty lines or comments (start with #)
            if not line or line.startswith("//"):
                continue
            
            # keyword handling
            for key in elements:
                if line.startswith(key):
                    elements[key](line[len(key):].strip())
                    break  # don’t double match
            
            # assignments (not declarations)
            if "=" in line and not line.startswith(("int", "str")):
                varName, expr = line.split("=", 1)
                varName = varName.strip()
                expr = expr.strip()
                cVar[varName] = math(expr)

except FileNotFoundError:
    print(f"Error: File '{file}' not found.")