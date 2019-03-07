// addrspace.cc
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "syscall.h"
#include "noff.h"
#ifdef HOST_SPARC
#include <strings.h>
#endif

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable)
{
    NoffHeader noffH;
    unsigned int i, size;

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && (WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
		if(noffH.noffMagic != NOFFMAGIC) {
			printf("There is a noff error with the desired user program. Exiting\n");
			Exit(-2);//MAKE THIS MEANINGFUL
		}
    //ASSERT(noffH.noffMagic == NOFFMAGIC);

// how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size
			+ UserStackSize;	// we need to increase the size
						// to leave room for the stack
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;

		if(numPages > NumPhysPages) {
			printf("The desired user program is bigger than physical memory can accept without virtual memory. Exiting\n");
			Exit(-3);//MAKE THIS MEANINGFUL
		}
    //ASSERT(numPages <= NumPhysPages);		// check we're not trying
						// to run anything too big --
						// at least until we have
						// virtual memory
    DEBUG('a', "Initializing address space, num pages %d, size %d\n",
					numPages, size);
// first, set up the translation
    pageTable = new TranslationEntry[numPages];

		//Begin Code Changes by Chau Cao

		//Print out the bitmap before attempting to bring the user program into memory
		if(memMap == NULL){
			printf("CRITICAL ERROR: GLOBAL BITMAP IS NULL. HOW DID THIS HAPPEN?!\n");
			Exit(-4);//legit don't know how this could happen but who knows
		}
		printf("Page availability before adding the process:\n");
		memMap->Print();

		//check if the number of pages needed is greater than the number of available frames
		//in memory. If numPages is greater, produce output stating that and Exit()
		if(numPages > memMap->NumClear()) {
			printf("There is not enough memeory available for the requested process\n");
			Exit(-1);//MAKE THIS MEANINGFUL
		}
		//else assign the pageTable values for the Virtual Address and take available
		//frames in main memory. Mark the appropriate bits as used.
		else {
			//**********Place Fit Alogorithm calls here and place memIndex equal to the return value************
			memIndex = 0; //setting it to 0 for now for compilation and usability of single user processes
			if(memIndex == -1) {
				printf("There is not a large enough contiguous block of memory for the requested process\n");
				//Exit this gracefully pls
			}
			else {
				setMemory();
			}
			/*//loops through the number of pages that need to be placed into main memory.
			//Virtual Page in page table is just the current index.
			//Physical Page: Loop through mainMemory searching for available memory. When
			//	found the start of the memoryblock is found, place that value into memIndex
			//Mark the coinciding bit in memMap as used, and assign that location to Physical Page
			//Call bzero on that location in mainMemory to clear that frame of size 256 for the
			//	instructions being stored then break
			//THe reset of the assignemnts mimic the one provided in the base exception
			for (int x = 0; x < numPages; x++) {
				pageTable[x].virtualPage = x;
				tempIndex = 0;
				for (int y = 0; y < NumPhysPages; y++) {
					if(memMap->Test(y) == true) {
						tempIndex = tempIndex + 256;
					}
					else
					{
						if(x == 0) {
							memIndex = tempIndex;
						}
						memMap->Mark(y);
						pageTable[x].physicalPage = y;
						bzero(machine->mainMemory + tempIndex, 256);
						break;
					}
				}
				pageTable[x].valid = true;
				pageTable[x].use = false;
				pageTable[x].dirty = false;
				pageTable[x].readOnly = false;
			}*/

		}

		//Calls OpenFile.ReadAt to pull the instructions and initData that needs to be
		//	read into memory.
		//The code instructions are read in and stored starting at memIndex
		//The init data are read in after at the location referenced by memIndex + code.size
		if(executable == NULL)
		{
			printf("Executable is null. Exiting\n");
			Exit(-4);
		}
    if (noffH.code.size > 0) {
    	DEBUG('a', "Initializing code segment, at 0x%x, size %d\n",noffH.code.virtualAddr, noffH.code.size);
			executable->ReadAt(machine->mainMemory+(memIndex*256), noffH.code.size, noffH.code.inFileAddr);
    }

    if (noffH.initData.size > 0) {
      DEBUG('a', "Initializing data segment, at 0x%x, size %d\n", noffH.initData.virtualAddr, noffH.initData.size);
			executable->ReadAt(machine->mainMemory + ((memIndex*256) + noffH.code.size),noffH.initData.size, noffH.initData.inFileAddr);
    }


		//Print out the bitmap after allocating the user program
		printf("Page availability after adding the process:\n");
		memMap->Print();

		//End changes by Chau Cao
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
   delete [] pageTable;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------


//setMemory function for address space constructor
//Begin Code Changes by Chau Cao
void AddrSpace::setMemory() {
	//loops through the number of pages that need to be placed into main memory.
	//Virtual Page in page table is just the current index.
	//Physical Page: startIndex is the starting index of the found contiguous memory
	//use this as the offset and increment by x while assigning the following.
	//Mark the coinciding bit in memMap as used, and assign that location to Physical Page
	//Call bzero on that location in mainMemory to clear that frame of size 256 for the
	//	instructions being stored then break
	//The assignemnts mimic the one provided in the base exception
	int startIndex = memIndex;
	for (int x = 0; x < numPages; x++) {
		pageTable[x].virtualPage = x;
		memMap->Mark(startIndex + x);
		pageTable[x].physicalPage = startIndex + x;
		bzero(machine->mainMemory + (startIndex * 256), 256);
		pageTable[x].valid = true;
		pageTable[x].use = false;
		pageTable[x].dirty = false;
		pageTable[x].readOnly = false;
	}
}
//end code changes by Chau Cao
void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++) {
			machine->WriteRegister(i, 0);
		}
    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, (memIndex*256));

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, (memIndex*256) + 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, ((memIndex*256) + (numPages * PageSize) - 16));
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState()
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState()
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}
