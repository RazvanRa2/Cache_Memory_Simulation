/// Copyright 2017 Razvan Radoi, first of his name
#include <iostream>
#include <fstream>
#include <string>
#include "./hashtable.h"
#include "./set.h"

// hashing function pentru level 1 cache
unsigned int hashingFunctionL1(int currentHash) {
		currentHash = currentHash >> 2;  // selecteaza bitii incepand cu 2
		return currentHash;	 // pana la 12 ii selectez in hashtable
}

// hashing function pentru level 2 cache
unsigned int hashingFunctionL2(int currentHash) {
		currentHash = currentHash >> 2;  // selecteaza bitii incepand cu 2
		return currentHash;  // pana la 14 ii selectez in hashtable
}

// initializeaza valorile din ram (ca string si ca int)
int setRamValues(int **ram, std::string **strRam) {
	std::string tempAddr;
	std::string tempValue;
	int usedSpaces = 0;

	std::ifstream ramIn("ram.in");

	while ( ramIn >> tempAddr >> tempValue ) {
		strRam[usedSpaces][0] = tempAddr;  // pune valorile string in strRam
		strRam[usedSpaces][1] = tempValue;
		ram[usedSpaces][0] = stoi(tempAddr);  // si ca int in ram
		ram[usedSpaces][1] = stoi(tempValue);

		usedSpaces++;
	}
	ramIn.close();

	return usedSpaces;  // return cate elemente sunt in ram
}

// cauta o valoarea din ram dupa adresa
int searchRamByAddress(int lookUpAddress, int ramSize, int **ram) {
	for( int i = 0; i < ramSize; i++ ) {
		if ( ram[i][0] == lookUpAddress ) {
			return ram[i][1];  // daca a fost gasita adresa, return valoare
		}
	}
  	// daca nu a fost gasita, nu returneaza nimic (mereu o gaseste, evident)
	return int();
}

// cauta in fisierul ram.aux valorile din ram (in cazurile cu write-back)
int searchAuxRamByAddress(int lookUpaddress) {
	int auxRamAddress, auxRamValue;

	std::ifstream auxRam("ram.aux");

	while(auxRam >> auxRamAddress >> auxRamValue) {
		if (lookUpaddress == auxRamAddress) {
			auxRam.close();
			return auxRamValue;
		}
	}  // cauta la fel ca in searchRamByAddress, dar in fisierul ram.aux

	auxRam.close();
	return -1;  // daca nu a fost gasita, return -1
}

// creeaza fisierul cache.out
void dumpCacheState(const Hashtable<int, int>& l2Cache,
	const Hashtable<int, int>& c0l1Cache,
	const Hashtable<int, int>& c1l1Cache) {
	c0l1Cache.dump(true);  // true ca sa puna si new line la final
	c1l1Cache.dump(true);
	l2Cache.dump(false);  // false ca sa nu puna new line la final
}

// creaza un string din int; de exemplu din 123 face 0000000123.
std::string makeString(int intNumber) {
	int aux = intNumber;
	int digits = 0;
	while( aux ) {
		digits++;
		aux /= 10;
	}  // numara cate cifre are
	if( intNumber == 0 ) {
		digits = 1;
	}  // si 0 are o cifra
	// pune cate zerouri trebuie in string
	std::string stringNumber;
	stringNumber = std::string(10 - digits, '0');
	// si adauga la final numarul convertit in string cu to_string
	stringNumber = stringNumber + std::to_string(intNumber);
	return stringNumber;
}

// creaza fisierul ram.out
void dumpRamState(int ramSize, std::string **strRam) {
	std::ifstream tempRam("ram.aux");
	int tempRamAddress, tempRamValue;
	// convert valorile din ram.aux in string apoi inlocuire in strRam
	while( tempRam >> tempRamAddress >> tempRamValue ) {
		for( int i = 0; i < ramSize; i++ ) {
			if( strRam[i][0] == makeString(tempRamAddress) ) {
				strRam[i][0] = makeString(tempRamAddress);
				strRam[i][1] = makeString(tempRamValue);
				break;
			}
		}
	}
	tempRam.close();
	// pune valorile updatate in ram.out
	std::ofstream ramOut("ram.out");

	for( int i = 0; i < ramSize; i++ ) {
		ramOut << strRam[i][0] << " " << strRam[i][1] << '\n';
	}

	ramOut.close();
}

// functia de read
void read(int coreNumber, int address, int ramSize, int **ram,
	const Hashtable<int, int>& l2Cache, const Hashtable<int, int>& c0l1Cache,
	const Hashtable<int, int>& c1l1Cache){
	// daca operatia se face pe core0
	if ( coreNumber == 0 ) {
		int foundInL1C0 = c0l1Cache.lookUpByKey(address);
		bool core0DirtyFlag = (c0l1Cache.lookUpByKey(address) != 0 &&
		c0l1Cache.getDirtyBit(address, foundInL1C0) != -1);
		// daca nu e in c0l1 sau e dirty in c0l1
		if( foundInL1C0 == 0 || core0DirtyFlag == true ) {
			// daca nu e in c0l1
			if (core0DirtyFlag == false) {
			// daca nu e nici in l2
			if ( l2Cache.lookUpByKey(address) == 0 ) {
				// cauta in valorile updatate din ram
				int valueToBeCached = searchAuxRamByAddress(address);
				if( valueToBeCached == -1 ) {
					// daca nu e printre cele updatate, cauta printre cele vechi
					valueToBeCached = searchRamByAddress(address, ramSize, ram);
				}
				// dupa ce le gaseste, le pune in l2 si in l1
				l2Cache.putL2(address, valueToBeCached);
				c0l1Cache.put(address, valueToBeCached);
			} else {
				// daca o gaseste in l2, pune-o in c0l1
				int valueToBeCached = l2Cache.get(address);
				c0l1Cache.put(address, valueToBeCached);
			}
			// daca e dirty
			} else {
				int valueToBeCached = l2Cache.get(address);
				c0l1Cache.update(address, valueToBeCached);  // update la valoare
			}
		}
	}

	if( coreNumber == 1 ) {  // aceleasi operatii ca pentru core0, in aceeasi ordine
		int foundInL1C1 = c1l1Cache.lookUpByKey(address);

		bool core1DirtyFlag = (c1l1Cache.lookUpByKey(address) != 0 &&
		c1l1Cache.getDirtyBit(address, foundInL1C1) != -1);

		if( foundInL1C1 == 0 || core1DirtyFlag == true ) {
			if( core1DirtyFlag == false ) {
			if( l2Cache.lookUpByKey(address) == 0 ) {
				int valueToBeCached = searchRamByAddress(address, ramSize, ram);
				l2Cache.putL2(address, valueToBeCached);
				c1l1Cache.put(address, valueToBeCached);
			} else {
				int valueToBeCached = l2Cache.get(address);
				c1l1Cache.put(address, valueToBeCached);
			}
			} else {
				int valueToBeCached = l2Cache.get(address);
				c1l1Cache.update(address, valueToBeCached);
				c1l1Cache.update(address, valueToBeCached);
			}
		}
	}
}

// functia de write
void write(int coreNumber, int address, int newData, int ramSize, int **ram,
	const Hashtable<int, int>& l2Cache, const Hashtable<int, int>& c0l1Cache,
	const Hashtable<int, int>& c1l1Cache) {
	// daca se fac operatii cu core0
	if( coreNumber == 0 ) {
		int foundInL1C0 = c0l1Cache.lookUpByKey(address);
		bool core0DirtyFlag = (foundInL1C0 != 0 &&
		c0l1Cache.getDirtyBit(address, foundInL1C0) != -1);
		// daca nu exista deja in c0l1 sau e dirty in c0l1
		if ( foundInL1C0 == 0 || core0DirtyFlag == true ) {
			   int foundInl2Cache = l2Cache.lookUpByKey(address);
			   // daca o gaseste in l2
			   if ( foundInl2Cache == 0 ) {
				   // ia valoarea din ram, o pune in l2 si ii face update
				   int oldData = searchRamByAddress(address, ramSize, ram);
				   l2Cache.putL2(address, oldData);
				   l2Cache.update(address, newData);
				   // ia valoarea si o pune si in c0l1
				   c0l1Cache.put(address, oldData);
				   c0l1Cache.update(address, newData);
				   c1l1Cache.setDirtyBit(address);
			   } else {
				   // daca nu e dirty in core0;
				   if (core0DirtyFlag == false){
					   c0l1Cache.put(address, newData);
					   l2Cache.update(address, newData);
					   c1l1Cache.setDirtyBit(address);
					   // o pune noua in c0l1 si o pune pe dirty in c1l1
				   } // daca e dirty in core0
				   if (core0DirtyFlag == true){
					   c0l1Cache.update(address, newData);
					   l2Cache.update(address, newData);
					   c1l1Cache.setDirtyBit(address);
					   // exact aceleasi operatii ca mai sus
				   }
			   }

		} else {  // daca nu o gaseste in core0
			c0l1Cache.update(address, newData);  // update in c0l1
			l2Cache.update(address, newData);  // update in l2
			c1l1Cache.setDirtyBit(address);  // set dirty pentru c1l1
		}
	}

	if( coreNumber == 1 ) {  // aceleasi operatii, dar pentru core1
		int foundInL1C1 = c1l1Cache.lookUpByKey(address);
		bool core1DirtyFlag = (foundInL1C1 !=0 &&
		c1l1Cache.getDirtyBit(address, foundInL1C1) != -1);

		if ( foundInL1C1 == 0 	|| core1DirtyFlag == true ) {
			int foundInl2Cache = l2Cache.lookUpByKey(address);
			if( foundInl2Cache == 0 ) {
				int oldData = searchRamByAddress(address, ramSize, ram);
				l2Cache.putL2(address, oldData);
				l2Cache.update(address, newData);

				c1l1Cache.put(address, oldData);
				c1l1Cache.update(address, newData);
				c0l1Cache.setDirtyBit(address);
			   } else {
				if (core1DirtyFlag == false){
				c1l1Cache.put(address, newData);
				l2Cache.update(address, newData);
				c0l1Cache.setDirtyBit(address);
			  	}
				if (core1DirtyFlag == true){
					c1l1Cache.update(address, newData);
					l2Cache.update(address, newData);
					c0l1Cache.setDirtyBit(address);
				  	}
			   }

		} else {
			c1l1Cache.update(address, newData);
			l2Cache.update(address, newData);
			c0l1Cache.setDirtyBit(address);
		}
	}
}

int main(){
	Hashtable<int, int> l2Cache(8192, &hashingFunctionL2);
	Hashtable<int, int>& l2c = l2Cache;
	Hashtable<int, int> c0l1Cache(2048, &hashingFunctionL1);
	Hashtable<int, int>& c0c = c0l1Cache;
	Hashtable<int, int> c1l1Cache(2048, &hashingFunctionL1);
	Hashtable<int, int>& c1c = c1l1Cache;
	int ramSize, **ram;
	std::string **strRam;
	int coreNumber, address, newData;
	char operation;

	// alocare pentru ram-uri (string si int)
	ram = new int*[1000005];
	for( int i = 0; i < 1000005; i++ ) {
		ram[i] = new int[2];
	}
	strRam = new std::string*[1000005];
	for( int i = 0; i < 1000005; i++ ) {
		strRam[i] = new std::string[2];
	}

	// citirea valorilor din RAM
	ramSize = setRamValues(ram, strRam);

	// citirea operatiilor si procesarea acestora
	std::ifstream operationsIn("operations.in");
	while ( operationsIn >> coreNumber >> operation >> address ) {
		if ( operation == 'r' ) {  // daca e o peratie de read
			read(coreNumber, address, ramSize, ram, l2c, c0c, c1c);
		}
		if ( operation == 'w' ) {  //daca e o operatie de write
			operationsIn >> newData;
			write(coreNumber, address, newData, ramSize, ram, l2c, c0c, c1c);
		}
	}
	dumpCacheState(l2c, c0c, c1c);
	dumpRamState(ramSize, strRam);
	// clean-up
	for( int i = 0; i < 1000005; i++ )
		delete[] ram[i];
	delete[] ram;

	for( int  i = 0; i < 1000005; i++)
		delete[] strRam[i];
	delete[] strRam;

	return 0;
}
