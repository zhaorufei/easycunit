#include <windows.h>
#include <tchar.h>
#include <psapi.h>
#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>
#include <memory>
#include <regex>
#include <iostream>
#include <string>

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x500
#endif

#include <Winternl.h>

#include "VC_ver_specific.h"
#include "ANSI_C_UnitTest.h"

using namespace std;
#pragma comment(lib, "psapi.lib")

HANDLE g_SubProcess_Handle = 0;
HANDLE g_SubProcess_Thread_Handle = 0;

inline static void FreeSubProcess(void)
{
	g_SubProcess_Handle && CloseHandle(g_SubProcess_Handle);
	g_SubProcess_Thread_Handle && CloseHandle(g_SubProcess_Thread_Handle);
}
static void safe_exit(int code)
{
	FreeSubProcess();
	exit(code);
}

typedef NTSTATUS (NTAPI *pfnNtQueryInformationProcess)(
    IN  HANDLE ProcessHandle,
    IN  PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN  ULONG ProcessInformationLength,
    OUT PULONG ReturnLength    OPTIONAL
    );

struct MyFixedPEB {
	BYTE Reserved1[2];
	BYTE BeingDebugged;
	BYTE Reserved2Before[5];
	VOID * BaseAddress;
	BYTE Reserved2[229 - sizeof(VOID*) - 5];
	PVOID Reserved3[59];
	ULONG SessionId;
} fixed_peb;

inline static  
BOOL SafeReadProcessMemory( HANDLE hProcess, LPCVOID lpBaseAddress,
  LPVOID lpBuffer, SIZE_T nSize, SIZE_T* lpNumberOfBytesRead)
{
	*lpNumberOfBytesRead = 0;
	BOOL ret = ReadProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesRead);
	if( ! ret || *lpNumberOfBytesRead != nSize) {
		fprintf(stderr, "Fail to ReadProcessMemory(%p, %p, %p, %d, ...) or read size != requested size.\n"
				, hProcess, lpBaseAddress, lpBuffer, nSize);
		safe_exit(1);
	}

	return TRUE;
}

static void * GetEntryPoint(HANDLE hProcess, const void * base_addr)
{
	assert( hProcess );
	assert( base_addr );
	IMAGE_DOS_HEADER DOS_header;
	IMAGE_NT_HEADERS NT_header;
	SIZE_T dwBytesRead = 0;

	SafeReadProcessMemory(hProcess, base_addr, &DOS_header, sizeof(DOS_header), &dwBytesRead);
	SafeReadProcessMemory(hProcess, (LPCVOID)( (DWORD)base_addr + DOS_header.e_lfanew),
			   	&NT_header, sizeof(NT_header), &dwBytesRead);
	// printf("AddressOfEntryPoint: %p\n", NT_header.OptionalHeader.AddressOfEntryPoint);
	return (void*)  ((DWORD)base_addr + NT_header.OptionalHeader.AddressOfEntryPoint);
}

enum SubSystem { NotSupported, Console, Windows };
static SubSystem GetSubSystem(HANDLE hProcess, const void * base_addr)
{
	assert( hProcess );
	assert( base_addr );
	IMAGE_DOS_HEADER DOS_header;
	IMAGE_NT_HEADERS NT_header;
	SIZE_T dwBytesRead = 0;

	SafeReadProcessMemory(hProcess, base_addr, &DOS_header, sizeof(DOS_header), &dwBytesRead);
	SafeReadProcessMemory(hProcess, (LPCVOID)( (DWORD)base_addr + DOS_header.e_lfanew),
			   	&NT_header, sizeof(NT_header), &dwBytesRead);
	switch(NT_header.OptionalHeader.Subsystem ) {
		case IMAGE_SUBSYSTEM_WINDOWS_CUI:
			return Console;
		case IMAGE_SUBSYSTEM_WINDOWS_GUI:
			return Windows;
		default:
			return NotSupported;
	}
}

static void *  GetProcessBaseLoadAddress(HANDLE hProcess)
{
    HMODULE hNtDll = LoadLibrary(_T("ntdll.dll"));
    if(hNtDll == NULL) {
		fprintf(stderr, "Fail to load ntdll.dll\n");
		safe_exit(1);
	}

    pfnNtQueryInformationProcess fn = (pfnNtQueryInformationProcess)GetProcAddress(hNtDll,
                                                        "NtQueryInformationProcess");
    if(fn == NULL) {
		fprintf(stderr, "Fail to NtQueryInformationProcess from ntdll.dll\n");
        FreeLibrary(hNtDll);
		safe_exit(1);
    }

	PROCESS_BASIC_INFORMATION pbi = {0};
	ULONG dwSizeNeeded = 0;
	NTSTATUS dwStatus = fn(hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), &dwSizeNeeded);
	if(dwStatus < 0) {
		fprintf(stderr, "call NtQueryInformationProcess from ntdll.dll failed\n");
		safe_exit(1);
	}

	if(pbi.PebBaseAddress)
	{
		SIZE_T dwBytesRead = 0;
		PEB peb = {0};
		MyFixedPEB fixed_peb;
		extern char not_exist[ ( sizeof(peb) == sizeof(fixed_peb) ) ? 1 : -1];
		// offset of 8 is get from windbg
		// the faked data structure is changed from structure PEB
		// assert( offsetof(fixed_peb, BaseAddress) == 8 );
		SafeReadProcessMemory(hProcess, pbi.PebBaseAddress, &fixed_peb, sizeof(peb), &dwBytesRead);
		return fixed_peb.BaseAddress;
	} else {
		fprintf(stderr, "process basic information->PebBaseAddress = NULL\n");
		safe_exit(1);
		return 0;
	}
}

static SupportedVisualCppVersion GetLinkerVersion(HANDLE hProcess)
{
	assert( hProcess );
	DWORD base_addr = (DWORD)GetProcessBaseLoadAddress(hProcess);
	assert( base_addr );

	IMAGE_DOS_HEADER DOS_header;
	IMAGE_NT_HEADERS NT_header;
	SIZE_T dwBytesRead = 0;
	SafeReadProcessMemory(hProcess, (LPCVOID)base_addr, &DOS_header, sizeof(DOS_header), &dwBytesRead);
	SafeReadProcessMemory(hProcess, (LPCVOID)(base_addr + DOS_header.e_lfanew), &NT_header
			, sizeof(NT_header), &dwBytesRead);
	bool is_VC6 = (NT_header.OptionalHeader.MajorLinkerVersion == 6
			&& NT_header.OptionalHeader.MinorLinkerVersion == 0) ;
	bool is_VC2003 = (NT_header.OptionalHeader.MajorLinkerVersion == 7
			&& NT_header.OptionalHeader.MinorLinkerVersion == 10) ;
	bool is_VC2005 = (NT_header.OptionalHeader.MajorLinkerVersion == 8
			&& NT_header.OptionalHeader.MinorLinkerVersion == 0) ;
	bool is_VC2008 = (NT_header.OptionalHeader.MajorLinkerVersion == 9
			&& NT_header.OptionalHeader.MinorLinkerVersion == 0) ;
	bool is_VC2010 = (NT_header.OptionalHeader.MajorLinkerVersion == 10
			&& NT_header.OptionalHeader.MinorLinkerVersion == 0) ;
	if( is_VC6 ) { return VC6;}
	else if( is_VC2003 ) { return VC2003;}
	else if( is_VC2005) { return VC2005;}
	else if( is_VC2008) { return VC2008;}
	else if( is_VC2010) { return VC2010;}
	else { return UnknownLinkerVersion; };
}

/// @brief: check the linker's version, only support VC6, VC2003,
///         VC2005, VC2008, VC2010
static bool is_linker_supported(HANDLE hProcess)
{
	assert( hProcess );
	DWORD base_addr = (DWORD)GetProcessBaseLoadAddress(hProcess);
	assert( base_addr );

	IMAGE_DOS_HEADER DOS_header;
	IMAGE_NT_HEADERS NT_header;
	SIZE_T dwBytesRead = 0;
	SafeReadProcessMemory(hProcess, (LPCVOID)base_addr, &DOS_header, sizeof(DOS_header), &dwBytesRead);
	SafeReadProcessMemory(hProcess, (LPCVOID)(base_addr + DOS_header.e_lfanew), &NT_header
			, sizeof(NT_header), &dwBytesRead);

	bool is_VC6 = (NT_header.OptionalHeader.MajorLinkerVersion == 6
			&& NT_header.OptionalHeader.MinorLinkerVersion == 0) ;
	bool is_VC2003 = (NT_header.OptionalHeader.MajorLinkerVersion == 7
			&& NT_header.OptionalHeader.MinorLinkerVersion == 10) ;
	bool is_VC2005 = (NT_header.OptionalHeader.MajorLinkerVersion == 8
			&& NT_header.OptionalHeader.MinorLinkerVersion == 0) ;
	bool is_VC2008 = (NT_header.OptionalHeader.MajorLinkerVersion == 9
			&& NT_header.OptionalHeader.MinorLinkerVersion == 0) ;
	bool is_VC2010 = (NT_header.OptionalHeader.MajorLinkerVersion == 10
			&& NT_header.OptionalHeader.MinorLinkerVersion == 0) ;
	if( !is_VC6 && !is_VC2003 && !is_VC2005 && !is_VC2008 && !is_VC2010 ) {
		fprintf(stderr, "Your linker version is %d.%d, Only VC6/VC200[358]/VC2010 supported(linker version 6.0/7.10/8.0/9.0/10.0)\n"
				, NT_header.OptionalHeader.MajorLinkerVersion , NT_header.OptionalHeader.MinorLinkerVersion );
		return false;
	}
	return true;
}

/// @brief:
static void * GetAnotherProcAddress(HANDLE hProcess, const char * func_name)
{
	assert( hProcess );
	DWORD base_addr = (DWORD)GetProcessBaseLoadAddress(hProcess);
	assert( base_addr );

	IMAGE_DOS_HEADER DOS_header;
	IMAGE_NT_HEADERS NT_header;
	SIZE_T dwBytesRead = 0;
	SafeReadProcessMemory(hProcess, (LPCVOID)base_addr, &DOS_header, sizeof(DOS_header), &dwBytesRead);
	SafeReadProcessMemory(hProcess, (LPCVOID)(base_addr + DOS_header.e_lfanew), &NT_header
			, sizeof(NT_header), &dwBytesRead);

	const int fixed_export_table_idx = 0;
	assert( NT_header.OptionalHeader.NumberOfRvaAndSizes > 0);
	IMAGE_DATA_DIRECTORY & exp_entry = NT_header.OptionalHeader.DataDirectory[fixed_export_table_idx];
	DWORD exp_table_rva              = exp_entry.VirtualAddress;
	DWORD exp_table_size             = exp_entry.Size;
	// printf("exp table rva: %p, size: %d\n", exp_table_rva, exp_table_size);
	// exit(1);

	// Read export directory
	IMAGE_EXPORT_DIRECTORY exp_dir = {0};
	if( exp_table_size < sizeof(exp_dir) ) {
		fprintf(stderr, "Fail to find unit test stub function [%s]'s addr, exp table size: %d\n"
				, func_name, exp_table_size);
		return NULL;
	}
	assert( exp_table_size >= sizeof(exp_dir) );
	SafeReadProcessMemory(hProcess, (LPCVOID)(base_addr + exp_table_rva), &exp_dir, sizeof(exp_dir), &dwBytesRead);

	auto_ptr<DWORD> func_name_table        ( new DWORD[exp_dir.NumberOfNames]);
	auto_ptr<WORD> func_name_ordinal_table ( new WORD[exp_dir.NumberOfNames]);
	auto_ptr<DWORD> func_addr_table        ( new DWORD [exp_dir.NumberOfFunctions]);

	SafeReadProcessMemory(hProcess, (LPCVOID)(base_addr + exp_dir.AddressOfNames), func_name_table.get(), 
			exp_dir.NumberOfNames * sizeof(DWORD), &dwBytesRead);
	SafeReadProcessMemory(hProcess, (LPCVOID)(base_addr + exp_dir.AddressOfNameOrdinals)
			, func_name_ordinal_table.get(), exp_dir.NumberOfNames * sizeof(WORD), &dwBytesRead);
	SafeReadProcessMemory(hProcess, (LPCVOID)(base_addr + exp_dir.AddressOfFunctions), func_addr_table.get(), 
			exp_dir.NumberOfNames * sizeof(DWORD), &dwBytesRead);

	const int name_len = strlen(func_name);
	auto_ptr<char> func_name_str ( new char[ name_len + 1]);
#if 0
	printf("name: %p, ordinal: %p, addr: %p, str: %p, name_len = %d\n"
			, func_name_table.get()
			, func_name_ordinal_table.get()
			, func_addr_table.get(), func_name_str.get()
			, name_len);
#endif

	for(DWORD i = 0; i < exp_dir.NumberOfNames; ++i) {
		// printf("function names: %p\n", func_name_table.get()[i] );
		SafeReadProcessMemory(hProcess, (LPCVOID)((DWORD)base_addr + func_name_table.get()[i])
				, func_name_str.get(), name_len, &dwBytesRead);
		if(strncmp(func_name, func_name_str.get(), name_len) == 0) {
			WORD ord_num = func_name_ordinal_table.get()[i];
			// printf("Function Address: %p\n", base_addr + func_addr_table.get()[ord_num]);
			return (void*) ((DWORD)base_addr + func_addr_table.get()[ord_num]);
		}
	}
	fprintf(stderr, "Fail to find unit test stub function [%s]'s addr\n", func_name);
	return NULL;
}

/// @return: the address of the sub-process's _tmainCRTStartup function
/// @param hProcess: the monitored process
/// @param entry_point: for hProcess
/// @note: Compiler version specific opcode analyze
static void * get_tmainStartup_address(HANDLE hProcess, LPCVOID entry_point)
{
	SIZE_T read_bytes = 0;
	// E9 B8 04 00 00    jmp  wmainCRTStartup(xxxx)
	unsigned char jmp_inst[5] = {0};
	unsigned char bp_sp_stack[] = { 0x55, 0x8b, 0xEC };
	// the address in monitored process to Read next instruction
	DWORD read_ptr = (DWORD) entry_point;
	// VC2003's EntryPoint itself is the main/wmain/WinMain/wWinMain
	// function
	SupportedVisualCppVersion linker_ver = GetLinkerVersion(hProcess);
	if( linker_ver == VC2003 || linker_ver == VC6) {
		return (void*)entry_point;
	}
	SafeReadProcessMemory(hProcess, (LPVOID)read_ptr , jmp_inst, sizeof(jmp_inst), &read_bytes);
	// printf("%x  %x , %x,  %x, %x\n" , jmp_inst[0] , jmp_inst[1] , jmp_inst[2] , jmp_inst[3] , jmp_inst[4]);
	assert( strncmp((const char*)jmp_inst, (const char*)bp_sp_stack, sizeof(bp_sp_stack) ) == 0
			|| jmp_inst[0] == 0xE9 || jmp_inst[0] == 0xE8);

	// If the first instruction is [push ebp], skip it
	if( jmp_inst[0] == bp_sp_stack[0] ) {
		read_ptr += sizeof(bp_sp_stack);
		SafeReadProcessMemory(hProcess, (LPCVOID) read_ptr, jmp_inst, sizeof(jmp_inst), &read_bytes);
		assert( jmp_inst[0] == 0xE9 || jmp_inst[0] == 0xE8);
	}
	read_ptr += sizeof(jmp_inst);

	long tmainCRTStartup_addr = 0;
	if( jmp_inst[0] == 0xE8 ) {
		// A 5-bytes call instruction
		//    193:         __security_init_cookie();
		// 004015AD E8 54 2E 00 00   call        __security_init_cookie (404406h) 
		//    194: 
		//    195:         return __tmainCRTStartup();
		// 004015B2 E9 40 FE FF FF   jmp         __tmainCRTStartup (4013F7h) 
		read_bytes = 0;
		SafeReadProcessMemory(hProcess, (LPVOID)read_ptr, jmp_inst, sizeof(jmp_inst), &read_bytes);
		assert( jmp_inst[0] == 0xE9 || jmp_inst[0] == 0xE8 );
		read_ptr += sizeof(jmp_inst);
		tmainCRTStartup_addr = read_ptr + *(long*)&jmp_inst[1];
	} else {
		long jmp_target = read_ptr + * (long *)&jmp_inst[1];
		// printf("wmainCRTStartup: %p\n", jmp_target);

		// 55              push ebp
		// 8B EC           mov ebp, esp
		// E8 13 FB FF FF  call __security_init_cookie
		// E8 13 00 00 00  call __tmainCRTStartup
		unsigned char func_body_wmainCRTStartup[13] = {0};
		read_bytes = 0;
		SafeReadProcessMemory(hProcess, (LPVOID)jmp_target
				,func_body_wmainCRTStartup, sizeof(func_body_wmainCRTStartup), &read_bytes);
		assert( read_bytes == sizeof(func_body_wmainCRTStartup) );
		assert( func_body_wmainCRTStartup[0] == 0x55 );
		assert( func_body_wmainCRTStartup[1] == 0x8B );
		assert( func_body_wmainCRTStartup[2] == 0xEC );
		assert( func_body_wmainCRTStartup[3] == 0xE8 );
		assert( func_body_wmainCRTStartup[8] == 0xE8 );
		tmainCRTStartup_addr = (long)jmp_target + 1 + 2 + 5 + *(long*)&func_body_wmainCRTStartup[8] + 5;
	}
	return (void*)tmainCRTStartup_addr;
}

static void * GetConsoleMainPatchAddress(const unsigned char * ins_begin, const unsigned char * ins_end )
{
	// 00401551 E8 B4 FA FF FF   call        @ILT+5(_main) (40100Ah) 
	// 00401556 83 C4 0C         add         esp,0Ch 
	static unsigned char add_esp_XXh[] = {0x83, 0xC4};
	// For VC2005, 2008 and 2010, the instruction is 0x83, 0xC4, 0xC
	// For VC2003, the instruction is 0x83, 0xC4, 0x14.  Because there's
	// an optimization that add stack size back for two consecutive
	// call instruction. because main has at most 3 parameters, so at
	// least 12 bytes is required for the stack
	const unsigned char  at_least_stacksize = 0xC;
	const unsigned char * ptr               = ins_begin;
	void * call_main_insn                   = NULL;
	while(1) {
		// printf("start: %p, len: %d", ptr, ins_end - ptr);
		unsigned char * e8 = (unsigned char *)memchr(ptr, 0xe8, ins_end - ptr);
		if(!e8) break;
		if( ins_end > e8 + 5 + sizeof(add_esp_XXh) + 1 ) {
			// printf(", found: %p( %x, %x, %x, %x, %x )\n", e8, e8[0], e8[1], e8[2], e8[3], e8[4]);
			const unsigned char stack_size = *(e8 + 5 + sizeof(add_esp_XXh) );
			bool is_add_esp_insn = memcmp(e8 + 5, add_esp_XXh, sizeof(add_esp_XXh)) == 0;
			if( is_add_esp_insn && stack_size >= at_least_stacksize  ) {
				call_main_insn = e8 + 1;
				break;
			}
		}
		ptr = e8 + 1;
		if(ptr >= ins_end) {
			fprintf(stderr, "Fail to find instruction pattern: call main, add esp, 0xCH\n");
			break;
		}
	}
	return call_main_insn;
}

/// @brief: replace the main/WinMain function with the exported unit
///         test stub function whose address is ut_main
/// @note: Compiler version specific opcode analyze, it's very nasty and
///        hardcoded
bool fix_main_addr(PROCESS_INFORMATION * procInfo, void * ut_main )
{
	void * base_addr = GetProcessBaseLoadAddress(procInfo->hProcess);
	assert( base_addr );

	// printf("Base Loaded Address: %p\n", base_addr);
	void * EntryPoint = GetEntryPoint(procInfo->hProcess, base_addr);
	// printf("EntryPoint : %p\n", EntryPoint);

	long tmainCRTStartup_addr = (long)get_tmainStartup_address(procInfo->hProcess, EntryPoint);
	printf("tmainCRTStartup: %p\n", tmainCRTStartup_addr);

	// Try to read 1K to find the call main instruction
	unsigned char func_body[1024] = {0};
	SIZE_T read_bytes = 0;
	SafeReadProcessMemory(procInfo->hProcess, (LPVOID)tmainCRTStartup_addr,
			func_body, sizeof(func_body), &read_bytes);
	assert( read_bytes == sizeof(func_body) );
	unsigned char * ptr = func_body;
	bool found = false;
	const unsigned char * ins_end = func_body + sizeof(func_body);
	// printf("try to found main:\n");
	const unsigned char * call_main_insn = NULL;

	SubSystem sub_sys = GetSubSystem(procInfo->hProcess, base_addr);
	SupportedVisualCppVersion vc_ver = GetLinkerVersion(procInfo->hProcess);
	auto_ptr<CallMainProcessorBase> call_main_proc(CreateCallMainProcessor(vc_ver, sub_sys == Windows ) );
	call_main_insn = (unsigned char *) call_main_proc->get_call_main_patch_address(func_body, ins_end);
	assert( call_main_insn );

	// printf("found main\n");
	SIZE_T write_bytes = 0;
	void * main_addr = (void*) (tmainCRTStartup_addr + (call_main_insn  - func_body));
	printf("replace call main/WinMain: %p with %p\n", main_addr,  ut_main);
	long next_ins_addr = (long)main_addr + 4;
	long addr_delta = (long)ut_main - next_ins_addr;
	BOOL write_ret = WriteProcessMemory(procInfo->hProcess, (LPVOID*) main_addr, &addr_delta, 4, &write_bytes);
	assert( write_ret );
	assert( write_bytes == 4);
	return true;
}

static void set_args(char * opt, string & command_line)
{
	if( ! opt ) return;

	assert( strncmp("/t:", opt, 3) == 0);
	opt += sizeof("/t:") - 1;

	string all_cases(opt);
	for(int i = 0; i < all_cases.length(); ++i)
	{
		if(all_cases[i] == ',')
			all_cases[i] = ' ';
	}
	command_line += " ";
	command_line += all_cases;
}

static void test_raw_CRTstartup_code(SupportedVisualCppVersion ver, bool is_WinMain)
{
	// Get all the *.cpu files
	printf("Test will read all the *.cpu files at current directory. Then try to\n"
			"Parse it as mainCRTStartup code to find the call main instruction\n");
	int sys_ret = system("dir /b *.cpu > tmp.files");
	assert( sys_ret == 0 || sys_ret == 1);

	FILE * fp_list = fopen("tmp.files", "r");
	assert( fp_list );

	auto_ptr<CallMainProcessorBase> call_main_proc(CreateCallMainProcessor(ver, is_WinMain) );

	char line_buf[1024] = {0};
	while ( fgets(line_buf, sizeof(line_buf) - 1, fp_list) ) {
		int len = strcspn(line_buf, "\r\n");
		assert( len > 0);
		line_buf[ len ] = 0;
		printf("Test file [%s]\n", line_buf);

		FILE * fp = fopen(line_buf, "rb");
		assert( fp );

		struct stat file_stat;
		fstat( fileno(fp), &file_stat);

		char * data = new char[ file_stat.st_size ];
		int n_rec = fread(data, file_stat.st_size, 1, fp);
		assert( file_stat.st_size == 0 || n_rec == 1);

		void * offset = call_main_proc->get_call_main_patch_address( (unsigned char *) data
				, (unsigned char *)data + file_stat.st_size );
		if( offset == NULL) {
			fprintf(stderr, "Not Found!!\n");
		}
		fclose(fp);
		delete[] data;
		memset(line_buf, 0, sizeof(line_buf) );
	}

	fclose(fp_list);
}

UNIT_TEST(vc2003_main)
{
	test_raw_CRTstartup_code( VC2003, false);
}

UNIT_TEST(vc2003_WinMain)
{
	test_raw_CRTstartup_code( VC2003, true);
}

int main(int argc, char* argv[])
{
	if( argc != 2 && argc != 3) {
		fprintf(stderr, "usage: %s /t:case1,case2 program", argv[0]);
		exit(1);
	}
	char * prog_to_test = (argc == 2) ? argv[1] : argv[2];
	char * opt = (argc == 2) ? NULL : argv[1];

	string command_line(prog_to_test);
	set_args(opt, command_line);
	STARTUPINFO stinfo;
	PROCESS_INFORMATION proinfo = {0};
	ZeroMemory(&stinfo,sizeof(stinfo));
	ZeroMemory(&proinfo,sizeof(proinfo));

	// printf ("Trying to create the process...");
	// the CREATE_SUSPEND flag to stop the process after creation.
	printf("command line: [%s]\n", command_line.c_str() );
	bool res = CreateProcess(prog_to_test, const_cast<char*>(command_line.c_str()), NULL, NULL,
			NULL, CREATE_SUSPENDED, NULL, NULL, &stinfo, &proinfo);
	if (res == false)
	{
		printf ("ERROR: Creating the Process [%s] failed!\n", prog_to_test);
		return (0);
	}
	g_SubProcess_Handle = proinfo.hProcess;
	g_SubProcess_Thread_Handle = proinfo.hThread;

	if( ! is_linker_supported(proinfo.hProcess) ) safe_exit(1);
	// The name should be exactly match the exported function name in
	// ANSI_C_UnitTest.c
	void * base_addr = GetProcessBaseLoadAddress(proinfo.hProcess);
	SubSystem sub_sys = GetSubSystem(proinfo.hProcess, base_addr);
	const char * unit_test_export_func = (sub_sys == Console)
		? "unit_test_main" : "unit_test_WinMain";

	void * ut_main = (void*)GetAnotherProcAddress(proinfo.hProcess, unit_test_export_func);
	bool fix_ok = false;
	if(ut_main) fix_ok = fix_main_addr(&proinfo, ut_main);

	if (fix_ok == false)
	{
		printf ("ERROR: " "Can't patch the Memory. Killing [%s] instance...", prog_to_test);
		fflush(stdout);
		TerminateProcess(proinfo.hProcess, 0);
		printf ("DONE\n");
	} else {
		printf("Resuming process [%s].\n", prog_to_test);
		void * var_addr = GetAnotherProcAddress(g_SubProcess_Handle, "g_unit_test_cases");
		struct UnitTestCases all_cases = {0, 0, 1};
		DWORD read_bytes = 0;

		ResumeThread(proinfo.hThread);

		// read the global data periodically, till the sub-process call
		// all the register functions before main/WinMain, thus the
		// num_of_cases_ will be set to 0 at least. The initial value is
		// -1.
		if( opt == NULL) {
			while ( 1 ) {
				SafeReadProcessMemory(g_SubProcess_Handle, var_addr, &all_cases, sizeof(all_cases), &read_bytes);
				if( all_cases.reg_done_ ) {
					printf("%p, num: %d\n", var_addr, all_cases.num_of_cases_);
					break;
				}
				Sleep(50);
			}

			for(int i = 0; i < all_cases.num_of_cases_; ++i) {
				int len = all_cases.test_cases_[i].name_len_;
				auto_ptr<char> name_buf ( new char[len + 1] );
				name_buf.get()[ len ] = 0;
				SafeReadProcessMemory(g_SubProcess_Handle, 
						all_cases.test_cases_[i].name_, name_buf.get(), len, &read_bytes);
				printf("unit test case: [%s]\n", name_buf.get() );
			}
			int exit_flag = 1;
			void * exit_flag_addr = GetAnotherProcAddress(g_SubProcess_Handle, "can_exit_flag");
			WriteProcessMemory(g_SubProcess_Handle, exit_flag_addr, &exit_flag, sizeof(exit_flag), &read_bytes);
		}
		FreeSubProcess();
	}

	return 0;
}
