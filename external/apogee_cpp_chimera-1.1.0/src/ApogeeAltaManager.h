/*
 * ApogeeAltaManager.h
 *
 *  Created on: Dec 15, 2012
 *      Author: hung
 */

#ifndef APOGEEALTAMANAGER_H_
#define APOGEEALTAMANAGER_H_

#include "fitsio.h"
#include "tcl.h"
#include "Alta.h"
#include "FindDeviceUsb.h"
#include "CameraInfo.h"
#include "apgSampleCmn.h"
#include "ccd.h"

/* Bias definitions used in the libccd functions */
extern int bias_start, bias_end, bcols;

typedef struct {
	unsigned short *pixels;
	int            size;
	int          xdim;
	int          ydim;
	int          zdim;
	int          xbin;
	int          ybin;
	int          type;
	char           name[64];
	int            shmid;
	size_t         shmsize;
	char           *shmem;
} CCD_FRAME;

typedef void *PDATA;
#define MAX_CCD_BUFFERS  1000
PDATA CCD_locate_buffer(char *name, int idepth, int imgcols, int imgrows, int hbin, int vbin);
int   CCD_free_buffer();
int   CCD_locate_buffernum(char *name);
extern CCD_FRAME CCD_Frame[MAX_CCD_BUFFERS];
extern int CCD_free_buffer(char *name);
extern void CCD_buffer_init();
void checkStatus( const Apg::Status status );

struct ApogeeAltaConfig {
	char imagename[256];
	double texposure;
	int shutter;
	int xbin;
	int ybin;
	int xstart;
	int xend;
	int ystart;
	int yend;
	int biascols;
	Apg::FanMode fanmode;
	double cooling;
	int numexp;
	int firmware;
	int modelnum;
	int ipause;
	int verbose;
	int camnum;
	Apg::AdcSpeed highspeed;
	int tdimode;
	int tdirows;
	int bulkseq;
};

class ApogeeAltaManager {
public:
	ApogeeAltaManager(int xbin, int ybin, int xstart, int xend, int ystart, int yend);
	virtual ~ApogeeAltaManager();

	// //  por algum motivo isso eh necessario no wrapper da boost para python
	// ApogeeAltaManager(const ApogeeAltaManager& manager) {}

	virtual bool setUp();
	virtual bool expose(char* image_name, double time_exposure, int shutter);
	virtual void stop();

	virtual void startCooling(double temperature);
	virtual void stopCooling();
	virtual double getTemperature();
	virtual std::vector<uint16_t> getImageData() { return imageData_; }	

	virtual void startFan();
	virtual void stopFan();

private :
	virtual int saveimage(unsigned short *src_buffer, char *filename, int nx, int ny);
	virtual int dobiassubtract(unsigned short *src,unsigned short *dest, int nx, int ny);
	virtual void checkStatus( const Apg::Status status );
	virtual bool checkReady();

	ApogeeAltaConfig config_;
	Alta apogee_;
	bool ready_;
	std::vector<uint16_t> imageData_;
};

extern "C"  //Tells the compile to use C-linkage for the next scope.
{
    ApogeeAltaManager* NewApogeeAltaManager(int xbin, int ybin, int xstart, int xend,
     int ystart, int yend)
    {
        return new ApogeeAltaManager(xbin, ybin, xstart, xend, ystart, yend);
    }

    void DeleteApogeeAltaManager(ApogeeAltaManager* manager)
    {
         delete manager; 
    }

    bool setUp(ApogeeAltaManager* manager)
    {
    	return manager->setUp();
    }

    bool expose(ApogeeAltaManager* manager, char* image_name, double time_exposure, int shutter)
    {
    	return manager->expose(image_name, time_exposure, shutter);
    }

	void startCooling(ApogeeAltaManager* manager, double temperature)
	{
		return manager->startCooling(temperature);
	}

	void stopCooling(ApogeeAltaManager* manager)
	{
		return manager->stopCooling();
	}

	double getTemperature(ApogeeAltaManager* manager)
	{
		return manager->getTemperature();
	}

	std::vector<uint16_t> getImageData(ApogeeAltaManager* manager)
	{
		return manager->getImageData();	
	} 

	void startFan(ApogeeAltaManager* manager)
	{
		return manager->startFan();
	}

	void stopFan(ApogeeAltaManager* manager)
	{
		return manager->stopFan();
	}

    void stop(ApogeeAltaManager* manager)
    {
    	manager->stop();
    }        

} //End C linkage scope.


#endif /* APOGEEALTAMANAGER_H_ */
