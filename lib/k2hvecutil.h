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

#ifndef	K2HVECUTIL_H
#define	K2HVECUTIL_H

#include <ctime>
#include <string>

#include "k2hveccommon.h"

//---------------------------------------------------------
// Utility Macros
//---------------------------------------------------------
#define	VEC_FREE(ptr)	\
		{ \
			if(ptr){ \
				free(ptr); \
				ptr = nullptr; \
			} \
		}

#define	VEC_DELETE(ptr)	\
		{ \
			if(ptr){ \
				delete ptr; \
				ptr = nullptr; \
			} \
		}

//---------------------------------------------------------
// Utility Functions - Endian
//---------------------------------------------------------
bool IsHostLittleEndian(void);

void CvtFloatLeToHost(float* parray, uint64_t length);
void CvtFloatHostToLe(float* parray, uint64_t length);

//---------------------------------------------------------
// Utility Functions - timespec
//---------------------------------------------------------
struct timespec GetCurrentTimespec();
struct timespec GetDurationTimespec(const struct timespec& start);
std::string CvtTimespecToTimeUnitString(const struct timespec& ts);
std::string CvtDurationTimespecToString(const struct timespec& start);
std::string CvtTimespecToYearString(struct timespec ts);
std::string CvtTimespecToDateString(struct timespec ts);
std::string CvtTimespecToShortDateString(struct timespec ts);
std::string GetCurrentTimespecToYearString();
std::string GetCurrentTimespecToDateString();
std::string GetCurrentTimespecToShortDateString();

//---------------------------------------------------------
// Utility Class - LapTime
//---------------------------------------------------------
class LapTime
{
	private:
		bool		isEnable;
		timespec	tsStart;

	public:
		LapTime() : isEnable(false), tsStart({0, 0}) {}
		explicit LapTime(bool enable) : isEnable(enable), tsStart({0, 0})
		{
			if(isEnable){
				tsStart = GetCurrentTimespec();
			}
		}
		~LapTime() {}

		void Start(bool enable = true)
		{
			isEnable	= enable;
			if(isEnable){
				tsStart = GetCurrentTimespec();
			}
		}

		timespec GetDuration(void) const
		{
			if(isEnable){
				return GetDurationTimespec(tsStart);
			}else{
				return {0, 0};
			}
		}

		std::string GetDurationString(void) const
		{
			if(isEnable){
				return CvtDurationTimespecToString(tsStart);
			}else{
				return "";
			}
		}
};

#endif	// K2HVECUTIL_H

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noexpandtab sw=4 ts=4 fdm=marker
 * vim<600: noexpandtab sw=4 ts=4
 */
