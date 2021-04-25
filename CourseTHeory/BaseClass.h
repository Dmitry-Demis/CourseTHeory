#pragma once
#include <string>
#include <iostream>
#include <iomanip>
using namespace std;
class BaseBank
{
public:
	BaseBank()
	{

	}
	BaseBank(const string& name): _name(name)
	{

	}
	virtual double Withdraw(const double& value)
	{
		if (value <= amount)
		{
			amount -= value;
			return value;
		}
		else
		{
			return amount;
		}
		
	}
	virtual void Put(const double& value)
	{
		amount += value;		
		
	}
	virtual double Amount()  const
	{
		return amount;
	}
	virtual string BankName() const
	{
		return _name;
	}
private:
	double amount = 0.0;
	const string _name;

};
class Sberbank: public BaseBank
{
public:
	Sberbank():BaseBank("Sberbank")
	{

	}
	
private:

};
class AlphaBank : public BaseBank
{
public:
	AlphaBank():BaseBank("Alphabank")
	{
		
	}


private:

};
