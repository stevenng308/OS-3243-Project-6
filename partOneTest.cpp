// CS3243 Operating Systems
// Fall 2013
// Project 6: Disks and File Systems
// Jestin Keaton and Steven Ng
// Date: 12/2/2013
// File: partone.cpp


#include <iostream>
#include <fstream>
#include <cstdlib>
#include <math.h>
#include <stdio.h>
#define BYTECOUNT 1474560
#define BEGIN_BYTE_ENTRY 16896
#define SECTOR_SIZE 512
#define MAX_FAT_ENTRY 2848 // 2879 - 33 + 1 = 2847 + 2 reserved = 2849 (entries from 0 to 2848 inclusive, 0 and 1 reserved)
#define START_FAT 2
#define FIRST_FAT_BYTE 512
#define FAT_SIZE 4608 // (bytes)
#define LAST_INVALID_ENTRY 3071 // first invalid entry is 2849
#define FIRST_FILE_BYTE 9728
#define FILE_ENTRY_SIZE 32
#define SECTOR_SIZE 512

using namespace std;

// Define Structs

typedef unsigned char byte;

string bar = "           |----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----\n";
string menuOptions = "\nMenu:\n1) List Directory\n2) Copy file to disk\n3) Delete file\n4) Rename a file\n5) Usage map\n6) Directory dump\n7) FAT dump\n8) FAT chain\n9) Sector dump\n0) Quit\n> ";
int freeFatEntries = MAX_FAT_ENTRY + 1 - 2; // added 1 to get quantity, subtract 2 since entries 0 and 1 are reserved

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
	uint size;
	
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
void setEntry(ushort pos, ushort val);
void printFAT();
ushort getEntry(ushort pos);
ushort findFreeFat(ushort a);
void insertFile(File &f, int start);
void createFile(byte n[8], byte e[3], byte a, ushort r, ushort ct, ushort cd, ushort lad, ushort i, ushort lmt, ushort lmd, ushort fls, uint s);
void copyFileToDisk();
int findEmptyDirectory();
ushort getCurrDate();
ushort getCurrTime();
ushort setFatChain(ushort pos, int size);


int main(){
    /*
    ifstream ifile("fdd.flp",std::ifstream::in);
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
    cout << "begin updating file bytes\n";
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
	memory.memArray[start + 12] = f.reserved >> 8;
	memory.memArray[start + 13] = f.reserved & 0xFF;
	memory.memArray[start + 14] = f.createTime >> 8;
	memory.memArray[start + 15] = f.createTime & 0xFF;
	memory.memArray[start + 16] = f.createDate >> 8;
	memory.memArray[start + 17] = f.createDate & 0xFF;
	memory.memArray[start + 18] = f.lastAccessDate >> 8;
	memory.memArray[start + 19] = f.lastAccessDate & 0xFF;
	memory.memArray[start + 20] = f.ignore >> 8;
	memory.memArray[start + 21] = f.ignore & 0xFF;
	memory.memArray[start + 22] = f.lastModifyTime >> 8;
	memory.memArray[start + 23] = f.lastModifyTime & 0xFF;
	memory.memArray[start + 24] = f.lastModfiyDate >> 8;
	memory.memArray[start + 25] = f.lastModfiyDate & 0xFF;
	memory.memArray[start + 26] = f.firstLogicalSector >> 8;
	memory.memArray[start + 27] = f.firstLogicalSector & 0xFF;
	memory.memArray[start + 28] = (f.size & 0xFF000000) >> 24;
	memory.memArray[start + 29] = (f.size & 0xFF0000) >> 16;
	memory.memArray[start + 30] = (f.size & 0xFF00) >> 8;
	memory.memArray[start + 31] = f.size & 0xFF;
    cout << "finished updating file bytes\n";

    // This part actual copies the file to the disk, 1 sector at a time
    // First get the file name
    cout << "test 1\n";
    string fHandle = "";
    for(int i = 0; i < 8; i++){
        if(f.name[i] != ' ')
            fHandle += f.name[i];
    }
    fHandle += '.';
    for(int i = 0; i < 3; i++){
        if(f.ext[i] != ' ')
            fHandle += f.ext[i];
    }
    cout << "test 2\n";
    // Create the input file stream to handle the correct file
    ifstream ifile(fHandle.c_str(),std::ifstream::in);
    // Set the beginning spot on disk where to insert the file
    ushort startSector = f.firstLogicalSector;
    int startByte = (startSector + 33 - 2) * 512;
    // take a byte from the file
    byte b = ifile.get();
    int counter = 0; 
    int filesize = 0;
    // while the file is still good, keep taking bytes out of it
    while(ifile.good()){
        cout << "test 3, startSector = " << startSector << "\n";
        memory.memArray[startByte + counter] = b; // set the byte on disk to the current byte from the file
        cout << "test 4\n";
        filesize++;
        b = ifile.get();
        counter++;
        // If we have reached the end of the sector, we must move to next sector
        if(counter==512/* && getEntry(startSector)!=0xFFF*/){
            startSector = getEntry(startSector);
            startByte = (startSector + 33 - 2) * 512;
            counter = 0; // restart the counter to begin at the start of next sector
        }
    }
    ifile.close();
    cout << "number of bytes copied to disk: " << filesize << endl;
}

void copyFileToDisk(){
    // These 11 following variables and 's' at the bottom will be passed to the createFile method
    byte n[8];
    byte e[3];
    byte a = 0;
    ushort r = 0;
    ushort ct = getCurrTime();
    ushort cd = getCurrDate();
    ushort lad = cd;
    ushort i = 0;
    ushort lmt = ct;
    ushort lmd = cd;
    ushort fls = findFreeFat(1); // These 11 

    string fHandle;
    string fName;
    string extension;
    cout << "Filename to copy to the simulated disk: ";
    cin >> fHandle;
    extension = fHandle.substr(fHandle.find(".")+1,3);
    fName = fHandle.substr(0,fHandle.find("."));
    fHandle = fName+'.'+extension;
    ifstream iFile(fHandle.c_str());
    if(iFile.good() && fName.length() < 9){
        unsigned k = 8 - fName.length(), j = 0; 
        for(;k < 8; ++k, ++j){
            if(j < fName.length())
                n[k] = fName.at(j);
        }
        for(k = 0; k < 8 - fName.length(); k++)
            n[k] = ' ';
        k = 3 - extension.length(), j = 0;
        for(;k < 3; ++k, ++j){
            if(j < extension.length())
                e[k] = extension.at(j);
        }
        for(k = 0; k < 3 - extension.length(); k++)
            e[k] = ' ';
        long start,finish;
        start = iFile.tellg();
        iFile.seekg (0, ios::end);
        finish = iFile.tellg();
        iFile.close();
        int s = (finish-start) & 0xFFFF;
        if(s <= freeFatEntries * 512){
            cout << "start createFile method" << endl;
            createFile(n,e,a,r,ct,cd,lad,i,lmt,lmd,fls,s);
            //printf("\nsize of '%s' : %db\n",fHandle.c_str(),s);
            cout << "end createFile method\n";
        }
        else
            cout << "\nError: Not enough space on disk for the file...\n";
    }
    else
        cout << "Bad File...\n";
}

ushort getCurrDate(){
    ushort result = 0;
    struct tm cd;
    result |= ((cd.tm_mday & 0x1F) << 11); // Day of month goes first (1-31)
    result |= ((cd.tm_mon & 0x0F) << 7); // then comes the month since January
    result |= (cd.tm_year & 0x7F); // and finally the year since 1990
    return result;
}

ushort getCurrTime(){
    ushort result = 0; 
    struct tm ct;
    result |= ((ct.tm_hour & 0x1F) << 11); // Hour of day comes first (0-23)
    result |= ((ct.tm_min & 0x3F) << 7); // then comes the minutes after the hour (0-59)
    result |= (ct.tm_sec%30 & 0x1F); // and finally the number pair of seconds (0-29)
    return result;
}

void createFile(byte n[8], byte e[3], byte a, ushort r, ushort ct, ushort cd, ushort lad, ushort i, ushort lmt, ushort lmd, ushort fls, uint s)
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
    setEntry(fls,setFatChain(fls,s)); // set up the FAT chain for this file
	int startIndex = findEmptyDirectory();
    cout << "start insertFile method with startIndex = " << startIndex << "\n";
	insertFile(myFile, startIndex);
    cout << "end insertFile method\n";
    cout << "Testing chain starting at FAT entry: " << fls << endl;
    printFAT();
}

/**
* Recursively finds free FAT entries to add to the chain for this file
* param fls the FAT entry short where we begin
* param size the size of the file, or what's left of it at this point
*/
ushort setFatChain(ushort pos, int size){
    int count = size - 512;
    if(count > 0){ // more bytes left in file
        setEntry(pos,findFreeFat(pos)); // set current FAT entry to point to new free FAT entry
        setFatChain(getEntry(pos),count);
        return getEntry(pos);
    }
    else // this is last FAT entry for the file
        setEntry(pos,0xFFF);
        return 0xFFF;
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
    for(ushort i = 2; i <= MAX_FAT_ENTRY; i++){
        setEntry(i, 0x00);
    }
    // The following FAT entries are invalid and do not represent
    // any available sector on disk.
    for(ushort i = MAX_FAT_ENTRY + 1; i <= LAST_INVALID_ENTRY; i++){
        setEntry(i, 0xFF7);
    }
}

/**
* Sets a FAT and FAT2 entries to be the short value that is entered
* param val the value we intend to set in the FAT
* param pos the index in the FAT tables
*/
void setEntry(ushort pos, ushort val){
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
ushort getEntry(ushort pos){
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

ushort findFreeFat(ushort a)
{
    for (int i = 2; i <= MAX_FAT_ENTRY; i++)
	{
		if (getEntry(i) == 0 && i != a)
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
