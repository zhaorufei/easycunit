#include <assert.h>
#include <memory>
#include <regex>
#include <iostream>
#include <algorithm>
#include <string>

#include "VC_ver_specific.h"

using namespace std;

void * CallMainProcessorBase::get_call_main_patch_address(const unsigned char * ins_begin
		, const unsigned char * ins_end )
{
	int offset = get_call_main_offset((char*) ins_begin, ins_end - ins_begin);

	// assert the offset is correct
	int len = 0;
	int * valid_offset = get_valid_offset(len);
	assert(valid_offset);
	assert( len > 0);

	int * result = find(valid_offset , valid_offset + len, offset);
	assert(result != valid_offset + len );

	return (void*) (ins_begin + offset);
}

class VC6_Call_main_Processor: public CallMainProcessorBase
{
	private:
		virtual int get_call_main_offset(char * data, int len) 
		{
			// Generate the following regex to recognize the call _main
			// instruction
			static char re_str[] =  
				"\x50"                        // push eax
				"(\xFF\x35(.|\\r|\\n){4}){2}" // call dword ptr [***]; submatach 1, 2
				"\xE8((.|\\r|\\n){4})"        // call _main; submach (3), 4
				"\x83\xC4\x0C"                // add esp, 0Ch
				"|"
				"\x50"                        // push eax
				"\x8B\x0D(.|\\r|\\n){4}"      // mov  ecx, dword ptr [XXX]; submatch 5
				"\x51"                        // push eax
				"\xE8((.|\\r|\\n){4})"        // call _main; submach (6), 7
				"\x83\xC4\x0C"                // add esp, 14h
				"|"
				"\x51"                        // push ecx
				"\x8B\x55\xE4"                // mov edx, dword ptr[ebp-1Ch]; submatach *,*
				"\x52"                        // push edx
				"\xE8((.|\\r|\\n){4})"        // call _main; submach (8), 9
				"\x83\xC4\x0C"                // add esp, 0Ch
				"|"
				"\xFF\x75\xE0"                // push dword ptr [ebp-20h]
				"\xFF\x75\xD4"                // push dword ptr [ebp-2Ch]
				"\xFF\x75\xE4"                // push dword ptr [ebp-1Ch]
				"\xE8((.|\\r|\\n){4})"        // call _main; submach (10), 11
				"\x83\xC4\x30"                // add esp, 0Ch
				;
			static tr1::regex pat(re_str, _countof(re_str) - 1);
			tr1::cmatch matches;
			if (tr1::regex_search((const char*)data, (const char*)data + len, matches, pat) )
			{
				// puts("Found");
				// printf("type: [%s]\n", typeid(matches[0]).name() );
				int idx = matches.position(0);
				if( data[idx] == '\x50' ) {
					return  matches.position( data[idx+1] == '\xFF' ? 3 : 6 ) ;
				} else if( data[idx] == '\x51' ) {
					return matches.position(8) ;
				} else {
					return matches.position(10) ;
				}
			}
			return -1;
			return NULL;
		}

		virtual int * get_valid_offset(int & len)
		{
			static int verified_main_offset[] = {
				0xB0, 0xAF, 0xE5, 0xE4, 0xC1, 0xC0,
				0xF8, 0xF7, 0xDF, 0xDF, 0xFB, 0xFB,
			};
			len = _countof(verified_main_offset);
			return verified_main_offset;
		}
};

class VC6_Call_WinMain_Processor: public CallMainProcessorBase
{
	private:
		virtual int get_call_main_offset(char * data, int len) 
		{
			// Generate the following regex to recognize the call _main
			// instruction
			static char re_str[] =  
				"(\x6A\x00|[\x56\x53])"   // push 0; push esi; push ebx; submatch 1
				"\xFF\x15(.|\\r|\\n){4}"  // call dword ptr [***]; submatch 2
				"\x50" // push eax
				"\xE8((.|\\r|\\n){4})" // call _WinMain@16; submatch (3), 4
				"\x89\x45[\xA0\x98]"  // mov dword ptr [ebp-60h]; 68h
				"|"
				"\xFF(\xD7|\xD3)\x50\xE8((.|\\r|\\n){4})\x8B(\xF0|\xF8)"; // submatch 4, (5), 6, 7
				;
			static tr1::regex pat(re_str, _countof(re_str) - 1);
			tr1::cmatch matches;
			if (tr1::regex_search((const char*)data, (const char*)data + len, matches, pat) )
			{
				// puts("Found");
				// printf("type: [%s]\n", typeid(matches[0]).name() );
				int idx = matches.position(0);
				if( data[idx] == '\x50' ) {
					return  matches.position( data[idx+1] == '\xFF' ? 3 : 6 ) ;
				} else if( data[idx] == '\x51' ) {
					return matches.position(8) ;
				} else {
					return matches.position(10) ;
				}
			}
			return -1;
			return NULL;
		}

		virtual int * get_valid_offset(int & len)
		{
			static int verified_main_offset[] = {
				0x0CA, 0x0C9, 0x10F, 0x10E, 0x0DC, 0x0DB,
				0x122, 0x121, 0x130, 0x157, 0x1AF, 0x1CD,
			};
			len = _countof(verified_main_offset);
			return verified_main_offset;
		}
};

class VC2003_Call_main_Processor: public CallMainProcessorBase
{
	private:
		virtual int get_call_main_offset(char * data, int len)
		{
			static char re_str[] =  
				"\x51"                        // push ecx
				"\x8B\x55\xE0"                // mov edx, [ebp-20h]
				"\x52"                        // push edx
				"\xE8((.|\\r|\\n){4})"        // call _main; submach (1), 2
				"\x83\xC4\x0C"                // add esp, 0Ch
				"|"
				"\xFF\x75\xE0"                // push dword ptr [ebp-20H]
				"\xFF\x75\xD8"                // push dword ptr [ebp-28H]
				"\xFF\x75\xD4"                // push dword ptr [ebp-2CH]
				"\xE8((.|\\r|\\n){4})"        // call _main; submach (3), 4
				"\x83\xC4\x14"                // add esp, 14h
				"|"
				"\x51"                        // push ecx
				"\x8B\x15(.|\\r|\\n){4}"      // call _wmain; submatach(5)
				"\x52"                        // push edx
				"\xE8((.|\\r|\\n){4})"        // call _main; submach (6), 7
				"\x83\xC4\x0C"                // add esp, 0Ch
				"|"
				"\x50"                        // push eax
				"(\xFF\x35(.|\\r|\\n){4}){2}" // call dword ptr [***]; submatach 8, 9
				"\xE8((.|\\r|\\n){4})"        // call _main; submach (10), 11
				"\x83\xC4\x0C"                // add esp, 0Ch
				;
			static tr1::regex pat(re_str, _countof(re_str) - 1);
			tr1::cmatch matches;
			if (tr1::regex_search((const char*)data, (const char*)data + len, matches, pat) )
			{
				// puts("Found");
				// printf("type: [%s]\n", typeid(matches[0]).name() );
				int idx = matches.position(0);
				if( data[idx] == '\x50' ) {
					return matches.position(10);
				} else if( data[idx] == '\xFF' ) {
					return matches.position(3) ;
				} else {
					if( data[idx + 2] == '\x55' ) {
						return matches.position(1) ;
					} else {
						return matches.position(6) ;
					}
				}
			}
			return -1;
		}

		virtual int * get_valid_offset(int & len)
		{
			static int verified_main_offset[] = {
				0x128, 0x128, 0x13F, 0x13F, 0x17E, 0x17F,
				0x167, 0x168, 0x16B, 0x16C, 0x16E, 0x16F,
			};
			len = _countof(verified_main_offset);
			return verified_main_offset;
		}
};

class VC2003_Call_WinMain_Processor: public CallMainProcessorBase
{
	private:
		virtual int get_call_main_offset(char * data, int len)
		{
			static char re_str[] =  
				"\x6A\x00\xFF\x15(.|\\r|\\n){4}\x50\xE8((.|\\r|\\n){4})\x89"  // submach 1, (2), 3
				"|"
				"\xFF(\xD7|\xD3)\x50\xE8((.|\\r|\\n){4})\x8B(\xF0|\xF8)"; // submatch 4, (5), 6, 7
			static tr1::regex pat(re_str, _countof(re_str) - 1);
			tr1::cmatch matches;
			if (tr1::regex_search((const char*)data, (const char*)data + len, matches, pat) )
			{
				// puts("Found");
				// printf("type: [%s]\n", typeid(matches[0]).name() );
				int idx = matches.position(0);
				if( data[idx] == '\x6A' ) {
					return matches.position(2);
				} else {
					return matches.position(5) ;
				}
			}
			return -1;
		}
		virtual int * get_valid_offset(int & len)
		{
			static int verified_WinMain_offset[] = {
				0x17F, 0x180, 0x181, 0x186, 0x187, 0x18F,
				0x190, 0x19C, 0x1A2, 0x1A3, 0x1EE, 0x1F4,
			};
			len = _countof(verified_WinMain_offset);
			return verified_WinMain_offset;
		}
};

class PostVC2003_Call_main_Processor: public CallMainProcessorBase
{
	private:
		virtual int get_call_main_offset(char * data, int len) 
		{
			// Generate the following regex to recognize the call _main
			// instruction
			static char re_str[] =  
				"\x50"                        // push eax
				"(\xFF\x35(.|\\r|\\n){4}){2}" // call dword ptr [***]; submatach 1, 2
				"\xE8((.|\\r|\\n){4})"        // call _main; submach (3), 4
				"\x83\xC4\x0C"                // add esp, 0Ch
				"|"
				"\x50"                        // push eax
				"\x8B\x0D(.|\\r|\\n){4}"      // mov  ecx, dword ptr [XXX]; submatch 5
				"\x51"                        // push eax
				"\xE8((.|\\r|\\n){4})"        // call _main; submach (6), 7
				"\x83\xC4\x0C"                // add esp, 14h
				"|"
				"(\xFF\x35(.|\\r|\\n){4}){3}" // call dword ptr [***]; submatach 8, 9
				"\xE8((.|\\r|\\n){4})"        // call _main; submach (10), 11
				"\x83\xC4\x0C"                // add esp, 0Ch
				"|"
				"\x51"                        // push ecx
				"\x8B\x15(.|\\r|\\n){4}"      // call dword ptr [***]; submatach 12
				"\x52"                        // push edx
				"\xE8((.|\\r|\\n){4})"        // call _main; submach (13), 14
				"\x83\xC4\x0C"                // add esp, 0Ch
				;
			static tr1::regex pat(re_str, _countof(re_str) - 1);
			tr1::cmatch matches;
			if (tr1::regex_search((const char*)data, (const char*)data + len, matches, pat) )
			{
				// puts("Found");
				// printf("type: [%s]\n", typeid(matches[0]).name() );
				int idx = matches.position(0);
				if( data[idx] == '\x50' ) {
					return  matches.position( data[idx+1] == '\xFF' ? 3 : 6 ) ;
				} else if( data[idx] == '\xFF' ) {
					return matches.position(10) ;
				} else {
					return matches.position(13) ;
				}
			}
			return -1;
			return NULL;
		}
		virtual int * get_valid_offset(int & len) = 0;
};

class PostVC2003_Call_WinMain_Processor: public CallMainProcessorBase
{
	private:
		virtual int * get_valid_offset(int & len) = 0;
		virtual int get_call_main_offset(char * data, int len) 
		{
			// Generate the following regex to recognize the call _main
			// instruction
			static char re_str[] =  
				"[\x50\x52\x56]"                // push eax|edx|esi
				"(\x56|\x53|\x6A\x00)"          // push esi; push ebx; push 0; submatch 1
				"\x68(.|\\r|\\n){4}"            // push offset XXX; submatach 2
				"\xE8((.|\\r|\\n){4})"          // call _main; submach (3), 4
				"(\x89\x45[\xE0\xE4\x90]|\xA3|\x89\x85\x78\xFF{3})" // mov dword ptr [ebp - XX], eax; mov dword [tr [XXX], eax
				;
			static tr1::regex pat(re_str, _countof(re_str) - 1);
			tr1::cmatch matches;
			if (tr1::regex_search((const char*)data, (const char*)data + len, matches, pat) )
			{
				// puts("Found");
				// printf("type: [%s]\n", typeid(matches[0]).name() );
				return matches.position(2) ;
			}
			return -1;
			return NULL;
		}
};

class VC2005_Call_main_Processor: public PostVC2003_Call_main_Processor
{
	private:
		virtual int * get_valid_offset(int & len)
		{
			// The following hard-coded offset is get from dumpbin /disasm:bytes
			// on all the possible *crt*.obj files, then analyze the _wmain
			// function
			static int verified_main_offset[] = {
				0x15B,0x15A,0x22F,0x22E,0x10B,0x10B,0x1BB,0x1A2
			};
			len = _countof(verified_main_offset);
			return verified_main_offset;
		}
};

class VC2005_Call_WinMain_Processor: public PostVC2003_Call_WinMain_Processor
{
	private:
		virtual int * get_valid_offset(int & len)
		{
			// The following hard-coded offset is get from dumpbin /disasm:bytes
			// on all the possible *crt*.obj files, then analyze the _wmain
			// function
			static int verified_main_offset[] = {
				0x173, 0x172, 0x2AC, 0x2AB, 0x13C, 0x14C, 0x282, 0x285,
			};
			len = _countof(verified_main_offset);
			return verified_main_offset;
		}
};

class VC2008_Call_main_Processor: public PostVC2003_Call_main_Processor
{
	private:
		virtual int * get_valid_offset(int & len)
		{
			// The following hard-coded offset is get from dumpbin /disasm:bytes
			// on all the possible *crt*.obj files, then analyze the _wmain
			// function
			static int verified_main_offset[] = {
				0x0F7, 0x0F6, 0x113, 0x112, 0x10B, 0x10B, 0x1A4, 0x1A4,
			};
			len = _countof(verified_main_offset);
			return verified_main_offset;
		}
};

class VC2008_Call_WinMain_Processor: public PostVC2003_Call_WinMain_Processor
{
	private:
		virtual int * get_valid_offset(int & len)
		{
			// The following hard-coded offset is get from dumpbin /disasm:bytes
			// on all the possible *crt*.obj files, then analyze the _wmain
			// function
			static int verified_main_offset[] = {
				0x10F, 0x10E, 0x162, 0x161, 0x13C, 0x14C, 0x284, 0x287,
			};
			len = _countof(verified_main_offset);
			return verified_main_offset;
		}
};

class VC2010_Call_main_Processor: public PostVC2003_Call_main_Processor
{
	private:
		virtual int * get_valid_offset(int & len)
		{
			// The following hard-coded offset is get from dumpbin /disasm:bytes
			// on all the possible *crt*.obj files, then analyze the _wmain
			// function
			static int verified_main_offset[] = {
				0x107, 0x107, 0x125, 0x125, 0x11E, 0x11E, 0x1BB, 0x1BB,
			};
			len = _countof(verified_main_offset);
			return verified_main_offset;
		}
};

class VC2010_Call_WinMain_Processor: public PostVC2003_Call_WinMain_Processor
{
	private:
		virtual int * get_valid_offset(int & len)
		{
			// The following hard-coded offset is get from dumpbin /disasm:bytes
			// on all the possible *crt*.obj files, then analyze the _wmain
			// function
			static int verified_main_offset[] = {
				0x116, 0x116, 0x13E, 0x13E, 0x14C, 0x154, 0x25C, 0x239,
			};
			len = _countof(verified_main_offset);
			return verified_main_offset;
		}
};

CallMainProcessorBase * CreateCallMainProcessor(SupportedVisualCppVersion ver, bool is_WinMain)
{
	switch(ver) {
		case VC6:
			return is_WinMain 
				? dynamic_cast<CallMainProcessorBase*>(new VC6_Call_WinMain_Processor())
			   	: dynamic_cast<CallMainProcessorBase*>(new VC6_Call_main_Processor());
		case VC2003:
			return is_WinMain 
				? dynamic_cast<CallMainProcessorBase*>(new VC2003_Call_WinMain_Processor())
			   	: dynamic_cast<CallMainProcessorBase*>(new VC2003_Call_main_Processor());
		case VC2005:
			return is_WinMain
				? dynamic_cast<CallMainProcessorBase*>(new VC2005_Call_WinMain_Processor() )
				: dynamic_cast<CallMainProcessorBase*>(new VC2005_Call_main_Processor());
		case VC2008:
			return is_WinMain 
				? dynamic_cast<CallMainProcessorBase*>(new VC2008_Call_WinMain_Processor() )
				: dynamic_cast<CallMainProcessorBase*>(new VC2008_Call_main_Processor());
		case VC2010:
			return is_WinMain 
				? dynamic_cast<CallMainProcessorBase*>(new VC2010_Call_WinMain_Processor() )
				: dynamic_cast<CallMainProcessorBase*>(new VC2010_Call_main_Processor() );
		default:
			assert( !"Unsupported version, only support VC6/VC2003/2005/2008/2010 at present!" );
			return NULL;
	}
}
