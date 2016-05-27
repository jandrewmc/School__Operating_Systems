

#include <iostream>
#include <iomanip>

using namespace std;

const int MAX_ARRAY_SIZE = 10000;
int largestValue = 0;

void getUserInput(int &startVal, int &endVal)
{
	while (true)
	{
		cout << "Please enter the starting value: ";
		cin >> startVal;
		cout << "Please enter the ending value: ";
		cin >> endVal;

		if (endVal < startVal || endVal < 0 || startVal < 0)
		{
			cout << "You entered an invalid value!" << endl;
		}
		else
		{
			break;
		}
	}
}

int computeColatz(int val)
{
	int count = 0;

	while (val != 1)
	{
		if (val % 2 == 0)
		{
			val /= 2;
		}
		else
		{
			val = 3 * val + 1;
		}
		count++;
	}

	if (count > largestValue)
		largestValue = count;

	return count;
}

void printFrequencyTable(int array[])
{
	cout << endl << endl << endl;
	cout << "---------------------------------" << endl;
	cout << "|" << setw(15) << "Stopping Time" << "|" << setw(15) << "Frequency" << "|" << endl;
	for (int i = 0; i <= largestValue; i++)
	{
		cout << "---------------------------------" << endl;
		cout << "|" << setw(15) << i << "|" << setw(15) << array[i] << "|" << endl;
	}
	cout << "---------------------------------" << endl;
}

int main()
{
	int array[MAX_ARRAY_SIZE];
	for (int i = 0; i < 0000; i++)
	{
		array[i] = 0;
	}

	int startVal = 0;
	int endVal = 0;

	getUserInput(startVal, endVal);

	for (int i = startVal; i <= endVal; i++)
	{
		array[computeColatz(i)]++;
	}

	printFrequencyTable(array);

	return 0;
}
