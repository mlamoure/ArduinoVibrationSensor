#include "XBee.h"
#include <SoftwareSerial.h>

// ##################################################################
// CONFIGURATION
bool publishRawResults = false;
int thresholdVibration = 3;
int thresholdIterations = 10;
int refreshRate = 3000; // in MS

// ##################################################################

bool dryerPreviousStatus;
bool dryerSyncStatus;
int numIterations;
int previousValues[3];

uint8_t recv = 2; 
uint8_t trans = 3; 
SoftwareSerial soft_serial(recv, trans);

XBee xbee = XBee(); 
ZBRxIoSampleResponse ioSample = ZBRxIoSampleResponse();

// Specify the address of the remote XBee (this is the SH + SL)
XBeeAddress64 addr64 = XBeeAddress64(0x00000000, 0x00000000);

const int xInput = A0;
const int yInput = A1;
const int zInput = A2;

int xRawMin = 403;
int xRawMax = 635;

int yRawMin = 399;
int yRawMax = 604;

int zRawMin = 432;
int zRawMax = 638;

// Take multiple samples to reduce noise
const int sampleSize = 10;

void setup() 
{
	analogReference(EXTERNAL);
	Serial.begin(9600);
	soft_serial.begin(9600);
	xbee.setSerial(soft_serial);
}

void outputToConsole(String x, String y, String z) {
//	AutoCalibrate(xRaw, yRaw, zRaw);
/*
	Serial.print("Raw Ranges: X: ");
	Serial.print(xRawMin);
	Serial.print("-");
	Serial.print(xRawMax);
	
	Serial.print(", Y: ");
	Serial.print(yRawMin);
	Serial.print("-");
	Serial.print(yRawMax);
	
	Serial.print(", Z: ");
	Serial.print(zRawMin);
	Serial.print("-");
	Serial.print(zRawMax);

	Serial.println();
*/

	Serial.print(x);
	Serial.print(", ");
	Serial.print(y);
	Serial.print(", ");
	Serial.print(z);
	Serial.println();
}

void loop() 
{	
	Serial.println("*************************");
	bool dryerStatus = false;
	int rawArray[3];

	rawArray[0] = ReadAxis(xInput);
	rawArray[1] = ReadAxis(yInput);
	rawArray[2] = ReadAxis(zInput);

	Serial.println("Old Values: ");
	outputToConsole(String(previousValues[0]), String(previousValues[1]), String(previousValues[2]));

	Serial.println("New Values: ");
	outputToConsole(String(rawArray[0]), String(rawArray[1]), String(rawArray[2]));

	
	// Convert raw values to 'milli-Gs"
	long xScaled = map(rawArray[0], xRawMin, xRawMax, -1000, 1000);
	long yScaled = map(rawArray[1], yRawMin, yRawMax, -1000, 1000);
	long zScaled = map(rawArray[2], zRawMin, zRawMax, -1000, 1000);

	float accelArray[3];

	// re-scale to fractional Gs
	accelArray[0] = xScaled / 1000.0;
	accelArray[1] = yScaled / 1000.0;
	accelArray[2] = zScaled / 1000.0;

	//outputToConsole(String(accelArray[0]), String(accelArray[1]), String(accelArray[2]));

	for (int xyz = 0; xyz < 3; xyz++) {
		if (rawArray[xyz] - previousValues[xyz] > thresholdVibration ||  previousValues[xyz] - rawArray[xyz] > thresholdVibration) {
			dryerStatus = true;
			previousValues[xyz] = rawArray[xyz];
		}
	}

	// Did the dryer status change?
	if (dryerStatus == dryerPreviousStatus) {
		numIterations++;
	}
	else {
		numIterations = 0;
	}

	if (dryerStatus != dryerSyncStatus && numIterations >= thresholdIterations) {
		dryerSyncStatus = dryerStatus;

		String payloadStr = buildJSON(dryerStatus, rawArray);
		publishZWaveTx(payloadStr);
	}

	dryerPreviousStatus = dryerStatus;

	Serial.println("dryerStatus: " + String(dryerStatus));
	Serial.println("dryerPreviousStatus: " + String(dryerPreviousStatus));
	Serial.println("dryerSyncStatus: " + String(dryerSyncStatus));
	Serial.println("numIterations: " + String(numIterations) + " / " +  String(thresholdIterations));

	Serial.println("*************************");
	delay(refreshRate);
}

void publishZWaveTx(String payloadStr) {
	char payload[payloadStr.length() + 1];
	payloadStr.toCharArray(payload, payloadStr.length() + 1); 	

	ZBTxRequest zbTx = ZBTxRequest(addr64, (uint8_t *) payload, strlen(payload));

	// Send your request
	xbee.send(zbTx);
}

String buildJSON(bool dryerStatus, int* rawArray) {
	String payloadStr;

	payloadStr = "{";
	payloadStr += "\"id\": \"dryer\",";
	payloadStr += "\"status\": ";

	if (dryerStatus) {
		payloadStr += "true";
	}
	else
	{
		payloadStr += "false";
	}

	if (publishRawResults) {
	
		for (int xyz = 0; xyz < 3; xyz++) {
			switch (xyz) {
				case 0:
					payloadStr += ", \"xRaw\": ";
					break;

				case 1:
					payloadStr += ", \"yRaw\": ";
					break;

				case 2:
					payloadStr += ", \"zRaw\": ";
					break;
			}

			for (int i = 0; i < String(rawArray[xyz]).length(); i++) {
				payloadStr += String(rawArray[xyz])[i];
			}
		}

	}

	payloadStr += "}";	

	return (payloadStr);
}

//
// Read "sampleSize" samples and report the average
//
int ReadAxis(int axisPin)
{
	long reading = 0;
	analogRead(axisPin);
	delay(1);
	for (int i = 0; i < sampleSize; i++)
	{
		reading += analogRead(axisPin);
	}
	return reading/sampleSize;
}

//
// Find the extreme raw readings from each axis
//
void AutoCalibrate(int xRaw, int yRaw, int zRaw)
{
	Serial.println("Calibrate");
	if (xRaw < xRawMin)
	{
		xRawMin = xRaw;
	}
	if (xRaw > xRawMax)
	{
		xRawMax = xRaw;
	}
	
	if (yRaw < yRawMin)
	{
		yRawMin = yRaw;
	}
	if (yRaw > yRawMax)
	{
		yRawMax = yRaw;
	}

	if (zRaw < zRawMin)
	{
		zRawMin = zRaw;
	}
	if (zRaw > zRawMax)
	{
		zRawMax = zRaw;
	}
}