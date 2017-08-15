// Copyright 2017 Razvan Radoi, first of his name
#ifndef _HOME_RAZVAN_DESKTOP_SD_TEMA1_HASHTABLE_H_
#define _HOME_RAZVAN_DESKTOP_SD_TEMA1_HASHTABLE_H_
#include <iostream>
#include <fstream>
#include "./set.h"

template <typename Tkey, typename Tvalue>
class Hashtable {
 private:
	int HMAX;		// nr de sets
	struct setOfWays <Tkey, Tvalue> *H;
	unsigned int(*hashFunction) (Tkey);  // functia de hashing

 public:
	// constructor
	Hashtable(int hmax, unsigned int (*h)(Tkey)){
		HMAX = hmax;  // setez valorile initiale cu parametri primiti
		hashFunction = h;
		H = new struct setOfWays <Tkey, Tvalue> [HMAX];
	}
	// deconstructor
	~Hashtable(){
		delete[] H;  // eliberez cache
	}
	// adaugarea unui element in cache level 1 dupa Last Recently Updated
	void put(Tkey key, Tvalue value) const {
		int mySet = 0;
		mySet = hashFunction(key) %HMAX;  // selecteaza bitii pana la 12 sau 14

		if(H[mySet].lru == -1) {	 // daca niciun way nu e folosit, ia way 0
			H[mySet].lru = 1;  // il marcheaza ca folosit
			H[mySet].value1 = value;  // atribuie
			H[mySet].address1 = key;
			H[mySet].hasBeenSet1 = true;  // marcheaza ca a fost atribuit
			H[mySet].dirtyBit1 = -1;	 // seteaza ca dirtybitul nu exista
		} else {
			if (H[mySet].lru == 1) {  // daca trebuie sa puna in way1
				H[mySet].lru = 2;  // ultimul adaugat devine way1
				H[mySet].value2 = value;  // atribuie
				H[mySet].address2 = key;
				H[mySet].hasBeenSet2 = true;  // arata ca e atribuit
				H[mySet].dirtyBit2 = -1;  // reset dirty bit
			} else {	 // daca ambele ways sunt ocupate, pute in way0
				H[mySet].lru = 1;
				H[mySet].value1 = value;
				H[mySet].address1 = key;
				H[mySet].hasBeenSet1 = true;
				H[mySet].dirtyBit1 = -1;
			}
		}
	}
	// put pentru cache level 2. Difera de put prin functia de write-back
	void putL2(Tkey key, Tvalue value) const {
		int mySet = 0;
		mySet = hashFunction(key) %HMAX;

		if(H[mySet].lru == -1) {  // atribuie dupa aceleasi reguli ca la l1
			H[mySet].lru = 1;
			H[mySet].value1 = value;
			H[mySet].address1 = key;
			H[mySet].hasBeenSet1 = true;
			H[mySet].dirtyBit1 = -1;
		} else {
			if (H[mySet].lru == 1) {  // write back
				// daca setul e plin, folosesc un fisier auxiliar de write-back
				if( H[mySet].hasBeenSet1 == true && H[mySet].hasBeenSet2 == true ) {
					std::ofstream auxRam;
					auxRam.open("ram.aux", std::ios_base::app);
						auxRam << H[mySet].address2 << " " << H[mySet].value2 << '\n';
					auxRam.close();
				}
				H[mySet].lru = 2;
				H[mySet].value2 = value;
				H[mySet].address2 = key;
				H[mySet].hasBeenSet2 = true;
				H[mySet].dirtyBit2 = -1;
			} else {
				if( H[mySet].hasBeenSet1 == true && H[mySet].hasBeenSet2 == true ) {
					std::ofstream auxRam;
					auxRam.open("ram.aux", std::ios_base::app);
						auxRam << H[mySet].address1 << " " << H[mySet].value1 << '\n';
					auxRam.close();
				}
				H[mySet].lru = 1;
				H[mySet].value1 = value;
				H[mySet].address1 = key;
				H[mySet].hasBeenSet1 = true;
				H[mySet].dirtyBit1 = -1;
			}
		}
	}

	// intoarce value, dupa key
	Tvalue get(Tkey key) const {
		int mySet = hashFunction(key) % HMAX;
		if(H[mySet].lru !=-1){  // cauta doar daca setul nu e gol
			if(H[mySet].address1 == key) {  // verifica daca e in way0
				return H[mySet].value1;
			} else {
				if (H[mySet].address2 == key)  // sau daca e in way1
				return H[mySet].value2;
			}
		} else {
			// daca nu exista elementul, return
			return Tvalue();
		}
		// la fel..
		return Tvalue();
	}
	// update element, difera de put l1 prin neschimbarea lui lru
	void update(Tkey key, Tvalue value) const {
		int mySet = 0;
		mySet = hashFunction(key) %HMAX;

		if( H[mySet].address1 == key ) {
			H[mySet].value1 = value;
			H[mySet].dirtyBit1 = -1;
		}

		if( H[mySet].address2 == key)
			H[mySet].value2 = value;
			H[mySet].dirtyBit2 = -1;
	}  // daca doar modific valoarea (nu si adresa), nu schimba ultimul element adaugat

	// spune dupa key daca exista un element in l* cache si in care way se afla
	int lookUpByKey(Tkey key) const {
		int mySet = hashFunction(key) % HMAX;
		if(key == H[mySet].address1) {  // daca e in way0
			return 1;
		} else {
			if( key == H[mySet].address2 )  // daca e in way1
				return 2;
			else
				return 0;  // daca nu l-a gasit
		}
	}
	// spune daca un element e dirty dupa set si way
	int getDirtyBit(Tkey key, int way) const {
		int mySet = hashFunction(key) % HMAX;

		if ( way  == 1 )
			return H[mySet].dirtyBit1;
		if ( way  == 2 )
			return H[mySet].dirtyBit2;
		return int();
	}
	// pune dirtybit-ul
	void setDirtyBit(Tkey key) const {
		int mySet = hashFunction(key) % HMAX;

		if(key == H[mySet].address1) {
			H[mySet].dirtyBit1 = 1;  // pune in way0
		}
		if(key == H[mySet].address2) {
			H[mySet].dirtyBit2 = 1;  // pune in way1
		}
	}
	// afiseaza starea memoriei cache la un moment dat
	void dump(bool newLineFlag) const {
		// deschide cu append ca sa adauge la sfarsit si celelalte caches
		std::ofstream cacheOut;
		cacheOut.open("cache.out", std::ios_base::app);

		for( int i = 0; i < HMAX; i++ ) {
			if ( H[i].lru != -1 ) {  // daca a fost pus un element in set
				if(H[i].hasBeenSet1 == true){  // daca e in way0 ceva
					cacheOut << i << " 0 ";
					cacheOut << H[i].address1 << " " << H[i].value1;
					if ( H[i].dirtyBit1 == 1 )  // daca e dirty
						cacheOut << " *" << '\n';
					else
						cacheOut << '\n';
				}
				if(H[i].hasBeenSet2 == true){  // daca e in way1 ceva
					cacheOut << i << " 1 ";
					cacheOut << H[i].address2 << " " << H[i].value2;
					if ( H[i].dirtyBit2 == 1 )  // daca e dirty in way1
						cacheOut << " *" << '\n';
					else
						cacheOut << '\n';
				}
			}
		}
		// true pentru L1, false pentru L2
		if ( newLineFlag == true ){
			cacheOut << '\n';
		}
		cacheOut.close();
	}
};
#endif  // _HOME_RAZVAN_DESKTOP_SD_TEMA1_HASHTABLE_H_
