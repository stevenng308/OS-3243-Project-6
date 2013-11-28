// CS3243 Operating Systems
// Fall 2013
// Project 6: Disks and File Systems
// Jestin Keaton and Steven Ng
// Date: 12/2/2013
// File: partone.cpp


#include<iostream>
#include<fstream>
#include<cstdlib>
#define BYTECOUNT 1474560
#define BEGIN_BYTE_ENTRY 16896
#define SECTOR_SIZE 512
#define MAX_FAT_ENTRY 2848 //2879 - 33 = 2846 + 2 reserved = 2848
#define START_FAT 2
#define FIRST_FAT_BYTE 512
#define FAT_SIZE 4608
#define LAST_INVALID_ENTRY 3071 // first invalid entry is 2849

using namespace std;

// Define Structs

typedef unsigned char byte;

string bar = "           |----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----\n";

struct MainMemory{
    byte memArray[BYTECOUNT]; // 0-511 is for the boot partition
    
    //MainMemory();
    void insertIntoMemory(int pos, byte data);
    void setEntry(ushort pos, ushort val);
	ushort getEntry(ushort pos);
    void print();
};

MainMemory memory;

// Declare Methods
void loadSystem();
void initializeFAT();

int main(){
    loadSystem();
    memory.print();
    /*for (int i = 0; i < SECTOR_SIZE; i++)
    {
		printf("%02x\n", memory.memArray[i]);
	}*/
	
	printf("%02x %02x\n", memory.getEntry(0), memory.getEntry(1));
    return 0;
}

void loadSystem()
{
	ifstream ifile("fdd.flp",std::ifstream::in);
    byte b = ifile.get();

    //byte copiedFile

    int c = 0;
    while (ifile.good()){
		memory.insertIntoMemory(c, b); 
		b = ifile.get();
		++c;
	}
	
	memory.setEntry(0, 0xFF0); //initialize the first 2 entries in FAT
	memory.setEntry(1, 0xFF1);
}

void MainMemory::setEntry(ushort pos, ushort val)
{
	if (pos < 0 || pos > LAST_INVALID_ENTRY || (pos + 1) % 3 == 0)
	{
		return;
	}
	
	if (pos % 2 == 0)
	{
		memArray[FIRST_FAT_BYTE + pos] = val & 0xFF; //keep the first 8 bits. FF = 1111 1111. whole byte no need for +=
		memArray[FIRST_FAT_BYTE + pos + 1] += (val & 0xF00) >> 8; //right shift the upper 4 bits by 8. F00 = 1111 0000 0000. (+=) to combine the 2 nibbles into a byte
	}
	else
	{
		memArray[FIRST_FAT_BYTE + pos] += (val & 0x0F) << 4; //keep the first 4 bits. left shift by 4 to fit half of the byte in memory
		memArray[FIRST_FAT_BYTE + pos + 1] = (val & 0xFF0) >> 4; //keep the 5-12 bits (1st bit count start at 1 not 0 for me). shift right by 4 to become 8 bits(1 byte). FF0 = 1111 1111 0000
	}
}

ushort MainMemory::getEntry(ushort pos)
{
	if (pos < 0 || pos > LAST_INVALID_ENTRY || (pos + 1) % 3 == 0)
	{
		return LAST_INVALID_ENTRY;
	}
	
	if (pos % 2 == 0)
	{
		return memArray[FIRST_FAT_BYTE + pos] + ((memArray[FIRST_FAT_BYTE + pos + 1] & 0x0F) << 8); //get the byte at pos. at pos + 1, get the first 4 bits and left shift by 8 to become the 4 msb(most sig bit)
	}
	else
	{
		return (memArray[FIRST_FAT_BYTE + pos] >> 4) + (memArray[FIRST_FAT_BYTE + pos + 1] << 4); //get the shared nibble and right shift by 4 to become the 4 lsb. get the byte at pos + 1 and left shift by 4 to make room for the incoming nibble
	}
}

void MainMemory::insertIntoMemory(int pos, byte data)
{
	memArray[pos] = data;
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
