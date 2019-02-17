/*
 *  RPLIDAR
 *  Ultra Simple Data Grabber Demo App
 *
 *  Copyright (c) 2009 - 2014 RoboPeak Team
 *  http://www.robopeak.com
 *  Copyright (c) 2014 - 2018 Shanghai Slamtec Co., Ltd.
 *  http://www.slamtec.com
 *
 */
/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <cmath> 

//includes for udp
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <string> 
#include <iostream>
#include <cstring>
#define PORT 7777 //IMPORTANT


#include <sstream>

template <typename T>
std::string to_string(T value)
{
    std::ostringstream os ;
    os << value ;
    return os.str() ;
}

////////////////////////////////////////////

#include "rplidar.h" //RPLIDAR standard sdk, all-in-one header

#ifndef _countof
#define _countof(_Array) (int)(sizeof(_Array) / sizeof(_Array[0]))
#endif

#ifdef _WIN32
#include <Windows.h>
#define delay(x)   ::Sleep(x)
#else
#include <unistd.h>
static inline void delay(_word_size_t ms){
    while (ms>=1000){
        usleep(1000*1000);
        ms-=1000;
    };
    if (ms!=0)
        usleep(ms*1000);
}
#endif

using namespace rp::standalone::rplidar;

bool checkRPLIDARHealth(RPlidarDriver * drv)
{
    u_result     op_result;
    rplidar_response_device_health_t healthinfo;


    op_result = drv->getHealth(healthinfo);
    if (IS_OK(op_result)) { // the macro IS_OK is the preperred way to judge whether the operation is succeed.
        printf("Staring Heidi's version 1\n"); 
        printf("RPLidar health status : %d\n", healthinfo.status);
        if (healthinfo.status == RPLIDAR_STATUS_ERROR) {
            fprintf(stderr, "Error, rplidar internal error detected. Please reboot the device to retry.\n");
            // enable the following code if you want rplidar to be reboot by software
            // drv->reset();
            return false;
        } else {
            return true;
        }

    } else {
        fprintf(stderr, "Error, cannot retrieve the lidar health code: %x\n", op_result);
        return false;
    }
}

#include <signal.h>
bool ctrl_c_pressed;
void ctrlc(int)
{
    ctrl_c_pressed = true;
}

int main(int argc, const char * argv[]) {
    const char * opt_com_path = NULL;
    _u32         baudrateArray[2] = {115200, 256000};
    _u32         opt_com_baudrate = 0;
    u_result     op_result;

    //-------Jerry's code
    //additional variables for our own purposes


    float myAngle;
    float myDistance;
    float expDistance;

    float frontDistance;
    float rightDistance;
    float leftDistance;

    //variables for changing dimension 
    float sidelength = 2150.0;
    float shortsidelength = 1200;

    /////////////////////////////////////////
    //UDP code initialization


    struct sockaddr_in address; 
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
     
    //string msg_str;
    char buffer[1024] = {0}; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
   
    memset(&serv_addr, '0', sizeof(serv_addr)); 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "192.168.8.101", &serv_addr.sin_addr)<=0) //IP address -> change to localhost 
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 


    //----the end of Jerry's code
    ////////////////////////////////////////////

    bool useArgcBaudrate = false;

    printf("starting Heidi's version 1\n");
    printf("Ultra simple LIDAR data grabber for RPLIDAR.\n"
           "Version: "RPLIDAR_SDK_VERSION"\n");


    // read serial port from the command line...
    if (argc>1) opt_com_path = argv[1]; // or set to a fixed value: e.g. "com3" 

    // read baud rate from the command line if specified...
    if (argc>2)
    {
        opt_com_baudrate = strtoul(argv[2], NULL, 10);
        useArgcBaudrate = true;
    }

    if (!opt_com_path) {
#ifdef _WIN32
        // use default com port
        opt_com_path = "\\\\.\\com3";
#else
        opt_com_path = "/dev/ttyUSB0";
#endif
    }

    // create the driver instance
	RPlidarDriver * drv = RPlidarDriver::CreateDriver(DRIVER_TYPE_SERIALPORT);
    if (!drv) {
        fprintf(stderr, "insufficent memory, exit\n");
        exit(-2);
    }
    
    rplidar_response_device_info_t devinfo;
    bool connectSuccess = false;
    // make connection...
    if(useArgcBaudrate)
    {
        if(!drv)
            drv = RPlidarDriver::CreateDriver(DRIVER_TYPE_SERIALPORT);
        if (IS_OK(drv->connect(opt_com_path, opt_com_baudrate)))
        {
            op_result = drv->getDeviceInfo(devinfo);

            if (IS_OK(op_result)) 
            {
                connectSuccess = true;
            }
            else
            {
                delete drv;
                drv = NULL;
            }
        }
    }
    else
    {
        size_t baudRateArraySize = (sizeof(baudrateArray))/ (sizeof(baudrateArray[0]));
        for(size_t i = 0; i < baudRateArraySize; ++i)
        {
            if(!drv)
                drv = RPlidarDriver::CreateDriver(DRIVER_TYPE_SERIALPORT);
            if(IS_OK(drv->connect(opt_com_path, baudrateArray[i])))
            {
                op_result = drv->getDeviceInfo(devinfo);

                if (IS_OK(op_result)) 
                {
                    connectSuccess = true;
                    break;
                }
                else
                {
                    delete drv;
                    drv = NULL;
                }
            }
        }
    }
    if (!connectSuccess) {
        
        fprintf(stderr, "Error, cannot bind to the specified serial port %s.\n"
            , opt_com_path);
        goto on_finished;
    }

    // print out the device serial number, firmware and hardware version number..
    printf("RPLIDAR S/N: ");
    for (int pos = 0; pos < 16 ;++pos) {
        printf("%02X", devinfo.serialnum[pos]);
    }

    printf("\n"
            "Firmware Ver: %d.%02d\n"
            "Hardware Rev: %d\n"
            , devinfo.firmware_version>>8
            , devinfo.firmware_version & 0xFF
            , (int)devinfo.hardware_version);



    // check health...
    if (!checkRPLIDARHealth(drv)) {
        goto on_finished;
    }

    signal(SIGINT, ctrlc);
    
    drv->startMotor();
    // start scan...
    drv->startScan(0,1);

    // fetech result and print it out...
    //one revolution
    while (1) {
        rplidar_response_measurement_node_t nodes[8192];
        size_t   count = _countof(nodes);

        op_result = drv->grabScanData(nodes, count);

        if (IS_OK(op_result)) {
            drv->ascendScanData(nodes, count);
            printf("\n\nstarting scan\n\n=====\n");

            std::string msg_str = "";

            //Jerry's code

            /*  first, check flag, 
                if false, 
                    then scan the scene and store the data in an 2D array
                    turn flag into false
                second, check flag, 
                if true, 
                    keep while() loop on scanning the scene
                    check every angle,    
                    

            */


            //660 
            // 2 or 3 circles
            for (int pos = 0; pos < (int)count ; ++pos) {

		        myAngle = (nodes[pos].angle_q6_checkbit >> RPLIDAR_RESP_MEASUREMENT_ANGLE_SHIFT)/64.0f;
                myDistance = nodes[pos].distance_q2/4.0f;

                // //front
                // if(myAngle >= 0.0 && myAngle <= 90.0){
                // 	frontDistance = sidelength/cos(myAngle * M_PI/180.0); //convert from angles to radians		
                // }
                // else if(myAngle >= 270.0 && myAngle <= 360.0){
                //     frontDistance = sidelength/cos((360.0-myAngle) * M_PI/180.0);
                // }
                // else{ frontDistance = 999999;}

                // //right
                // if(myAngle >= 0.0 && myAngle <= 90.0){
                //     rightDistance = (shortsidelength)/cos((90.0 - myAngle) * M_PI/180.0);
                // }
                // else{ rightDistance = 999999;}

                // //left
                // if(myAngle >= 270 && myAngle <= 360.0){
                // 	leftDistance = (shortsidelength)/cos((90.0-(360.0-myAngle))*M_PI/180.0);		
                // }
                // else{ leftDistance = 999999;}
                
                // //get smallest
                // if( frontDistance < rightDistance){
                //     expDistance = frontDistance;
                // }
                // else{ expDistance = rightDistance;}
                // if( leftDistance < expDistance){ 
                //     expDistance = leftDistance;
                // }

                //expDistance -> reference array

                if( (myAngle >= 0.0 && myAngle <= 85.0) || (myAngle >= 275 && myAngle <= 360)){
                              
                	if( abs(expDistance - myDistance) > 250.0){ //mm threshold
                	   // printf("object detected at angle %03.2f and distance %f\n", myAngle, myDistance);
                	   // printf("expected distance at angle %03.2f is %f mm\n", myAngle, expDistance);
                	   // printf("actual distance at angle %03.2f is %f mm\n", myAngle, myDistance);
                	   // printf("Front Distance %03.2f, left distance, %03.2f, right distance %03.2f \n", frontDistance, leftDistance, rightDistance);
                       printf("--------------------------------\n"); 

                       msg_str += to_string(myAngle);
                       msg_str += ",";
                       msg_str += to_string(myDistance);
                       msg_str += "\n";
                    }               

		        }
            }

            char msg[msg_str.length() + 1];
            strcpy(msg, msg_str.c_str());
            //array
            send(sock , msg , strlen(msg) , 0 ); 

        }

        if (ctrl_c_pressed){ 
            break;
        }
    }

    drv->stop();
    drv->stopMotor();
    // done!
on_finished:
    RPlidarDriver::DisposeDriver(drv);
    drv = NULL;
    return 0;
}

