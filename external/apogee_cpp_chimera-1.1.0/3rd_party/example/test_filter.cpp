/*  Program : test_filter.cpp
 *  Version : 1.0
 *  Author  : Dave Mills
 *  Copyright : The Random Factory 2004-2008
 *  License : GPL
 *
 *
 *  This program provides a limited test environment for Apogee ALTA 
 *  filter wheels. It utilises the same low-level API provided by the
 *  filter_USB.so and filter_NET.so shared object libraries.
 * 
 *
 *  To build for ALTA-U
 *
 *      g++ -I/opt/apogee/include -I./ApogeeUsb -DLINUX -c ApnFilterWheel.cpp
 *      g++ -I/opt/apogee/include  -c test_filter.cpp -IFpgaRegs -o test_filteru \
 *                       ApnFilterWheel.o \
 *                       ApogeeUsbLinux.o \
 *                       /opt/apogee/lib/libusb.so
 *
 *
 *  The program is controlled by a set of command line options
 *  Usage information is obtained by invoking the program with -h
 *
 *  eg   ./alta_filteru -h
 *
 *
 *  Caveats : There is limited error checking on the input options, if you 
 * 	      hang the onboard software, simply power cycle the 
 * 	      unit.
 */

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <stdint.h>
#include <stdexcept>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#ifdef APOGEE_ALTA
#include "ApogeeFilterWheel.h"
#endif
#ifdef APOGEE_ASCENT
#include "Ascent.h"
#endif
#include "FindDeviceUsb.h"
#include "CameraInfo.h"
#include "apgSampleCmn.h"



int parse_options (int argc, char **argv);

/* Declare globals to store the input command line options */

int ip[4]={0,0,0,0};
int verbose=0;
int unitnum=1;
#ifdef APOGEE_ALTA
ApogeeFilterWheel::Type wheeltype = ApogeeFilterWheel::FW50_7S;;
#else
Ascent::FilterWheelType wheeltype = Ascent::FW_UNKNOWN_TYPE;
#endif
unsigned long maxpositions=0;
unsigned long position=0;
unsigned long requested=0;

/* Main executable starts here -----------------------------------------------*/

int main (int argc, char **argv) 
{

	int status;
        unsigned int ipaddr;
        int move;

/*	Obtain user provided options */
	status = parse_options(argc,argv);

#ifdef APOGEE_ALTA
        ApogeeFilterWheel::Type ftype;
/*	Create the camera object , this will reserve memory */
	FindDeviceUsb look4FilterWheel;
        std::string msg = look4FilterWheel.Find();
	if( !apgSampleCmn::IsDeviceFilterWheel( msg ) )
		{
			std::string errMsg = "Device not a filter wheel = " + msg;
			std::runtime_error except( errMsg );
			throw except;
		}
		
	std::cout << "USB filter wheel found" << std::endl;
	std::string addr = apgSampleCmn::GetUsbAddress( msg );
	ApogeeFilterWheel filterWheel;
	filterWheel.Init(wheeltype,"1");
	maxpositions = filterWheel.GetMaxPositions();
	position = filterWheel.GetPosition();
#endif

#ifdef APOGEE_ASCENT
        std::string ioInterface("usb");
        FindDeviceUsb look4cam;
        std::string msg = look4cam.Find();
	std::cout << msg << std::endl;
                
        std::string addr = apgSampleCmn::GetUsbAddress( msg );
       	uint16_t id = apgSampleCmn::GetID( msg );
	uint16_t frmwrRev = apgSampleCmn::GetFrmwrRev( msg );
        
	Ascent filterWheel;
	filterWheel.OpenConnection( ioInterface, addr, frmwrRev, id );
	filterWheel.FilterWheelOpen(wheeltype);
	maxpositions = filterWheel.GetFilterWheelMaxPositions();
	position = filterWheel.GetFilterWheelPos();
#endif



	printf ("Max number of filters = %ld\n",maxpositions);
	printf ("Current position = %ld\n",position);

	if (requested > 0) {
#ifdef APOGEE_ALTA
	   filterWheel.SetPosition(requested);
#endif
#ifdef APOGEE_ASCENT
	   filterWheel.SetFilterWheelPos(requested);
#endif
           move = requested-position;
           if (move < 0) {move=move+maxpositions;}
           sleep(move);
	}

/*	 All done, tidy up */
#ifdef APOGEE_ALTA
	 filterWheel.Close();
#endif
#ifdef APOGEE_ASCENT
	 filterWheel.FilterWheelClose();
#endif

}

/*  Helper routines start here-------------------------------------------------*/

/*  This routine provides a very simple command line parser
 *  Unknown options should be ignored, but strict type
 *  checking is NOT done.
 */

int parse_options (int argc, char **argv)
{
   int i;
   int goti,gott,gots,gota;

/* Zero out counters for required options */
   goti=0;
   gott=0;
   gots=0;
   gota=0;
   i=1;


/* Loop thru all provided options */
   while (i<argc) {


/*     Unit type */
       if (!strncmp(argv[i],"-t",2)) {
          sscanf(argv[i+1],"%d",&wheeltype);
          gott = 1;
       }

/*     Position */
       if (!strncmp(argv[i],"-p",2)) {
          sscanf(argv[i+1],"%ld",&requested);
       }

/*     IP address for ALTA-E models */
       if (!strncmp(argv[i],"-a",2)) {
          sscanf(argv[i+1],"%d.%d.%d.%d",ip,ip+1,ip+2,ip+3);
          gota = 1;
       }


/*     USB camera number */
       if (!strncmp(argv[i],"-u",2)) {
          sscanf(argv[i+1],"%d",&unitnum);
       }


/*     Be more verbose */
       if (!strncmp(argv[i],"-v",2)) {
          sscanf(argv[i+1],"%d",&verbose);
       }

/*     Print usage info */
       if (!strncmp(argv[i],"-h",2)) {
          printf("Apogee filter wheel tester -  Usage: \n \
	 -t num          Type of unit\n \n");
#ifdef APOGEE_ALTA
          printf("            FW50_9R = 1, \
            FW50_7S = 2, \
            AFW50_10S = 6 \
	 -a a.b.c.d      IP address of unit e.g. 192.168.0.1 (required for ethernet models only)\n");
#endif
#ifdef APOGEE_ASCENT
          printf("            CFW25_6R	= 7, \
            CFW31_8R	= 8 \n");
#endif
          printf("	 -u num          Unit number (default 1 , USB only) \n \
	 -p position     Filter position to select \n \
	 -v verbosity    Print more details\n");
         exit(0);
        }

/*      All options are 2 args long! */
        i = i+2;
   } 

/* Complain about missing required options, then give up */
   if ( gott == 0 ) printf("Missing argument  -t unit type, try -h for help\n");
#ifdef ALTA_NET
   if ( gota == 0 ) printf("Missing argument  -a IP address, try -h for help\n");
   if ( gota == 0 ) exit(1);
#else
   if (gott != 1) exit(1);
#endif

/* Print exposure details */
   if (verbose > 0) {
      printf("Apogee Filter Wheel test - V1.0\n");
#ifdef ALTA_NET
      if (ip[0]     != 0) printf("	ALTA-E ip address is %d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
#endif
      if (requested > 0) printf("	Move to position %ld\n",requested);
   }
   return(0);

}

