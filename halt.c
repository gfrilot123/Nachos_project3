/* halt.c
 *	Simple program to test whether running a user program works.
 *
 *	Just do a "syscall" that shuts down the OS.
 *
 * 	NOTE: for some reason, user programs with global data structures
 *	sometimes haven't worked in the Nachos environment.  So be careful
 *	out there!  One option is to allocate data structures as
 * 	automatics within a procedure, but if you do this, you have to
 *	be careful to allocate a big enough stack to hold the automatics!
 */

#include "syscall.h"

int
main()
{

// Note for chau when there is not enough memory it doesn't properly deallocate the call
// Begin code added by Joseph Aucoin
    int i;
    for (i = 0; i < 10; i++)
    {
        // Calls halt to run if memory is available
        Exec("../test/halt");
        if(i == 2 || i == 4 || i == 6 || i == 8 || i == 10)
        {
          Exit(0);
        }
    }

// End code added by Joseph Aucoin

    Exit(0);
    //Halt();
    //Join(Exec("../test/halt"));
    /* not reached */
}
