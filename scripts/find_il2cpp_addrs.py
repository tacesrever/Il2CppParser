# encoding: utf-8
# for arm(armeabi-v7a) only
import traceback, json
import idaapi, idc, idautils
# https://github.com/tmr232/Sark
import sark

s_Il2CppMetadataRegistration = 0
s_ImagesTable = 0
s_ImagesCount = 0
s_GlobalMetadata = 0
MetadataUsagePairsOffset = 0
MetadataUsagePairsCountOffset = 0

GetTypeInfoFromTypeIndex = 0
GetTypeInfoFromTypeDefinitionIndex = 0
GetStringLiteralFromIndex = 0
Class_InitFromCodegen = 0
GetMethodInfoFromIndex = 0

def showAll():
    print "finded addrs:"
    print "s_Il2CppMetadataRegistration = il2appbase +", hex(s_Il2CppMetadataRegistration)
    print "s_ImagesTable = il2appbase +", hex(s_ImagesTable)
    print "s_ImagesCount = il2appbase +", hex(s_ImagesCount)
    print "s_GlobalMetadata = il2appbase +", hex(s_GlobalMetadata)
    print "MetadataUsagePairsOffset =", hex(MetadataUsagePairsOffset)
    print "MetadataUsagePairsCountOffset =", hex(MetadataUsagePairsCountOffset)
    print "GetStringLiteralFromIndex = il2appbase +", hex(GetStringLiteralFromIndex)
    print "GetTypeInfoFromTypeIndex = il2appbase +", hex(GetTypeInfoFromTypeIndex)
    print "GetTypeInfoFromTypeDefinitionIndex = il2appbase +", hex(GetTypeInfoFromTypeDefinitionIndex)
    print "Class_InitFromCodegen = il2appbase +", hex(Class_InitFromCodegen)
    print "GetMethodInfoFromIndex = il2appbase +", hex(GetMethodInfoFromIndex)

def doOutput():
    result = {}
    result["s_Il2CppMetadataRegistration"] = s_Il2CppMetadataRegistration
    result["s_ImagesTable"] = s_ImagesTable
    result["s_ImagesCount"] = s_ImagesCount
    result["s_GlobalMetadata"] = s_GlobalMetadata
    result["MetadataUsagePairsOffset"] = MetadataUsagePairsOffset
    result["MetadataUsagePairsCountOffset"] = MetadataUsagePairsCountOffset
    result["GetStringLiteralFromIndex"] = GetStringLiteralFromIndex
    result["GetTypeInfoFromTypeIndex"] = GetTypeInfoFromTypeIndex
    result["GetTypeInfoFromTypeDefinitionIndex"] = GetTypeInfoFromTypeDefinitionIndex
    result["Class_InitFromCodegen"] = Class_InitFromCodegen
    result["GetMethodInfoFromIndex"] = GetMethodInfoFromIndex
    with open("il2cpp_addrs.json", 'w') as f:
        json.dump(result, f)

def getNamedFunc(name):
    func = sark.Function(name=name)
    if func:
        # if func just jump to another, return jump target.
        for line in func.lines:
            for xref in line.xrefs_from:
                func = sark.Function(xref.to)
                break
            break
    return func

def findMetadataCacheInitialize():
    def checkTarget(func):
        #check write to global values' count.
        hitcount = 0
        for xref in func.xrefs_from:
            segment = idaapi.getseg(xref.to)
            if idaapi.get_visible_segm_name(segment) == '.bss' and repr(xref.type) == "Data_Write":
                hitcount += 1
        if hitcount >= 8 and hitcount < 12:
            return 1
        return 0
    
    # find addr of "global-metadata.dat"
    global_metadata = None
    s = idaapi.string_info_t()
    for i in range(0, idaapi.get_strlist_qty()):
        idaapi.get_strlist_item(s, i)
        if idaapi.get_ascii_contents(s.ea, s.length, s.type) == "global-metadata.dat":
            global_metadata = s.ea
            break
    
    # xref of "global-metadata.dat"
    for xref in sark.Line(global_metadata).xrefs_to:
        if sark.Function.is_function(xref.frm):
            target_func = sark.Function(xref.frm)
            if checkTarget(target_func):
                # print "find MetadataCache::Initialize at", hex(int(target_func.startEA))
                idc.set_name(target_func.startEA, "MetadataCache_Initialize", SN_NOWARN | SN_NOCHECK)
                return
            else:
                for txref in target_func.xrefs_to:
                    if sark.Function.is_function(txref.frm):
                        caller = sark.Function(txref.frm)
                        if checkTarget(caller):
                            # print "find MetadataCache::Initialize at", hex(int(caller.startEA))
                            idc.set_name(caller.startEA, "MetadataCache_Initialize", SN_NOWARN | SN_NOCHECK)
                            return
    print "can't find MetadataCache_Initialize"
    
def findGlobalsByMetadataCacheInitialize():
    fInitialize = getNamedFunc("MetadataCache_Initialize")
    writeOrders = [
        's_GlobalMetadata',
        's_GlobalMetadataHeader',
        's_TypeInfoTable',
        's_TypeInfoDefinitionTable',
        's_MethodInfoDefinitionTable',
        's_GenericMethodTable',
        's_ImagesCount',
        's_ImagesTable',
        's_AssembliesCount',
        's_AssembliesTable'
    ]
    i = 0
    for xref in fInitialize.xrefs_from:
        segment = idaapi.getseg(xref.to)
        if idaapi.get_visible_segm_name(segment) == '.bss' and repr(xref.type) == "Data_Write":
            name = writeOrders[i]
            # print "find", name, "at", hex(int(xref.to))
            idc.set_name(xref.to, name, SN_NOWARN | SN_NOCHECK)
            globals()[name] = int(xref.to)
            i += 1
            if i >= 10:
                break

def findGetTypeInfoFromTypeDefinitionIndex():
    global GetTypeInfoFromTypeDefinitionIndex
    func = getNamedFunc("il2cpp_type_get_class_or_element_class")
    if func:
        for xref in func.xrefs_from:
            refname = sark.core.get_name_or_address(xref.to)
            if refname.find("il2cpp_class_from_il2cpp_type") < 0:
                # print "find GetTypeInfoFromTypeDefinitionIndex at", hex(int(xref.to))
                idc.set_name(xref.to, "GetTypeInfoFromTypeDefinitionIndex", SN_NOWARN | SN_NOCHECK)
                GetTypeInfoFromTypeDefinitionIndex = int(xref.to)

def findMetadataRegistration():
    global s_Il2CppMetadataRegistration
    fgetType = getNamedFunc("GetTypeInfoFromTypeDefinitionIndex")
    readRefCount = {}
    for xref in fgetType.xrefs_from:
        segment = idaapi.getseg(xref.to)
        if idaapi.get_visible_segm_name(segment) == '.bss' and repr(xref.type) == "Data_Read":
            if readRefCount.get(xref.to, -1) == -1:
                readRefCount[xref.to] = 1
            else:
                readRefCount[xref.to] += 1
    
    for ref in readRefCount:
        if readRefCount[ref] > 4:
            # print "find s_Il2CppMetadataRegistration at", hex(int(ref))
            idc.set_name(ref, "s_Il2CppMetadataRegistration", SN_NOWARN | SN_NOCHECK)
            s_Il2CppMetadataRegistration = int(ref)
            return
    
    print "can't find s_Il2CppMetadataRegistration"

def findInitializeMethodMetadata():
    fgetType = getNamedFunc("GetTypeInfoFromTypeDefinitionIndex")
    finit = getNamedFunc("MetadataCache_Initialize")
    refcounts = {}
    for xref in sark.Line(s_Il2CppMetadataRegistration).xrefs_to:
        if sark.Function.is_function(xref.frm):
            func = sark.Function(xref.frm)
            if func.startEA == fgetType.startEA or func.startEA == finit.startEA:
                continue
            if refcounts.get(func.startEA, -1) == -1:
                refcounts[func.startEA] = 1
            else:
                refcounts[func.startEA] += 1
    
    # find function IntializeMethodMetadataRange here.
    # IntializeMethodMetadataRange usually inlined into InitializeMethodMetadata.
    for faddr in refcounts:
        if refcounts[faddr] >= 5:
            # print "find MetadataCache::InitializeMethodMetadata at", hex(int(faddr))
            idc.set_name(faddr, "InitializeMethodMetadata", SN_NOWARN | SN_NOCHECK)
            return
    
    print "Can't find MetadataCache::InitializeMethodMetadata."
    
def findMetadataUsagePairs():
    global MetadataUsagePairsOffset
    global MetadataUsagePairsCountOffset
    finituseage = getNamedFunc("InitializeMethodMetadata")
    for line in finituseage.lines:
        if idaapi.print_insn_mnem(line.ea) == 'B':
            # B     loc_??
            target = line.insn.operands[0].op_t.addr
            # LDR   R0, [R2,#0x??]
            insn = sark.Line(target).insn
            MetadataUsagePairsOffset = int(insn.operands[1].offset)
            # print "find MetadataUsagePairsOffset:", hex(MetadataUsagePairsOffset)
            MetadataUsagePairsCountOffset = MetadataUsagePairsOffset + 4
            return
    print "can't find MetadataUsagePairsOffset"
    
def findGetTypeInfoFromTypeIndex():
    global GetTypeInfoFromTypeIndex
    finit = getNamedFunc("MetadataCache_Initialize")
    for xref in sark.Line(s_TypeInfoTable).xrefs_to:
        if sark.Function.is_function(xref.frm):
            func = sark.Function(xref.frm)
            if func.startEA == finit.startEA:
                continue
            # print "find findGetTypeInfoFromTypeIndex at", hex(int(func.startEA))
            idc.set_name(func.startEA, "GetTypeInfoFromTypeIndex", SN_NOWARN | SN_NOCHECK)
            GetTypeInfoFromTypeIndex = int(func.startEA)
            return

def findClass_InitFromCodegen():
    global Class_InitFromCodegen
    fgetType = getNamedFunc("GetTypeInfoFromTypeIndex")
    fgetClass = getNamedFunc("il2cpp_class_from_il2cpp_type")
    for xref in fgetType.xrefs_from:
        if xref.to == fgetClass.startEA:
            continue
        if sark.Function.is_function(xref.to):
            # print "find Class::InitFromCodegen at", hex(int(xref.to))
            idc.set_name(xref.to, "Class_InitFromCodegen", SN_NOWARN | SN_NOCHECK)
            Class_InitFromCodegen = int(xref.to)
            return
    print "can't find Class::InitFromCodegen"
    
def findFuncsByInitializeMethodMetadata():
    global GetMethodInfoFromIndex
    global GetStringLiteralFromIndex
    finituseage = getNamedFunc("InitializeMethodMetadata")
    
    def findFirstCall(startAddr, endAddr):
        while 1:
            line = sark.Line(startAddr)
            for xref in line.xrefs_from:
                if repr(xref.type) != "Ordinary_Flow" and sark.Function.is_function(xref.to):
                    return xref.to
            startAddr += len(line.bytes)
            if target >= endAddr:
                break
        return None
    
    for line in finituseage.lines:
        switch_info = idaapi.get_switch_info_ex(line.ea)
        if switch_info:
            case3 = idc.Dword(switch_info.jumps + 2*4)
            case5 = idc.Dword(switch_info.jumps + 4*4)
            case6 = idc.Dword(switch_info.jumps + 5*4)
            if case3 == case6:
                target = switch_info.jumps + case3
                taddr = findFirstCall(target, finituseage.endEA)
                if taddr:
                    # print "find GetMethodInfoFromIndex at", hex(int(taddr))
                    idc.set_name(taddr, "GetMethodInfoFromIndex", SN_NOWARN | SN_NOCHECK)
                    GetMethodInfoFromIndex = int(taddr)
            target = switch_info.jumps + case5
            taddr = findFirstCall(target, finituseage.endEA)
            if taddr:
                # print "find GetStringLiteralFromIndex at", hex(int(taddr))
                idc.set_name(taddr, "GetStringLiteralFromIndex", SN_NOWARN | SN_NOCHECK)
                GetStringLiteralFromIndex = int(taddr)
            break
    
def findAll():
    findMetadataCacheInitialize()
    findGlobalsByMetadataCacheInitialize()
    findGetTypeInfoFromTypeDefinitionIndex()
    findGetTypeInfoFromTypeIndex()
    findMetadataRegistration()
    findClass_InitFromCodegen()
    findInitializeMethodMetadata()
    findMetadataUsagePairs()
    findFuncsByInitializeMethodMetadata()

findAll()
showAll()
doOutput()