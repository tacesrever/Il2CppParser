
import json
data = json.loads(open("output.json", 'rb').read().decode('utf-8'))

def ksearch(words, target):
    for word in words:
        if target.find(word) < 0:
            return 0
    return 1

def printmethod(clz, method):
    thistype = clz["tdf"]
    classname = clz["namespace"] + "." + clz["name"]
    t = method["type"] + " " + classname + "." + method["name"] + "(" + thistype + " this"
    if method["params"]:
        for param in method["params"]:
            t += "," + param["type"] + " " + param.get("name", "")
    t += ");"
    print(hex(method["addr"]), t)

def printclass(clz):
    if clz["methods"]:
        for midx in clz["methods"]:
            printmethod(clz, clz["methods"][midx])

def sf(words):
    words = words.split(' ')
    images = data["images"]
    for image in images:
        classes = images[image]
        for typeidx in classes:
            clz = classes[typeidx]
            if ksearch(words, clz["name"]):
                printclass(clz)
                continue
            if clz["methods"]:
                for midx in clz["methods"]:
                    method = clz["methods"][midx]
                    if ksearch(words, method["name"]):
                        printmethod(clz, method)
def ss(wordoraddr):
    printed = []
    searchmod = 'string'
    if isinstance(wordoraddr, int):
        searchmod = 'addr'
    else:
        wordoraddr = wordoraddr.split(' ')
    for useage in data["useages"]:
        if searchmod == 'addr':
            if int(useage["addr"]) == wordoraddr:
                print(useage)
                break
        elif useage["type"] == "str":
            if  printed.count(useage["addr"]) == 0 and ksearch(wordoraddr, useage['s']):
                print(hex(useage["addr"]), useage['s'])
                printed.append(useage["addr"])