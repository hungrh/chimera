#include <iostream>
#include <string>
#include <vector>
#include <stdint.h>
#include <sys/time.h>


#include "Alta.h"
#include "Ascent.h"
#include "CameraInfo.h"
#include "FindDeviceUsb.h"
#include "apgSampleCmn.h"
//#include "Apogee.h"

////////////////////////////////////
//
class Timer
{
	public:
		Timer()
		{

		}

		~Timer()
		{

		}

		void Start()
		{
			gettimeofday( &m_start, NULL);
		}

		void Stop()
		{
			gettimeofday( &m_end, NULL);
		}

		long GetTimeInMs()
		{
			long seconds  = m_end.tv_sec  - m_start.tv_sec;
			long useconds = m_end.tv_usec - m_start.tv_usec;
			long mtime = ( (seconds * 1000) + useconds/1000.0) + 0.5;
			return mtime;
		}

		double GetTimeInSec()
		{
			long seconds  = m_end.tv_sec  - m_start.tv_sec;
			long useconds = m_end.tv_usec - m_start.tv_usec;
			double mtime = (double)seconds + ((double)useconds/1000000.0);
			return mtime;
		}

	private:
		struct timeval m_start;
		struct timeval m_end;

};

///////////////////////////
// MAKE	  TOKENS
std::vector<std::string> makeTokens(const std::string &str, const std::string &separator)
{
	std::vector<std::string> returnVector;
	std::string::size_type start = 0;
	std::string::size_type end = 0;

	while( (end = str.find(separator, start)) != std::string::npos)
	{
		returnVector.push_back (str.substr (start, end-start));
		start = end + separator.size();
	}

	returnVector.push_back( str.substr(start) );

	return returnVector;
}

////////////////////////////
//	GET		ADDRESS
std::string GetAddress( const std::string & msg )
{

	//search the find string for an attached camera address
	std::vector<std::string> params = makeTokens( msg, "," );
	std::vector<std::string>::iterator iter;

	for(iter = params.begin(); iter != params.end(); ++iter)
	{
	   if( std::string::npos != (*iter).find("address=") )
	   {
		 std::string DeviceAddr = makeTokens( (*iter), "=" ).at(1);
		 //only looking for one camera
		 return DeviceAddr ;
	   }
	} //for

	std::string noOp;
	return noOp;
}

////////////////////////////
// MAIN
int main()
{
	try
	{
		//only looking for usb device

        	std::string ioInterface("usb");
        	FindDeviceUsb look4cam;
        	std::string msg = look4cam.Find();
		std::cout << msg << std::endl;
                
        	std::string addr = apgSampleCmn::GetUsbAddress( msg );
       		uint16_t id = apgSampleCmn::GetID( msg );
	        uint16_t frmwrRev = apgSampleCmn::GetFrmwrRev( msg );
        
 	        //create the camera object and conneting to the camera
		// assuming alta camera for this example, use the Ascent
		// camera object for Ascent cameras.
                
#ifdef APOGEE_ASCENT
                Ascent cam;
#else
   	        Alta cam;
#endif

	        cam.OpenConnection( ioInterface, addr, frmwrRev, id );

		std::cout << "model = " << cam.GetModel().c_str() << std::endl;

		cam.Init();

        	std::cout << "Info= " << cam.GetInfo().c_str() << std::endl;
        	std::cout << "Model= " << cam.GetModel().c_str() << std::endl;
        	std::cout << "Sensor= " << cam.GetSensor().c_str() << std::endl;
        	std::cout << "DriverVersion= " << cam.GetDriverVersion().c_str() << std::endl;
        	std::cout << "UsbFirmwareVersion= " << cam.GetUsbFirmwareVersion().c_str() << std::endl;
        	std::cout << "InterfaceType= " << cam.GetInterfaceType() << std::endl;
        	std::cout << "FirmwareRev = " << cam.GetFirmwareRev() << std::endl;
        	std::cout << "SerialNumber= " << cam.GetSerialNumber().c_str() << std::endl;
        	std::cout << "AvailableMemory= " << cam.GetAvailableMemory() << std::endl;
        	std::cout << "MaxBinCols= " << cam.GetMaxBinCols() << std::endl;
        	std::cout << "MaxBinRows= " << cam.GetMaxBinRows() << std::endl;
        	std::cout << "MaxImgCols = " << cam.GetMaxImgCols() << std::endl;
        	std::cout << "MaxImgRows= " << cam.GetMaxImgRows() << std::endl;
        	std::cout << "TotalRows= " << cam.GetTotalRows() << std::endl;
        	std::cout << "TotalCols = " << cam.GetTotalCols() << std::endl;
        	std::cout << "NumOverscanCols = " << cam.GetNumOverscanCols() << std::endl;
	        std::cout << "RoiNumRows= " << cam.GetRoiNumRows() << std::endl;
        	std::cout << "RoiNumCols= " << cam. GetRoiNumCols() << std::endl; 
        	std::cout << "RoiStartRow= " << cam.GetRoiStartRow() << std::endl;
       		std::cout << "RoiStartCol= " << cam.GetRoiStartCol() << std::endl; 
       		std::cout << "RoiBinRow= " << cam.GetRoiBinRow() << std::endl;
        	std::cout << "RoiBinCol= " << cam.GetRoiBinCol() << std::endl;
        	std::cout << "ImageCount= " << cam. GetImageCount() << std::endl;
        	std::cout << "ImgSequenceCount= " << cam.GetImgSequenceCount() << std::endl;
        	std::cout << "SequenceDelay= " << cam.GetSequenceDelay() << std::endl;
        	std::cout << "VariableSequenceDelay= " << cam.GetVariableSequenceDelay() << std::endl;
        	std::cout << "TdiRate= " << cam.GetTdiRate() << std::endl;
        	std::cout << "TdiRows= " << cam.GetTdiRows() << std::endl;
        	std::cout << "TdiCounter= " << cam.GetTdiCounter() << std::endl;
        	std::cout << "TdiBinningRows= " << cam.GetTdiBinningRows() << std::endl;
        	std::cout << "KineticsSectionHeight= " << cam.GetKineticsSectionHeight() << std::endl;
        	std::cout << "KineticsSections= " << cam.GetKineticsSections() << std::endl;
        	std::cout << "KineticsShiftInterval= " << cam.GetKineticsShiftInterval() << std::endl;
        	std::cout << "ShutterStrobePosition= " << cam.GetShutterStrobePosition() << std::endl;
        	std::cout << "ShutterStrobePeriod= " << cam.GetShutterStrobePeriod() << std::endl;
        	std::cout << "ShutterCloseDelay= " << cam.GetShutterCloseDelay() << std::endl;
#ifdef ALTA_NET
        	std::cout << "MacAddress= " << cam.GetMacAddress().c_str() << std::endl;
#endif
        	std::cout << "CameraMode= " << cam.GetCameraMode() << std::endl;
        	std::cout << "IoPortAssignment= " << cam.GetIoPortAssignment() << std::endl;
        	std::cout << "IoPortBlankingBits= " << cam.GetIoPortBlankingBits() << std::endl;
        	std::cout << "IoPortDirection= " << cam.GetIoPortDirection() << std::endl;
        	std::cout << "IoPortData= " << cam.GetIoPortData() << std::endl;
        	std::cout << "ShutterState= " << cam.GetShutterState() << std::endl;
        	std::cout << "CcdAdcResolution = " << cam.GetCcdAdcResolution() << std::endl;
        	std::cout << "CcdAdcSpeed= " << cam.GetCcdAdcSpeed() << std::endl;
        	std::cout << "PlatformType = " << cam.GetPlatformType() << std::endl;
        	std::cout << "LedAState= " << cam.GetLedAState() << std::endl;
        	std::cout << "LedBState= " << cam.GetLedBState() << std::endl;
        	std::cout << "LedMode= " << cam.GetLedMode() << std::endl;
        	std::cout << "PixelWidth= " << cam.GetPixelWidth() << std::endl;
        	std::cout << "PixelHeight= " << cam.GetPixelHeight() << std::endl;
        	std::cout << "MinExposureTime= " << cam.GetMinExposureTime() << std::endl;
        	std::cout << "MaxExposureTime= " << cam.GetMaxExposureTime() << std::endl;
        	std::cout << "InputVoltage= " << cam.GetInputVoltage() << std::endl;
        	std::cout << "FlushBinningRows= " << cam.GetFlushBinningRows() << std::endl;
//      	  std::cout << "LedBrightness= " << cam.GetLedBrightness() << std::endl;
        	std::cout << "NumAds= " << cam.GetNumAds() << std::endl;
        	std::cout << "NumAdChannels= " << cam.GetNumAdChannels() << std::endl;
        	std::cout << "CoolerDrive= " << cam.GetCoolerDrive() << std::endl;
        	std::cout << "CoolerStatus= " << cam.GetCoolerStatus() << std::endl;
        	std::cout << "CoolerBackoffPoint= " << cam.GetCoolerBackoffPoint() << std::endl;
        	std::cout << "CoolerSetPoint= " << cam.GetCoolerSetPoint() << std::endl;
        	std::cout << "TempCcd= " << cam.GetTempCcd() << std::endl;
        	std::cout << "TempHeatsink= " << cam.GetTempHeatsink() << std::endl;
        	std::cout << "FanMode= " << cam.GetFanMode() << std::endl;	

	}
	catch( std::exception & err )
	{
		std::cout << "std exception what = " << err.what() << std::endl;
		return 1;
	}
	catch( ... )
	{
		std::cout << "Unknown exception caught." << std::endl;
		return 1;
	}
	return 0;
}
