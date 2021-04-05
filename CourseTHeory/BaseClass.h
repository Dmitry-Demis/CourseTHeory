#pragma once
#include <string>
#include <iostream>
#include <iomanip>
using std::string;
using std::cout;
using std::endl;
#define A(x)  #x << " = " << x << " RUB."
class BaseBank
{
public:
	BaseBank()
	{

	}
	BaseBank(const string& name): _name(name)
	{

	}
	virtual void Withdraw(const double& value)
	{
		if (value <= amount)
		{
			amount -= value;
			BankName();
			std::cout << std::setprecision(2)<< std::fixed<<value << " RUB has been withdrawn"<<endl;
		}
		else
		{
			BankName();
			std::cout << std::setprecision(2) << std::fixed<<"Now you have " << amount << " RUB. But you want to withdraw " << value << " RUB" << endl;
		}
	}
	virtual void Put(const double& value)
	{
		amount += value;
		BankName();
		std::cout << std::setprecision(2) << std::fixed << value << " RUB has been put on your account"<<endl;
	}
	virtual void Amount()  const
	{
		BankName();
		cout << std::setprecision(2) << std::fixed << "Current " << A(amount) << endl;
	}
private:
	double amount = 0.0;
	const string _name;
	void BankName() const
	{
		cout << "Bank " << _name << ". ";
	}

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
