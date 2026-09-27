#pragma once
#include <iostream>

void Log(const char type[], const char message[]) {
	std::cout << type << message << std::endl;
}
