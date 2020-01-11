

#include <cstdio>
#include <cstdarg>
#include <link.h>

#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <set>
#include <locale>
#include <codecvt>

#include "nlohmann/json.hpp"
#include "utf8.h"

#include "il2cpp-object-internals.h"
#include "il2cpp-class-internals.h"
#include "il2cpp-tabledefs.h"

// for vs IDE
#ifdef _MSVC_LANG
#define EXPORT
#else
#define EXPORT __attribute__((visibility("default")))
#endif

using nlohmann::json;
using std::string;
using std::endl;

std::ofstream headerfile;
std::ofstream jsonfile;

size_t il2cppBase;

Il2CppMetadataRegistration* s_Il2CppMetadataRegistration;
Il2CppImage* s_ImagesTable;
size_t s_ImagesCount;
size_t s_GlobalMetadata;
Il2CppMetadataUsagePair* MetadataUsagePairs;
size_t MetadataUsagePairsCount;

Il2CppString* (*GetStringLiteralFromIndex)(size_t index);

Il2CppClass* (*GetTypeInfoFromTypeIndex)(size_t index);

Il2CppClass* (*GetTypeInfoFromTypeDefinitionIndex)(size_t index);

Il2CppClass* (*Class_InitFromCodegen)(Il2CppClass* clz);

MethodInfo* (*GetMethodInfoFromIndex)(size_t encodedmidx);

//EXPORT FieldInfo* (*GetFieldInfoFromIndex)(size_t encodedidx);
extern "C" {
EXPORT void (*parser_log)(const char* msg);
EXPORT void init(const char* addrfile);
EXPORT void dumpAll(const char* filename, const char* headername);
}

void _parser_log(const char* fmt, ...) {
	if (parser_log) {
		va_list args;
		char msg[1024];
		va_start(args, fmt);
		vsnprintf(msg, sizeof(msg) - 1, fmt, args);
		va_end(args);
		parser_log(msg);
	}
}
json il2cppaddrs;

int dl_iterate_cb(struct dl_phdr_info *info, size_t size, void *data) {
	string name = info->dlpi_name;
	if (name.find("libil2cpp.so") != name.npos) {
		size_t taddr;
		_parser_log("find %s base:%p", name.c_str(), info->dlpi_addr);
		il2cppBase = (size_t)info->dlpi_addr;
		taddr = il2cppaddrs["s_Il2CppMetadataRegistration"].get<size_t>() + il2cppBase;
		s_Il2CppMetadataRegistration = *(Il2CppMetadataRegistration**)taddr;
		taddr = il2cppaddrs["s_ImagesTable"].get<size_t>() + il2cppBase;
		s_ImagesTable = *(Il2CppImage**)taddr;
		taddr = il2cppaddrs["s_ImagesCount"].get<size_t>() + il2cppBase;
		s_ImagesCount = *(size_t*)taddr;
		taddr = il2cppaddrs["s_GlobalMetadata"].get<size_t>() + il2cppBase;
		s_GlobalMetadata = *(size_t*)taddr;

		taddr = il2cppaddrs["MetadataUsagePairsOffset"].get<size_t>() + s_GlobalMetadata;
		MetadataUsagePairs = (Il2CppMetadataUsagePair*)(s_GlobalMetadata + *(size_t*)taddr);
		taddr = il2cppaddrs["MetadataUsagePairsCountOffset"].get<size_t>() + s_GlobalMetadata;
		MetadataUsagePairsCount = (*(size_t*)taddr) / sizeof(Il2CppMetadataUsagePair);
		
		taddr = il2cppaddrs["GetStringLiteralFromIndex"].get<size_t>() + il2cppBase;
		GetStringLiteralFromIndex = (Il2CppString * (*)(size_t))taddr;
		taddr = il2cppaddrs["GetTypeInfoFromTypeIndex"].get<size_t>() + il2cppBase;
		GetTypeInfoFromTypeIndex = (Il2CppClass * (*)(size_t))taddr;
		taddr = il2cppaddrs["GetTypeInfoFromTypeDefinitionIndex"].get<size_t>() + il2cppBase;
		GetTypeInfoFromTypeDefinitionIndex = (Il2CppClass * (*)(size_t))taddr;
		taddr = il2cppaddrs["Class_InitFromCodegen"].get<size_t>() + il2cppBase;
		Class_InitFromCodegen = (Il2CppClass * (*)(Il2CppClass*))taddr;
		taddr = il2cppaddrs["GetMethodInfoFromIndex"].get<size_t>() + il2cppBase;
		GetMethodInfoFromIndex = (MethodInfo * (*)(size_t))taddr;
	}
	return 0;
}


void init(const char* addrfile) {
	std::ifstream addrfstream(addrfile);
	addrfstream >> il2cppaddrs;
	dl_iterate_phdr(dl_iterate_cb, nullptr);
}

void dealStaticField(Il2CppClass* classinfo, size_t fieldidx) {
	// TODO
}

void fixCDefName(string& name) {
	size_t pos = name.find("`");
	if (pos != name.npos) {
		size_t epos = name.find("_", pos);
		name.erase(pos, epos - pos);
	}
	for (pos = name.find("."); pos != name.npos; pos = name.find(".")) {
		name[pos] = '_';
	}
	for (pos = name.find(":"); pos != name.npos; pos = name.find(":")) {
		name[pos] = '_';
	}
	for (pos = name.find("<"); pos != name.npos; pos = name.find("<")) {
		name[pos] = '_';
	}

	for (pos = name.find(">"); pos != name.npos; pos = name.find(">")) {
		name[pos] = '_';
	}
}

string getStructName(size_t clzindex, Il2CppClass* classinfo) {
	Il2CppClass* tclz = classinfo;
	while (tclz) {
		Class_InitFromCodegen(tclz);
		for (size_t fidx = 0; fidx < tclz->field_count; ++fidx) {
			auto field = tclz->fields[fidx];
			if (field.offset == THREAD_STATIC_FIELD_OFFSET || field.type->attrs & FIELD_ATTRIBUTE_STATIC || field.offset == 0)
				continue;
			else {
				std::stringstream sstream;
				sstream << classinfo->namespaze << "_" << classinfo->name << "_" << clzindex;
				string classname = sstream.str();
				fixCDefName(classname);
				return classname;
			}
		}
		tclz = tclz->parent;
	}
	return "Il2CppObject";
}

string getClassName(Il2CppClass* classinfo) {
	std::stringstream sstream;
	sstream << classinfo->namespaze << "_" << classinfo->name;
	string classname = sstream.str();
	fixCDefName(classname);
	return classname;
}

bool fieldCompare(FieldInfo* a, FieldInfo* b) {
	return a->offset < b->offset;
}

std::set<std::string> predefs;
// Il2CppType to C struct defname
string parseType(const Il2CppType* type, size_t* vsize, size_t* arrsize) {
	string strtype;
	size_t tmpsize;
	size_t clzidx;
	Il2CppClass* clz;
	*arrsize = 0;
	switch (type->type)
	{
	case IL2CPP_TYPE_END:
	case IL2CPP_TYPE_VOID:
		strtype = "void";
		*vsize = 0;
		break;
	case IL2CPP_TYPE_I1:
	case IL2CPP_TYPE_U1:
	case IL2CPP_TYPE_BOOLEAN:
		strtype = "uint8_t";
		*vsize = 1;
		break;
	case IL2CPP_TYPE_I2:
	case IL2CPP_TYPE_U2:
	case IL2CPP_TYPE_CHAR:
		strtype = "uint16_t";
		*vsize = 2;
		break;
	case IL2CPP_TYPE_I4:
	case IL2CPP_TYPE_U4:
		strtype = "uint32_t";
		*vsize = 4;
		break;
	case IL2CPP_TYPE_I8:
	case IL2CPP_TYPE_U8:
		strtype = "uint64_t";
		*vsize = 8;
		break;
	case IL2CPP_TYPE_R4:
		strtype = "float";
		*vsize = sizeof(float);
		break;
	case IL2CPP_TYPE_R8:
		strtype = "double";
		*vsize = sizeof(double);
		break;
	case IL2CPP_TYPE_STRING:
		strtype = "Il2CppString*";
		*vsize = sizeof(Il2CppString*);
		break;
	case IL2CPP_TYPE_PTR:
		strtype = parseType(type->data.type, &tmpsize, &tmpsize) + "*";
		*vsize = sizeof(void*);
		break;
	case IL2CPP_TYPE_BYREF:
		_parser_log("IL2CPP_TYPE_BYREF");
		break;
	case IL2CPP_TYPE_VALUETYPE:
		clz = GetTypeInfoFromTypeDefinitionIndex(type->data.klassIndex);
		Class_InitFromCodegen(clz);
		if (clz->enumtype) {
			strtype = parseType(clz->element_class->byval_arg, vsize, &tmpsize);
			/*if (tmpsize) {
				_parser_log("array in valuetype");
			}*/
			break;
		}
		else {
			strtype = getStructName(type->data.klassIndex, clz);
			if (predefs.find(strtype) == predefs.end()) {
				// use empty definiton here instead of declare
				// for ida's `parse C header file`, struct allow redefine,
				// but can't unsee definiton of struct in 20 errors.

				//headerfile << "struct " << strtype << ";\n";
				headerfile << "typedef struct " << strtype << " {} " << strtype << ";\n";
				predefs.emplace(strtype);
			}
			*vsize = clz->instance_size - sizeof(Il2CppObject);
			break;
		}
		break;
	case IL2CPP_TYPE_CLASS:
		clz = GetTypeInfoFromTypeDefinitionIndex(type->data.klassIndex);
		Class_InitFromCodegen(clz);
		strtype = getStructName(type->data.klassIndex, clz) + "_obj";
		if (predefs.find(strtype) == predefs.end()) {
			//headerfile << "struct " << strtype << ";\n";
			headerfile << "typedef struct " << strtype << " {} " << strtype << ";\n";
			predefs.emplace(strtype);
		}
		strtype += "*";
		*vsize = sizeof(void*);
		break;
	case IL2CPP_TYPE_VAR:
		strtype = "Il2CppObject*";
		*vsize = sizeof(Il2CppObject*);
		break;
	case IL2CPP_TYPE_MVAR:
		strtype = "Il2CppObject*";
		*vsize = sizeof(Il2CppObject*);
		break;
	case IL2CPP_TYPE_ARRAY:
	case IL2CPP_TYPE_SZARRAY:
		//size_t esize;
		//etype = parseType(type->data.array->etype, &esize, &tmpsize);
		//*vsize = esize * type->data.array->rank;
		//*arrsize = type->data.array->rank;
		strtype = "Il2CppArray*";
		*vsize = sizeof(void*);
		break;
	case IL2CPP_TYPE_GENERICINST:
		clzidx = type->data.generic_class->typeDefinitionIndex;
		clz = GetTypeInfoFromTypeDefinitionIndex(clzidx);
		Class_InitFromCodegen(clz);
		if (clz->valuetype) {
			if (clz->enumtype) {
				strtype = parseType(clz->element_class->byval_arg, vsize, &tmpsize);
				break;
			}
			else {
				strtype = getStructName(clzidx, clz);
				if (predefs.find(strtype) == predefs.end()) {
					//headerfile << "struct " << strtype << ";\n";
					headerfile << "typedef struct " << strtype << " {} " << strtype << ";\n";
					predefs.emplace(strtype);
				}
				*vsize = clz->instance_size - sizeof(Il2CppObject);
				break;
			}
		}
		else {
			strtype = getStructName(clzidx, clz) + "_obj";
			if (predefs.find(strtype) == predefs.end()) {
				//headerfile << "struct " << strtype << ";\n";
				headerfile << "typedef struct " << strtype << " {} " << strtype << ";\n";
				predefs.emplace(strtype);
			}
			strtype += "*";
			*vsize = sizeof(void*);
		}
		break;
	case IL2CPP_TYPE_TYPEDBYREF:
		//_parser_log("IL2CPP_TYPE_TYPEDBYREF");
		strtype = "Il2CppObject*";
		*vsize = sizeof(void*);
		break;
	case IL2CPP_TYPE_I:
	case IL2CPP_TYPE_U:
		strtype = "uint*";
		*vsize = sizeof(void*);
		break;
	case IL2CPP_TYPE_FNPTR:
		_parser_log("[!] IL2CPP_TYPE_FNPTR");
		break;
	case IL2CPP_TYPE_OBJECT:
		strtype = "Il2CppObject*";
		*vsize = sizeof(void*);
		break;
	case IL2CPP_TYPE_CMOD_REQD:
		_parser_log("[!] IL2CPP_TYPE_CMOD_REQD");
		break;
	case IL2CPP_TYPE_CMOD_OPT:
		_parser_log("[!] IL2CPP_TYPE_CMOD_OPT");
		break;
	case IL2CPP_TYPE_INTERNAL:
		_parser_log("[!] IL2CPP_TYPE_INTERNAL");
		break;
	case IL2CPP_TYPE_MODIFIER:
		_parser_log("[!] IL2CPP_TYPE_MODIFIER");
		break;
	case IL2CPP_TYPE_SENTINEL:
		_parser_log("[!] IL2CPP_TYPE_SENTINEL");
		break;
	case IL2CPP_TYPE_PINNED:
		_parser_log("[!] IL2CPP_TYPE_PINNED");
		break;
	case IL2CPP_TYPE_ENUM:
		_parser_log("[!] IL2CPP_TYPE_ENUM");
		break;
	}
	if (type->byref) {
		strtype += "*";
		*vsize = sizeof(void*);
	}
	return strtype;
}

// add all fields for class and parent classes.
void addfields(std::vector<FieldInfo*>& fields, Il2CppClass* classinfo) {
	Class_InitFromCodegen(classinfo);
	if(classinfo->parent) addfields(fields, classinfo->parent);
	for (size_t fidx = 0; fidx < classinfo->field_count; ++fidx) {
		auto field = classinfo->fields[fidx];
		if (field.offset == THREAD_STATIC_FIELD_OFFSET 
			|| field.type->attrs & FIELD_ATTRIBUTE_STATIC 
			// why sometime offset == 0?
			|| field.offset == 0)
			dealStaticField(classinfo, fidx);
		else
			fields.push_back(&classinfo->fields[fidx]);
	}
}

void genClassDef(size_t index, Il2CppClass* classinfo) {
	std::vector<FieldInfo*> fields;
	addfields(fields, classinfo);
	if (fields.size() == 0) return;
	
	string struname = getStructName(index, classinfo);
	std::sort(fields.begin(), fields.end(), fieldCompare);
	std::ostringstream def;
	def << std::hex;
	def << "typedef struct " << struname << endl;
	def << "{\n";
	//_parser_log("classdef %s", struname.c_str());
	size_t current_offset = 0;
	size_t padidx = 0;
	size_t preoffset = 0xffffffff;
	for (auto field : fields) {
		if (field->offset == preoffset) {
			continue;
			// TODO: union here?
		}
		preoffset = field->offset;
		if (field->offset > current_offset + sizeof(Il2CppObject)) {
			size_t padlen = field->offset - current_offset - sizeof(Il2CppObject);
			def << "\tuint8_t padding_" << padidx << "[0x" << padlen << "];\n";
			padidx++;
			current_offset = field->offset - sizeof(Il2CppObject);
		}
		if (field->type->type == IL2CPP_TYPE_VOID) continue;
		
		size_t vsize;
		size_t arrsize;
		string fieldType = parseType(field->type, &vsize, &arrsize);
		string fieldName = field->name;
		fixCDefName(fieldName);
		def << "\t" << fieldType << " " << fieldName << "_" << field->offset;
		current_offset += vsize;
		if (arrsize) {
			def << "[0x" << arrsize << "]";
		}
		def << ";\n";
	}
	def << "} " << struname << ";\n";

	def << "typedef struct " << struname << "_obj\n";
	def << "{\n";
	def << "\tIl2CppObject object;\n";
	def << "\t" << struname << " fields;\n";
	def << "} " << struname << "_obj;\n";
	headerfile << def.str();
	headerfile.flush();
	predefs.emplace(struname);
	predefs.emplace(struname + "_obj");
}

void dumpAll(const char* filename, const char* headername) {
	json root;
	json images;
	size_t tsize;
	Il2CppImage* image = s_ImagesTable;

	headerfile.open(headername);
	headerfile << "#include \"il2cpp-object-internals.h\"" << endl;
	headerfile << "#include \"il2cpp-class-internals.h\"" << endl;
	headerfile << "#define Il2CppObject_obj Il2CppObject" << endl;
	headerfile << endl;
	predefs.emplace("Il2CppObject");
	predefs.emplace("Il2CppObject_obj");

	_parser_log("[+] dumping classes...");
	for (size_t imageidx = 0; imageidx < s_ImagesCount; ++imageidx, ++image) {
        _parser_log("[+] dumping classes for image %s (%d:%d)...", image->name, image->typeStart, image->typeCount);
		json classes;
		for (size_t typeidx = image->typeStart; typeidx < image->typeStart + image->typeCount; ++typeidx) {
			Il2CppClass* il2cppclass = GetTypeInfoFromTypeDefinitionIndex(typeidx);
			Class_InitFromCodegen(il2cppclass);
			
			json classtype;
			classtype["name"] = il2cppclass->name;
			classtype["namespace"] = il2cppclass->namespaze;
			classtype["methods"] = {};
			classtype["tdf"] = getStructName(typeidx, il2cppclass) + "_obj*";
			
			genClassDef(typeidx, il2cppclass);

			for (size_t midx = 0; midx < il2cppclass->method_count; ++midx) {
				const MethodInfo* method = il2cppclass->methods[midx];
				classtype["methods"][std::to_string(midx)] = {
					{"name", method->name},
					{"addr", ((size_t)method->methodPointer) - il2cppBase},
					{"type", parseType(method->return_type, &tsize, &tsize)}
				};

				json parameters;
				for (size_t pidx = 0; pidx < method->parameters_count; ++pidx) {
					auto param = method->parameters[pidx];
					parameters[param.position] = {
						{"type", parseType(param.parameter_type, &tsize, &tsize)},
						{"name", param.name}
					};
				}
				classtype["methods"][std::to_string(midx)]["params"] = std::move(parameters);
			}
			classes[std::to_string(typeidx)] = std::move(classtype);
		}
		images[image->name] = std::move(classes);
	}
	_parser_log("[+] dump classes fin.");
	_parser_log("[+] dumping useage... total: %d", MetadataUsagePairsCount);
	json useage;
	Il2CppClass* clz;
	Il2CppType* type;
	MethodInfo* minfo;
	FieldInfo* fieldinfo;
	string u8s;
	std::u16string u16s;
	for (size_t i = 0; i < MetadataUsagePairsCount; ++i) {
		if (i % 50000 == 0) {
			_parser_log("%u...", i);
		}
		uint32_t destinationIndex = MetadataUsagePairs->destinationIndex;
		uint32_t encodedSourceIndex = MetadataUsagePairs->encodedSourceIndex;
		MetadataUsagePairs++;
		Il2CppMetadataUsage usage = GetEncodedIndexType(encodedSourceIndex);
		uint32_t decodedIndex = GetDecodedMethodIndex(encodedSourceIndex);
		uint32_t symaddr = (uint32_t)s_Il2CppMetadataRegistration->metadataUsages[destinationIndex];
		symaddr -= il2cppBase;
		useage[i] = {};
		switch (usage) {
		case kIl2CppMetadataUsageTypeInfo:
			clz = GetTypeInfoFromTypeIndex(decodedIndex);
			useage[i]["type"] = "class";
			useage[i]["name"] = getClassName(clz);
			useage[i]["addr"] = symaddr;
			break;
		case kIl2CppMetadataUsageIl2CppType:
			type = (Il2CppType*)s_Il2CppMetadataRegistration->types[decodedIndex];
			useage[i]["type"] = "type";
			useage[i]["name"] = parseType(type, &tsize, &tsize);
			useage[i]["addr"] = symaddr;
			break;
		case kIl2CppMetadataUsageMethodDef:
		case kIl2CppMetadataUsageMethodRef:
			minfo = GetMethodInfoFromIndex(encodedSourceIndex);
			useage[i]["type"] = "method";
			useage[i]["name"] = getClassName(minfo->declaring_type) + "." + minfo->name;
			useage[i]["faddr"] = (size_t)minfo->methodPointer - il2cppBase;
			useage[i]["addr"] = symaddr;
			break;
		case kIl2CppMetadataUsageFieldInfo:
			//fieldinfo = GetFieldInfoFromIndex(decodedIndex);
			useage[i]["type"] = "field";
			//useage[i]["name"] = getClassName(fieldinfo->parent) + "." + fieldinfo->name;
			useage[i]["addr"] = symaddr;
			break;
		case kIl2CppMetadataUsageStringLiteral:
			useage[i]["type"] = "str";
			Il2CppString* il2cppstr = GetStringLiteralFromIndex(decodedIndex);
			u16s.assign((char16_t*)il2cppstr->chars, il2cppstr->length);
			u8s = "";
			utf8::utf16to8(u16s.begin(), u16s.end(), std::back_inserter(u8s));
			useage[i]["s"] = u8s;
			useage[i]["addr"] = symaddr;
			break;
		}
	}
	_parser_log("[+] dump useage fin.");
	root["images"] = std::move(images);
	root["useages"] = std::move(useage);
	_parser_log("[+] writing to file...");
	try {
		jsonfile.open(filename);
		jsonfile << root;
		jsonfile.close();
	}
	catch (nlohmann::detail::exception& e) {
		_parser_log("Json Exception: %s", e.what());
	}
	
	_parser_log("[+] done.");
}