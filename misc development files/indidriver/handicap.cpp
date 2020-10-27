/*******************************************************************************
  Copyright(c) 2020 Ray Wells. All rights reserved.
 
  Based on Snapcap driver by:
 
  Copyright(c) 2017 Jarno Paananen. All right reserved.

  Based on Flip Flat driver by:

  Copyright(c) 2015 Jasem Mutlaq. All rights reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.

  The full GNU General Public License is included in this distribution in the
  file called LICENSE.
*******************************************************************************/

#include "handicap.h"

#include "indicom.h"
#include "connectionplugins/connectionserial.h"

#include <cerrno>
#include <cstring>
#include <memory>
#include <termios.h>
#include <sys/ioctl.h>

// We declare an auto pointer to handiCap.
std::unique_ptr<handiCap> handicap(new handiCap());

#define handi_CMD 7
#define handi_RES 8 // Includes terminating null
#define handi_TIMEOUT 3

void ISGetProperties(const char *dev)
{
    handicap->ISGetProperties(dev);
}

void ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{
    handicap->ISNewSwitch(dev, name, states, names, n);
}

void ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
    handicap->ISNewText(dev, name, texts, names, n);
}

void ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    handicap->ISNewNumber(dev, name, values, names, n);
}

void ISNewBLOB(const char *dev, const char *name, int sizes[], int blobsizes[], char *blobs[], char *formats[],
               char *names[], int n)
{
    INDI_UNUSED(dev);
    INDI_UNUSED(name);
    INDI_UNUSED(sizes);
    INDI_UNUSED(blobsizes);
    INDI_UNUSED(blobs);
    INDI_UNUSED(formats);
    INDI_UNUSED(names);
    INDI_UNUSED(n);
}

void ISSnoopDevice(XMLEle *root)
{
    handicap->ISSnoopDevice(root);
}

handiCap::handiCap() : LightBoxInterface(this, true)
{
    setVersion(1, 1);
}

bool handiCap::initProperties()
{
    INDI::DefaultDevice::initProperties();

    // Status
    IUFillText(&StatusT[0], "Cover", "", nullptr);
    IUFillText(&StatusT[1], "Light", "", nullptr);
    IUFillText(&StatusT[2], "Motor", "", nullptr);
    IUFillTextVector(&StatusTP, StatusT, 3, getDeviceName(), "Status", "", MAIN_CONTROL_TAB, IP_RO, 60, IPS_IDLE);

    // Firmware version
    IUFillText(&FirmwareT[0], "Version", "", nullptr);
    IUFillTextVector(&FirmwareTP, FirmwareT, 1, getDeviceName(), "Firmware", "", MAIN_CONTROL_TAB, IP_RO, 60, IPS_IDLE);

    // Abort and force open/close buttons
    IUFillSwitch(&AbortS[0], "Abort", "", ISS_OFF);
    IUFillSwitchVector(&AbortSP, AbortS, 1, getDeviceName(), "Abort", "", MAIN_CONTROL_TAB, IP_RW, ISR_1OFMANY, 0,
                       IPS_IDLE);

    IUFillSwitch(&ForceS[0], "Off", "", ISS_ON);
    IUFillSwitch(&ForceS[1], "On", "", ISS_OFF);
    IUFillSwitchVector(&ForceSP, ForceS, 2, getDeviceName(), "Force movement", "", MAIN_CONTROL_TAB, IP_RW, ISR_1OFMANY,
                       0, IPS_IDLE);

    
   initDustCapProperties(getDeviceName(), MAIN_CONTROL_TAB);
		
    maxCapAngleN[0].min  = 0;
    maxCapAngleN[0].max  = 255;
    maxCapAngleN[0].step = 10;
    
    setDriverInterface(AUX_INTERFACE | DUSTCAP_INTERFACE);

    addAuxControls();

    serialConnection = new Connection::Serial(this);
    serialConnection->registerHandshake([&]() { return Handshake(); });
    registerConnection(serialConnection);
    serialConnection->setDefaultBaudRate(Connection::Serial::B_9600);
    return true;
}

void handiCap::ISGetProperties(const char *dev)
{
    INDI::DefaultDevice::ISGetProperties(dev);

    // Get Light box properties
//    isGetLightBoxProperties(dev);
}

bool handiCap::updateProperties()
{
    INDI::DefaultDevice::updateProperties();

    if (isConnected())
    {
        defineSwitch(&ParkCapSP);
        defineText(&StatusTP);
        defineText(&FirmwareTP);
        defineSwitch(&AbortSP);
        defineSwitch(&ForceSP);
        getStartupData();
    }
    else
    {
        deleteProperty(ParkCapSP.name);
        deleteProperty(StatusTP.name);
        deleteProperty(FirmwareTP.name);
        deleteProperty(AbortSP.name);
        deleteProperty(ForceSP.name);
    }

    return true;
}

const char *handiCap::getDefaultName()
{
    return (const char *)"handiCap";
}

bool handiCap::Handshake()
{
    if (isSimulation())
    {
        LOGF_INFO("Connected successfully to simulated %s. Retrieving startup data...", getDeviceName());

        SetTimer(POLLMS);
        return true;
    }

    PortFD = serialConnection->getPortFD();

    if (!ping())
    {
        LOG_ERROR("Device ping failed.");
        return false;
    }

    return true;
}

bool handiCap::ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
 //   if (processLightBoxNumber(dev, name, values, names, n))
 //       return true;

    return INDI::DefaultDevice::ISNewNumber(dev, name, values, names, n);
}

bool handiCap::ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
   //     if (processLightBoxText(dev, name, texts, names, n))
    //        return true;
    }

    return INDI::DefaultDevice::ISNewText(dev, name, texts, names, n);
}

bool handiCap::ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{
    if (dev != nullptr && strcmp(dev, getDeviceName()) == 0)
    {
        if (strcmp(AbortSP.name, name) == 0)
        {
            IUResetSwitch(&AbortSP);
            AbortSP.s = Abort();
            IDSetSwitch(&AbortSP, nullptr);
            return true;
        }
        if (strcmp(ForceSP.name, name) == 0)
        {
            IUUpdateSwitch(&ForceSP, states, names, n);
            IDSetSwitch(&AbortSP, nullptr);
            return true;
        }

        if (processDustCapSwitch(dev, name, states, names, n))
            return true;

      //  if (processLightBoxSwitch(dev, name, states, names, n))
      //    return true;
    }

    return INDI::DefaultDevice::ISNewSwitch(dev, name, states, names, n);
}

bool handiCap::ISSnoopDevice(XMLEle *root)
{
    snoopLightBox(root);

    return INDI::DefaultDevice::ISSnoopDevice(root);
}

bool handiCap::saveConfigItems(FILE *fp)
{
    INDI::DefaultDevice::saveConfigItems(fp);

    //return saveLightBoxConfigItems(fp);
}

bool handiCap::ping()
{
    bool found = getFirmwareVersion();
    // Sometimes the controller does a corrupt reply at first connect
    // so retry once just in case
    if (!found)
        found = getFirmwareVersion();
    return found;
}

bool handiCap::sendCommand(const char *command, char *response)
{
    int nbytes_written = 0, nbytes_read = 0, rc = -1;
    char errstr[MAXRBUF];

    tcflush(PortFD, TCIOFLUSH);

    LOGF_DEBUG("CMD (%s)", command);

    char buffer[handi_CMD + 1]; // space for terminating null
    snprintf(buffer, handi_CMD + 1, "%s\r\n", command);

    if ((rc = tty_write(PortFD, buffer, handi_CMD, &nbytes_written)) != TTY_OK)
    {
        tty_error_msg(rc, errstr, MAXRBUF);
        LOGF_ERROR("%s error: %s.", command, errstr);
        return false;
    }

    if ((rc = tty_nread_section(PortFD, response, handi_RES, '\n', handi_TIMEOUT, &nbytes_read)) != TTY_OK)
    {
        tty_error_msg(rc, errstr, MAXRBUF);
        LOGF_ERROR("%s: %s.", command, errstr);
        return false;
    }

    response[nbytes_read - 2] = 0; // strip \r\n

    LOGF_DEBUG("RES (%s)", response);
    return true;
}

bool handiCap::getStartupData()
{
    bool rc1 = getFirmwareVersion();
    bool rc2 = getStatus();
    //bool rc3 = getBrightness();

    return (rc1 && rc2 /*&& rc3*/);
}

IPState handiCap::ParkCap()
{
    if (isSimulation())
    {
        simulationWorkCounter = 3;
        return IPS_BUSY;
    }

    char command[handi_CMD];
    char response[handi_RES];

    if (ForceS[1].s == ISS_ON)
        strncpy(command, ">c000", handi_CMD); // Force close command
    else
        strncpy(command, ">C000", handi_CMD);

    if (!sendCommand(command, response))
        return IPS_ALERT;

    if (strcmp(response, "*C000") == 0 || strcmp(response, "*c000") == 0)
    {
        // Set cover status to random value outside of range to force it to refresh
        prevCoverStatus   = 10;
        targetCoverStatus = 2;
        return IPS_BUSY;
    }
    else
        return IPS_ALERT;
}

IPState handiCap::UnParkCap()
{
    if (isSimulation())
    {
        simulationWorkCounter = 3;
        return IPS_BUSY;
    }

    char command[handi_CMD];
    char response[handi_RES];

    if (ForceS[1].s == ISS_ON)
        strncpy(command, ">o000", handi_CMD); // Force open command
    else
        strncpy(command, ">O000", handi_CMD);

    if (!sendCommand(command, response))
        return IPS_ALERT;

    if (strcmp(response, "*O000") == 0 || strcmp(response, "*o000") == 0)
    {
        // Set cover status to random value outside of range to force it to refresh
        prevCoverStatus   = 10;
        targetCoverStatus = 1;
        return IPS_BUSY;
    }
    else
        return IPS_ALERT;
}

IPState handiCap::Abort()
{
    if (isSimulation())
    {
        simulationWorkCounter = 0;
        return IPS_OK;
    }

    char response[handi_RES];

    if (!sendCommand(">A000", response))
        return IPS_ALERT;

    if (strcmp(response, "*A000") == 0)
    {
        // Set cover status to random value outside of range to force it to refresh
        prevCoverStatus = 10;
        return IPS_OK;
    }
    else
        return IPS_ALERT;
}

bool handiCap::getStatus()
{
    char response[handi_RES];

    if (isSimulation())
    {
        if (ParkCapSP.s == IPS_BUSY && --simulationWorkCounter <= 0)
        {
            ParkCapSP.s = IPS_IDLE;
            IDSetSwitch(&ParkCapSP, nullptr);
            simulationWorkCounter = 0;
        }

        if (ParkCapSP.s == IPS_BUSY)
        {
            response[2] = '1';
            response[4] = '0';
        }
        else
        {
            response[2] = '0';
            // Parked/Closed
            if (ParkCapS[CAP_PARK].s == ISS_ON)
                response[4] = '2';
            else
                response[4] = '1';
        }

        //response[3] = (LightS[FLAT_LIGHT_ON].s == ISS_ON) ? '1' : '0';
    }
    else
    {
        if (!sendCommand(">S000", response))
            return false;
    }

    char motorStatus = response[2] - '0';
    //char lightStatus = response[3] - '0';
    char coverStatus = response[4] - '0';

    // Force cover status as it doesn't reflect moving state otherwise...
    if (motorStatus)
    {
        coverStatus = 0;
    }
    bool statusUpdated = false;

    if (coverStatus != prevCoverStatus)
    {
        prevCoverStatus = coverStatus;

        statusUpdated = true;

        switch (coverStatus)
        {
            case 0:
                IUSaveText(&StatusT[0], "Opening/closing");
                break;

            case 1:
                if ((targetCoverStatus == 1 && ParkCapSP.s == IPS_BUSY) || ParkCapSP.s == IPS_IDLE)
                {
                    IUSaveText(&StatusT[0], "Open");
                    IUResetSwitch(&ParkCapSP);
                    ParkCapS[CAP_UNPARK].s = ISS_ON;
                    ParkCapSP.s            = IPS_OK;
                    LOG_INFO("Cover open.");
                    IDSetSwitch(&ParkCapSP, nullptr);
                }
                break;

            case 2:
                if ((targetCoverStatus == 2 && ParkCapSP.s == IPS_BUSY) || ParkCapSP.s == IPS_IDLE)
                {
                    IUSaveText(&StatusT[0], "Closed");
                    IUResetSwitch(&ParkCapSP);
                    ParkCapS[CAP_PARK].s = ISS_ON;
                    ParkCapSP.s          = IPS_OK;
                    LOG_INFO("Cover closed.");
                    IDSetSwitch(&ParkCapSP, nullptr);
                }
                break;

            case 3:
                IUSaveText(&StatusT[0], "Timed out");
                break;

            case 4:
                IUSaveText(&StatusT[0], "Open circuit");
                break;

            case 5:
                IUSaveText(&StatusT[0], "Overcurrent");
                break;

            case 6:
                IUSaveText(&StatusT[0], "User abort");
                break;
        }
    }



    if (motorStatus != prevMotorStatus)
    {
        prevMotorStatus = motorStatus;

        statusUpdated = true;

        switch (motorStatus)
        {
            case 0:
                IUSaveText(&StatusT[2], "Stopped");
                break;

            case 1:
                IUSaveText(&StatusT[2], "Running");
                break;
        }
    }

    if (statusUpdated)
        IDSetText(&StatusTP, nullptr);

    return true;
}

bool handiCap::getFirmwareVersion()
{
    if (isSimulation())
    {
        IUSaveText(&FirmwareT[0], "Simulation");
        IDSetText(&FirmwareTP, nullptr);
        return true;
    }

    char response[handi_RES];

    if (!sendCommand(">V000", response))
        return false;

    char versionString[4] = { 0 };
    snprintf(versionString, 4, "%s", response + 2);
    IUSaveText(&FirmwareT[0], versionString);
    IDSetText(&FirmwareTP, nullptr);

    return true;
}

void handiCap::TimerHit()
{
    if (!isConnected())
        return;

    getStatus();

    SetTimer(POLLMS);
}

}
