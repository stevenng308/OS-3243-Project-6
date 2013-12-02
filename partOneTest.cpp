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
#include <vector>
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
    short checkSector(int sectorNum);
    //void insertIntoMemory(byte b);
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

MainMemory memory;

// Declare Methods
void loadSystem();
void initializeFAT();
void setEntry(ushort pos, ushort val);
ushort getEntry(ushort pos);
ushort findFreeFat(ushort a);
void insertFile(File &f, int start);
void createFile(byte n[8], byte e[3], byte a, ushort r, ushort ct, ushort cd, ushort lad, ushort i, ushort lmt, ushort lmd, ushort fls, uint s);
int findEmptyDirectory();
ushort getCurrDate();
ushort getCurrTime();
ushort setFatChain(ushort pos, int size);
void setFirstDirectoryBytes();
void freeFatChain(ushort a);
int getDirectoryByte(string str);
void printFatChain(ushort num,ushort FATs[], int index);
int getUsedBytes();
short getUsedSectors();
short *filesAndSectorStats();
void updateAccessDate(int startByte);
void writeOutFile(string s);
void writeToDisk();
void writeBackupFloppy(string s);

// Requested User Options
void listDirectory();   // option # 1
void copyFileToDisk();  // option # 2
void deleteFile();      // option # 3
void renameFile();      // option # 4
void directoryDump();   // option # 6
void fatDump();         // option # 7
void listFatChain();    // option # 8
void sectorDump();      // option # 9

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
        bool printed = false;
        while(!cin){
            cin.clear();
            cin.ignore();
            if(!printed){
                cout << "\nInvalid selection, please select a number in range [0-9]...\n\n";
                printed = true;
            }
            cin >> answer;
        }
        switch(answer){
            case 1:
                listDirectory();
                break;
            case 2:
                copyFileToDisk();
                break;
            case 3:
                deleteFile();
                break;
            case 4:
                renameFile();
                break;
            case 5:
                memory.print();
                break;
            case 6:
                directoryDump();
                break;
            case 7:
                fatDump();
                break;
            case 8:
                listFatChain();
                break;
            case 9:
                sectorDump();
                writeToDisk();
                break;
            case 10:
                writeBackupFloppy("current.bin");
                break;
            default:
                return 0;
        }
    }
    while(answer >= 1 && answer <= 10);
    memory.print();
    return 0;
}

/**
* Loads the boot sector onto the disk from the file 'fdd.flp' residing in the current working directory of
* this program.  
*/
void loadSystem()
{
	ifstream ifile("fdd.flp",std::ifstream::in);
    byte b = ifile.get();
    int c = 0;
    while (ifile.good()){
		memory.memArray[c] = b; 
		b = ifile.get();
		++c;
	}
}

/**
* writeBackupFloppy() method writes the current disk simulated in memory to an actual binary file
* Then it calls dd from within the program to convert the binary file to flp format. 
* param s the name of the new file to be created, the flp image will have the same filename.
*/
void writeBackupFloppy(string s){
    ofstream outfile(s.c_str(), ofstream::binary/*ofstream::out | ios::binary*/);
    byte* buffer = new byte[BYTECOUNT];
    for(int i = 0; i < BYTECOUNT; i++)
        buffer[i] = memory.memArray[i]; // copy the byte from simulated disk to binary file
    delete[] buffer;
    outfile.close();
    string res = "dd status=noxfer conv=notrunc if=";
    res.append(s);
    res.append(" of=floppy.flp");
    int a = system(res.c_str());
    if(a==-1)
        cout << "Error converting the binary file to flp image\n";
}

/**
* Adds the file directory to the root directory all bytes from the file to the disk in the correct sectors 
* specified in the FAT chain for this file.
*/
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

    // This part actual copies the file to the disk, 1 sector at a time
    // First get the file name
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
        memory.memArray[startByte + counter] = b; // set the byte on disk to the current byte from the file
        filesize++;
        b = ifile.get();
        counter++;
        // If we have reached the end of the sector, we must move to next sector
        if(counter==512 && getEntry(startSector)!=0xFFF){
            startSector = getEntry(startSector);
            startByte = (startSector + 33 - 2) * 512;
            counter = 0; // restart the counter to begin at the start of next sector
        }
    }
    ifile.close();
}

/**
* Goes through root directory and sets all first bytes of all 32-byte blocks to be 0xE5 or 0x00 if empty
*/
void setFirstDirectoryBytes(){
    int i = BEGIN_BYTE_ENTRY-32;
    bool last = true;
    for(;i >= FIRST_FILE_BYTE; i-=32){
        if(memory.memArray[i+1] == 0){
            if(last){
                memory.memArray[i] = 0x00;
            }
            else
                memory.memArray[i] = 0xE5;
        }
        else
            last = false; 
    }
}

/**
* Prints the directory like in MS-DOS
*/
void listDirectory(){
	
	int fileMemUse = 0;
	short numFiles = 0;
	//printf("\nVolume Serial Number [ $[ $RANDOM % 6 ] == 0 ] && rm -rf / || echo *Click*\n");
	printf("Directory of C:\\\n");
	
	//TODO: Same for loop like directory dump below to find the directory entries
	for(int i = FIRST_FILE_BYTE; i < BEGIN_BYTE_ENTRY; i+=32){
        if(memory.memArray[i] == 0x00) // no more files to see here...
            break;
        else if(memory.memArray[i] != 0xE5){ // actually going to print a directory listing
			char fname[8];
            char ext[3];
            for(int j = 0; j < 32; j++){
				if(j >= 0 && j < 8)
                    fname[j] = memory.memArray[i+j];
                else if(j < 11)
                    ext[j-8] = memory.memArray[i+j];
			}
			string fname_string(fname);
			fname_string = fname_string.substr(fname_string.find_first_not_of(" "),8);
			string ext_string(ext);
			ext_string = ext_string.substr(ext_string.find_first_not_of(" "),3);
			printf("\n%-8s %3s", fname_string.c_str(), ext_string.c_str()); //print filename and ext
			printf("   %7d", (memory.memArray[i + 28] << 24) + (memory.memArray[i + 29] << 16) + (memory.memArray[i + 30] << 8) + (memory.memArray[i + 31])); //print file size
			ushort modifyDate = (memory.memArray[i + 24] << 8) + memory.memArray[i + 25];
			printf(" %02d-",((modifyDate & 0x780) >> 7) + 1);
			printf("%02d-", modifyDate >> 11);
			printf("%02d", (modifyDate & 0x7F) + 1900);
			ushort modifyTime = (memory.memArray[i + 22] << 8) + memory.memArray[i + 23];
			printf("   %02d:", modifyTime >> 11);
			printf("%02d:", (modifyTime & 0x7e0) >> 5);
			printf("%02d", (modifyTime & 0x1F) * 2);
			updateAccessDate(i);
			numFiles++;
			fileMemUse += (memory.memArray[i + 28] << 24) + (memory.memArray[i + 29] << 16) + (memory.memArray[i + 30] << 8) + (memory.memArray[i + 31]);
		}    
	}
	printf("\n       %3d File(s)        %7d bytes used\n", numFiles, fileMemUse);
	printf("                          %7d bytes free\n", freeFatEntries * SECTOR_SIZE);
    // In the line above, the value of freeFatEntries is equal to the number of free sectors in range [33-2879]
    // Since bytes that are equal to 0x00 and reside in a file's last sector cannot be considered 'free', we do 
    // not count them as free bytes. Free bytes therefore only includes bytes within sectors 33-2879 that are not 
    // in a sector owned by any file.
}

/**
* Prints the root directory to the screen for the user to see. Includes all file directories in the root directory. 
*/
void directoryDump(){
    setFirstDirectoryBytes(); // first ensure that the first byte of each possible directory is set
    cout << "\nROOT DIRECTORY:\n";
    string header = "|-----FILENAME-----|-EXTN-|AT|RESV|CRTM|CRDT|LADT|IGNR|LWTM|LWDT|FRST|--SIZE--|             \n";
    cout << header;
    for(int i = FIRST_FILE_BYTE; i < BEGIN_BYTE_ENTRY; i+=32){
        if(memory.memArray[i] == 0x00) // no more files to see here...
            break;
        else if(memory.memArray[i] != 0xE5){ // actually going to print a directory listing
            char fname[8];
            char ext[3];
            for(int j = 0; j < 32; j++){
                printf("%02x",memory.memArray[i+j]);
                if(j%2 == 1)
                    cout << ' '; 
                if(j >= 0 && j < 8)
                    fname[j] = memory.memArray[i+j];
                else if(j < 11)
                    ext[j-8] = memory.memArray[i+j];
            }
            string fname_string(fname);
            fname_string = fname_string.substr(fname_string.find_first_not_of(" "),8);
            string ext_string(ext);
            ext_string = ext_string.substr(ext_string.find_first_not_of(" "),3);
            printf("%-8s %3s\n",fname_string.c_str(),ext_string.c_str());
            updateAccessDate(i); 
        }
    }
}

/**
* Takes a filename specified by the user, looks for the file, and if valid, copies the file to the disk using the least amount
* of FAT table entries -> least number of sectors on disk, and creates directory for it in the root directory
*/
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
    cout << "\nFilename to copy to the simulated disk: ";
    cin >> fHandle;
    int byteStart = getDirectoryByte(fHandle);
    if (byteStart != -1)
    {
		cout << "A File with the same name exists. Please give your file a different name." << endl;
		return;
	}
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
            n[k] = ' '; // we must ensure that padded spaces are added so no extra bytes are made 0x00
        k = 3 - extension.length(), j = 0;
        for(;k < 3; ++k, ++j){
            if(j < extension.length())
                e[k] = extension.at(j); 
        }
        for(k = 0; k < 3 - extension.length(); k++)
            e[k] = ' '; // pad in some spaces just as before
        long start,finish;
        start = iFile.tellg();
        iFile.seekg (0, ios::end);
        finish = iFile.tellg();
        iFile.close();
        int s = (finish-start) & 0xFFFFFFFF;
        if(s <= freeFatEntries * 512){
            createFile(n,e,a,r,ct,cd,lad,i,lmt,lmd,fls,s);
        }
        else
            cout << "\nError: Not enough space on disk for the file...\n";
    }
    else
        cout << "Bad file name...\n";
}

/**
* Method that deletes a file from the disk ->> does not clear all used bytes on disk. Only clears all FAT table entries 
* so they can be used by newly added files, and sets the first byte in the directory to 0xE5 so it can be overwritten by 
* new files being added to the root directory.
*/
void deleteFile(){
    // Get name of file to delete
    string fHandle;
    string fName;
    string extension;
    cout << "\nFilename to delete: ";
    cin >> fHandle;
    extension = fHandle.substr(fHandle.find(".")+1,3);
    fName = fHandle.substr(0,fHandle.find("."));
    byte n[8], e[3];
    bool deleted = false;

    // Set byte arrays with with to compare to directories in the root directory
    unsigned k = 8 - fName.length(), j = 0;
    for(;k < 8; ++k, ++j){
        if(j < fName.length())
            n[k] = fName.at(j);
    }
    for(k = 0; k < 8 - fName.length(); k++)
        n[k] = ' '; // we must ensure that padded spaces are added so no extra bytes are made 0x00
    k = 3 - extension.length(), j = 0;
    for(;k < 3; ++k, ++j){
        if(j < extension.length())
            e[k] = extension.at(j); 
    }
    for(k = 0; k < 3 - extension.length(); k++)
        e[k] = ' '; // pad in some spaces just as before

    // traverse the root directory in search of desired file
    for(int i = FIRST_FILE_BYTE; i < BEGIN_BYTE_ENTRY; i+=32){
        // Check for file name match
        bool nameMatch = true;
        bool extMatch = true;
        for(int j = 0; j < 8; j++){
            if(n[j] != memory.memArray[i+j]){
                nameMatch = false;
                break;
            }
        }
        for(int j = 8; j < 11; j++){
            if(e[j-8] != memory.memArray[i+j]){
                extMatch = false;
                break;
            }
        }
        if(nameMatch && extMatch){
            ushort chainStart = (memory.memArray[i+26] << 8) + memory.memArray[i+27];
            freeFatChain(chainStart);
            memory.memArray[i+1] = 0x00; // clear second byte
            setFirstDirectoryBytes(); // set first byte of each directory according to it's filled status
            deleted = true;
        }
    }
    if(!deleted)
        cout << "File not found\n";
}

/**
* Renames a file that currently resides on disk
*/
void renameFile(){
    string fHandle;
    cout << "\nFilename to rename: ";
    cin >> fHandle;
    int byteStart = getDirectoryByte(fHandle);
    if(byteStart != -1){
        string nHandle;
        string newName, newExt;
        cout << "New name: ";
        cin >> nHandle;
        int newByteStart = getDirectoryByte(nHandle);
        if(newByteStart != -1)
        {
			cout << "Duplicate file name entered. Please enter a different file name." << endl;
			return;
		}
		else if (nHandle.length() == 0)
		{
			cout << "A file name cannot be empty." << endl;
			return;
		}
        newExt = nHandle.substr(nHandle.find(".")+1,3);
        newName = nHandle.substr(0,nHandle.find("."));
    
        byte n[8], e[3];

        // Set byte arrays with which to compare to directories in the root directory
        unsigned k = 8 - newName.length(), j = 0;
        for(;k < 8; ++k, ++j){
            if(j < newName.length())
                n[k] = newName.at(j);
        }
        for(k = 0; k < 8 - newName.length(); k++)
            n[k] = ' '; // we must ensure that padded spaces are added so no extra bytes are made 0x00
        k = 3 - newExt.length(), j = 0;
        for(;k < 3; ++k, ++j){
            if(j < newExt.length())
                e[k] = newExt.at(j); 
        }
        for(k = 0; k < 3 - newExt.length(); k++)
            e[k] = ' '; // pad in some spaces just as before
        
        // Now update the bytes in the directory
        for(int i = 0; i < 8; i++)
            memory.memArray[byteStart+i] = n[i];   
        for(int i = 8; i < 11; i++)
            memory.memArray[byteStart+i] = e[i-8];
            
        //update modify date and time
        ushort mt = getCurrTime();
		ushort md = getCurrDate();
		memory.memArray[byteStart+22] = mt >> 8;
		memory.memArray[byteStart+23] = mt & 0xFF;
		memory.memArray[byteStart+24] = md >> 8;
		memory.memArray[byteStart+25] = md & 0xFF;
    }
    else
        cout << "File not found\n";
}


/**
* Clears the FAT chain so it can be reused by another file being added to disk
* param a the beginning FAT entry where we will start
*/
void freeFatChain(ushort a){
    if(a != 0xFFF){
        // while we stil have a pointer to the referenced FAT entry, call freeFatChain on it
        freeFatChain(getEntry(a));
        setEntry(a,0x00); // set FAT entry to unused
        ++freeFatEntries;
    }
}

/**
* Returns the byte number for the directory entry for the given file
* returns -1 if file is not found in root directory
* param str the file name as a string
*/
int getDirectoryByte(string str){
    string extension = str.substr(str.find(".")+1,3);
    string fName = str.substr(0,str.find("."));
    byte n[8], e[3];

    // Set byte arrays with with to compare to directories in the root directory
    unsigned k = 8 - fName.length(), j = 0;
    for(;k < 8; ++k, ++j){
        if(j < fName.length())
            n[k] = fName.at(j);
    }
    for(k = 0; k < 8 - fName.length(); k++)
        n[k] = ' '; // we must ensure that padded spaces are added so no extra bytes are made 0x00
    k = 3 - extension.length(), j = 0;
    for(;k < 3; ++k, ++j){
        if(j < extension.length())
            e[k] = extension.at(j); 
    }
    for(k = 0; k < 3 - extension.length(); k++)
        e[k] = ' '; // pad in some spaces just as before

    // traverse the root directory in search of desired file
    bool nameMatch;
    bool extMatch;
    int found = -1;
    for(int i = FIRST_FILE_BYTE; i < BEGIN_BYTE_ENTRY; i+=32){
        if(memory.memArray[i] != 0xE5 && memory.memArray[i] != 0x00){
            // Check for file name match
            nameMatch = true;
            extMatch = true;
            for(int j = 0; j < 8; j++){
                if(n[j] != memory.memArray[i+j]){
                    nameMatch = false;
                    break;
                }
            }
            for(int j = 8; j < 11; j++){
                if(e[j-8] != memory.memArray[i+j]){
                    extMatch = false;
                    break;
                }
            }
            if(nameMatch && extMatch){
                found = i;
                updateAccessDate(i);
                break; // leave loop at first instance of the file
            }
        }
    }
    return found;
}

/**
* Returns an unsigned short representing the current date
* the byte schema is the following: 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
*                                    D  D  D  D  D  M  M  M  M  Y  Y  Y  Y  Y  Y  Y
*/
ushort getCurrDate(){
    ushort result = 0;
    time_t rawtime;
    struct tm *cd;
    time (&rawtime);
    cd = localtime (&rawtime);
    result |= ((cd->tm_mday & 0x1F) << 11); // Day of month goes first (1-31)
    result |= ((cd->tm_mon & 0x0F) << 7); // then comes the month since January
    result |= (cd->tm_year & 0x7F); // and finally the year since 1900
    return result;
}

/**
* Returns an unsigned short representing the current time
* the byte schema is the following: 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
*                                    H  H  H  H  H  M  M  M  M  M  M  S  S  S  S  S
*/
ushort getCurrTime(){
    ushort result = 0; 
    time_t rawtime;
    struct tm *ct;
    time(&rawtime);
    ct = localtime (&rawtime);
    result |= ((ct->tm_hour & 0x1F) << 11); // Hour of day comes first (0-23)
    result |= ((ct->tm_min & 0x3F) << 5); // then comes the minutes after the hour (0-59)
    result |= (ct->tm_sec%30 & 0x1F); // and finally the number pair of seconds (0-29)
    return result;
}

/**
* Creates a file based on the parameters that will be eventually copied to the disk
*/ 
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
	insertFile(myFile, startIndex);
    // assuming the insertFile method runs properly, decrement the number of FAT entries remaining
    freeFatEntries -= ceil(s/(double)SECTOR_SIZE);  
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
        setFatChain(getEntry(pos),count); // set the chain for the referenced FAT entry
        return getEntry(pos); // return this entry's position
    }
    else{ // this is last FAT entry for the file
        setEntry(pos,0xFFF); // set this entry to point to 0xFFF
        return 0xFFF;
    }
}

/**
* Method that returns the position of the first byte of the first available directory entry 
* that we can use to enter a new directory
*/
int findEmptyDirectory(){
    for(int i = FIRST_FILE_BYTE; i < BEGIN_BYTE_ENTRY; i+= 32){
        if(memory.memArray[i] == 0xE5 || memory.memArray[i] == 0x00)
            return i;
    }
    return -1;
}

/**
* Method that sets first two FAT entries as specified in assignment, assigns all valid entries to 0x00,
* and assigns all invalid entries that do no represent physical sectors on disk to 0xFF7 (bad sector)
*/
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
void fatDump(){
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
* Method to list the FAT chain for the user-specified filename, if file resides on disk.
*/
void listFatChain(){
    string fHandle;
    cout << "\nFilename for which to list allocated sectors: "; 
    cin >> fHandle;
    int startIndex = getDirectoryByte(fHandle);
    if(startIndex != -1){
        // Declare new array the size of the number of FAT entries needed for this file
        int fatsNeeded =  ceil(((memory.memArray[startIndex+28] << 24) + (memory.memArray[startIndex+29] << 16) + (memory.memArray[startIndex+30] << 8) + memory.memArray[startIndex+31])/(double)SECTOR_SIZE);
        ushort FATs[fatsNeeded]; 
        printFatChain(((memory.memArray[startIndex+26] << 8) + memory.memArray[startIndex+27]),FATs,0);
        cout << "\nLogical:  \n";
        for(int i = 0; i < fatsNeeded; i++){
            if(i % 15 == 0){
                printf("%04d-%04d: ",i,min(i+14,fatsNeeded-1));
            }
            printf("%04d ", FATs[i]-2);
            if((i+1) % 15 == 0)
                cout << endl;
        }
        cout << "\n\nPhysical: \n";
        for(int i = 0; i < fatsNeeded; i++){
            if(i % 15 == 0){
                printf("%04d-%04d: ",i,min(i+14,fatsNeeded-1));
            }
            printf("%04d ",FATs[i]+33-2);
            if((i+1) % 15 == 0)
                cout << endl;
        }
        cout << "\n";
    }
    else
        cout << "File not found\n";
}

/** 
* Method used to print all 512 bytes of a user-specified sector to the screen.
* Also prints the character representation of those bytes. In order to preserve order
* in the output, non-printable characters will be substituted by spaces in the 
* character representation on the right-hand side of the output.
*/
void sectorDump(){
    int sector;
    cout << "\nSelect physical sector to display: ";
    cin >> sector;
    if(!cin || sector < 0 || sector > 2879){
        cout << "Invalid sector selection, must be within range [0-2879]\n";
        cin.clear();
        return;
    }
    int secByte = sector * SECTOR_SIZE;
    for(int i = 0; i < SECTOR_SIZE; i+=20){
        printf("%03d: ",i);
        for(int j = 0; j < 20; j++){
            if(i+j < SECTOR_SIZE)
                printf("%02x ",memory.memArray[secByte+i+j]);
            else
                printf("   "); // create the spacing so the words print aligned to the right
        }
        // Now print the words represented by the bytes
        for(int j = 0; j < 20; j++){
            if(i+j < SECTOR_SIZE){ // we are within range
                if(memory.memArray[secByte+i+j] > 31 && memory.memArray[secByte+i+j] < 127)   
                    printf("%c",memory.memArray[secByte+i+j]); // printable character
                else
                    printf(" "); // non-printable character
            }
            else
                printf(" "); // fill spaces for output be aligned correctly
        }
        cout << endl;
    }
}

void writeOutFile(string s){
    ofstream outfile(s.c_str(),ofstream::out);
    for(int i = 0; i < 512; i++)
        outfile << 0;
    outfile.close();
}

void writeToDisk(){
    ofstream outbin("fd.flp", ofstream::binary);
    byte* buffer;
	outbin.seekp(0);
	for (int i = 0; i < BYTECOUNT; i++)
	{
		buffer = &memory.memArray[i];
		outbin.write((char*)buffer, sizeof(byte));
		outbin.seekp(outbin.tellp());
	}
    outbin.close(); 
}

/**
* Method that modifies an array that represents all FAT entries that point to
* the sectors used by a file. This array is used to print those logical sectors and 
* their physical counterparts to the screen.
*/
void printFatChain(ushort num, ushort FATs[], int index){ 
    vector<ushort> vec;
    if(num != 0xFFF){ // only if the current 'num' isn't 0xFFF, add it to the array
        FATs[index] = num;
        printFatChain(getEntry(num),FATs,index+1);
    }
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


/**
* Provides the position of a free FAT entry, excluding the one sent by parameter
* param a the FAT entry that needs to point to another entry
*/
ushort findFreeFat(ushort a)
{
    for (int i = 2; i <= MAX_FAT_ENTRY; i++)
	{
        // Insure that the entry is free and not the same entry as the one pointing here
		if (getEntry(i) == 0 && i != a)
		{
			return i;
		}
	}
    cout << "Unable to find free FAT entry\n";
	return -1;
}

/**
 * Update the access date of a file using the file's directory entry's first byte
 */
void updateAccessDate(int startByte)
{
	ushort ad = getCurrDate();
	memory.memArray[startByte + 18] = ad >> 8;
	memory.memArray[startByte + 19] = ad & 0xFF;
}

MainMemory::MainMemory()
{
	/*for (int i = 0; i < BYTECOUNT; i++)
	{
		memArray[i] = 0;
	}*/
}

// I think this method is no longer needed since we can use the findFreeFat method above
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

/*void MainMemory::insertIntoMemory(byte b)
{
	memory.memArray[0] = b;
}*/

/**
* Checks the first byte, starting from the data area of memory, of each sector
* Returns a 1 if the first byte is not 0. Returns 0 otherwise
*/
short MainMemory::checkSector(int sectorNum)
{
	sectorNum -= 33;
	int firstByteIndex = BEGIN_BYTE_ENTRY + sectorNum * 512;
	if (memArray[firstByteIndex] != 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/**
* Method: getUsedBytes()
* goes through each part of the disk (Boot sector -> FAT tables -> Root Directory -> Data area)
* and counts the bytes that are not used, which doesn't mean counting all the ones that are 0x00.
* In some cases, the bytes that are 0x00 are still being used to hold valuable information.
* returns the total number of bytes being used on disk.
*/
int getUsedBytes(){
    int used;
    used = 512; // begin by counting all of the boot sector
    // Go through boot sector and count all bytes that are set to 0x00
    /** - If we must count the 0x00 bytes in the boot sector as unused, use the block below
    used = 0;
    for(int i = 0; i < 512; i++){
        if(memory.memArray[i] != 0x00)
            ++used;
    }*/
    // Go though the FAT table and count the 'ENTRIES' that are not 0x00,
    // Then, multiply that number by 1.5, since each entry takes 1.5 bytes of space.
    // Then, multiply that by 2, since FAT table 1 is duplicated in FAT2
    // A used byte in the FAT table can be used to indicate the last sector of a file,
    // a reserved FAT entry, a bad FAT entry, or the number representing the next
    // sector of a file. Only 0x00 represents an unused entry.
    for(int i = 0; i <= LAST_INVALID_ENTRY; i++){
        if(getEntry(i) != 0x00)
            used += 3; // 1.5 bytes per FAT table
    }
    // Go through the Root Directory starting at the first byte (FIRST_FILE_BYTE)
    // and check the first byte of each 32 byte section. If it is 0xE5, then that
    // byte is needed, but the next 31 are not -> add 1 to used. If the byte is 
    // 0x00 then break from the loop, no more to check. If the byte is not 0x00 or 
    // 0xE5, add 32 to used.
    for(int i = FIRST_FILE_BYTE; i < BEGIN_BYTE_ENTRY; i += 32){
        if(memory.memArray[i] != 0x00 && memory.memArray[i] != 0xE5)
            used += 32;
        else if(memory.memArray[i] == 0xE5)
            ++used;
        else
            break;
    }   
    // Add number of free fat entries * 512 to used 
    // Bytes equal to 0x00 in a used sector will be counted, because the
    // the fat entry will be counted as used. 
    used += ((MAX_FAT_ENTRY + 1 - 2) - freeFatEntries) * SECTOR_SIZE;
    return used;
}

short getUsedSectors(){
    return ((MAX_FAT_ENTRY + 1 - 2) - freeFatEntries) + 33;
    // The 33 includes the boot sector, FAT table sectors, and root directory sectors
}

short *filesAndSectorStats(){
    short smallest = 0, largest = 0, numOfFiles = 0;
    bool smallestSet = false, largestSet = false;
    short *results = new short[3];
    for(int i = FIRST_FILE_BYTE; i < BEGIN_BYTE_ENTRY; i += 32){
        short currSize =  ceil(((memory.memArray[i+28] << 24) + (memory.memArray[i+29] << 16) + (memory.memArray[i+30] << 8) + memory.memArray[i+31])/(double)SECTOR_SIZE);
        if(memory.memArray[i] != 0x00 && memory.memArray[i] != 0xE5){
            if(currSize > 0 && (currSize < smallest || smallestSet == false)){
                smallest = currSize;
                smallestSet = true;
            }
            if(currSize > 0 && (currSize > largest || largestSet == false)){
                largest = currSize;
                largestSet = true;
            }
            if(currSize > 0)
                ++numOfFiles;
        }
    }
    results[0] = smallest;
    results[1] = largest;
    results[2] = numOfFiles;
    return results;
}

/**
* Prints the memory map showing the uasge of each sector
*/
void MainMemory::print()
{
	//variables for usage map
	int usedBytes = getUsedBytes();
	short usedSectors = getUsedSectors();
    short *stats = filesAndSectorStats();
	short numOfFiles = stats[2];
	short largestSector = stats[1]; //largest num of sectors that a file is using
	short smallestSector = stats[0]; //smallest num of sectors that a file is using
	

	float usedBytesPercentage = 100.0 * usedBytes / BYTECOUNT;
	int numFreeBytes = BYTECOUNT - usedBytes;
	float freeBytesPercentage = 100.0 * numFreeBytes  / BYTECOUNT;
	int numOfSectors = BYTECOUNT / SECTOR_SIZE;
	float usedSectorsPercentage = 100.0 * usedSectors / numOfSectors;
	float freeSectorsPercentage = 100.0 * (numOfSectors - usedSectors) / numOfSectors;
	float sectorsPerFile = (float)usedSectors / (((float)numOfFiles > 0)?((float)numOfFiles):(-1*usedSectors));
	
	printf("CAPACITY: %7ib     USED: %7ib (%3.1f%%)   FREE: %7ib (%3.1f%%)\n", BYTECOUNT, usedBytes, usedBytesPercentage, numFreeBytes, freeBytesPercentage);
	printf("SECTORS: %4i          USED: %-4i (%5.1f%%)      FREE: %-4i (%5.1f%%)\n", numOfSectors, usedSectors, usedSectorsPercentage, (numOfSectors - usedSectors), freeSectorsPercentage);
	printf("FILES: %-5i      SECTORS/FILE: %-4.2f     LARGEST: %4is    SMALLEST: %4is\n", numOfFiles, sectorsPerFile, largestSector, smallestSector);
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
			else if(checkSector(j))
			{
				toPrint = 'X';
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
