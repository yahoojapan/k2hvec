/*
 * K2HVEC
 *
 * Copyright 2026 LY Corporation.
 *
 * K2HVEC is a high-performance vector database for fast vector search,
 * built on top of the K2HASH key-value store.
 *
 * For the full copyright and license information, please view
 * the license file that was distributed with this source code.
 *
 * AUTHOR:   Takeshi Nakatani
 * CREATE:   Tue, Jul 21 2026
 * REVISION:
 *
 */

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <climits>
#include <sys/time.h>

#include <ctime>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "k2hvecutil.h"
#include "k2hveccommon.h"
#include "k2hvecdbg.h"

using namespace std;

//---------------------------------------------------------
// Utilities : Endian
//---------------------------------------------------------
//
// Host endian type
//
// Determines and holds a class member variable at compile time for
// endianness identification.
//
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	static constexpr bool HostLE = true;
#else
	static constexpr bool HostLE = false;
#endif

//
// Host endian
//
bool IsHostLittleEndian(void)
{
	return HostLE;
}

//
// Endianness conversion utilities
//
void CvtFloatLeToHost(float* parray, uint64_t length)
{
	if(!HostLE){
		if(parray){
			uint64_t count = length / sizeof(float);
			for(uint64_t pos = 0; pos < count; ++pos){
				uint32_t val;
				std::memcpy(&val, &parray[pos], sizeof(uint32_t));
				val = le32toh(val);
				std::memcpy(&parray[pos], &val, sizeof(float));
			}
		}
	}
}

void CvtFloatHostToLe(float* parray, uint64_t length)
{
	if(!HostLE){
		if(parray){
			uint64_t count = length / sizeof(float);
			for(uint64_t pos = 0; pos < count; ++pos){
				uint32_t val;
				std::memcpy(&val, &parray[pos], sizeof(uint32_t));
				val = htole32(val);
				std::memcpy(&parray[pos], &val, sizeof(float));
			}
		}
	}
}

//---------------------------------------------------------
// Utilities : Timespec
//---------------------------------------------------------
// [NOTE]
// In the future, we will migrate to the std::chrono class.
//
struct timespec GetCurrentTimespec()
{
	struct timespec	ts;
	if(-1 == clock_gettime(CLOCK_REALTIME, &ts)){			// Do not use CLOCK_REALTIME_COARSE
		ts.tv_sec	= time(nullptr);
		ts.tv_nsec	= 0;
	}
	return ts;
}

struct timespec GetDurationTimespec(const struct timespec& start)
{
	struct timespec now			= GetCurrentTimespec();
	struct timespec duration	= {0, 0};

	if(now.tv_nsec < start.tv_nsec){
		duration.tv_nsec = (1000 * 1000 * 1000) + now.tv_nsec - start.tv_nsec;
		now.tv_sec		-= 1;
	}else{
		duration.tv_nsec = now.tv_nsec - start.tv_nsec;
	}
	duration.tv_sec = now.tv_sec - start.tv_sec;

	return duration;
}

std::string CvtTimespecToTimeUnitString(const struct timespec& ts)
{
	std::string	result("");

	if(0 < ts.tv_sec){
		result += std::to_string(ts.tv_sec);
		result += "s ";
	}
	if(!result.empty() || 0 < (ts.tv_nsec / (1000 * 1000))){
		result += std::to_string(ts.tv_nsec / (1000 * 1000));
		result += "ms ";
	}
	if(!result.empty() || 0 < ((ts.tv_nsec % (1000 * 1000)) / 1000)){
		result += std::to_string((ts.tv_nsec % (1000 * 1000)) / 1000);
		result += "us ";
	}
	result += std::to_string(ts.tv_nsec % 1000);
	result += "ns (";

	result += std::to_string((ts.tv_sec * 1000 * 1000 * 1000) + ts.tv_nsec);
	result += "ns)";

	return result;
}

std::string CvtDurationTimespecToString(const struct timespec& start)
{
	return CvtTimespecToTimeUnitString(GetDurationTimespec(start));
}

std::string CvtTimespecToYearString(struct timespec ts)
{
	struct tm	local_time;
	localtime_r(&ts.tv_sec, &local_time);

	char	tmpbuf[8];
	std::strftime(tmpbuf, sizeof(tmpbuf), "%Y", &local_time);

	return tmpbuf;
}

std::string CvtTimespecToDateString(struct timespec ts)
{
	struct tm	local_time;
	localtime_r(&ts.tv_sec, &local_time);

	char		tmpbuf[64];
	std::strftime(tmpbuf, sizeof(tmpbuf), "%Y/%m/%d %H:%M:%S", &local_time);

    std::stringstream	ss;
    ss << tmpbuf << "." << std::setfill('0') << std::setw(3) << (ts.tv_nsec / 1000000);

	return ss.str();
}

std::string CvtTimespecToShortDateString(struct timespec ts)
{
	struct tm	local_time;
	localtime_r(&ts.tv_sec, &local_time);

	char		tmpbuf[64];
	std::strftime(tmpbuf, sizeof(tmpbuf), "%Y%m%d%H%M%S", &local_time);

    std::stringstream	ss;
    ss << tmpbuf << "." << std::setfill('0') << std::setw(3) << (ts.tv_nsec / 1000000);

	return ss.str();
}

std::string GetCurrentTimespecToYearString()
{
	return CvtTimespecToYearString(GetCurrentTimespec());
}

std::string GetCurrentTimespecToDateString()
{
	return CvtTimespecToDateString(GetCurrentTimespec());
}

std::string GetCurrentTimespecToShortDateString()
{
	return CvtTimespecToShortDateString(GetCurrentTimespec());
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noexpandtab sw=4 ts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4
 */
