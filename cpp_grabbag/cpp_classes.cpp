#include <windows.h>
#include <stdint.h>
#include <vector>
#include <iostream>
using namespace std;
#include <wincrypt.h>
#include <filesystem>


using namespace std;

//useful when you only need *ONE* class instance at a time!
//Quite hacky!
class staticclass {
protected:
	int x, y;
public:
	staticclass() {

	};
	staticclass(int x, int y) {
		this->x = x, this->y = y;
	}
	void set_values(int a, int b)
	{
		x = a; y = b;
	}
	void add(int y, int x) {
		x += y;
		y += x;
	}
	static staticclass* get_classinstance(int x = NULL, int y = NULL)
	{
			static thread_local staticclass* instance = new staticclass(x,y);
			return instance;
	}
};


class base_class {
protected:
	int x, y;
public:
	base_class() {

	};
	base_class(int x, int y) {
		this->x = x, this->y = y;
	}
	void set_values(int a, int b)
	{
		x = a; y = b;
	}
	void add(int, int);

};

void base_class::add(int xshit, int yshit) {
	x += xshit;
	y += yshit;
}

class abstract_class {
protected:
	int x, y;
public:
	abstract_class() {
		x = 0; y = 1;
	}
	abstract_class(int x, int y) {
		this->x = x, this->y = y;
	}
	void set_values(int a, int b)
	{
		x = a; y = b;
	}
	void add(int x, int y)
	{
		this->x += y;
		this->y += x;

	}
	int fuck()
	{
		return x + y;
	}

};
/*
In principle, a publicly derived class inherits access to every member of a base class except:

	its constructors and its destructor
	its assignment operator members (operator=)
	its friends
	its private members
*/
/*
inherit from base class, but also have a definable class method
which can be *overridden*/
class fuckitclass : abstract_class
{
public:
	int fuck()
	{
		return 0x44 + x + y;
	}
};





void classes()
{
	fuckitclass fuckyou;
	abstract_class test;
	test.set_values(0xFF, 0x90);
	int arr = test.fuck();
	test.add(0x33, 0x66);
	arr = fuckyou.fuck();
	base_class* shit = new base_class(0xFF, 0x90);
	shit->add(20, 40);
	delete shit;
}