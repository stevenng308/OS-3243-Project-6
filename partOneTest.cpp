// CS3243 Operating Systems
// Fall 2013
// Project 6: Disks and File Systems
// Jestin Keaton and Steven Ng
// Date: 12/2/2013
// File: partone.cpp


#include<iostream>
#include<fstream>
#include<deque>
#include<cstdlib>
#define BYTECOUNT 1474560
#define BEGIN_BYTE_ENTRY 16896
#define SECTOR_SIZE 512

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

MainMemory memory;
deque<int> freeSectors;

void loadSystem();
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
    memory.findFreeSector();
	for (uint i = 0; i < freeSectors.size(); i++)
	{
		printf("%d", freeSectors[i]);
		printf("\n");
	}
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

MainMemory::MainMemory()
{
	for (int i = 0; i < BYTECOUNT; i++)
	{
		memArray[i] = 157;
	}
}

void MainMemory::findFreeSector(){
	int sector = 33;
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
	}
	
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
