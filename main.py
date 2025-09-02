#import modules
import os
import math as meth
import random as rndm
#startup variables
cVar = {}
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
        val = val.strip()
        # remove wrapping quotes if present
        if (val.startswith('"') and val.endswith('"')) or (val.startswith("'") and val.endswith("'")):
            val = val[1:-1]
        cVar[name.strip()] = str(val)


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
    
    #powers of ...
    if "^" in expr:
        parts = expr.split("^")
        total = float(cVar.get(parts[0], parts[0]))
        for p in parts[1:]:
            if p in cVar:
                total **= float(cVar[p])
            else:
                total **= float(p)
        return total

    # If it's just a number or variable
    if expr in cVar:
        return float(cVar[expr])
    return float(expr)

#string combination
def stringcomb(expr):
    expr = expr.strip()
    
    if "+" in expr:
        parts = expr.split("+")
        total = ""
        for p in parts:
            p = p.strip()
            if p in cVar:  # variable
                total += str(cVar[p])
            else:
                # remove wrapping quotes if present
                if (p.startswith('"') and p.endswith('"')) or (p.startswith("'") and p.endswith("'")):
                    p = p[1:-1]
                total += str(p)
        return total
    
    if expr in cVar:
        return str(cVar[expr])
    return str(expr)

#evaluate state of if statement
def evaluate_condition(cond: str) -> bool:
    cond = cond.strip()

    # supported operators in order of precedence
    ops = ["<=", ">=", "==", "!=", "<", ">"]

    for op in ops:
        if op in cond:
            left, right = cond.split(op, 1)
            left = left.strip()
            right = right.strip()

            # resolve variables
            if left in cVar:
                left = cVar[left]
            if right in cVar:
                right = cVar[right]

            # try numeric conversion
            try:
                left = float(left)
                right = float(right)
            except:
                # keep as string if not numbers
                left = str(left)
                right = str(right)

            # comparisons
            if op == "<":  return left < right
            if op == ">":  return left > right
            if op == "<=": return left <= right
            if op == ">=": return left >= right
            if op == "==": return left == right
            if op == "!=": return left != right

    # no operator found → default False
    return False




#input module
def minput(value):
    value = value.removeprefix(":").strip()
    if "," in value:
        var, quest = value.split(",", 1)
        var = var.strip()
        quest = quest.strip()
        answer = input(f"{quest} ")
        if isinstance(cVar[var], (int, float)):
            answer = float(answer)

        if var in cVar:
            cVar[var] = answer
        else:
            # default new input as string
            cVar[var] = answer

#pi module
def pi(value):
    value = value.removeprefix(":").strip()
    temp = meth.pi
    if value in cVar:
        cVar[value] = float(temp)
    
#meth module
def rmeth(value):
    value = value.removeprefix(":").strip()
    var, range1, range2 = value.split(",")
    range1, range2 = int(range1), int(range2)
    temp = rndm.randint(range1, range2)
    if var in cVar:
        cVar[var] = float(temp)

#module mapping
elements = {'int' : integ,
            'str' : string,
            'print' : mprint,
            'input' : minput,
            'pi' : pi,
             'meth' : rmeth }

# load Sprout program
file = "test.spt" #input("Document to open: ") 
in_if_block = False
run_block = False
skip_until_end = False

try:
    with open(file, "r", encoding="utf-8") as program:
        for raw_line in program:
            line = raw_line.strip()

            if not line or line.startswith("//"):
                continue

            # IF
            if line.startswith("if "):
                cond = line[line.find("(")+1 : line.find(")")]
                result = evaluate_condition(cond)
                in_if_block = True
                run_block = result
                skip_until_end = not result
                continue

            # ELIF
            if line.startswith("elif "):
                if in_if_block:
                    cond = line[line.find("(")+1 : line.find(")")]
                    result = evaluate_condition(cond)
                    if not run_block and result:  # only run if previous didn't run
                        run_block = True
                        skip_until_end = False
                    else:
                        skip_until_end = True
                continue

            # ELSE
            if line.startswith("else"):
                if in_if_block:
                    if not run_block:  # only run if nothing before was true
                        run_block = True
                        skip_until_end = False
                    else:
                        skip_until_end = True
                continue

            # END of block
            if line == ";":
                in_if_block = False
                run_block = False
                skip_until_end = False
                continue

            # Normal statement
            if not skip_until_end:
                # existing keyword handling (int, str, print, input, etc.)
                for key in elements:
                    if line.startswith(key):
                        elements[key](line[len(key):].strip())
                        break
                if "=" in line and not line.startswith(("int", "str")):
                    varName, expr = line.split("=", 1)
                    varName = varName.strip()
                    expr = expr.strip()
                    if isinstance(cVar[varName], (int, float)):
                        cVar[varName] = math(expr)
                    else:
                        cVar[varName] = stringcomb(expr)

except FileNotFoundError:
    print(f"Error: File '{file}' not found.")