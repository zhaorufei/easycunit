#ifndef VC_VER_SPECIFIC_H_
#define VC_VER_SPECIFIC_H_ 1

class CallMainProcessorBase {
	public:
		void * get_call_main_patch_address(const unsigned char * ins_begin, const unsigned char * ins_end );
	private:
		virtual int * get_valid_offset(int & len) = 0;
		virtual int get_call_main_offset(char * data, int len) = 0;
};

enum SupportedVisualCppVersion {
	UnknownLinkerVersion,
	VC2003,
	VC2005,
	VC2008,
	VC2010,
	VC6,
};

CallMainProcessorBase * CreateCallMainProcessor(SupportedVisualCppVersion ver, bool is_WinMain);
#endif
