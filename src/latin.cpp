#include <iostream> 
#include <stdlib.h> 
#include <iomanip> 

using namespace std; 

void shuffle(int array1[], int size) 
{ 
	int i, temp, random, last; 

	for (i = 0; i < size; i++) 
		array1[i] = i + 1; 

	for (last = size; last > 1; last--) 
	{ 
		random = rand() % last; 
		temp = array1[random]; 
		array1[random] = array1[last - 1]; 
		array1[last - 1] = temp; 
	} 
} 

void rotate(int array2[], int size) 
{ 
	int temp, i; 

	temp = array2[0]; 
	for (i = 0; i < size - 1; i++) 
	{array2[i] = array2[i+1];} 
	array2[size - 1] = temp; 
} 

#define MAX_SCALE 50 
int main(int argc, char** argv) 
{ 
	int sequence[MAX_SCALE],jumbled[MAX_SCALE]; 
	int square[MAX_SCALE][MAX_SCALE]; 
	int position, value, i, j, size; 

	srand((unsigned)time(NULL)); 
	if ( argc < 2 ) { 
		cout<<"Enter the Dimension of Square you need (<=50): "; 
		cin>>size; 
	} 
	else { 
		size = atoi(argv[1]); 
	} 

	if ( size > MAX_SCALE ) { 
		cerr << "too large scale to be tractable, need recompile the program with increased scale.\n"; 
		return -1; 
	} 

	//while (size != 0) 
	{ 
		shuffle(jumbled, size); 
		for (i = 0; i < size; i++){ 
			sequence[i] = jumbled[i];} 

		for (i = 0; i < size; i++) 
		{ 
			position = sequence[0]; 
			value = jumbled[position - 1]; 

			for (j = 0; j < size; j++){ 
				square[j][sequence[j] - 1] = value;} 

			rotate(sequence, size); 
		} 
		cerr << "A Latin Square of Order " << size << " is: " << endl << endl; 

		for (i = 0; i < size; i++) 
		{ 
			for (j = 0; j < size; j++) 
			{ 
				cout << square[i][j] << "\t"; 
			} 
			cout << endl; 
		} 
		size--; 
	} 
	return 0; 
} 

