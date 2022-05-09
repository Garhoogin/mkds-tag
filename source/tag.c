#include <nds.h>

#define RACE_MODE_MG 2
#define RACE_MG_MODE_SHINE_RUNNERS 1

#define TAG_COOLDOWN 60 /* full second cooldown */

int g_playerIt = 0;
static u32 g_lastTagFrame = 0; //used to have a slight cooldown, for points reasons mostly
static int g_indicatorTimer = 0; //used to animate the It indicator
static int g_isTagMode = 1; 

//externs
void mgcnt_updateDriverBalloonShineCount(u16 driverId, int delta);
void mgcnt_killDriverDirect(u16 driverId);

static inline int rconf_getRaceMode() {
	return *(u32 *) ((*(u32 *) 0x021759C0) + 8);
}

static inline u32 rconf_getDriverCount() {
	return (u32) *(u8 *) ((*(u32 *) 0x021759C0) + 0x3D0);
}

static inline int rconf_getMgMode() {
	return *(int *) ((*(u32 *) 0x021759C0) + 0x18);
}

static inline u32 OS_GetVBlankCount() {
	return *(vu32 *) 0x027FFC3C;
}

typedef struct {
	u64 x;
	u64 mul;
	u64 add;
} MATHRandContext32;

static inline u32 MATH_Rand32(MATHRandContext32 *context, u32 max) {
	context->x = context->mul * context->x + context->add;
	
	if (max == 0) {
		return context->x >> 32;
	} else {
		return ((context->x >> 32) * max) >> 32;
	}
}

static inline int GetShineCount(int driverIndex) {
	return *(u32 *) ((*(u32 *) 0x0217B1FC) + 0x70 * driverIndex + 0x4C);
}

static inline int GetMaxShineCount() {
	return *(int *) ((*(u32 *) 0x0217B1FC) + 0x3AC);
}

static inline int DriverIsDead(int driverIndex) {
	return GetShineCount(driverIndex) == -1;
}

static inline int GetDriverTeam(int driverIndex) {
	return (int) *(u16 *) ((*(u32 *) 0x0217B1FC) + driverIndex * 0x70 + 0x46);
}

static inline int GetDriverIndex(void *driver) {
	return (int) *(u16 *) (((u32) driver) + 0x74);
}

static void LogItTransfer(int srcIndex, int dstIndex) {
#if 0
	char bf[32];
	OS_SPrintf(bf, "It: %d -> %d     %d", srcIndex, dstIndex, OS_GetVBlankCount());
	nocashMessage(bf);
#endif
}

int IsTagMode() {
	if(rconf_getRaceMode() != RACE_MODE_MG) return 0;
	if(rconf_getMgMode() != RACE_MG_MODE_SHINE_RUNNERS) return 0;

	//TODO: implement logic here
	return g_isTagMode;
}

void SetTagMode() {
	g_isTagMode = 1;
}

void UnsetTagMode() {
	g_isTagMode = 0;
}

void DriverHandleCallback(void *srcDriver, void *dstDriver, s32 *pushbackDir) {
	//only do this if we're in a minigame
	if(!IsTagMode()) return;
	
	//void *driverList = *(void **) 0x0217D028;
	//int srcDriverIndex = (((u32) srcDriver) - (u32) driverList) / 0x5A8;
	int srcDriverIndex = GetDriverIndex(srcDriver);
	
	//is this player it?
	if(srcDriverIndex != g_playerIt) return;

	//check: has the cooldown expired?
	u32 frame = OS_GetVBlankCount();
	if(frame >= g_lastTagFrame + TAG_COOLDOWN) {

		//check that, if in teams mode, the two drivers aren't on the same team
		//int dstDriverIndex = (((u32) dstDriver) - (u32) driverList) / 0x5A8;
		int dstDriverIndex = GetDriverIndex(dstDriver);
		if(GetDriverTeam(srcDriverIndex) != GetDriverTeam(dstDriverIndex)) {		
			g_playerIt = dstDriverIndex;
			
			//add 1 point to srcDriver
			mgcnt_updateDriverBalloonShineCount(srcDriverIndex, 1);
			LogItTransfer(srcDriverIndex, dstDriverIndex);
			
			g_lastTagFrame = frame;
		}
	}
}

int MarkerDrawCallback(int defaultRender, int driverIndex) {
	//if driver is not It, use its normal color
	if(!IsTagMode() || driverIndex != g_playerIt) return defaultRender;
	
	//driver is It, render one of the It indicators
	g_indicatorTimer++;
	g_indicatorTimer &= 0x1F;
	int frame = defaultRender; //default
	if(g_indicatorTimer & 0x10) {
		if(defaultRender == 1 || defaultRender == 0) frame = 2; //red
		else frame = 1; //blue
	}
	return frame;
}

void ChooseNextPlayerIt() {
	//select the next It player.

	//add up total of (maxShines + 1 - player[i].nShines)
	//generate random number in this range
	int ptTotal = 0;
	int i = 0, nDrivers = rconf_getDriverCount();
	int nMaxShines = GetMaxShineCount();

	for(i = 0; i < nDrivers; i++) {
		if(DriverIsDead(i)) continue;
		ptTotal += nMaxShines + 1 - GetShineCount(i);
	}
	int rnd = MATH_Rand32((MATHRandContext32 *) ((*(u32 *) 0x0217561C) + 0x47C), ptTotal);
	for(i = 0; i < nDrivers; i++) {
		if(DriverIsDead(i)) continue;
		rnd -= (nMaxShines + 1 - GetShineCount(i));
		if(rnd < 0) break;
	}
	
	LogItTransfer(g_playerIt, i);
	g_playerIt = i;
}

int IsDriverIt(int driverId) {
	return g_playerIt == driverId;
}
