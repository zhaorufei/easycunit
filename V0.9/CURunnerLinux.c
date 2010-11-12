#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <dlfcn.h>
#include <stdbool.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <gelf.h>
#include "ANSI_C_UnitTest.h"

// void *__dso_handle = NULL ;
extern struct UnitTestCases g_unit_test_cases ;

static int list_all_tests(void)
{
    int i = 0;
    puts("Tests:");
    for(i = 0; i < g_unit_test_cases.num_of_cases_; ++i)
    {
        puts(g_unit_test_cases.test_cases_[i].name_);
    }
    return 0;
}

static const char * get_sig_name(int sig_no)
{
    struct {
        int sig_no;
        const char * sig_name;
    } all_sigs[] = {
          { 1, "SIGHUP"       } , { 2, "SIGINT"       } , { 3, "SIGQUIT"      } , { 4, "SIGILL"       } ,
          { 5, "SIGTRAP"      } , { 6, "SIGABRT"      } , { 7, "SIGBUS"       } , { 8, "SIGFPE"       } ,
          { 9, "SIGKILL"      } , { 10, "SIGUSR1"     } , { 11, "SIGSEGV"     } , { 12, "SIGUSR2"     } ,
          { 13, "SIGPIPE"     } , { 14, "SIGALRM"     } , { 15, "SIGTERM"     } , { 16, "SIGSTKFLT"   } ,
          { 17, "SIGCHLD"     } , { 18, "SIGCONT"     } , { 19, "SIGSTOP"     } , { 20, "SIGTSTP"     } ,
          { 21, "SIGTTIN"     } , { 22, "SIGTTOU"     } , { 23, "SIGURG"      } , { 24, "SIGXCPU"     } ,
          { 25, "SIGXFSZ"     } , { 26, "SIGVTALRM"   } , { 27, "SIGPROF"     } , { 28, "SIGWINCH"    } ,
          { 29, "SIGIO"       } , { 30, "SIGPWR"      } , { 31, "SIGSYS"      } , { 34, "SIGRTMIN"    } ,
          { 35, "SIGRTMIN+1"  } , { 36, "SIGRTMIN+2"  } , { 37, "SIGRTMIN+3"  } , { 38, "SIGRTMIN+4"  } ,
          { 39, "SIGRTMIN+5"  } , { 40, "SIGRTMIN+6"  } , { 41, "SIGRTMIN+7"  } , { 42, "SIGRTMIN+8"  } ,
          { 43, "SIGRTMIN+9"  } , { 44, "SIGRTMIN+10" } , { 45, "SIGRTMIN+11" } , { 46, "SIGRTMIN+12" } ,
          { 47, "SIGRTMIN+13" } , { 48, "SIGRTMIN+14" } , { 49, "SIGRTMIN+15" } , { 50, "SIGRTMAX-14" } ,
          { 51, "SIGRTMAX-13" } , { 52, "SIGRTMAX-12" } , { 53, "SIGRTMAX-11" } , { 54, "SIGRTMAX-10" } ,
          { 55, "SIGRTMAX-9"  } , { 56, "SIGRTMAX-8"  } , { 57, "SIGRTMAX-7"  } , { 58, "SIGRTMAX-6"  } ,
          { 59, "SIGRTMAX-5"  } , { 60, "SIGRTMAX-4"  } , { 61, "SIGRTMAX-3"  } , { 62, "SIGRTMAX-2"  } ,
          { 63, "SIGRTMAX-1"  } , { 64, "SIGRTMAX"    } ,
    };
    int sig_idx = sig_no - 1;
    if( sig_idx > -1 && sig_idx < sizeof(all_sigs) / sizeof(all_sigs[0]) ) {
        return all_sigs[sig_idx].sig_name;
    }
    return "Unknown";
}

static int console_window_shared_unit_test_main(int console_app, int argc, char *argv[])
{
	g_unit_test_cases.reg_done_ = 1;

    // Run specified tests
    int i = 0;
    ( argc == 1 ) && list_all_tests();
    int n_runned_test = 0;
    int success_tests = 0;
    for(i = 1; i < argc; ++i) {
        int k = 0;
        bool found = false;
        for(k = 0; k < g_unit_test_cases.num_of_cases_; ++k)
        {
            if( strcmp(argv[i],  g_unit_test_cases.test_cases_[k].name_) == 0) {
                found = true;
                n_runned_test ++;
                printf("Run test [%s] ...\n", argv[i]);
                int test_pid = fork();
                if( test_pid == -1) {
                    fprintf(stderr, "fork() Failed: %s\n", strerror(errno) );
                    exit(1);
                }
                else if( test_pid == 0) { // I'm child
                    g_unit_test_cases.test_cases_[k].fp_();
                    exit(0);
                }
                else { // I'm parent
                    int status = 0;
                    pid_t child = wait(&status);
                    assert( child == test_pid );
                    char exit_desc[ 256 ] = {0};
                    if( WIFSIGNALED(status) ) {
                        snprintf(exit_desc, sizeof(exit_desc) - 1, "Failed by signal %s(%d)"
                                , get_sig_name(WTERMSIG(status)), WTERMSIG(status) );
                    }
                    else if( WIFEXITED(status) ) {
                        char exit_n[20] = {0};
                        if( WEXITSTATUS(status) ) {
                            snprintf(exit_n, sizeof(exit_n) - 1, " by exit(%d) or return %d in main", WEXITSTATUS(status));
                        }
                        else {
                            ++success_tests;
                        }
                        snprintf(exit_desc, sizeof(exit_desc) - 1, "%s%s"
                                , (WEXITSTATUS(status) == 0) ? "Success" : "Failed", exit_n);
                    }
                    else {
                        snprintf(exit_desc, sizeof(exit_desc) - 1, "Failed, but neither WIFEXITED nor WIFSIGNALED.");
                    }
                    printf("Test [%s] %s\n", argv[i], exit_desc);
                }

            }
        }
        if ( ! found ) {
            fprintf(stderr, "test [%s] not found\n", argv[i]);
        }

    }

    if( argc > 1) {
        fprintf(stdout, "==========================================\n");
        fprintf(stdout, "Total test: %d, Not found: %d, Success: %d(%.1f%%), Failed: %d\n"
                , argc - 1, argc - 1 - n_runned_test, success_tests
                , 100.0 * success_tests / n_runned_test, n_runned_test - success_tests);
    }
	return 0;
}

static int inject_main(int argc, char *argv[])
{
	return console_window_shared_unit_test_main(1, argc, argv);
}

int my_init(void)
{
	Elf * e = NULL;
	GElf_Ehdr ehdr;
	pid_t pid = getpid();
	char proc_exe_fname[100] = {0};
	char real_exe_file[100] = {0};
	snprintf(proc_exe_fname, sizeof(proc_exe_fname)-1, "/proc/%d/exe", pid);
	int n_real_path = readlink(proc_exe_fname, real_exe_file, sizeof(real_exe_file) - 1);
	if(n_real_path >= sizeof(real_exe_file) - 1)
	{
		fprintf(stderr, "Executable file name is too long( > %llu chars)",
				(long long unsigned) (sizeof(real_exe_file) - 2U) );
		exit(-1);
	}
	int fd = open(proc_exe_fname, O_RDONLY, 0);
	if( fd < 0)
	{
		fprintf(stderr, "Fail to open [%s]", real_exe_file);
		exit(-1);
	}

	elf_version(EV_CURRENT) ;

	e = elf_begin(fd, ELF_C_READ, NULL);
	if( e == NULL)
	{
		fprintf(stderr, "Fail to open [%s] as ELF", real_exe_file);
		exit(-1);
	}
	if(gelf_getehdr(e, &ehdr) == NULL)
	{
		fprintf(stderr, "Fail to get hdr of [%s]", real_exe_file);
		exit(-1);
	}

	unsigned int * addr = (unsigned int*) ( (unsigned int)ehdr.e_entry + 24 );
	int ret = mprotect( (void*)  ( (unsigned int)addr / 4096 * 4096), 1024, PROT_READ | PROT_WRITE | PROT_EXEC);
	if( ret < 0 )
	{
		printf("mprotect ret: %d: %s\n", ret, strerror(errno) );
        exit(-1);
	}
    close(fd);
	unsigned int orig_main = *addr;
	*addr = (unsigned int)inject_main;
	printf("======  EasyCUnit start...======\n"
			"Replace executable [%s]'s main function(%p) with Unit Test main: %p\n"
			"\n"
			, real_exe_file
			, (void *) orig_main, (void *)inject_main );
	
	return 0;
}
