# encoding: utf-8
import idaapi
import json

data = json.loads(open("output.json", 'rb').read().decode('utf-8'))

def SetString(addr, s):
    name = "s_"
    if(len(s) > 10):
        name += s[:10]
    else:
        name += s
    
    idc.set_cmt(addr, s, 1)
    try:
        idc.set_name(addr, name, SN_NOWARN)
    except:
        idc.set_name(addr, "str_" + hex(addr), SN_NOWARN)

def SetName(addr, name):
    ret = idc.set_name(addr, name, SN_NOWARN | SN_NOCHECK)
    if ret == 0:
        new_name = name + '_' + str(addr)
        ret = idc.set_name(addr, new_name, SN_NOWARN | SN_NOCHECK)
        if ret == 0:
            idc.set_cmt(addr, name, 1)

images = data["images"]
for image in images:
    classes = images[image]
    for typeidx in classes:
        clz = classes[typeidx]
        clzname = clz["namespace"] +"." + clz["name"]
        clzname = clzname.replace("<", "_")
        clzname = clzname.replace(">", "_")
        clzname = clzname.replace(":", "_")
        clzname = clzname.replace("`", "_")
        if clz["methods"]:
            thistype = clz["tdf"]
            for midx in clz["methods"]:
                method = clz["methods"][midx]
                typedef = method["type"] + " f(" + thistype + " this"
                name = method["name"].replace("<", "_")
                name = name.replace(">", "_")
                name = name.replace(":", "_")
                name = name.replace("`", "_")
                name = clzname + "." + name
                if method["params"]:
                    for param in method["params"]:
                        typedef += "," + param["type"] + " " + param["name"]
                typedef += ");"
                SetName(method["addr"], str(name))
                if not apply_type(method["addr"], parse_decl(str(typedef), 0), 1):
                    print "apply_type failed:", hex(method["addr"]), typedef
                    idc.set_cmt(method["addr"], str(typedef), 1)
print("funcs done")
useages = data["useages"]
total = len(useages)
print "total", total
i = 0
for useage in useages:
    i += 1
    if(i % 10000 == 0):
        print i, "..."
    
    if(useage["type"] == "class"):
        SetName(useage["addr"], str(useage["name"]))
        apply_type(useage["addr"], parse_decl(r"Il2CppClass*", 0), 1)
    elif(useage["type"] == "type"):
        apply_type(useage["addr"], parse_decl(r"Il2CppType*", 0), 1)
        idc.set_cmt(useage["addr"], str(useage["name"]), 1)
    elif(useage["type"] == "method"):
        SetName(useage["addr"], str(useage["name"]))
        apply_type(useage["addr"], parse_decl(r"MethodInfo*", 0), 1)
        idc.set_cmt(useage["addr"], hex(useage["faddr"]), 1)
    elif(useage["type"] == "field"):
        apply_type(useage["addr"], parse_decl(r"FieldInfo*", 0), 1)
    elif(useage["type"] == "str"):
        SetString(useage["addr"], str(useage["s"].encode("utf-8")))
    