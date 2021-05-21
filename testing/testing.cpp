// testing.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <Windows.h>
#include<thread>
#include<chrono>
#include <vector>
#include "FileParser.h"
#include <optional>
#include "Mouse.h"

using std::this_thread::sleep_for;
using namespace std::chrono;
bool continuerecoilthread = true;

void parsefile(Mouse* pMouse)
{

    //if not recording, get the coordinates/delays from the file and store them in our buffer
    FileParser fp("coordinates.txt");

    //process line twice, first for the coordinates, second for the delay
    while (fp.ProcessLine() != FileParser::result::error)
    {
        if (fp.ProcessLine() != FileParser::result::error)
            pMouse->movements.push_back(std::make_pair(Mouse::Coordinates{ fp.x,fp.y }, fp.delay));
    }
   
}

void recordmovements(Mouse * pMouse)
{
    pMouse->RecordMovements();
  
    while (true)
    {
        std::unique_lock<std::mutex> lock(pMouse->mu);
        pMouse->cond_var.wait(lock);
        
        if (pMouse->RightIsDown())
        {
            pMouse->StopRecordingMovements();

            std::ofstream myfile("coordinates.txt");
            std::cout << "Writing to coordinates.txt " << std::endl;
           
            //write the new coordinates to the file
            for (auto it{ pMouse->movements.begin() }; it != pMouse->movements.end(); std::advance(it, 1))
            {
                myfile << '{' << it->first.x << ',' << it->first.y << '}' << "\n";
                myfile << it->second << "\n";
            }

            myfile.close();
            break;
        }
       
    }
}
void recoil_handler(size_t option)
{

    auto pMouse{ Mouse::getInstance() };

    switch (option)
    {
    case 1:
    {
        std::cout << "\n\n3\n\n";
        sleep_for(seconds(1));
        std::cout << "2\n\n";
        sleep_for(seconds(1));
        std::cout << "1\n\n";
        sleep_for(seconds(1));


        std::cout << "\nRecording movements..." << std::endl;
        recordmovements(pMouse);
        break;
    }
    case 2:
    {
        std::cout << "\nParsing coordinates.txt..." << std::endl;
        parsefile(pMouse);
        break;
    }

    }
   
    std::cout << "Hold L And R click to simulate movements now. " << "Coordinates Buffer Size: " << pMouse->movements.size() << std::endl;
    

    /***********************************************************************************************************************************************/
    //https://stackoverflow.com/questions/2763714/why-do-pthreads-condition-variable-functions-require-a-mutex#:~:text
    //=The%20mutex%20is%20used%20to,before%20you%20do%20a%20wait.&text=Then%20when%20the%20condition%20variable,locked%20again%20for%20that%20thread.
    /***********************************************************************************************************************************************/



;        
    while (continuerecoilthread)
    {                                            //lock the mutex
        std::unique_lock<std::mutex> lock(pMouse->mu);   //unique lock used RAII in other words scope-bound memory management. in the case the code throws an exception, the lock will release and unlock the mutex.
                                                 //using mutex alone can lock the mutex forever in the case an exception is thrown.
        
        pMouse->cond_var.wait(lock);             //condition variable unlocks the mutex until the condition is met and it locks the mutex again when notified. when notified, the conditionvariable allows
                                                 //the thread to execute


        while (pMouse->RightIsDown() && pMouse->LeftIsDown())
        {
            for (auto it{ pMouse->movements.begin() }; it != pMouse->movements.end(); std::advance(it, 1))
            {
                if (pMouse->LeftIsDown())
                {
                    //keep trying to send the input in the case sendinput fails
                    while (Mouse::RelativeMove( it->first) != 1 );
                    sleep_for(milliseconds(it->second));
                }
                else break;
            }
        }
    }
    
}
int main()
{

    
    SetConsoleTitle(L"Annie's Anti-Recoil Pattern Reader");

    std::cout << "This program will read the coordinates from the coordinates.txt file. Make sure they are both in the same directory.\n";
    std::cout << "Type and [ENTER]:\n\nOption 1: Record Mouse Movements As Soon As The Option Is Entered. Mouse Coordinates Will Be overwritten and logged to coordinates.txt" << std::endl;
    std::cout << "Hit R-Click To Stop Recording\n\n";
    std::cout << "Option 2: Use Already Existing Coordinates In coordinates.txt\n";
    std::cout << "Choose Option :";


    size_t option{ 0 };
    std::cin >> option;

    //launch the function asynchronously so our main function can handle the message queue for the hook or whatever
    std::thread worker(recoil_handler, option);

    Mouse::getInstance()->SetHook(SetWindowsHookEx(WH_MOUSE_LL, Mouse::MouseProc, NULL, NULL));

    MSG msg{ 0 };
    //our application loop
    while (GetMessage(&msg, NULL, 0, 0) != 0);
    UnhookWindowsHookEx(Mouse::getInstance()->GetHook());

    continuerecoilthread = false;
    worker.join();
    
    return 0;
}