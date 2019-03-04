// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include <stdio.h>        // FA98
#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "addrspace.h"   // FA98
#include "sysdep.h"   // FA98

// begin FA98

void processCreator(int arg);
static int SRead(int addr, int size, int id);
static void SWrite(char *buffer, int size, int id);

// end FA98

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions
//	are in machine.h.
//----------------------------------------------------------------------

void processCreator(int arg);
int i,j,k,pc,jpc,buffAdd,jBuffAdd,caller;
AddrSpace *space;
AddrSpace *jspace;
char *file;
char *jFile;
OpenFile *executable;
OpenFile *jexecutable;
Thread *t;
Thread *joinThread;


void
ExceptionHandler(ExceptionType which)
{
	int type = machine->ReadRegister(2);

	int arg1 = machine->ReadRegister(4);
	int arg2 = machine->ReadRegister(5);
	int arg3 = machine->ReadRegister(6);
	int Result;


	char *ch = new char [500];
	switch ( which )
	{
	case NoException :
		break;
	case SyscallException :

		// for debugging, in case we are jumping into lala-land
		// Advance program counters.
		machine->registers[PrevPCReg] = machine->registers[PCReg];
		machine->registers[PCReg] = machine->registers[NextPCReg];
		machine->registers[NextPCReg] = machine->registers[NextPCReg] + 4;

		switch ( type )
		{

		case SC_Halt :
			DEBUG('t',"Shutdown, initiated by user program.\n");
			interrupt->Halt();
			break;

		case SC_Exit :
			if(type==1 & arg1==0)
			{

			printf("Exit() syscall received by process number: %d ",buffAdd);
			printf("\n");
			printf("Process completed with return value: %d \n",arg1);
			}

			break;


		case SC_Exec :

			if(type==2)
			caller=2;
			printf("EXEC() syscall invoked in ");
			printf(currentThread->getName());
			printf(" by ");
			if(buffAdd!=0)
			printf("Process number # :%d\n",buffAdd);
			buffAdd=machine->ReadRegister(4);

			file = new char[100];
			if(!machine->ReadMem(buffAdd,1,&arg1))return;
			i=0;

			while(arg1!=0)
			{
				file[i]=(char)arg1;
				buffAdd+=1;
				i++;
				if(!machine->ReadMem(buffAdd,1,&arg1))return;

			}
			file[i]=(char)0;
			Exec(file);
			caller=0;
			break;

		case SC_Join:

			if(type==3)
			{
				
				printf("Join() is requesting a new process from Exec() \n");
				printf("Join() syscall invoked in ");
				printf(currentThread->getName());
				printf(" by ");
				if(jBuffAdd!=0)
				printf("Process number # :%d\n",jBuffAdd);
				jBuffAdd=machine->ReadRegister(4);

				jFile=new char[100];
				if(!machine->ReadMem(jBuffAdd,1,&arg1))return;
				k=0;

				while(arg1!=0)
				{
					jFile[k]=(char)arg1;
					jBuffAdd+=1;
					j++;
					if(!machine->ReadMem(jBuffAdd,1,&arg1))return;

				}

				jFile[j]=(char)0;
				if(caller==3)
				Exec(jFile);

}

			break;


	 case SC_Yield:

			if(type==10)
			{
			printf("Yield() syscall received by process number: %d ",buffAdd);
			printf("\n");
	 		currentThread->Yield();
			}
	 		break;


		case SC_Read :
			if (arg2 <= 0 || arg3 < 0){
				printf("\nRead 0 byte.\n");
			}
			Result = SRead(arg1, arg2, arg3);
			machine->WriteRegister(2, Result);
			DEBUG('t',"Read %d bytes from the open file(OpenFileId is %d)",
			arg2, arg3);
			break;

		case SC_Write :
			for (j = 0; ; j++) {
				if(!machine->ReadMem((arg1+j), 1, &i))
					j=j-1;
				else{
					ch[j] = (char) i;
					if (ch[j] == '\0')
						break;
				}
			}
			if (j == 0){
				printf("\nWrite 0 byte.\n");
				// SExit(1);
			} else {
				DEBUG('t', "\nWrite %d bytes from %s to the open file(OpenFileId is %d).", arg2, ch, arg3);
				SWrite(ch, j, arg3);
			}
			break;


			default :
			printf("Not a valid syscall\n");

			break;
		}
		break;

	case ReadOnlyException :
		puts ("ReadOnlyException");
		if (currentThread->getName() == "main")
		ASSERT(FALSE);  //Not the way of handling an exception.
		//SExit(1);
		break;
	case BusErrorException :
		puts ("BusErrorException");
		if (currentThread->getName() == "main")
		ASSERT(FALSE);  //Not the way of handling an exception.
		//SExit(1);
		break;
	case AddressErrorException :
		puts ("AddressErrorException");
		if (currentThread->getName() == "main")
		ASSERT(FALSE);  //Not the way of handling an exception.
		//SExit(1);
		break;
	case OverflowException :
		puts ("OverflowException");
		if (currentThread->getName() == "main")
		ASSERT(FALSE);  //Not the way of handling an exception.
		//SExit(1);
		break;
	case IllegalInstrException :
		puts ("IllegalInstrException");
		if (currentThread->getName() == "main")
		ASSERT(FALSE);  //Not the way of handling an exception.
		//SExit(1);
		break;
	case NumExceptionTypes :
		puts ("NumExceptionTypes");
		if (currentThread->getName() == "main")
		ASSERT(FALSE);  //Not the way of handling an exception.
		//SExit(1);
		break;

		default :
		//      printf("Unexpected user mode exception %d %d\n", which, type);
		//      if (currentThread->getName() == "main")
		//      ASSERT(FALSE);
		//      SExit(1);
		break;
	}
	delete [] ch;
}


static int SRead(int addr, int size, int id)  //input 0  output 1
{
	char buffer[size+10];
	int num,Result;

	//read from keyboard, try writing your own code using console class.
	if (id == 0)
	{
		scanf("%s",buffer);

		num=strlen(buffer);
		if(num>(size+1)) {

			buffer[size+1] = '\0';
			Result = size+1;
		}
		else {
			buffer[num+1]='\0';
			Result = num + 1;
		}

		for (num=0; num<Result; num++)
		{  machine->WriteMem((addr+num), 1, (int) buffer[num]);
			if (buffer[num] == '\0')
			break; }
		return num;

	}
	//read from a unix file, later you need change to nachos file system.
	else
	{
		for(num=0;num<size;num++){
			Read(id,&buffer[num],1);
			machine->WriteMem((addr+num), 1, (int) buffer[num]);
			if(buffer[num]=='\0') break;
		}
		return num;
	}
}



static void SWrite(char *buffer, int size, int id)
{
	//write to terminal, try writting your own code using console class.
	if (id == 1)
	printf("%s", buffer);
	//write to a unix file, later you need change to nachos file system.
	if (id >= 2)
	WriteFile(id,buffer,size);
}

void processCreator(int arg)
{

	currentThread->space->InitRegisters();
	currentThread->space->RestoreState();

	machine->Run();



	ASSERT(FALSE);
}

SpaceId Exec(char *name)
{
	switch (caller)
	{
	case 2:

	printf("filename : ( %s ",name);
	printf(" )");
	printf("\n");
	t = new Thread("thisThread");
	executable = fileSystem->Open(file);
	interrupt->SetLevel(IntOff);
	space = new AddrSpace(executable);
	t->space=space;
	delete executable;
	t->Fork(processCreator,buffAdd);
	pc=machine->ReadRegister(PCReg);
	machine->WriteRegister(PrevPCReg,pc);
	pc=machine->ReadRegister(NextPCReg);
	machine->WriteRegister(PCReg,pc);
	pc+=4;
	machine->WriteRegister(NextPCReg,pc);
	interrupt->SetLevel(IntOn);
	return buffAdd;

	break;


		case 3:

		printf("filename : ( %s ",name);
		printf(" )");
		printf("\n");
		joinThread = new Thread("thisThread");
		jexecutable = fileSystem->Open(file);
		interrupt->SetLevel(IntOff);
		jspace = new AddrSpace(executable);
		joinThread->space=jspace;
		delete jexecutable;
		joinThread->Fork(processCreator,jBuffAdd);
		jpc=machine->ReadRegister(PCReg);
		machine->WriteRegister(PrevPCReg,jpc);
		jpc=machine->ReadRegister(NextPCReg);
		machine->WriteRegister(PCReg,jpc);
		jpc+=4;
		machine->WriteRegister(NextPCReg,jpc);
		interrupt->SetLevel(IntOn);


		return jBuffAdd;
		break;

	}

}




// end FA98
