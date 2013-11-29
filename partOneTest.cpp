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
#define MAX_FAT_ENTRY 2848 //2879 - 33 = 2846 + 2 reserved = 2848
#define START_FAT 2
#define FIRST_FAT_BYTE 512
#define FAT_SIZE 4608 //going to have to rethink the limits on FAT size. I am getting different #. 5119 is the last spot for the last entry in FAT table.
#define LAST_INVALID_ENTRY 3071 // first invalid entry is 2849
#define FIRST_FILE_BYTE 9728
#define FILE_ENTRY_SIZE 32
using namespace std;

// Define Structs

typedef unsigned char byte;

string bar = "           |----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----\n";
string menuOptions = "\nMenu:\n1) List Directory\n2) Copy file to disk\n3) Delete file\n4) Rename a file\n5) Usage map\n6) Directory dump\n7) FAT dump\n8) FAT chain\n9) Sector dump\n10) Quit\n> ";

struct MainMemory{
    byte memArray[BYTECOUNT]; // 0-511 is for the boot partition
    
    MainMemory();
    void findFreeSector();
    void insertIntoMemory(byte b);
    void print();
};

struct File
{
	byte name[8];
	byte ext[3];
	byte attr;
	ushort reserved;
	ushort createTime;
	ushort createDate;
	ushort lastAccessDate;
	ushort ignore;
	ushort lastModifyTime;
	ushort lastModfiyDate;
	ushort firstLogicalSector;
	int size;
	
	File(); 
};


// Since we can access the FAT directly, there may be no need for these extra structs
/**
struct Entry
{
	char a : 8;
	char b : 8;
	char c : 8;
};/create


struct FileTable
{
	Entry table[MAX_FAT_ENTRY];
};*/



MainMemory memory;
//deque<int> freeSectors;


// Declare Methods
void loadSystem();
void initializeFAT();
void setEntry(short pos, short val);
void printFAT();
short getEntry(short pos);
short findFreeFat();
void insertFile(File &f, int start);
void createFile(byte n, byte e, byte a, ushort r, ushort ct, ushort cd, ushort lad, ushort i, ushort lmt, ushort lmd, ushort fls, ushort s);
void copyFileToDisk();
int findEmptyDirectory();


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
    initializeFAT();
    int answer;
    do{
        cout << menuOptions;
        cin >> answer;
        switch(answer){
            case 1:
                //something
                break;
            case 2:
                // Ask for name of file from user
                copyFileToDisk();
                break;
            case 3:
                //something
                break;
            case 4:
                //something
                break;
            case 5:
                //something
                break;
            case 6:
                //something
                break;
            case 7:
                //something
                break;
            case 8:
                //something
                break;
            case 9:
                //something
                break;
            default:
                return 0;
        }
    }
    while(answer >= 1 && answer <= 9);
    memory.print();
    printFAT();
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

void insertFile(File &f, int start)
{
	memory.memArray[start] = f.name[0];
	memory.memArray[start + 1] = f.name[1];
	memory.memArray[start + 2] = f.name[2];
	memory.memArray[start + 3] = f.name[3];
	memory.memArray[start + 4] = f.name[4];
	memory.memArray[start + 5] = f.name[5];
	memory.memArray[start + 6] = f.name[6];
	memory.memArray[start + 7] = f.name[7];
	memory.memArray[start + 8] = f.ext[0];
	memory.memArray[start + 9] = f.ext[1];
	memory.memArray[start + 10] = f.ext[2];
	memory.memArray[start + 11] = f.attr;
	memory.memArray[start + 12] = f.reserved & 0xFF;
	memory.memArray[start + 13] = f.reserved >> 8;
	memory.memArray[start + 14] = f.createTime & 0xFF;
	memory.memArray[start + 15] = f.createTime >> 8;
	memory.memArray[start + 16] = f.createDate & 0xFF;
	memory.memArray[start + 17] = f.createDate >> 8;
	memory.memArray[start + 18] = f.lastAccessDate & 0xFF;
	memory.memArray[start + 19] = f.lastAccessDate >> 8;
	memory.memArray[start + 20] = f.ignore & 0xFF;
	memory.memArray[start + 21] = f.ignore >> 8;
	memory.memArray[start + 22] = f.lastModifyTime & 0xFF;
	memory.memArray[start + 23] = f.lastModifyTime >> 8;
	memory.memArray[start + 24] = f.lastModfiyDate & 0xFF;
	memory.memArray[start + 25] = f.lastModfiyDate >> 8;
	memory.memArray[start + 26] = f.firstLogicalSector & 0xFF;
	memory.memArray[start + 27] = f.firstLogicalSector >> 8;
	memory.memArray[start + 28] = f.size & 0xFF;
	memory.memArray[start + 29] = f.size & 0xFF00;
	memory.memArray[start + 30] = f.size & 0xFF0000;
	memory.memArray[start + 31] = f.size & 0xFF000000;
}

void copyFileToDisk(){
    string fHandle;
    string fName;
    string extension;
    cout << "Filename to copy to the simulated disk: ";
    cin >> fHandle;
    extension = fHandle.substr(fHandle.find(".")+1,3);
    fName = fHandle.substr(0,fHandle.find("."));
    cout << "\nfile name = " << fName << endl;
    cout << "extension = " << extension << endl;
     


}

void createFile(byte n[8], byte e[3], byte a, ushort r, ushort ct, ushort cd, ushort lad, ushort i, ushort lmt, ushort lmd, ushort fls, int s)
{
	File myFile;
	myFile.name[0] = n[0];
	myFile.name[1] = n[1];
	myFile.name[2] = n[2];
	myFile.name[3] = n[3];
	myFile.name[4] = n[4];
	myFile.name[5] = n[5];
	myFile.name[6] = n[6];
	myFile.name[7] = n[7];
	myFile.ext[0] = e[0];
	myFile.ext[1] = e[1];
	myFile.ext[2] = e[2];
	myFile.attr = a;
    myFile.reserved = r;
	myFile.createTime = ct;
	myFile.createDate = cd;
	myFile.lastAccessDate = lad;
	myFile.ignore = i;
	myFile.lastModifyTime = lmt;
	myFile.lastModfiyDate = lmd;
	myFile.firstLogicalSector = fls;
	myFile.size = s;
	
	int startIndex = findEmptyDirectory();
	insertFile(myFile, startIndex);
}

int findEmptyDirectory(){
    for(int i = FIRST_FILE_BYTE; i < BEGIN_BYTE_ENTRY; i+= 32){
        if(memory.memArray[i] == 0xE5 || memory.memArray[i] == 0x00){
            return i;
        }
    }
    return -1;
}

void initializeFAT(){
    setEntry(0, 0xFF0);
    setEntry(1, 0xFF1);
    // Initialize all the unused sectors
    for(short i = 2; i <= MAX_FAT_ENTRY; i++){
        setEntry(i, 0x00);
    }
    // The following FAT entries are invalid and do not represent
    // any available sector on disk.
    for(short i = MAX_FAT_ENTRY + 1; i <= LAST_INVALID_ENTRY; i++){
        setEntry(i, 0xFF7);
    }
}

/**
* Sets a FAT and FAT2 entries to be the short value that is entered
* param val the value we intend to set in the FAT
* param pos the index in the FAT tables
*/
void setEntry(short pos, short val){
    // In the schema [yz Zx XY], the lower-case letters represent what we call
    // the low-order entry, the high-order entry would be the upper-case letters.
    if(pos<0 || pos > LAST_INVALID_ENTRY)
        return; // faulty sector selection, do nothing
    // Set both FAT and FAT2 entries 
    int start;
    for(int i = 0; i < 2; i++){
        start = (FIRST_FAT_BYTE + i * FAT_SIZE) + (pos/2) * 3; // set first byte of entry pair
        if(pos%2==0){ // setting a low-order entry
            memory.memArray[start] = (val &  0xFF); // set yz to the lowest byte of short <val>
            memory.memArray[start + 1] &= 0xF0; // clear low nibble here
            memory.memArray[start + 1] |= ((val >> 8) & 0x0F); // set x to nibble 2 of <val>
        }
        else{ // setting a high-order entry
            memory.memArray[start + 2] = ((val & 0x0FF0) >> 4); // set XY to nibbles 2 and 3 of <val>
            memory.memArray[start + 1] &= 0x0F; // clear high nibble here
            memory.memArray[start + 1] |= ((val & 0xF) << 4); // set Z to nibble 4 in <val>
        }
    }
}

/**
* Prints the contents of the FAT tables
*/
void printFAT(){
    cout << "\nPRIMARY FAT TABLE:\n";
    for(int i = 0; i < (LAST_INVALID_ENTRY+1)/20 + 1; i++){
        if((i+1)*20-1 <= LAST_INVALID_ENTRY)
            printf("%04d-%04d: ",i*20,(i+1)*20-1);
        else
            printf("%04d-%04d: ",i*20,LAST_INVALID_ENTRY);
        for(int j = 0; j < 20; j++){
            if(i*20+j <= LAST_INVALID_ENTRY)
                printf("%03x ",getEntry(i*20+j));   
        }
        cout << endl;
    }
    cout << endl;
}

/**
* Retrieves the value sent in the parameter from the FAT table
* param b the index in the FAT table that we want
*/
short getEntry(short pos){
    // Using the same schema as in 'setEntry()' we will extract a FAT entry's
    // value and return it as a short [schema: yz Zx XY]
    if(pos<0 || pos > LAST_INVALID_ENTRY)
        return -1; // faulty request here, return error code (-1)
    int start = FIRST_FAT_BYTE + (pos/2) * 3; // set first byte of entry pair
    if(pos%2==0){ // requesting a low-order entry
        return memory.memArray[start] + ((memory.memArray[start + 1] & 0x0F) << 8);
    }
    else{ // requesting a high-order entry
        return (memory.memArray[start + 2] << 4) + (memory.memArray[start + 1] >> 4);
    }
}

short findFreeFat()
{
	for (int i = FIRST_FAT_BYTE; i <= MAX_FAT_ENTRY; i++)
	{
		if (memory.memArray[i] == 0)
		{
			return i;
		}
	}
	return -1;
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

File::File()
{
	name[0] = 0;
	attr = 0;
	reserved = createDate = createTime = lastAccessDate = lastModfiyDate = lastModifyTime = ignore = 0;
	size = 0.0f;
	firstLogicalSector = 0;
}
