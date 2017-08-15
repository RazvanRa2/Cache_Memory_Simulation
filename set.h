// Copyright 2017 Razvan Radoi, first of his name
#ifndef _HOME_RAZVAN_DESKTOP_SD_TEMA1_SET_H_
#define _HOME_RAZVAN_DESKTOP_SD_TEMA1_SET_H_
template <typename Tkey, typename Tvalue>
struct setOfWays {
	Tkey address1;   // adresa valorii din way0
	Tvalue value1;  // valoarea din way0

	Tkey address2;  // adresa valorii din way1
	Tvalue value2;  // valoarea din way1

	bool hasBeenSet1;  // true daca in way0 e pus un element, altfel false
	bool hasBeenSet2;  // true daca in way1 e pus un element, altfel false

	int lru;  // last recently added ( u vine de la used )
	int dirtyBit1;  // 1 daca valoarea din way0 este dirty, altfel -1
	int dirtyBit2;  // 1 daca valoarea din way1 este dirty, altfel -1

	setOfWays(){  // constructor default pentru un set
		lru = -1;
		dirtyBit1 = -1;
		dirtyBit2 = -1;
		hasBeenSet1 = false;
		hasBeenSet2 = false;
		address1 = -1;
		address2 = -1;
	}
};
#endif  // _HOME_RAZVAN_DESKTOP_SD_TEMA1_SET_H_
