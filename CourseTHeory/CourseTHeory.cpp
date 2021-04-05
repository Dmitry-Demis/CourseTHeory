// CourseTHeory.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include "BaseClass.h"
#include <Windows.h>
#include <vector>
#include <memory>
#include "mpi.h"
#include <fstream>
#include <queue>
using namespace std;

#define comm MPI_COMM_WORLD
int rankp, sizep;

#define lmax 10
struct ProcBankSum
{
    int processor = 0;
    int bank = 0;
    int action = 0;
};
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
    int msgtag = 4;
    const int maxSum = 50'000;
    int amountCounter = 0;
    MPI_Status status;
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
    srand((unsigned)time(NULL)+rankp*10'000*sizep);
   doDebug(rankp);
   // for (int l = 0; l < lmax; l++) // a life cicle
    {
        
        if (!rankp)
        {
            queue<ProcBankSum> even; // .first - processor, .second - value
            queue<ProcBankSum> odd;
            for (int i = 3; i < sizep; i++)
            {
                MPI_Recv(&chosenBank, 1, MPI_INT, MPI_ANY_SOURCE, msgtag, comm, &status);
                MPI_Recv(&action, 1, MPI_INT, MPI_ANY_SOURCE, msgtag, comm, &status);
                int fromWhom = status.MPI_SOURCE;
                ProcBankSum pbs;
                pbs.processor = fromWhom;
                pbs.bank = chosenBank;
                pbs.action = action;
                if (i%2==0) // even
                {
                    
                    even.push(pbs);
                }
                else
                {
                    odd.push(pbs);
                }
            }
            int position = 0;
            void* evenBuf = malloc(even.size() * sizeof(ProcBankSum));
            void* oddBuf = malloc(odd.size() * sizeof(ProcBankSum));
            int sizeForEven = even.size();
            MPI_Send(&sizeForEven, 1, MPI_INT, 1, msgtag, comm);
            
            while (even.size())
            {
                 auto res = even.front();
                even.pop();
                int outBufSize = even.size() * sizeof(ProcBankSum);
                MPI_Pack(&res.processor, 1, MPI_INT, evenBuf, outBufSize, &position, comm);
                MPI_Pack(&res.bank, 1, MPI_INT, evenBuf, outBufSize, &position, comm);
                MPI_Pack(&res.action, 1, MPI_INT, evenBuf, outBufSize, &position, comm);
                MPI_Send(&position, 1, MPI_INT, 1, msgtag, comm);
                MPI_Send(evenBuf, position, MPI_PACKED, 1, msgtag, comm);
                position = 0;
            }

            free(evenBuf);
            free(oddBuf);
        }
        if (rankp == 1)
        {
            int sizeForEven;
            int position;
            
            ProcBankSum pbs;
            int pos = 0;
            queue<ProcBankSum> even;
            MPI_Recv(&sizeForEven, 1, MPI_INT, 0, msgtag, comm, &status);
            void* evenBuf = malloc(sizeForEven * sizeof(ProcBankSum));
            cout << "AlphaBank has received " << sizeForEven << " users" << endl;
            for (int i = 0; i < sizeForEven; i++)
            {
                MPI_Recv(&position, 1, MPI_INT, 1, msgtag, comm, &status);
                MPI_Recv(evenBuf, position, MPI_PACKED, 0, msgtag, comm, &status);
                MPI_Unpack(evenBuf, position, &pos, &pbs.processor, 1, MPI_INT, comm);
                MPI_Unpack(evenBuf, position, &pos, &pbs.bank, 1, MPI_INT, comm);
                MPI_Unpack(evenBuf, position, &pos, &pbs.action, 1, MPI_INT, comm);
                even.push(pbs);
            }
            int a = 5;
        }
        if (rankp == 2)
        {

        }
        if (rankp >= 3)
        {
            //Choose the bank - 1. Alphabank, 2. Sberbank
             chosenBank = rand() % 2 + 1;
            //What to do. 0 - Amount, +SUM - Put, -Sum - Withdraw;
             int sign = (rand() % 2 == 0) ? +1 : -1;
             action = (amountCounter == 3) ? 0 : sign* rand() % maxSum;
            MPI_Send(&chosenBank, 1, MPI_INT, 0, msgtag, comm);
            MPI_Send(&action, 1, MPI_INT, 0, msgtag, comm);

            output.open(filePath, ios::app);
           // output << "****************The " << l + 1 << " iteration****************\n";
            output << "Choose the bank - 1. Alphabank, 2. Sberbank\n";
            output << "Processor " << rankp << " chosen the " << chosenBank << " bank\n";
            output << "What to do. 0 - Amount, +SUM - Put, -Sum - Withdraw:\n";
            output << "Processor " << rankp << " chosen the next action: " << action << "\n";


            output.close();
        }
    }

    MPI_Finalize();
    return MPI_SUCCESS;
}
