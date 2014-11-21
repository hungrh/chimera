#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <stdint.h>
#include <stdexcept>
#include <sys/time.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "Ascent.h"
#include "FindDeviceUsb.h"
#include "CameraInfo.h"

namespace
{
	const uint16_t OP_B_REG = 0x0003;
	const uint16_t SIMULATION_BIT = 0x8000;
	
	void SetSimMode( Ascent & cam, const bool TurnOn )
	{
		const uint16_t opBValue = cam.ReadReg( OP_B_REG );
		uint16_t val2Write = opBValue;
		
		if( TurnOn )
		{
			val2Write = opBValue |  SIMULATION_BIT;
		}
		else
		{
			val2Write = opBValue &  ~SIMULATION_BIT;
		}
		
		cam.WriteReg( OP_B_REG, val2Write );
	}
	
	bool IsSimOn(  Ascent & cam )
	{
		const uint16_t opBValue = cam.ReadReg( OP_B_REG );
		
		 return(  SIMULATION_BIT == (opBValue &  SIMULATION_BIT) ? true : false );
	}
	
}

