// CS3243 Operating Systems
// Fall 2013
// Project 6: Disks and File Systems
// Jestin Keaton and Steven Ng
// Date: 12/2/2013
// File: partone.cpp


#include<iostream>
#include<fstream>
#define BYTECOUNT 1474560

using namespace std;

// Define Structs

typedef unsigned char byte;

struct MainMemory{
    byte memArray[BYTECOUNT]; // 0-511 is for the boot partition
    int findFreeMemory();
};



int main(){
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
    return 0;
}

int MainMemory::findFreeMemory(){
    return 0;
}
