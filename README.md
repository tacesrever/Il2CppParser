# Il2CppParser  
libil2cpp symbol parser for ida.  

# Useage  

## prepare  

### find libil2cpp  
firstly you should find target's internal libil2cpp headerfile version,  
by analsys libil2cpp.so or try every finded version by github or etc.  
once you find it, replace libil2cpp.  

### fix libil2cpp for ida  
copy libil2cpp to another path for ida load only;  
add `__ANDROID__=1;__arm__=1;` to `Options->Compiler... Predefined macros`;  
add `{your_ndk_toolchain_path}/sysroot/usr/include` to `Options->Compiler... include`;  
add libil2cpp for ida to `Options->Compiler... include`;  
remove all include <...> from il2cpp-class-internals.h and il2cpp-object-internals.h,  
try `File->Load file->Parse C header file...` to load il2cpp-class-internals.h and il2cpp-object-internals.h,  
if error happend, just locate and fix or remove it.  

### install sark  
see https://github.com/tmr232/Sark   
  
## make
edit CMakeLists.txt, setup ndk toolchain path;  
run `mkdir build; cd build; cmake ..;make`;  
push build/libparser.so on your device;  
  
## use  
open libil2cpp.so(arm) with ida;  
wait for ida's thinking is done(sometimes it take a while);  
Click `File->Script file...` or press `Alt+F7`, run `scripts/find_il2cpp_addrs.py`,  
`find_il2cpp_addrs.py` will generate `il2cpp_addrs.json` at same path with libil2cpp.so;  
push `il2cpp_addrs.json` on your device;  
  
open app, let it load libparser.so, and run these after unity has started:  

    libparser.parser_init(il2cpp_addrs.json);  
    libparser.dumpAll(jsonoutpath, headeroutpath);  
for frida, you can `use scripts/frida_il2cpp.js` at this step.  

pull headerfile from headeroutpath, put it in libil2cpp for ida,  
Click File->Load file->Parse C header file... or press Ctrl+F9, load headerfile.  

pull jsonfile from jsonoutpath, put it at same path with libil2cpp.so,rename it to `output.json`;  
run `scripts/load_symbols.py`, it may take a long time.  

then enjoy your reversing with full structs and symbols info.  