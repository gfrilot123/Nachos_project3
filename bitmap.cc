// bitmap.c
//	Routines to manage a bitmap -- an array of bits each of which
//	can be either on or off.  Represented as an array of integers.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "bitmap.h"

//----------------------------------------------------------------------
// BitMap::BitMap
// 	Initialize a bitmap with "nitems" bits, so that every bit is clear.
//	it can be added somewhere on a list.
//
//	"nitems" is the number of bits in the bitmap.
//----------------------------------------------------------------------

BitMap::BitMap(int nitems)
{
    numBits = nitems;
    numWords = divRoundUp(numBits, BitsInWord);
    map = new unsigned int[numWords];
    for (int i = 0; i < numBits; i++)
        Clear(i);
}

//----------------------------------------------------------------------
// BitMap::~BitMap
// 	De-allocate a bitmap.
//----------------------------------------------------------------------

BitMap::~BitMap()
{
    delete map;
}

//----------------------------------------------------------------------
// BitMap::Set
// 	Set the "nth" bit in a bitmap.
//
//	"which" is the number of the bit to be set.
//----------------------------------------------------------------------

void
BitMap::Mark(int which)
{
    ASSERT(which >= 0 && which < numBits);
    map[which / BitsInWord] |= 1 << (which % BitsInWord);
}

//----------------------------------------------------------------------
// BitMap::Clear
// 	Clear the "nth" bit in a bitmap.
//
//	"which" is the number of the bit to be cleared.
//----------------------------------------------------------------------

void
BitMap::Clear(int which)
{
    ASSERT(which >= 0 && which < numBits);
    map[which / BitsInWord] &= ~(1 << (which % BitsInWord));
}

//----------------------------------------------------------------------
// BitMap::Test
// 	Return TRUE if the "nth" bit is set.
//
//	"which" is the number of the bit to be tested.
//----------------------------------------------------------------------

bool
BitMap::Test(int which)
{
    ASSERT(which >= 0 && which < numBits);

    if (map[which / BitsInWord] & (1 << (which % BitsInWord)))
	return TRUE;
    else
	return FALSE;
}

//----------------------------------------------------------------------
// BitMap::Find
// 	Return the number of the first bit which is clear.
//	As a side effect, set the bit (mark it as in use).
//	(In other words, find and allocate a bit.)
//
//	If no bits are clear, return -1.
//----------------------------------------------------------------------

int
BitMap::Find()
{
    for (int i = 0; i < numBits; i++)
	if (!Test(i)) {
	    Mark(i);
	    return i;
	}
    return -1;
}

//----------------------------------------------------------------------
// BitMap::NumClear
// 	Return the number of clear bits in the bitmap.
//	(In other words, how many bits are unallocated?)
//----------------------------------------------------------------------

int
BitMap::NumClear()
{
    int count = 0;

    for (int i = 0; i < numBits; i++)
	if (!Test(i)) count++;
    return count;
}

//----------------------------------------------------------------------
// BitMap::Print
// 	Print the contents of the bitmap, for debugging.
//
//	Could be done in a number of ways, but we just print the #'s of
//	all the bits that are set in the bitmap.
//----------------------------------------------------------------------

void
BitMap::Print()
{
    int numFree = 0;
    printf("Bitmap set:\n");
    for (int i = 0; i < numBits; i++)
    {//begin code changes by Chau Cao
	if (Test(i))
	{
	    printf("%d, ", 1); //changed functionality to print out in terms of 1's and 0's
	}
  	else
	{
	    printf("%d, ", 0);
	}
    }
    printf("\n");
	//end code changes by Chau Cao
}

// These aren't needed until the FILESYS assignment

//begin code changes by Robert Knott
//BitMap::IndividPrint
//prints each bitmap section as Clear/Not Clear
//meant for testing/debugging, not used in final
void BitMap::IndividPrint()
{
    for(int r = 0; r < numBits; r++)
    {
	if(!Test(r))
	    printf("MemMap(%d) is Clear\n", r);
	else
	    printf("MemMap(%d) is Not Clear\n", r);
	}
}

//BitMap::FirstFit
//finds the first set of free memory pages whose size can fit the fileSize of
	//the file being passed in
//if true, this puts the file in the memory
//if false, ignore the file
//fileSize = the size of the file being passed in
//recurIndex = the address of the last-seen index from a previous
	//iteration of FirstFit.  Only used if the program encounters
	//a contiguous space that ends before the last memory space.
    //Default value is 0. --MUST BE 0 ON FIRST CALL--
int BitMap::FirstFit(int fileSize, int recurIndex)
{
    int firstAdd = 0;		//-First address seen that is available.  This value is
				//returned and is used by the bitmap to tell where the
				//executable file is stored in memory
				//--FUNCTIONALITY CURRENTLY MISSING!--
				//How to put a file onto memory?

    int secondAdd = 0;		//-Second address.  holds the bitmap address where the first
				//taken bit is seen.  secondAdd - firstAdd = contigSize
    int contigSize = 0;		//-The size of contiguous free memory that FirstFit is looking
				//at.  Must be at least fileSize for the function to allow the
				//file to be entered into the memory
    //printf("RecurIndex = %d\n", recurIndex);  //print statement to verify the recurIndex
    if(NumClear() < fileSize)  //if there isn't enough available total memory for the file,
    {
	printf("Insufficent Memory Remaining.  Ignoring File...\n");  //alert the user and
	return -1;							//end FirstFit
    }

	for(int n = recurIndex; n < numBits; n++)  //search for the first available bit
	{
	    if(!Test(n))
	    {
		firstAdd = n;			//save the first available bitMap space
						//as firstAdd
		break;				//end the loop; go to next loop for secondAdd
	    }
	}

	for(int o = firstAdd; o < numBits; o++)  //search for the first bit that is no
							//longer available.  Start search
							//at firstAdd.
	{
	    if(Test(o) || o == 31)
	    {
		secondAdd = o;			//save the first taken bitMap space seen
						//as secondAdd
		break;				//end the loop; verify that there is enough
						//space between firstAdd and secondAdd to fit
						//file.
	    }
	}

    contigSize = (secondAdd - firstAdd) + 1;	//calculate the contiguous memory space
    printf("%d to %d is free, contiguous size = %d/%d\n", firstAdd, secondAdd, contigSize, numBits);

    if(fileSize <= contigSize)			//if the size of the file can fit into the
							//amount of memory available,
    {
	printf("FILE ADDED TO MEMORY!\n");
	//add file to memory
	return firstAdd;			//end FirstFit; objective complete
    }
    else					//if it cannot fit, see if it can
	printf("CANNOT FIT INTO THIS SPACE!\n");	//keep going

    if(secondAdd < numBits-1)			//if secondAdd has not hit the end of
							//the bitMap,
    {
	printf("NOT DONE!\n");
	FirstFit(fileSize, secondAdd);		//recursively call this function using
							//secondAdd as the starting point
    }

    //insert the file to the memory -OR- allow adderspace to add
	//to the memory
    //...
}

//BitMap::BestFit
//finds the contiguous set of free memory pages whose size is
//the closest match to the fileSize of the file being passed in
//if true, this puts the file in the memory
//if false, ignore the file
void BitMap::BestFit(int fileSize)
{
    //traverse the entire bitmap
	//-if there isn't enough total space available,
	    //then ignore file
    	//-if there is no contiguous memory space large
	    //enough, then ignore file
    //add the file to the memory that best fits the
    //space of available memory
	//-if there are two spaces that qualify as best, 
	    //then enter into the first one
}

//BitMap::WorstFit
//finds the contiguous set of free memory pages with
//the largest size of free memory available.
//if true, this puts the file in the memory
//if false, ignore the file
void BitMap::WorstFit(int fileSize)
{
    //traverse the entire bitmap
	//-if there isn't enough total space available,
	    //then ignore file
    	//-if there is no contiguous memory space large
	    //enough, then ignore file
    //add the file to the memory that has the largest
    //contiguous space of free memory
	//-if there are two spaces that qualify as largest, 
	    //then enter into the first one
}
//end code changes by Robert Knott

//----------------------------------------------------------------------
// BitMap::FetchFromFile
// 	Initialize the contents of a bitmap from a Nachos file.
//
//	"file" is the place to read the bitmap from
//----------------------------------------------------------------------

void
BitMap::FetchFrom(OpenFile *file)
{
    file->ReadAt((char *)map, numWords * sizeof(unsigned), 0);
}

//----------------------------------------------------------------------
// BitMap::WriteBack
// 	Store the contents of a bitmap to a Nachos file.
//
//	"file" is the place to write the bitmap to
//----------------------------------------------------------------------

void
BitMap::WriteBack(OpenFile *file)
{
   file->WriteAt((char *)map, numWords * sizeof(unsigned), 0);
}
