#include <iostream>
#include <math.h>
#include <fstream>
#include <vector>
#include <pthread.h>
#include <iomanip>
#include <chrono>

using namespace std;

typedef uint32_t uint;

//these are the values used to calculate SHA-256
uint h[] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};

//these are values needed to calculate SHA-256
uint k[] =
  {0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
   0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
   0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
   0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
   0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
   0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
   0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
   0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

//file input through ifstream
ifstream myfile;

//keep track of the size of the file
uint64_t inputCount = 0; 

//flags for input
bool oneHasBeenAppended = false;
bool lengthHasBeenAppended = false;

//rolling index for start and end
int startIndex = 0;
int endIndex = 0;

//array that will hold input values to be hashed
uint mArray[5][16];

//this function handles right rotate
uint rightRotate(uint valueToRotate, int numberOfRotations)
{
	//start with 32 1's
	uint mask = 0xFFFFFFFF;
	
	//shift the mask so that you can pull only the values you want out
	mask >>= 32 - numberOfRotations;

	//get the bits to rotate to the front
	uint numShiftedOut = valueToRotate & mask;

	//shift the original value to open space in the front
	valueToRotate >>= numberOfRotations;

	//add the value you rotated out to the front of the integer
	valueToRotate |= (numShiftedOut << (32 - numberOfRotations));

	return valueToRotate;
}

//function to run on a thread.  Gets the input from a file
void *calculateInput(void *blank)
{
	//holds the string as ints
	uint *messageArray = mArray[endIndex % 5];

	//holds the input string
	string msg;
	char charIn;

	int currentCount = 0;

	//we want 64 characters to fit into 16 32bit integers
	while (currentCount < 64)
	{ 
		//get all you can
		if (myfile.get(charIn))
		{
			msg += charIn;
			inputCount++;
			currentCount++;
		}
		//unless you reach the end of the file
		else
			break;
	}

	//as long as the message length in bits is less than 504, we can append a 1 to the end of the message
	//we only want to do this once
	if (msg.length() * 8 < 504 && !oneHasBeenAppended)
	{
		msg += 0x80;
		oneHasBeenAppended = true;
	}

	//set all values to zero
	for(int i = 0; i < 16; i++)
		messageArray[i] = 0;

	//stuff the message into the array
	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (((i * 4) + j) < msg.length())
				messageArray[i] |= ((unsigned char)(msg.at((i * 4) + j)) << (32 - (8 * (j + 1))));
		}
	}

	//as long as the message length is less than 448 and a 1 has been appended to the string, 
	//we can append the files length to the end of the array
	if (msg.length() * 8 <= 448 && oneHasBeenAppended)
	{
		uint temp1 = (uint)(inputCount * 8 >> 32);
		uint temp2 = (uint)(inputCount * 8);
		messageArray[14] = temp1;
		messageArray[15] = temp2;
		lengthHasBeenAppended = true;
	}

	//exit the thread
	pthread_exit(NULL);
}

//here we will calculate the hash from the input values
void *calculateHash(void *blank)
{
	//get the value to be hashed
	uint *messageArray = mArray[startIndex % 5];

	//create and zero out the w array
	uint w[64];
	for (int i = 0; i < 64; i++)
		w[i] = 0;

	//copy the input values to the 64 integer array
	for (int i = 0; i < 16; i++)
	{
		w[i] = messageArray[i];
	}

	//extend the first 16 words into the remaining 48 words
	for (int i = 16; i < 64; i++)
	{
		uint s0 = (rightRotate(w[i-15], 7) ^ rightRotate(w[i-15],18) ^ (w[i-15] >> 3));
		uint s1 = (rightRotate(w[i-2], 17) ^ rightRotate(w[i-2], 19) ^ (w[i-2] >> 10));
		w[i] = w[i-16] + s0 + w[i-7] + s1;
	}

	//initialize working variables
	uint a = h[0];
	uint b = h[1];
	uint c = h[2];
	uint d = h[3];
	uint e = h[4];
	uint f = h[5];
	uint g = h[6];
	uint hv = h[7];

	//calculate hash
	for (int i = 0; i < 64; i++)
	{
		uint s1 = (rightRotate(e, 6) ^ rightRotate(e, 11) ^ rightRotate(e, 25));
		uint ch = ((e & f) ^ ((~e) & g));
		uint temp1 = hv + s1 + ch + k[i] + w[i];
		uint s0 = (rightRotate(a, 2) ^ rightRotate(a, 13) ^ rightRotate(a, 22));
		uint maj = ((a & b) ^ (a & c) ^ (b & c));
		uint temp2 = s0 + maj;
		
		hv = g;
		g = f;
		f = e;
		e = d + temp1;
		d = c;
		c = b;
		b = a;
		a = temp1 + temp2;
	}

	//update hash variables
	h[0] += a;
	h[1] += b;
	h[2] += c;
	h[3] += d;
	h[4] += e;
	h[5] += f;
	h[6] += g;
	h[7] += hv;
	
	//exit thread
	pthread_exit(NULL);
}

//this method is called from main on its own thread
//it allows only 1 input to be obtained at a time
void *calcInput(void *blank)
{
	pthread_t input_thread;
	//we will continue until we reach the end of the file
	while (!oneHasBeenAppended || !lengthHasBeenAppended)
	{
		//and we will create our thread as long as the array is not full
		if (endIndex - startIndex < 5)
		{
			pthread_create(&input_thread, NULL, calculateInput, NULL);
			pthread_join(input_thread, NULL);
			endIndex++;
		}
	}
	pthread_exit(NULL);
}

//this method is called form main on its own thread
//it allows only 1 has to be calculated at a time
void *calcHash(void *blank)
{
	pthread_t hash_thread;
	//continue until the entire file has been read in
	while (!oneHasBeenAppended || !lengthHasBeenAppended)
	{
		if (endIndex != startIndex)
		{
			pthread_create(&hash_thread, NULL, calculateHash, NULL);
			pthread_join(hash_thread, NULL);
			startIndex++;
		}
	}
	//empty the array
	while (endIndex != startIndex)
	{
		pthread_create(&hash_thread, NULL, calculateHash, NULL);
		pthread_join(hash_thread, NULL);
		startIndex++;
	}
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	//take the file name in as an argument
	//only one argument allowed
	if (argc != 2)
	{
		cout << "invalid number of arguments" << endl;
		return 0;
	}

	//the argument is the filename
	string filename = argv[1];
	myfile.open(filename);

	//create initial threads
	pthread_t hash_thread, input_thread;

	//send each off on their own thread
	pthread_create(&input_thread, NULL, calcInput, NULL);
	pthread_create(&hash_thread, NULL, calcHash, NULL);

	//dont continue until they are finished
	pthread_join(input_thread, NULL);
	pthread_join(hash_thread, NULL);

	//validate that the array is completed
	while (startIndex != endIndex)
	{
		pthread_create(&hash_thread, NULL, calcHash, NULL);
		pthread_join(hash_thread, NULL);
	}

	//close the file
	myfile.close();

	//output the hash to the console
	cout << setfill('0') << setw(8) << hex << h[0];
	cout << setfill('0') << setw(8) << hex << h[1];
	cout << setfill('0') << setw(8) << hex << h[2];
	cout << setfill('0') << setw(8) << hex << h[3];
	cout << setfill('0') << setw(8) << hex << h[4];
	cout << setfill('0') << setw(8) << hex << h[5];
	cout << setfill('0') << setw(8) << hex << h[6];
	cout << setfill('0') << setw(8) << hex << h[7];
	cout << endl;
}
