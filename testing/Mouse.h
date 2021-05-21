#pragma once
#include <Windows.h>
#include <vector>
#include <optional>
#include <condition_variable>
#include <mutex>

class Mouse
{
    //make this a noncopyable singleton class,
    //private constructor will prevent instantiations

private:
    Mouse()
        :
        coordinates{ 0,0 },
        Hook{ 0 }
    {
    }


public:
    struct Coordinates
    {
        Coordinates(int32_t x, int32_t y)
            :
            x(x),
            y(y)
        {

        }
        Coordinates(const Coordinates& Other)
        {
            x = Other.x;
            y = Other.y;
        }
        void operator = (const Coordinates& Other)
        {
            x = Other.x;
            y = Other.y;
        }
        bool operator == (const Coordinates& Other)
        {
            return ((x == Other.x) && (y == Other.y));
        }
        bool operator < (const Coordinates& Other)
        {
            return (x < Other.x && y < Other.y) ? true : false;
        }
        bool operator > (const Coordinates& Other)
        {
            return (x > Other.x && y > Other.y) ? true : false;
        }
        const Coordinates operator - (const Coordinates& Other)
        {
            return { x - Other.x, y - Other.y };
        }
        const Coordinates operator * (const Coordinates& Other)
        {
            return { x * Other.x, y * Other.y };
        }
        int32_t x = 0, y = 0;

    };
    /*---------------------------------------------------------------------------------------------------------*/

    //singleton class

    Mouse(Mouse& other) = delete;
    Mouse& operator= (Mouse& other) = delete;
    static Mouse* getInstance()
    {
        static Mouse mouse;   //one instance of the class only
        return &mouse;
    }
    static bool RelativeMove(const Coordinates& co)
    {
        /******************************************************************************************
        IMPORTANT REGARDING RELATIVE MOVES (without ABSOLUTE FLAG) WITH SENDINPUT. MOUSE ACCELERATION NEEDS TO BE OFF
        https://stackoverflow.com/questions/65150862/sendinput-does-not-send-the-exact-amount-of-pixels-i-specify

        If MOUSEEVENTF_ABSOLUTE value is specified, dxand dy contain normalized absolute coordinates between 0 and 65, 535.
        The event procedure maps these coordinates onto the display surface.Coordinate(0, 0) maps onto the upper - left corner
        of the display surface; coordinate(65535, 65535) maps onto the lower - right corner.In a multimonitor system, the coordinates map to the primary monitor.
       " This means when using MOUSEEVENTF_ABSOLUTE, you will have to set mi.dx = (x * 65535) / (screen width); and mi.dy = (y * 65535) / (screen height);
        ******************************************************************************************/

        static INPUT input{ 0 };
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_MOVE;
        input.mi.dx = co.x;
        input.mi.dy = co.y;

        return SendInput(1, &input, sizeof(input));
    }
    static LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
    {

        auto pmousellhookstruct{ reinterpret_cast<PMSLLHOOKSTRUCT>(lParam) };

        if (nCode == HC_ACTION)
        {
          

            switch (wParam)
            {
            case WM_MOUSEMOVE:
            {

                static size_t time_last_move{ 0 };
                static Coordinates prev_co{ 0,0 };
 
                getInstance()->coordinates = { pmousellhookstruct->pt.x ,pmousellhookstruct->pt.y };

                if (getInstance()->record)
                {
                                     
                    if (time_last_move == 0)
                    {
                        //we do this to ensure we don't log unwanted data when both variables are not initialized
                        time_last_move = pmousellhookstruct->time;
                        prev_co = getInstance()->coordinates;
                        break;
                    }

                    auto movedby{ getInstance()->coordinates - prev_co };   //get the amount of pixels the mouse moved by
                    auto delay{ pmousellhookstruct->time - time_last_move };

                    if (movedby == Coordinates{ 0,0 })
                        break;
                    else if (movedby < Coordinates{ 4,4 } && movedby > Coordinates{ -4,-4 })
                        break;

                    getInstance()->movements.push_back(std::make_pair(movedby, delay));
                    time_last_move = pmousellhookstruct->time;
                    prev_co = getInstance()->coordinates;

                }


                break;
            }
            case WM_LBUTTONDOWN:
            {

                getInstance()->leftisdown = true;

                break;
            }
            case WM_LBUTTONUP:
            {

                getInstance()->leftisdown = false;

                break;
            }
            case WM_RBUTTONDOWN:
            {

                getInstance()->rightisdown = true;

                break;
            }
            case WM_RBUTTONUP:
            {
                getInstance()->rightisdown = false;
                break;
            }
            case WM_KILLFOCUS:
            {
                return 1;
            }
        
            }
         
        }

        if (wParam != WM_MOUSEMOVE)
            getInstance()->cond_var.notify_all();


        return CallNextHookEx(getInstance()->Hook, nCode, wParam, lParam);
    }
    void RecordMovements()
    {
        record = true;
    }
    void StopRecordingMovements()
    {
        record = false;
    }
    const bool LeftIsDown()const
    {
        return leftisdown;
    }
    const bool RightIsDown()const
    {
        return rightisdown;
    }
    void SetHook(const HHOOK& hook)
    {
        Hook = hook;
    }
    const HHOOK& GetHook()const
    {
        return Hook;
    }
    /*---------------------------------------------------------------------------------------------------------*/

public:
    std::vector<std::pair<Mouse::Coordinates, unsigned long>> movements;
    std::condition_variable cond_var;
    std::mutex mu;
private:
    Coordinates coordinates;
    HHOOK Hook;

private:
    bool leftisdown = false;
    bool rightisdown = false;
    bool record = false;

};
