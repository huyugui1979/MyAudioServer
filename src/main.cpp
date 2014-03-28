
#include "audio_server.h"
#include "audio_log.h"
#include "audio_hall.h"
#include <execinfo.h>
#include <signal.h>
#include <string.h>

#include <iostream>
#include <cstdlib>
#include <stdexcept>
/* This structure mirrors the one found in /usr/include/asm/ucontext.h */
typedef struct _sig_ucontext {
 unsigned long     uc_flags;
 struct ucontext   *uc_link;
 stack_t           uc_stack;
 struct sigcontext uc_mcontext;
 sigset_t          uc_sigmask;
} sig_ucontext_t;

void crit_err_hdlr(int sig_num, siginfo_t * info, void * ucontext)
{
 void *             array[50];
 void *             caller_address;
 char **            messages;
 int                size, i;
 sig_ucontext_t *   uc;

 uc = (sig_ucontext_t *)ucontext;

 /* Get the address at the time the signal was raised */
#if defined(__i386__) // gcc specific
 caller_address = (void *) uc->uc_mcontext.eip; // EIP: x86 specific
#elif defined(__x86_64__) // gcc specific
 caller_address = (void *) uc->uc_mcontext.rip; // RIP: x86_64 specific
#else
#error Unsupported architecture. // TODO: Add support for other arch.
#endif
 size = backtrace(array, 50);

 /* overwrite sigaction with caller's address */
 array[1] = caller_address;

 messages = backtrace_symbols(array, size);

 /* skip first stack frame (points here) */
 ostringstream strbuffer;
 for (i = 1; i < size && messages != NULL; ++i)
 {
     strbuffer<<"[bt]:("<<i<<") "<<messages[i]<<endl;
 }
 BOOST_LOG_TRIVIAL(error)<<strbuffer.str();
 free(messages);

 exit(EXIT_FAILURE);
}
using namespace boost::asio::ip;


int main(int argc, char *argv[])
{
	 g_InitLog();
	struct sigaction sigact;

	 sigact.sa_sigaction = crit_err_hdlr;
	 sigact.sa_flags = SA_RESTART | SA_SIGINFO;

	 if (sigaction(SIGSEGV, &sigact, (struct sigaction *)NULL) != 0)
	 {
	  fprintf(stderr, "error setting signal handler for %d (%s)\n",
	    SIGSEGV, strsignal(SIGSEGV));

	  exit(EXIT_FAILURE);
	 }


    BOOST_LOG_TRIVIAL(trace)<<"app begin";
   // BOOST_LOG_SEV(g_logger,Log_Info)<<"helo";
    boost::asio::io_service io;
    audio_server server(io,9009);
    audio_hall hall(io);

    io.run();
//    std::vector<std::shared_ptr<std::thread> > threads;
//    for (std::size_t i = 0; i < 2; ++i)
//    {
//        std::shared_ptr<std::thread> thread(new std::thread(
//                                                boost::bind(&asio::io_service::run, &io_service)));
//        threads.push_back(thread);
//    }

//    // Wait for all threads in the pool to exit.
//    for (std::size_t i = 0; i < threads.size(); ++i)
//        threads[i]->join();

    return 0;
}
