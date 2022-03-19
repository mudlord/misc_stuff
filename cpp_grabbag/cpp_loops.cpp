
#include <windows.h>
#include <stdint.h>
#include <vector>
#include <iostream>
using namespace std;
#include <wincrypt.h>
#include <filesystem>

void loops()
{
	/*
	For loops are fucking easy to understand
	just spin around up and down until a condition is met
	*/
	for (int i = 0; i < 10; i++)
	{
		//set j to 0
		int j = 0;
		j++; //post increment j
		j += 2; //add 2 to j
		j -= 2; //take away 2
		j *= 2; //multiply by 2
	}

	for (float f = 0; f < 10.0; f += 0.1)
		float f1 = f;

	bool flag = false;
	int j = 0;
	do
	{
		//while "flag" is false, just keep swimming :3
		j++;
		if (j > 5)flag = true;
	} while (!flag);


}