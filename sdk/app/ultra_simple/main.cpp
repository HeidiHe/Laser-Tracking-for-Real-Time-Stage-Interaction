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

    float refArray[600][2]; //2D array, 570 elements, then 0-> angle, 1->distance
    int checkcounter = 0;

    bool checked = false;//flag, if has reference array, then 1

    // float frontDistance;
    // float rightDistance;
    // float leftDistance;

    //variables for changing dimension 
    float sidelength = 2150.0;
    float shortsidelength = 1200;

    /////////////////////////////////////////
    //UDP code initialization


    struct sockaddr_in address; 
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    
    printf("debug 1, line 141\n");
    //string msg_str;
    char buffer[1024] = {0}; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 

    memset(&serv_addr, '0', sizeof(serv_addr)); 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 
    
    printf("debug 1.2, line 154\n");

    // Convert IPv4 and IPv6 addresses from text to binary form 
    //137.146.126.135
    if(inet_pton(AF_INET, "137.146.126.135", &serv_addr.sin_addr)<=0) //IP address -> change to localhost 
    // if(inet_pton(AF_INET, "192.168.8.101", &serv_addr.sin_addr)<=0) //IP address -> change to localhost 
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 

    printf("debug 1.3, line 163\n");

   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("debug 1.4, line 163\n");

        printf("\nConnection Failed \n"); 
        return -1; 
    } 
    printf("debug 2, line 167\n");


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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // fetech result and print it out...
    //one revolution
    while (1) {
        rplidar_response_measurement_node_t nodes[8192];
        size_t   count = _countof(nodes);

        op_result = drv->grabScanData(nodes, count);

        if (IS_OK(op_result)) {
            drv->ascendScanData(nodes, count);
            // printf("\n\nstarting scan\n\n=====\n");

            std::string msg_str = "";


            /*  first, check flag, 
                if false, 
                    then scan the scene and store the data in an 2D array
                        array: approx 570 depth
                    repeat three times and get an average, turn flag into false
                second, check flag, 
                if true, 
                    keep while() loop on scanning the scene
                        check every angle, 
                            compare reference array with current angle
                            if distance changed
                                record angle
            check angle:
                method 1: check array list index. cons: small deviation - within half a degree
                method 2: average each angle. 0-180, find index, if distance not zero -> add and average

                final -> method 1
            */
            //check and build up reference array
            if(!checked){
                checkcounter++;
                //loop and calculate average
                // float refArray[700][2]; //2D array, 570 elements, then 0-> angle, 1->distance
                // build up reference array 
                for (int pos = 0; pos < (int)count ; ++pos) {
                    float curA = (nodes[pos].angle_q6_checkbit >> RPLIDAR_RESP_MEASUREMENT_ANGLE_SHIFT)/64.0f; // my angle
                    if(fabs(curA)>360 && pos>0){
                        curA = refArray[pos-1][0];
                        printf("angle data out of boundadry\n");
                    }
                    float curD = nodes[pos].distance_q2/4.0f; // my distance 
                    if(curD > 12000 && pos>0){
                        curD = refArray[pos-1][1];
                        curA = refArray[pos-1][0];
                        printf("distance data out of boundadry\n");
                    }
                    refArray[pos][0] += curA;
                    refArray[pos][1] += curD;
                }
                //print the array in the first loop
                if(checkcounter==1){
                    printf("first round///////////////////////////////////////\n");
                    //average each node and print reference array
                    for(int i=0; i<700; i++)    //This loops on the rows.
                    {
                        for(int j=0; j<2; j++) //This loops on the columns
                        {
                            printf("%f  ", refArray[i][j] );
                        }
                        printf("\n");
                    }
                }

                if(checkcounter == 200){
                    //average each node and print reference array
                    printf("final round/////////////////////////////////////\n");
                    for(int i=0; i<700; i++)    //This loops on the rows.
                    {
                        for(int j=0; j<2; j++) //This loops on the columns
                        {
                            refArray[i][j] = refArray[i][j]/200;
                            printf("%f  ", refArray[i][j] );
                        }
                        printf("\n");
                    }
                    checked = true;
                    printf("finished reference Arr\n\n");
                }
 
                
            }

            else{

                 //660 
                for (int pos = 0; pos < (int)count ; ++pos) {

                    myAngle = (nodes[pos].angle_q6_checkbit >> RPLIDAR_RESP_MEASUREMENT_ANGLE_SHIFT)/64.0f; // my angle
                    if(abs(myAngle)>360){
                        printf("anlge out of boundary\n");
                        myAngle = refArray[pos][0];
                    }
                    myDistance = nodes[pos].distance_q2/4.0f; // my distance 
                    if(myDistance > 12000){
                        myDistance = refArray[pos][1];
                        myAngle = refArray[pos][0];
                    }

                    // myAngle = (nodes[pos].angle_q6_checkbit >> RPLIDAR_RESP_MEASUREMENT_ANGLE_SHIFT)/64.0f;
                    // myDistance = nodes[pos].distance_q2/4.0f;

                    expDistance = refArray[pos][1];

                    //compare the distance
                    // if( (myAngle >= 0.0 && myAngle <= 175.0) || (myAngle >= 275 && myAngle <= 360)){
                    if(myAngle >= 5.0 && myAngle <= 175.0){
                                  
                        //mm threshold, abs stands for absolute threshold
                        if( fabs(expDistance - myDistance) > 3000.0){ 
                           // printf("object detected at angle %03.2f and distance %f\n", myAngle, myDistance);
                           // printf("expected distance at angle %03.2f is %f mm\n", myAngle, expDistance);
                           // printf("actual distance at angle %03.2f is %f mm\n", myAngle, myDistance);
                           // printf("Front Distance %03.2f, left distance, %03.2f, right distance %03.2f \n", frontDistance, leftDistance, rightDistance);
                           // printf("--------------------------------\n"); 


                            msg_str += to_string(myAngle);
                            msg_str += ",";
                            msg_str += to_string(myDistance);
                            msg_str += ",";
                            // msg_str += "\n";
                        }

                    }
                }

                // printf("sending data debug 4\n");
                char msg[msg_str.length() + 1];
                strcpy(msg, msg_str.c_str());
                //array
                send(sock , msg , strlen(msg) , 0 ); 
            }
            



        }

        if (ctrl_c_pressed){ 
            break;
        }
    }

    drv->stop();
    drv->stopMotor();
    // done!
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
on_finished:
    RPlidarDriver::DisposeDriver(drv);
    drv = NULL;
    return 0;
}

