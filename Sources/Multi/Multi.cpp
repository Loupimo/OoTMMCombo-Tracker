#include "Multi/Multi.h"

static MultiLogger Logger;


MultiLogger* MultiLogger::GetLogger()
{
	return &Logger;
}