#include "legato.h"
#include "interfaces.h"

#define LEDMATRIX_NAME "ledmatrix/value"

#define LEDMATRIX_CONTROL "/app/ledmatrixControl/ledmatrix/led1/value"

#define LEDMATRIX_OBS "ledmatrixOffLimits"

#define LEDMATRIX_LOWER_LIMIT 50.000000
#define LEDMATRIX_UPPER_LIMIT 50.000000

//--------------------------------------------------------------------------------------------------
/**
 * Call-back function called when an update is received from the Data Hub for the latitude value.
 */
//--------------------------------------------------------------------------------------------------
static void LedmatrixUpdateHandler
(
	double timestamp,
	double value,
	void* contextPtr
)
{
	LE_DEBUG("ledmatrix = %lf (timestamped %lf)", value, timestamp);
}


//--------------------------------------------------------------------------------------------------
/**
 * Call-back function called when an update is received from the Data Hub for the led matrix value.
 */
//--------------------------------------------------------------------------------------------------
static void LedmatrixObservationUpdateHandler
(
	double timestamp,	///< time stamp
	double value,		///< ledmatrix value,
	void* contextPtr	///< not used 
)
{
	LE_INFO("Observed filtered led matrix = %lf (timestamped %lf)", value, timestamp);

	char led_value[50];
	snprintf(led_value, 50, "%f", value);

	ma_ledmatrix_displayString(led_value, 10000, true, MA_LEDMATRIX_green);

}


COMPONENT_INIT
{
	le_result_t result;
	ma_ledmatrix_init("/dev/i2c-0", 0, 0);
	
	uint16_t pid = ma_ledmatrix_getDevicePID();
	uint16_t vid = ma_ledmatrix_getDeviceVID();
	LE_INFO("PID: %x; VID: %x\n", pid, vid);

	// This will be received from the Data Hub.
	result = io_CreateOutput(LEDMATRIX_NAME, IO_DATA_TYPE_NUMERIC, "degrees");
	LE_ASSERT(result == LE_OK);

	// Register for notification of updates to the counter value.
	io_AddNumericPushHandler(LEDMATRIX_NAME, LedmatrixUpdateHandler, NULL);

	// Connect to the sensor
	result = admin_SetSource("/app/ledmatrixDisplay/" LEDMATRIX_NAME, LEDMATRIX_CONTROL);
	LE_ASSERT(result == LE_OK);

	// Create observation (filter) for ledmatrix. To set up the "dead band" filter,
	// lower limit assigned to high limit and vice versa (see admin.io doc for details).
	admin_CreateObs(LEDMATRIX_OBS);
	admin_SetLowLimit(LEDMATRIX_OBS, LEDMATRIX_UPPER_LIMIT);
	admin_SetHighLimit(LEDMATRIX_OBS, LEDMATRIX_LOWER_LIMIT);
	result = admin_SetSource("/obs/" LEDMATRIX_OBS, LEDMATRIX_CONTROL);
	LE_ASSERT(result == LE_OK);
	admin_AddNumericPushHandler("/obs/" LEDMATRIX_OBS, LedmatrixObservationUpdateHandler, NULL);

	LE_ASSERT(result == LE_OK);
}
