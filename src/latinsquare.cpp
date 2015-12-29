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

int main()
{
	int sequence[10],jumbled[10];
	int square[10][10];
	int position, value, i, j, size;

	srand((unsigned)time(NULL));
	cout<<"Enter the Dimension of Square you need \n";
	cin>>size;	

	while (size != 0)
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
		cout << endl;
		cout << "A Latin Square of Order " << size << " is: " << endl << endl;

		for (i = 0; i < size; i++)
		{
			for (j = 0; j < size; j++)
			{
				cout << setw(5) << square[i][j];
			}
			cout << endl;
		}
		size--;
	}
	return 0;
}

