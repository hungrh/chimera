/*
 * ApogeeAltaManager.cpp
 *
 *  Created on: Dec 15, 2012
 *      Author: hung
 */

// TODO : fazer um grande refactoring para "proteger" os metodos, note que estao simplesmente chamando
// e nao estao fazendo nenhuma verificacao se foi chamado setUp() ou algo assim

#include "ApogeeAltaManager.h"

#include <iostream>
#include <stdexcept>
#include <string.h>
#include <string>
#include <cmath>

ApogeeAltaManager::ApogeeAltaManager(int xbin, int ybin, int xstart, int xend,
 int ystart, int yend) : ready_(false)
{
	config_.xbin = xbin;
	config_.ybin = ybin;
	config_.xstart = xstart;
	config_.xend = xend;
	config_.ystart = ystart;
	config_.yend =yend;

	config_.biascols=0;
    config_.fanmode=Apg::FanMode_Medium;
	config_.cooling=99.0;
	config_.numexp=1;
	config_.firmware=33;
	config_.modelnum=0;
	config_.ipause=0;
	config_.verbose=0;
	config_.camnum=1;
	config_.highspeed=Apg::AdcSpeed_Normal;
	config_.tdimode=0;
	config_.tdirows=0;
	config_.bulkseq = 0;
}

ApogeeAltaManager::~ApogeeAltaManager()
{
	stop();
}

bool ApogeeAltaManager::checkReady()
{
	if (!ready_)
		throw std::runtime_error("The camera is not ready! Please check the configurations");
	return true;
}

bool ApogeeAltaManager::setUp()
{
	try 
	{
		/*	Default the bias to no-bias-subtraction */
		bias_start = 0;
		bias_end = 0;
		bcols = 0;

		std::string ioInterface("usb");
		FindDeviceUsb lookUsb;
		std::string msg = lookUsb.Find();
		std::string addr = apgSampleCmn::GetUsbAddress( msg );

		uint16_t id = apgSampleCmn::GetID( msg );
		uint16_t frmwrRev = apgSampleCmn::GetFrmwrRev( msg );

		std::cout << "ApogeeAltaManager::setUp() - openning the connection..." << std::endl;

		/*	Create the camera object , this will reserve memory */
		apogee_.OpenConnection(ioInterface, addr, frmwrRev, id);

		/*	Do a system Init to ensure known state, flushing enabled etc */
		apogee_.Init();
		CCD_buffer_init();

		std::cout << "ApogeeAltaManager::setUp() - initialized successfully!" << std::endl;
		ready_ = true;
		return true;
	} catch (std::exception& e)
	{
		std::cerr << "ApogeeAltaManager::setUp() - exception = " << e.what() << std::endl;
	} catch (...)
	{
		std::cerr << "ApogeeAltaManager::setUp() - unkown exception" << std::endl;
	}

	return false;
}

bool ApogeeAltaManager::expose(char* image_name, double time_exposure, int shutter)
{
	try 
	{
		strcpy(config_.imagename, image_name);
		config_.texposure = time_exposure;

		/*	Obtain user provided options */
		/*     Image name */
		if( access(config_.imagename, F_OK ) != -1 ) {
			std::cout << "filename already exists, deleting it..." << std::endl;
			// file exists
			if (remove(config_.imagename) != 0)
			{
				std::cerr << "error deleting file" << std::endl;
				//  como diz no comentario do metodo saveimage(), caso o arquivo ja existir vai dar erro
				return false;
			}
		}

		std::cout << "config_.imagename = " << config_.imagename << std::endl;
		std::cout << "config_.texposure = " << config_.texposure << std::endl;

		unsigned short ir, Reg;
		/*      Special verbosity to dump regs and exit */
		if (config_.verbose == 99)
		{
			for(ir=0;ir<106;ir++)
			{
				Reg=apogee_.ReadReg(ir);
				printf ("Register %d = %d (%x)\n",ir,Reg,Reg);
			}
			return false;
		}

		/*	If bias subtraction requested, set it up */
		if (config_.biascols != 0)
		{
			bcols = config_.biascols;
		}

		/*	Set up a region of interest, defaults to full frame */
		if (config_.xstart > 0)
		{
			apogee_.SetRoiStartCol(config_.xstart);
			apogee_.SetRoiStartRow(config_.ystart);
			apogee_.SetRoiNumCols(config_.xend-config_.xstart+1);
			apogee_.SetRoiNumRows(config_.yend-config_.ystart+1);
		}

		/*      Set up binning */
		apogee_.SetRoiBinCol(config_.xbin);
		apogee_.SetRoiBinRow(config_.ybin);

		/*	Set the required fan mode */
		apogee_.SetFanMode(config_.fanmode,false);

		/*	If a particular CCD temperature was requested, then enable
		cooling and set the correct setpoint value */
		if (config_.cooling < 99.0)
		{
			printf("Waiting for requested temperature of %6.1lf \r",config_.cooling);
			apogee_.SetCooler(1);
			apogee_.SetCoolerSetPoint(config_.cooling);
			double t = apogee_.GetTempCcd();

			/*	   Then loop until we get within 0.2 degrees, about the best we can hope for */
			while (fabs(t-config_.cooling) > 0.2)
			{
				printf("Waiting for requested temperature of %6.1lf, current value is %6.1lf \r",
						config_.cooling, t);
				sleep(1);
				t = apogee_.GetCoolerStatus();
				t = apogee_.GetTempCcd();
			}
			printf("\n	Temperature is now %6.1lf\n",t);
		}

		//	/*	Add a second to ensure readout will be complete when we try to read */
		//	int iexposure = (int)config_.texposure+1;

		/*	Setup binning, defaults to full frame */
		int i = 0;

		unsigned short *pccdData;
		unsigned short *image;
		int nx,ny, bnum;
		std::vector<uint16_t> pImageData;
		char seqname[256];

		/*	Loop until all exposures completed */
		while ( i < config_.numexp )
		{
			//  clear the image data...
			imageData_.clear();
			
			if ( config_.bulkseq )
			{
				printf("Bulk Image Sequence mode \n");
				apogee_.SetImageCount(config_.numexp);
				apogee_.SetBulkDownload (true);
				apogee_.SetSequenceDelay(0.001);
				apogee_.SetVariableSequenceDelay(false);
				apogee_.SetShutterCloseDelay(0.0);
				apogee_.SetKineticsShiftInterval(0.0001);
				apogee_.SetFlushBinningRows(512);
				apogee_.SetPostExposeFlushing(true);
				apogee_.SetFlushCommands(true);
			} else
			{
				apogee_.SetImageCount(1);
			}

			/*          Setup TDI if requested */
			if (config_.tdimode > 0)
			{
				// Toggle the camera mode for TDI
				apogee_.SetCameraMode(Apg::CameraMode_TDI);
				printf("SetCameraMode ");
				// Set the TDI row count
				apogee_.SetTdiRows (config_.tdirows);
				printf("SetTdiRows ");
				// Set the TDI rate
				apogee_.SetTdiRate (config_.texposure);
				printf("SetTdiRate ");
				apogee_.SetTdiBinningRows(32);
				printf("SetTdiBinningRows\n ");

				// Toggle the sequence download variable
				apogee_.SetBulkDownload (true );
			}

			/*	    Start an exposure */
			apogee_.StartExposure(config_.texposure, shutter);

			// Check camera status to make sure image data is ready
			Apg::Status status = Apg::Status_Flushing;
			while( Apg::Status_ImageReady !=  status )
			{
				status = apogee_.GetImagingStatus();
				//make sure there isn't an error
				//throw here if there is
				checkStatus( status );
			}

			/*	    Readout the image and save in a named buffer (tempobs) */
			nx = apogee_.GetRoiNumCols();
			ny = apogee_.GetRoiNumRows();
			pccdData = (unsigned short *)CCD_locate_buffer(const_cast<char *>("tempobs"), 2 , nx, ny, 1, 1 );
			if (pccdData == NULL)
			{
				printf("ERROR - no CCD_Buffer\n");
				exit(1);
			}
			apogee_.GetImage(imageData_);
			copy(imageData_.begin(), imageData_.end(), pccdData);

			// /*	    Use the libccd routine to find the corresponding buffer index */
			// bnum = CCD_locate_buffernum( const_cast<char *>("tempobs") );

			// /*	    Print details about the buffer for debug purposes */
			// printf("Buffer %4d %s = %d bytes cols=%d rows=%d depth=%d\n",bnum,CCD_Frame[bnum].name,
			// 		CCD_Frame[bnum].size,CCD_Frame[bnum].xdim,CCD_Frame[bnum].ydim,CCD_Frame[bnum].zdim);

			// /*	    Obtain the memory address of the actual image data, and x,y dimensions */
			// image = CCD_Frame[bnum].pixels;
			// nx = apogee_.GetRoiNumCols();
			// ny = apogee_.GetRoiNumRows();

			// /*	    If this is part of a sequence, prefix image name with the number */
			// if (config_.numexp > 1)
			// {
			// 	sprintf(seqname,"%d_%s",i, config_.imagename);
			// 	if ( config_.bulkseq )
			// 	{
			// 		ny = apogee_.GetRoiNumRows() * apogee_.GetImageCount();
			// 		saveimage(image, seqname, nx, ny);
			// 		i = config_.numexp;
			// 	} else
			// 	{
			// 		saveimage(image, seqname, nx, ny);
			// 	}
			// } else
			// {
			// 	saveimage(image, config_.imagename, nx, ny);
			// }

			/*	    Wait requested interval between exposures (default is 0) */
			sleep(config_.ipause);
			i++;
		}
		return true;
	} catch (std::exception& e)
	{
		std::cerr << "ApogeeAltaManager::run() - exception = " << e.what() << std::endl;
	} catch (...)
	{
		std::cerr << "ApogeeAltaManager::run() - unkown exception" << std::endl;
	}
	return false;
}

void ApogeeAltaManager::stop()
{
	try
	{
		/*	 All done, tidy up */
		apogee_.CloseConnection();
		std::cout << "ApogeeAltaManager::stop() - stopped gracefully" << std::endl;
	} catch (std::exception& e)
	{
		std::cerr << "ApogeeAltaManager::stop() - exception = " << e.what() << std::endl;
	} catch (...)
	{
		std::cerr << "ApogeeAltaManager::stop() - unkown exception" << std::endl;
	}
}

void ApogeeAltaManager::startCooling(double temperature)
{
	printf("Waiting for requested temperature of %6.1lf \r",temperature);
	apogee_.SetCooler(true);
	apogee_.SetCoolerSetPoint(temperature);
	double t = apogee_.GetTempCcd();

	/*	   Then loop until we get within 0.2 degrees, about the best we can hope for */
	while (fabs(t-temperature) > 0.2)
	{
		printf("Waiting for requested temperature of %6.1lf, current value is %6.1lf \r",
				temperature, t);
		sleep(1);
		t = apogee_.GetCoolerStatus();
		t = apogee_.GetTempCcd();
	}
	printf("\n	Temperature is now %6.1lf\n",t);
}

void ApogeeAltaManager::stopCooling()
{
	apogee_.SetCooler(false);
}

double ApogeeAltaManager::getTemperature()
{
	double temperature = apogee_.GetTempCcd();
	return temperature;
}

void ApogeeAltaManager::startFan()
{
	apogee_.SetFanMode(Apg::FanMode_Medium,false);
}

void ApogeeAltaManager::stopFan()
{
	apogee_.SetFanMode(Apg::FanMode_Off,false);
}


/*  This routine provides simple FITS writer. It uses the routines
 *  provided by the fitsTcl/cfitsio libraries
 *
 *  NOTE : It will fail if the image already exists
 */
int ApogeeAltaManager::saveimage(unsigned short *src_buffer, char *filename, int nx, int ny)
{
	fitsfile *fptr;       /* pointer to the FITS file, defined in fitsio.h */
	long  fpixel, nelements;
	unsigned short *array;
	unsigned short *simg;
	int status;
	/* initialize FITS image parameters */
	int bitpix   =  USHORT_IMG; /* 16-bit unsigned short pixel values       */
	long naxis    =   2;  /* 2-dimensional image                            */
	long naxes[2];

	naxes[0] = nx-bcols;
	naxes[1] = ny;
	array = src_buffer;
	status = 0;         /* initialize status before calling fitsio routines */
	simg = (unsigned short *)CCD_locate_buffer(const_cast<char *>("stemp"),2,nx-bcols,ny,1,1);

	if (fits_create_file(&fptr, filename, &status)) /* create new FITS file */
		printerror( status );           /* call printerror if error occurs */

	/* write the required keywords for the primary array image.     */
	/* Since bitpix = USHORT_IMG, this will cause cfitsio to create */
	/* a FITS image with BITPIX = 16 (signed short integers) with   */
	/* BSCALE = 1.0 and BZERO = 32768.  This is the convention that */
	/* FITS uses to store unsigned integers.  Note that the BSCALE  */
	/* and BZERO keywords will be automatically written by cfitsio  */
	/* in this case.                                                */

	if ( fits_create_img(fptr,  bitpix, naxis, naxes, &status) )
		printerror( status );

	fpixel = 1;                               /* first pixel to write      */
	nelements = naxes[0] * naxes[1];          /* number of pixels to write */

	if (bcols > 0)
	{
		dobiassubtract(src_buffer,simg,naxes[0],naxes[1]);

		/* write the array of unsigned integers to the FITS file */
		if ( fits_write_img(fptr, TUSHORT, fpixel, nelements, simg, &status) )
			printerror( status );
	} else
	{
		/* write the array of unsigned integers to the FITS file */
		if ( fits_write_img(fptr, TUSHORT, fpixel, nelements, src_buffer, &status) )
			printerror( status );
	}

	if ( fits_close_file(fptr, &status) )                /* close the file */
		printerror( status );

	return(status);
}

/*  This routine should do bias subtraction. At present it
 *  uses the minium pixel DN as the bias value, instead of
 *  averaging the bias columns. This is because on the
 *  test unit I have, averaging these columns does not seem
 *  to give values consistently lower than those in the
 *  exposed region.
 *
 *  src is the input image with bias columns
 *  dest is a smaller output image with the bias columns trimmed off
 *       and the "bias" subtracted from the image pixels.
 */
int ApogeeAltaManager::dobiassubtract(unsigned short *src,unsigned short *dest, int nx, int ny)
{
	double biases[128000];
	int ix,iy, oix;
	int ipix, opix;
	unsigned short minbias;
	minbias = 65535;
	if (bcols == 0)
	{
		for (iy=0;iy<ny;iy++)
			biases[iy] = 0.0;

		minbias = 0;
	} else
	{
		for (iy=0;iy<ny;iy++)
		{
			biases[iy] = 0.0;
			for (ix=bias_start;ix<=bias_end;ix++)
			{
				ipix = (nx+bcols)*iy + ix-1;
				biases[iy] = biases[iy] + (float)src[ipix];
				if (src[ipix]<minbias) minbias = src[ipix];
			}
			biases[iy] =  biases[iy] / (float)bcols;
		}
	}

	for (iy=0;iy<ny;iy++)
	{
		oix = 0;
		for (ix=0;ix<nx+bcols;ix++)
		{
			if (ix < bias_start || ix > bias_end)
			{
				ipix = (nx+bcols)*iy + ix;
				opix = nx*iy + oix;
				if (src[ipix] < minbias)
					dest[opix] = 0;
				else
					dest[opix] = src[ipix] - (int)minbias;
				oix++;
			}
		}
	}
	return(0);
}

void ApogeeAltaManager::checkStatus( const Apg::Status status )
{
	switch( status )
	{
	case Apg::Status_ConnectionError:
	{
		std::string errMsg("Status_ConnectionError");
		std::runtime_error except( errMsg );
		throw except;
	}
	break;

	case Apg::Status_DataError:
	{
		std::string errMsg("Status_DataError");
		std::runtime_error except( errMsg );
		throw except;
	}
	break;

	case Apg::Status_PatternError:
	{
		std::string errMsg("Status_PatternError");
		std::runtime_error except( errMsg );
		throw except;
	}
	break;

	case Apg::Status_Idle:
	{
		std::string errMsg("Status_Idle");
		std::runtime_error except( errMsg );
		throw except;
	}
	break;

	default:
		//no op on purpose
		break;
	}
}
