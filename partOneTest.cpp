// CS3243 Operating Systems
// Fall 2013
// Project 6: Disks and File Systems
// Jestin Keaton and Steven Ng
// Date: 12/2/2013
// File: partone.cpp


#include<iostream>
#include<fstream>
//#include<deque>
#include<cstdlib>
#define BYTECOUNT 1474560
#define BEGIN_BYTE_ENTRY 16896
#define SECTOR_SIZE 512
#define MAX_NUM_ENTRY 2849 //2880 - 33 = 2847 + 2 reserved = 2849
#define START_FAT 2
#define FIRST_FAT_BYTE 512
#define FAT_SIZE 4608

using namespace std;

// Define Structs

typedef unsigned char byte;

string bar = "           |----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----\n";

struct MainMemory{
    byte memArray[BYTECOUNT]; // 0-511 is for the boot partition
    
    MainMemory();
    void findFreeSector();
    void insertIntoMemory(byte b);
    void print();
};


// Since we can access the FAT directly, there may be no need for these extra structs
/**
struct Entry
{
	char a : 8;
	char b : 8;
	char c : 8;
};

struct FileTable
{
	Entry table[MAX_NUM_ENTRY];
};*/



MainMemory memory;
//deque<int> freeSectors;


// Declare Methods
void loadSystem();
void setEntry(short a);
short getEntry(short b);

int main(){
    /*ifstream ifile("fdd.flp",std::ifstream::in);
    byte b = ifile.get();


    //byte copiedFile

    int c = 0;
    while (ifile.good()){
        printf("%02x ",b);
        b = ifile.get();
        if(c == 7){
            printf(" ");
        }
        if(c == 15){
            printf("\n");
            c = 0;
        }
        else
            ++c;
    }
    printf("%d", memory.findFreeMemory());*/
    loadSystem();
    memory.print();
    //Entry e = {0, 11, 63};
    //printf("%d\n", e.b);
    //printf("%d\n", e.c);
    //printf("%d\n", (e.b << 8) + e.c);
    /*memory.findFreeSector();
	for (uint i = 0; i < freeSectors.size(); i++)
	{
		printf("%d", freeSectors[i]);
		printf("\n");
	}*/
    return 0;
}

void loadSystem()
{
	ifstream ifile("fdd.flp",std::ifstream::in);
    byte b = ifile.get();

    //byte copiedFile

    int c = 0;
    while (ifile.good()){
		memory.memArray[c] = b; 
		b = ifile.get();
		++c;
	}
}

/**
* Sets a FAT and FAT2 entries to be the short value that is entered
* param a the value we intend to set in the FAT
*/
void setEntry(short a){
    // In the schema [yz Zx XY], the lower-case letters represent what we call
    // the low-order entry, the high-order entry would be the upper-case letters.
    if(a<0 || a >= MAX_NUM_ENTRY)
        return; // faulty sector selection, do nothing
    // Set both FAT and FAT2 entries 
    for(int i = 0; i < 2; i++){
        int start = (FIRST_FAT_BYTE + i * FAT_SIZE) + (a/2) * 3; // set first byte of entry pair
        if(a%2==0){ // setting a low-order entry
            memory.memArray[start] = (a &  0xFF); // set yz to the lowest byte of short <a>
            memory.memArray[start + 1] &= 0xF0; // clear low nibble here
            memory.memArray[start + 1] |= ((a >> 8) & 0x0F); // set x to nibble 2 of <a>
        }
        else{ // setting a high-order entry
            memory.memArray[start + 2] = ((a & 0x0FF0) >> 4); // set XY to nibbles 2 and 3 of <a>
            memory.memArray[start + 1] &= 0x0F; // clear high nibble here
            memory.memArray[start + 1] |= ((a & 0xF) << 4); // set Z to nibble 4 in <a>
        }
    }
}

/**
* Retrieves the value sent in the parameter from the FAT table
* param b the index in the FAT table that we want
*/
short getEntry(short b){
    // Using the same schema as in 'setEntry()' we will extract a FAT entry's
    // value and return it as a short [schema: yz Zx XY]
    if(b<0 || b >= MAX_NUM_ENTRY)
        return -1; // faulty request here, return error code (-1)
    if(b%2==0){ // requesting a low-order entry
        return memory.memArray[b/2] + ((memory.memArray[b/2 + 1] & 0x0F) << 8);
    }
    else{ // requesting a high-order entry
        return (memory.memArray[b/2 + 2] << 4) + (memory.memArray[b/2 + 1] >> 4);
    }
}

MainMemory::MainMemory()
{
	/*for (int i = 0; i < BYTECOUNT; i++)
	{
		memArray[i] = 157;
	}*/
}

void MainMemory::findFreeSector(){
	/*int sector = 33;
	bool empty;
    for (int i = BEGIN_BYTE_ENTRY; i < BYTECOUNT; )
    {
		empty = true;
		for (int j = i; j < i + SECTOR_SIZE; j++)
		{
			if (memory.memArray[j])
			{
				empty = false;
				break;
			} 
		}
		if (empty)
		{
			freeSectors.push_back(sector);
		}
		sector++;
		i += SECTOR_SIZE;
	}*/
	
}

void MainMemory::insertIntoMemory(byte b)
{
	memory.memArray[0] = b;
}

void MainMemory::print()
{
	cout << "\nDISK USAGE BY SECTOR:\n";
	cout << bar;
	for(int i = 0; i < 36; i++){
		int begin = (i*80);
		int end = (i+1)*80-1;
		printf("%04d-%04d: ",begin,end);
		char toPrint;
		for(int j = begin; j <= end; j++){
			toPrint = '.';
			if(j==0){
				toPrint = 'B';
			}
			else if(j < 19){
				toPrint = 'F';
			}
			else if(j < 33){
				toPrint = 'R';
			}
			printf("%c",toPrint);
		}
		cout << endl;
	}
	cout << endl;
}
