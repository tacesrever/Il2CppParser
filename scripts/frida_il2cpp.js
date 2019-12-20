
//warpper for NativeFunction, add 'string' type.
function makefunction(liborAddr, name, retType, argList, options) {
    var faddr;
    if (liborAddr == null || typeof liborAddr == 'string') {
        faddr = Module.findExportByName(liborAddr, name);
    } else faddr = ptr(liborAddr);
    if(!faddr) {
        console.log("[+] makefunction failed to find faddr for", name);
        return null;
    }
    var argType = [];
    var nativef;
    for(var i in argList) {
        if(argList[i] == 'string') argType.push('pointer');
        else argType.push(argList[i]);
    }
    if(retType == 'string') nativef = new NativeFunction(faddr, 'pointer', argType, options);
    else nativef = new NativeFunction(faddr, retType, argType, options);
    
    return function() {
        var args = [];
        for(var i in arguments) {
            if(argList[i] == 'string' && typeof arguments[i] == 'string') {
                args.push(Memory.allocUtf8String(arguments[i]));
            }
            else args.push(arguments[i]);
        }
        var retVal = nativef.apply(this, args);
        if(retType == 'string') {
            retVal = retVal.readCString();
        }
        return retVal;
    }
}

var _dlopen = makefunction(null, 'dlopen', 'pointer', ['string', 'int']);
var dlerror = makefunction(null, 'dlerror', 'string', []);
function dlopen(dlname) {
    const RTLD_LOCAL=0, RTLD_LAZY=1, RTLD_NOW=2, RTLD_NOLOAD=4, RTLD_DEEPBIND=8;
    const RTLD_GLOBAL=0x100, RTLD_NODELETE=0x1000;
    if(!_dlopen) return null;
    var ret = _dlopen(dlname, RTLD_NOW);
    if(ret.isNull()) console.log(dlerror());
    else log("[+] loaded:", dlname);
    return ret;
}

function il2cpp_dump(addrfile, output) {
    const libpath = "/data/local/tmp/libparser.so";
    const libname = "libparser.so";
    
    const il2cpp = Process.findModuleByName("libil2cpp.so");
    dlopen(libpath);
    var parser_init = makefunction(libname, "init", 'pointer', ['string']);
    var dumpAll = makefunction(libname, "dumpAll", 'pointer', ['string', 'string']);
    var parser_log = new NativeCallback(function() {
        console.log(arguments[0].readCString());
    }, 'void', ['pointer']);
    var t = Module.findExportByName(libname, "parser_log");
    t.writePointer(parser_log);
    
    parser_init(addrfile);
    dumpAll(output+'.json', output+'.h');
}