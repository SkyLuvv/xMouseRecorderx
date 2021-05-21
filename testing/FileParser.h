#pragma once
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>

class FileParser
{
public:
	enum class result
	{
		coordinates,
		delay,
		error
	};
	
public:
	FileParser(const std::string& filename)
		:
		filename(filename)
	{
		is.open(filename);

		if (!fileisopen())
			std::cout << "Failed to open file. Check if the file name is correct and if it's in the same directory as the exe file." << std::endl;


	};
	FileParser::result ProcessLine()
	{
		if (!std::getline(is, currentline))
		{
			curr_result = result::error;
			return curr_result;
		}

		auto start{ currentline.find('{') };

		//if this fails, then it could be a delay
		if (start != std::string::npos)
		{
	
			auto end{ currentline.find(',') }; 	
		
			if (end != std::string::npos)
			{
				auto xcoord{ currentline.substr(start + 1, (end - start) - 1) };

				try
				{
					x = std::stoi(xcoord);
				}
				catch (...)
				{
					std::cout << "couldn't convert line. make sure it is correct input" << std::endl;
					curr_result = result::error;
					return curr_result;
				}

			}
			else
			{
				curr_result = result::error;
				return curr_result;
			}
		
			start = end; //grab the y coordinate

			end = currentline.find('}');

			if (end != std::string::npos)
			{
				
				auto ycoord{ currentline.substr(start + 1, (end - start) - 1) };

				try
				{					
					y = std::stoi(ycoord);
				}
				catch (...)
				{
					std::cout << "couldn't convert line. make sure it is correct input" << std::endl;
					curr_result = result::error;
					return curr_result;
				}
			}
			else
			{
				return result::error;
			}

			curr_result = result::coordinates;
			return curr_result;
			
		}
		else
		{
						
			try
			{
				delay = std::stoi(currentline);
			}
			catch (...)
			{
				std::cout << "couldn't convert line. make sure it is correct input" << std::endl;
				curr_result = result::error;
				return curr_result;
			}

			curr_result = result::delay;
			return curr_result;
		}
		
		curr_result = result::error;
		return curr_result;
	}
	bool fileisopen()
	{
		return is.is_open();
	}
private:
	std::ifstream is;
	std::string filename;
	std::string currentline;

public:
	result curr_result;
	int32_t x;
	int32_t y;
	size_t delay;
};