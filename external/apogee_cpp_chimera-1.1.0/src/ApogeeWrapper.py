#!/usr/bin/python

from ctypes import cdll
import ctypes as c
lib = cdll.LoadLibrary('./libapogee_chimera.so')

class ApogeeManager(object):
    def __init__(self, filename):
	    # ApogeeAltaManager* NewApogeeAltaManager(char* image_name, double time_exposure, 
	    # 	int shutter, int xbin, int ybin, int xstart, int xend, int ystart, int yend)
		#
		# image_name - path of file
		# time_exposure - exposure time is seconds 
		# shutter - 1 = Shutter open, 0 = Shutter closed
		# xbin - Horizontal binning - Binning factor in x, default 1
		# xbin - Vertical binning - Binning factor in y, default 1
		# xstart - Region of interest - Image subregion in the format startx,starty,endx,endy
		# xend - Region of interest - Image subregion in the format startx,starty,endx,endy
		# ystart - Region of interest - Image subregion in the format startx,starty,endx,endy
		# yend - Region of interest - Image subregion in the format startx,starty,endx,endy

        self.obj = lib.NewApogeeAltaManager(c.c_char_p(filename), c.c_double(3.0), 0, 1, 1, 0, 0, 0, 0)

    def setUp(self):
        lib.setUp(self.obj)

    def run(self):
        lib.run(self.obj)

    def stop(self):
        lib.stop(self.obj)               