// CourseTheoryProject.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "BaseClass.h"
#include <vector>
#include <memory>
#include <fstream>
#include <queue>
#include <map>
#include "mpi.h"
#include <sstream>

using namespace std;
int msgtag = 4;
MPI_Status status;
#define comm MPI_COMM_WORLD
int rankp, sizep;
#define lmax 10
void ConvertVectorToArray(const vector<int>& vec, int* arr)
{
    for (size_t i = 0; i < vec.size(); i++)
    {
        arr[i] = vec[i];
    }
}
void QueueShow(const int& vecSize, const int* vec, string fileName)
{
    ofstream out(fileName, ios::app);
    for (int i = 0; i < vecSize; i += 3)
    {
        out << " Processor: " << vec[i] << " Bank = "
            << ((vec[i + 1] == 1) ? "Alphabank" : "Sberbank") << ", Action: " <<
            ((vec[i + 2] > 0) ? "Put" : (vec[i + 2] == 0) ? "Amount" : "Withdraw") << endl;
    }
    out << endl;
    out.close();
}
void QueueShow(const vector<int>& vec, string fileName)
{
    ofstream out(fileName, ios::app);
    for (size_t i = 0; i < vec.size(); i += 3)
    {
        out << " Processor: " << vec[i] << " Bank = "
            << ((vec[i + 1] == 1) ? "Alphabank" : "Sberbank") << ", Action: " <<
            ((vec[i + 2] > 0) ? "Put" : (vec[i + 2] == 0) ? "Amount" : "Withdraw") << endl;
    }
    out << endl;
    out.close();
}
void SendToBank(vector<int>& excess, int toWhichProcess, string fileName)
{
   
    int excessSize = excess.size();
    if (MPI_Send(&excessSize, 1, MPI_INT, toWhichProcess, msgtag, comm))
    {
        cout << "Processor " << rankp << ". Error: " << __LINE__ << endl;
        return;
    }; //sending to N process an excess array size
    if (excessSize)
    {
        int* excessArray = new int[excessSize];
        ConvertVectorToArray(excess, excessArray);
        excess.clear();
        if (MPI_Send(excessArray, excessSize, MPI_INT, toWhichProcess, msgtag, comm))
        {
            cout << "Processor " << rankp << ". Error: " << __LINE__ << endl;
            return;
        };
    }
    if (MPI_Recv(&excessSize, 1, MPI_INT, toWhichProcess, msgtag, comm, &status))
    {
        cout << "Processor " << rankp << ". Error: " << __LINE__ << endl;
        return;
    }; // receiving from an N process an excess array size
    if (excessSize)
    {
        int* needArray = new int[excessSize];
        if (MPI_Recv(needArray, excessSize, MPI_INT, toWhichProcess, msgtag, comm, &status))
        {
            cout << "Processor " << rankp << ". Error: " << __LINE__ << endl;
            return;
        };
        for (int i = 0; i < excessSize; i+=3)
        {
            ofstream out(fileName, ios::app);
            excess.push_back(needArray[i]);
            excess.push_back(needArray[i + 1]);
            excess.push_back(needArray[i + 2]);
            out << "[]Process " << needArray[i] << " has been received from the other bank" << endl;
            out << endl;
            out.close();
        }
    }
    
}
void SendToProcesses(map<int, BaseBank>& base, const vector<int>& arr)
{
    for (size_t i = 0; i < arr.size(); i+=3)
    {
               if (base.find(arr[i])!= base.end())
               {
                   stringstream s;                   
                   int value = arr[i + 2];
                   double previousAmount = base.at(arr[i]).Amount();
                   if (value > 0)
                   {
                       base.at(arr[i]).Put(value);
                       s << "User " << arr[i] << ". You've put " << value << " RUB to your account. Current balance is " << base.at(arr[i]).Amount()<<" RUB"<< '\0';
                   }
                   else if (value < 0)
                   {                        
                       double currentAmount = base.at(arr[i]).Withdraw(-value);
                       if (previousAmount == currentAmount)
                       {
                           s << "User " << arr[i] << ". You want to withdraw " << -value << " RUB, but your current balance is " << currentAmount << " RUB" << '\0';
                       }
                       else
                       {
                           s << "User " << arr[i] << ". You've withdrawn " << (-value) << " RUB. Current balance is " << base.at(arr[i]).Amount() << " RUB"<< '\0';
                       }
                   }
                   else
                   {                       
                       s << "User " << arr[i] << ". Your balance is " << (previousAmount) << " RUB" << '\0';
                   }
                   int strSize = s.str().size();
                   char* str = new char[strSize] {0};
                   strcpy(str, s.str().c_str());
                   if (MPI_Send(&strSize, 1, MPI_INT, arr[i], msgtag, comm))
                   {
                       cout << "Processor " << rankp << ". Error: " << __LINE__ << endl;
                       return;
                   };
                   if (MPI_Send(str, strSize, MPI_CHAR, arr[i], msgtag, comm))
                   {
                       cout << "Processor " << rankp << ". Error: " << __LINE__ << endl;
                       return;
                   };
               }
    }
}

void DeriveTwoOnVectors(int* arr, const int& arrSize, vector<int>& need, vector<int>& excess, string fileName, int whichProcess)
{
    ofstream out(fileName, ios::app);
    for (int i = 0; i < arrSize; i+=3)
    {
        if (arr[i+1] == whichProcess)
        {
            need.push_back(arr[i]);
            need.push_back(arr[i + 1]);
            need.push_back(arr[i + 2]);
        }
        else
        {
            excess.push_back(arr[i]);
            excess.push_back(arr[i + 1]);
            excess.push_back(arr[i + 2]);
            out << "Process " << arr[i] << " is absent in the base. Resending to the other bank" << endl;
        }
    }
    out << endl;
    out.close();
}

void doDebug(int rank)
{
    if (!rank)
    {
        system("pause");
    }
    MPI_Barrier(comm);
}
int main(int argc, char** argv)
{
   
#pragma region HEADER
    if (MPI_Init(&argc, &argv) != MPI_SUCCESS)
    {
        return 1;
    }
    if (MPI_Comm_size(MPI_COMM_WORLD, &sizep) != MPI_SUCCESS)
    {
        MPI_Finalize();
        return 2;
    }
    if (MPI_Comm_rank(MPI_COMM_WORLD, &rankp) != MPI_SUCCESS)
    {
        MPI_Finalize();
        return 3;
    }
#pragma endregion
   
    const int maxSum = 50'000;

    string filePath = "result" + to_string(rankp) + ".txt";
    ofstream output(filePath);
    output.close();
    /* 0 process - server, it provides queues
     * 1 process - Alphabank
     * 2 process - Sberbank
     * others - terminals
     */
    int chosenBank = 0;
    int action = 0;
    srand((unsigned)time(NULL) + rankp * 1'000'000 * sizep);
    AlphaBank alphabank;
    Sberbank sberbank;
    map<int, BaseBank> process_Alphabank_action =
    {
        {4, alphabank},
        {6, alphabank},
        {8, alphabank},
        {10, alphabank},
        {12, alphabank}
    };
    map<int, BaseBank> process_Sberbank_action =
    {
        {3, sberbank},
        {5, sberbank},
        {7, sberbank},
        {9, sberbank},
        {11, sberbank}
    };
  //doDebug(rankp);
    for (int l = 0; l < lmax; l++) // a life cicle
    {
        int evenSize = 0, alhaSize = 0;
        int oddSize = 0, sberSize = 0;
        int* evenArray = NULL, *oddArray = NULL;

        if (!rankp)
        {
            vector<int> even; 
            vector<int> odd;
            output.open(filePath, ios::app);
            output << l + 1 << " iteration" << endl;
            output.close();
            //getting from all processes which bank and action they've chosen            
            for (int i = 3; i < sizep; i++)
            {
                if (MPI_Recv(&chosenBank, 1, MPI_INT, MPI_ANY_SOURCE, msgtag, comm, &status))
                {
                    cout << "Processor " << rankp << ". Error: " << __LINE__ << endl;
                    return -5;
                };

                if (MPI_Recv(&action, 1, MPI_INT, MPI_ANY_SOURCE, msgtag, comm, &status))
                {
                    cout << "Processor " << rankp << ". Error: " << __LINE__ << endl;
                    return -5;
                };
                int fromWhichProcess = status.MPI_SOURCE;
               
                if (i % 2 == 0) // even
                {
                    even.push_back(fromWhichProcess);
                    even.push_back(chosenBank);
                    even.push_back(action);
                }
                else
                {
                    odd.push_back(fromWhichProcess);
                    odd.push_back(chosenBank);
                    odd.push_back(action);
                }
            }
            evenSize = even.size();
            oddSize = odd.size();
            evenArray = new int[evenSize];
            oddArray = new int[oddSize];
            ConvertVectorToArray(even, evenArray);
            ConvertVectorToArray(odd, oddArray);
            QueueShow(evenSize, evenArray, filePath);
            QueueShow(oddSize, oddArray, filePath);
            //Sending a size of an array and the array to Alphabank processor - 1
            if (MPI_Send(&evenSize, 1, MPI_INT, 1, msgtag, comm))
            {
                cout << "Processor " << rankp << ". Error: " << __LINE__ << endl;
                return -5;
            };
            if (MPI_Send(evenArray, evenSize, MPI_INT, 1, msgtag, comm))
            {
                cout << "Processor " << rankp << ". Error: " << __LINE__ << endl;
                return -5;
            };
            //Sending a size of an array and the array to Sberbank processor - 2
            if (MPI_Send(&oddSize, 1, MPI_INT, 2, msgtag, comm))
            {
                cout << "Processor " << rankp << ". Error: " << __LINE__ << endl;
                return -5;
            };
            if (MPI_Send(oddArray, oddSize, MPI_INT, 2, msgtag, comm))
            {
                cout << "Processor " << rankp << ". Error: " << __LINE__ << endl;
                return -5;
            };

            delete[] 
                evenArray, 
                oddArray;
            
        }
        if (rankp == 1) // Alphabank
        {
            ofstream out(filePath, ios::app);
            out << l + 1 << " iteration" << endl;
            out.close();            
            int arrSize = 0;
            if (MPI_Recv(&arrSize, 1, MPI_INT, 0, msgtag, comm, &status))
            {
                cout << "Processor " << rankp << ". Error: " << __LINE__ << endl;
                return -5;
            }; //get a size of an array
            int* arr = new int[arrSize] {0}; // create the array
            if (MPI_Recv(arr, arrSize, MPI_INT, 0, msgtag, comm, &status))
            {
                cout << "Processor " << rankp << ". Error: " << __LINE__ << endl;
                return -5;
            }; // get the array
            QueueShow(arrSize, arr, filePath);
            vector<int> need, excess;
            DeriveTwoOnVectors(arr, arrSize, need, excess, filePath, 1);           
            
            SendToBank(excess, 2, filePath);
            for (size_t i = 0; i < excess.size(); i++)
            {
                need.push_back(excess[i]);
            }
            
            QueueShow(need, filePath);
            SendToProcesses(process_Alphabank_action, need);
            delete[] arr;
            excess.clear();
        }
        if (rankp == 2) // Sberbank
        {
            ofstream out(filePath, ios::app);
            out << l + 1 << " iteration" << endl;
            out.close();
            int arrSize = 0;
            if (MPI_Recv(&arrSize, 1, MPI_INT, 0, msgtag, comm, &status))
            {
                cout << "Processor " << rankp << ". Error: " << __LINE__ << endl;
                return -5;
            }; //get a size of an array
            int* arr = new int[arrSize] {0}; // create the array
            if (MPI_Recv(arr, arrSize, MPI_INT, 0, msgtag, comm, &status))
            {
                cout << "Processor " << rankp << ". Error: " << __LINE__ << endl;
                return -5;
            }; // get the array
            QueueShow(arrSize, arr, filePath);
            vector<int> need, excess;
            DeriveTwoOnVectors(arr, arrSize, need, excess, filePath, 2);
           
            SendToBank(excess, 1, filePath);
            for (size_t i = 0; i < excess.size(); i++)
            {
                need.push_back(excess[i]);
            }            
            QueueShow(need, filePath);
            SendToProcesses(process_Sberbank_action, need);
            delete[] arr;
            excess.clear();
        }
        if (rankp >= 3)
        {
            //Choose the bank - 1. Alphabank, 2. Sberbank, 0. I don't want to do anything
            chosenBank = rankp % 2 + 1;
            //chosenBank = (rand() % 2 == 0) ? chosenBank : 0;
            //What to do. 0 - Amount, +SUM - Put, -Sum - Withdraw;
            int sign = (rand() % 2 == 0) ? +1 : -1;
            action = ((l+1) % 3 == 0) ? 0 : sign * rand() % maxSum;
            // X process sends to zero process a number of a bank
            if (MPI_Send(&chosenBank, 1, MPI_INT, 0, msgtag, comm)!=MPI_SUCCESS)
            {
                cout << "Processor " << rankp << ". Error: " << __LINE__ << endl; return -5;
            }
            
            // X process sends to zero process what to do
            if (MPI_Send(&action, 1, MPI_INT, 0, msgtag, comm) != MPI_SUCCESS)
            {
                cout << "Processor " << rankp << ". Error: " << __LINE__ << endl; return -5;
            }

            output.open(filePath, ios::app);
            output << "\n\n****************The " << l + 1 << " iteration****************\n";
            output << "Choose the bank - 1. Alphabank, 2. Sberbank\n";
            output << "Processor " << rankp << " chosen the " << ((chosenBank==1)? "AlphaBank":"Sberbank") << " bank\n";
            output << "What to do. 0 - Amount, +SUM - Put, -Sum - Withdraw:\n";
            output << "Processor " << rankp << " chosen the next action: " << action << "\n\n\n";
            output.close();
            //getting string size
            output.open(filePath, ios::app);
            int sizeOfString = 0;           
            if (MPI_Recv(&sizeOfString, 1, MPI_INT, MPI_ANY_SOURCE, msgtag, comm, &status))
            {
                cout << "Processor " << rankp << ". Error: " << __LINE__ << endl;
                return -5;
            }
            char* str = new char[sizeOfString];
            if (MPI_Recv(str, sizeOfString, MPI_CHAR, MPI_ANY_SOURCE, msgtag, comm, &status))
            {
                cout << "Processor " << rankp << ". Error: " << __LINE__ << endl;
                return -5;
            };
            string s = str;
            output << s << endl;
            output.close();
        }
       MPI_Barrier(comm);
    }

    MPI_Finalize();
    return MPI_SUCCESS;
}

