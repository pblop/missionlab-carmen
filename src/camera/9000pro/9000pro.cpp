/*
 *      webcam9000pro.cpp
 *      
 *      Copyright 2009 boss <jefe@jefe-desktop>
 *      
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *      
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *      
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#include <math.h>
#include <sys/select.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9000pro.h"

using namespace std;

/*
Format  Resolution  Maximum_frame_rate
MJPG    1600x1200   10 fps
        1600x1000   10 fps
        1600x904    10 fps
        1280x800    25 fps
        1280x720    30 fps
        960x720     30 fps
        864x480     30 fps
        800x600     30 fps
        800x504     30 fps
        800x456     30 fps
        768x480     30 fps
        640x480     30 fps
        640x400     30 fps
        640x360     30 fps
        352x288     30 fps
        320x240     30 fps
        176x144     30 fps
        160x120     5 fps

YUYV    1600x1200   5 fps 
        1600x1000   5 fps
        1600x904    5 fps   
        1280x800    7.5 fps
        1280x720    7.5 fps
        960x720     15 fps
        864x480     25 fps
        800x600     25 fps
        800x504     25 fps
        800x456     25 fps
        768x480     25 fps
        640x480     30 fps
        640x400     30 fps
        640x360     30 fps
        352x288     30 fps
        320x240     30 fps
        176x144     30 fps
        160x120     30 fps
*/

#define IMAGE_WIDTH  640
#define IMAGE_HEIGHT 480

Webcam9000Pro::Webcam9000Pro(const string &deviceName, 
                             const string &videoFormat)
{   
    // Copy the device name of the camera
    int nb = deviceName.size();
    dev_name = (char *)malloc(nb);
    memcpy(dev_name, deviceName.c_str(), nb);
    
    // Init pixel video format: MJPEG or YUYV
    if(videoFormat.compare("MJPEG") == 0)
    {
        pixel_format = V4L2_PIX_FMT_MJPEG;
        std::cerr << "FORMATO: MJPEG" << std::endl;
    }
    else
    {
        pixel_format = V4L2_PIX_FMT_YUYV;
        std::cerr << "FORMATO: YUYV" << std::endl;
    }
    
    // Init some variables
    buffers   = NULL;
    n_buffers = 0;
    continue_reading = true;
    
    // Webcam image size
    image_width = IMAGE_WIDTH;
    image_height = IMAGE_HEIGHT;
    
    counter = 0;
}


bool Webcam9000Pro::initialize(void)
{
     ///////////////////////////////////
    // CREATE A SHARED MEMORY SEGMENT
    ///////////////////////////////////
    
    // Construct a shared memory object
 /*   sharedMemory = new QSharedMemory("Webcam9000ProData", this);
    // Attach the process to the shared memory segment
    const int SHARED_MEMORY_SIZE = 10000000; // 10 MB, memory is cheap nowadays :)
    if(!sharedMemory->create(SHARED_MEMORY_SIZE, QSharedMemory::ReadWrite))
    {
        printf("Couldn't create shared memory segment. Error %d\n", sharedMemory->error());
        return false;
    }
    
    printf("Shared Memory segment created\n");
    sharedMemory->lock();
    int *memdata = (int *)sharedMemory->data();
    memdata[0] = image_width;
    memdata[1] = image_height;
    sharedMemory->unlock();*/
        
    ////////////////////////////
    // INIT V4L2 FOR CAPTURING
    ////////////////////////////
    if(!open_device())
        return false;
	
	if(!init_device())
		return false;
		
	if(!init_mmap())
		return false;
	
	if(!start_capturing())
		return false;
    
    return true;
}


bool Webcam9000Pro::uninitialize(void)
{
    if(!stop_capturing())
        return false;
        
    if(!uninit_device())
        return false;
        
    if(!close_device())
        return false;
        
    // Detach the shared memory segment. Otherwise it will survive forever
 /*   if(!sharedMemory->detach())
    {
        fprintf(stderr, "\nCouldn't detach shared memory segment. Error %d\n", sharedMemory->error());
        return false;
    }
     
    fprintf(stderr, "\nMemory detached successfully.\n");*/
        
    return true;
}


void Webcam9000Pro::run(void)
{
    fd_set fds;
    struct timeval tv;
    int r;
    
    // continue_reading is set from Server::handleSigInt, when 
    // the user close the server by means of ctrl+c
    do
    {
        FD_ZERO (&fds);
        FD_SET (fd, &fds);

        /* Timeout. */
        tv.tv_sec = 0;
        tv.tv_usec = 20000;

        r = select(fd + 1, &fds, NULL, NULL, &tv);
        
        if(r < 1)
            continue;
            
        read_frame();
    }
    while(continue_reading);
}


int Webcam9000Pro::xioctl(int request, void *arg)
{
        int r;

        do r = ioctl(fd, request, arg);
        while(-1 == r && EINTR == errno);

        return r;
}



bool Webcam9000Pro::open_device(void)
{
	struct stat st; 

	if(stat(dev_name, &st) == -1) 
	{
		fprintf(stderr, "Cannot identify '%s': %d, %s\n",
				 dev_name, errno, strerror(errno));
		return false;
	}

	if(!S_ISCHR(st.st_mode)) 
	{
		fprintf(stderr, "%s is no device\n", dev_name);
		return false;
	}

	fd = open(dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

	if(fd == -1) 
	{
		fprintf(stderr, "Cannot open '%s': %d, %s\n",
				dev_name, errno, strerror(errno));
		return false;
	}
	
	return true;
}



bool Webcam9000Pro::init_device(void)
{
	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;

	if(xioctl(VIDIOC_QUERYCAP, &cap) == -1)
	{
		if(errno == EINVAL)
		{
			fprintf(stderr, "%s is no V4L2 device\n", dev_name);
			return false;
		}
		else
		{
			fprintf(stderr, "%s error %d, %s\n", "VIDIOC_QUERYCAP",
			        errno, strerror (errno));
			return false;
		}
	}
	
	/*printf("Name of the driver: %s\n", cap.driver);
	printf("Name of the device: %s\n", cap.card);
	printf("Location of the device: %s\n", cap.bus_info);*/

	if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
	{
		fprintf(stderr, "%s is no video capture device\n", dev_name);
		return false;
	}

	////////////////////////////////////////////////////////////
	// Check if the device supports the STREAMING I/O method
	////////////////////////////////////////////////////////////
	if(!(cap.capabilities & V4L2_CAP_STREAMING))
	{
		fprintf(stderr, "%s does not support streaming i/o\n", dev_name);
		return false;
	}

	/* Select video input, video standard and tune here. */

	CLEAR(cropcap);

	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if(xioctl(VIDIOC_CROPCAP, &cropcap) == 0) 
	{
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		crop.c = cropcap.defrect; /* reset to default */

		if(xioctl(VIDIOC_S_CROP, &crop) == -1) 
		{
			switch(errno) 
			{
				case EINVAL:
					/* Cropping not supported. */
					break;
				default:
					/* Errors ignored. */
					break;
			}
		}
	}
	else 
	{        
		/* Errors ignored. */
	}

	CLEAR(fmt);

	fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = image_width;
	fmt.fmt.pix.height      = image_height;
    fmt.fmt.pix.pixelformat = pixel_format;
	fmt.fmt.pix.field       = V4L2_FIELD_NONE;

	if(xioctl(VIDIOC_S_FMT, &fmt) == -1)
	{
		fprintf(stderr, "%s error %d, %s\n", "VIDIOC_S_FMT",
			        errno, strerror (errno));
		return false;
	}

	/* Note VIDIOC_S_FMT may change width and height. */

	/* Buggy driver paranoia. */
	/*unsigned int min;
	min = fmt.fmt.pix.width * 2;
	if(fmt.fmt.pix.bytesperline < min)
	{
		printf("Error1. Width = %d. Bytes_per_line %d, but should be %d\n", 
				fmt.fmt.pix.width, fmt.fmt.pix.bytesperline, min);
		fmt.fmt.pix.bytesperline = min;
	}
	
	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if(fmt.fmt.pix.sizeimage < min)
	{
		printf("Error2\n");
		fmt.fmt.pix.sizeimage = min;
	}*/
	
	return true;
}



bool Webcam9000Pro::init_mmap(void)
{
	struct v4l2_requestbuffers req;

	// Allocate device buffers
	CLEAR (req);

	//Sólo nos interesa la última imagen
	req.count  = 1; //4;  // Number of buffers requested
	req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE; // Buffer of a video capture stream
	req.memory = V4L2_MEMORY_MMAP;            // The buffer is used for memory mapping I/O

	// Call the VIDIOC_REQBUFS ioctl to initiate memory mapping
	if(xioctl(VIDIOC_REQBUFS, &req) == -1) 
	{
		if(errno == EINVAL) 
		{
			fprintf(stderr, "%s does not support memory mapping\n", dev_name);
			return false;
		}
		else 
		{
			fprintf(stderr, "%s error %d, %s\n", "VIDIOC_REQBUFS",
			        errno, strerror (errno));
			return false;
		}
	}
	
	// Check the number of buffers actually allocated by the driver
	/*if(req.count < 2)
	{
		fprintf (stderr, "Insufficient buffer memory on %s\n", dev_name);
		return false;
	}*/

	// Allocate memory for the buffers 
	buffers = (struct buffer *)calloc(req.count, sizeof(*buffers));
	if(!buffers)
	{
		fprintf (stderr, "Out of memory\n");
		return false;
	}

	for(n_buffers = 0; n_buffers < req.count; ++n_buffers) 
	{
		struct v4l2_buffer buf;

		CLEAR(buf);

		buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index  = n_buffers;

		if(xioctl(VIDIOC_QUERYBUF, &buf) == -1)
		{
			fprintf(stderr, "%s error %d, %s\n", "VIDIOC_QUERYBUF",
			        errno, strerror (errno));
			return false;
		}

		buffers[n_buffers].length = buf.length;
		buffers[n_buffers].start = mmap (NULL /* start anywhere */,
										 buf.length,
										 PROT_READ | PROT_WRITE /* required */,
										 MAP_SHARED /* recommended */,
										 fd,
										 buf.m.offset);

		if(buffers[n_buffers].start == MAP_FAILED)
		{
			fprintf(stderr, "%s error %d, %s\n", "init_mmap",
			        errno, strerror (errno));
			return false;
		}
	}
	
	return true;
}


bool Webcam9000Pro::start_capturing(void)
{
	unsigned int i;
	enum v4l2_buf_type type;

	for(i = 0; i < n_buffers; ++i) 
	{
		struct v4l2_buffer buf;

		CLEAR(buf);

		buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index  = i;

		if(xioctl(VIDIOC_QBUF, &buf) == -1)
		{
			fprintf(stderr, "%s error %d, %s\n", "VIDIOC_QBUF",
			        errno, strerror (errno));
			return false;
		}
	}

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if(xioctl(VIDIOC_STREAMON, &type) == -1)
	{
		fprintf(stderr, "%s error %d, %s\n", "VIDIOC_STREAMON",
			        errno, strerror (errno));
		return false;
	}
	
	return true;
}



bool Webcam9000Pro::stop_capturing(void)
{
	enum v4l2_buf_type type;

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if(xioctl(VIDIOC_STREAMOFF, &type) == -1)
	{
		fprintf(stderr, "%s error %d, %s\n", "VIDIOC_STREAMOFF",
			        errno, strerror (errno));
		return false;
	}
	
	return true;
}


bool Webcam9000Pro::uninit_device(void)
{
	unsigned int i;

	for (i = 0; i < n_buffers; ++i)
	{
		if (-1 == munmap (buffers[i].start, buffers[i].length))
		{
			fprintf(stderr, "%s error %d, %s\n", "munmap",
    		        errno, strerror (errno));
			return false;
		}
	}

	free(buffers);
	return true;
}



bool Webcam9000Pro::close_device(void)
{
	if(close(fd) == -1)
	{
		fprintf(stderr, "%s error %d, %s\n", "close",
    		        errno, strerror (errno));
		return false;
	}

	fd = -1;
	
	return true;
}

bool Webcam9000Pro::read_frame(unsigned char *frame_data)
{
	struct v4l2_buffer buf;
    	
	int r;
	fd_set fds;
    	struct timeval tv;
	do{
      		FD_ZERO (&fds);
        	FD_SET (fd, &fds);
        	tv.tv_sec = 5;
        	tv.tv_usec = 0;
        	r = select(fd + 1, &fds, NULL, NULL, &tv);
	}while(r<1);



	CLEAR (buf);

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	if(xioctl(VIDIOC_DQBUF, &buf) == -1) 
	{
		switch (errno) 
		{
			case EAGAIN:
				//return false;

			case EIO:
				// Could ignore EIO, see spec. 

				// fall through 

			default:
				fprintf(stderr, "%s error %d, %s\n", "VIDIOC_DQBUF",
						errno, strerror (errno));
				return false;
		}
	}

	if(!(buf.index < n_buffers))
	{
		printf("Bufindex %d < n_buffers %d\n", buf.index, n_buffers);
		return false;
	}

    // Update the buffer pointer and buffer size
    current_buffer = (unsigned char *)buffers[buf.index].start;
    cb_size        = buf.bytesused;

    // Compute frame size
    int frame_size;
    if(pixel_format == V4L2_PIX_FMT_YUYV)
    {
        // We just save the luminance (Y) part of YUYV
         frame_size = buf.bytesused/2;
    }
    if(pixel_format == V4L2_PIX_FMT_MJPEG)
    {
        // We need to add the huffman header
        frame_size = buf.bytesused + sizeof(dht_data);
    }
    
    unsigned char *c = (unsigned char *)buffers[buf.index].start;
    
    if(pixel_format == V4L2_PIX_FMT_YUYV)
    {
        // We save just the even pixels (Y):
        // Y0 Cb0 Y1 Cr0 Y2 Cb1 Y3 Cr1
        for(unsigned int i = 0, j = 0; i < buf.bytesused; i+=2, j++)
            frame_data[j]  = c[i];
    }
    else if(pixel_format == V4L2_PIX_FMT_MJPEG)
    {
        const unsigned int HEADERFRAME = 0xAF;
        for(unsigned int i = 0; i < buf.bytesused; i++)
        {
            if(i < HEADERFRAME)
            {
                //frame[i] = c[i]; 
                frame_data[i]  = c[i];
            }
            else
            {
                //frame[sizeof(dht_data)+i] = c[i];
                frame_data[sizeof(dht_data) + i] = c[i];
            }
        }
            
        for(unsigned int i = 0; i < sizeof(dht_data); i++)
        {
            //frame[HEADERFRAME+i] = dht_data[i];
            frame_data[HEADERFRAME + i] = dht_data[i];
        }
    }
   


   //printf("%d. Frame size = %d\n",counter, frame_size);counter++;
	
    // Dequeue video buffers
	if(xioctl(VIDIOC_QBUF, &buf) == -1)
	{
		fprintf(stderr, "%s error %d, %s\n", "VIDIOC_QBUF",
						errno, strerror (errno));
		return false;
	}

	return true;

}

bool Webcam9000Pro::read_frame(void)
{
	struct v4l2_buffer buf;

	CLEAR (buf);

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	if(xioctl(VIDIOC_DQBUF, &buf) == -1) 
	{
		switch (errno) 
		{
			case EAGAIN:
				//return false;

			case EIO:
				// Could ignore EIO, see spec. 

				// fall through 

			default:
				fprintf(stderr, "%s error %d, %s\n", "VIDIOC_DQBUF",
						errno, strerror (errno));
				return false;
		}
	}

	if(!(buf.index < n_buffers))
	{
		printf("Bufindex %d < n_buffers %d\n", buf.index, n_buffers);
		return false;
	}

    // Update the buffer pointer and buffer size
    current_buffer = (unsigned char *)buffers[buf.index].start;
    cb_size        = buf.bytesused;

    // Compute frame size
    int frame_size;
    if(pixel_format == V4L2_PIX_FMT_YUYV)
    {
        // We just save the luminance (Y) part of YUYV
         frame_size = buf.bytesused/2;
    }
    if(pixel_format == V4L2_PIX_FMT_MJPEG)
    {
        // We need to add the huffman header
        frame_size = buf.bytesused + sizeof(dht_data);
    }
    
    unsigned char *c = (unsigned char *)buffers[buf.index].start;
    
    //////////////////////////////////////////////
    // COPY FRAME SIZE AND DATA TO SHARED MEMORY
    //////////////////////////////////////////////
    /**//*sharedMemory->lock();
    
    // sharedMemory->data
    // 4 bytes : image_width (int)
    // 4 bytes : image_height (int)
    // 4 bytes : frame_size (int)
    // frame_size bytes : frame_data (unsigned char)
    int *intdata = (int *)sharedMemory->data();
    intdata[2]  = frame_size;
    
    unsigned char *frame_data = (unsigned char *)sharedMemory->data();
    int fd_offset = sizeof(int)*3; //frame data offset 
    
    if(pixel_format == V4L2_PIX_FMT_YUYV)
    {
        // We save just the even pixels (Y):
        // Y0 Cb0 Y1 Cr0 Y2 Cb1 Y3 Cr1
        for(unsigned int i = 0, j = 0; i < buf.bytesused; i+=2, j++)
            frame_data[fd_offset+j]  = c[i];
    }
    else if(pixel_format == V4L2_PIX_FMT_MJPEG)
    {
        const unsigned int HEADERFRAME = 0xAF;
        for(unsigned int i = 0; i < buf.bytesused; i++)
        {
            if(i < HEADERFRAME)
            {
                //frame[i] = c[i]; 
                frame_data[i + fd_offset]  = c[i];
            }
            else
            {
                //frame[sizeof(dht_data)+i] = c[i];
                frame_data[sizeof(dht_data) + i + fd_offset] = c[i];
            }
        }
            
        for(unsigned int i = 0; i < sizeof(dht_data); i++)
        {
            //frame[HEADERFRAME+i] = dht_data[i];
            frame_data[HEADERFRAME + i + fd_offset] = dht_data[i];
        }
    }
    
    sharedMemory->unlock();*/
    
    //printf("%d. Frame size = %d\n",counter, frame_size);counter++;
	
    // Dequeue video buffers
	if(xioctl(VIDIOC_QBUF, &buf) == -1)
	{
		fprintf(stderr, "%s error %d, %s\n", "VIDIOC_QBUF",
						errno, strerror (errno));
		return false;
	}

	return true;
}


/*bool Webcam9000Pro::read_frame(unsigned char **frame, int *frame_size)
{
	struct v4l2_buffer buf;

	CLEAR (buf);

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	if(xioctl(VIDIOC_DQBUF, &buf) == -1) 
	{
		switch (errno) 
		{
			case EAGAIN:
				//return false;

			case EIO:
				// Could ignore EIO, see spec. 

				// fall through 

			default:
				fprintf(stderr, "%s error %d, %s\n", "VIDIOC_DQBUF",
						errno, strerror (errno));
				return false;
		}
	}

	if(!(buf.index < n_buffers))
	{
		printf("Bufindex %d < n_buffers %d\n", buf.index, n_buffers);
		return false;
	}

	//int sizemibufer = buf.bytesused+sizeof(dht_data);
	//unsigned char *mibufer = calloc(sizemibufer, 1);
	//memcpy_picture(mibufer, buffers[buf.index].start, buf.bytesused);
	FILE *file;
	file = fopen ("archivo.jpg", "wb");
	//fwrite (buffers[buf.index].start, buf.bytesused, 1, file);
	fwrite(mibufer, sizemibufer, 1, file);
	fclose(file);

	*frame_size = buf.bytesused+sizeof(dht_data);
	*frame = (unsigned char *)calloc(*frame_size, 1);
	memcpy_picture((unsigned char *)*frame, 
				   (unsigned char *)buffers[buf.index].start,
				   buf.bytesused);
    
	
	if(xioctl(VIDIOC_QBUF, &buf) == -1)
	{
		fprintf(stderr, "%s error %d, %s\n", "VIDIOC_QBUF",
						errno, strerror (errno));
		return false;
	}

	return true;
}
*/
