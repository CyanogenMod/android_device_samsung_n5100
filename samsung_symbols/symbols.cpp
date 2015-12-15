/*
 * Copyright (C) 2015 The CyanogenMod Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <malloc.h>

/***********************************************************************************/
// LIBRIL //
/***********************************************************************************/

// android::Parcel::writeString16(unsigned short const*, unsigned int)
extern "C" int _ZN7android6Parcel13writeString16EPKtj();
extern "C" int _ZN7android6Parcel13writeString16EPKtj(){
    return _ZN7android6Parcel13writeString16EPKtj();
}

/***********************************************************************************/
// GPSD //
/***********************************************************************************/

// android::Singleton<android::SensorManager>::sLock
extern "C" int _ZN7android9SingletonINS_13SensorManagerEE5sLockE = NULL;

// android::Singleton<android::SensorManager>::sInstance
extern "C" int _ZN7android9SingletonINS_13SensorManagerEE9sInstanceE = NULL;

// android::SensorManager::SensorManager()
extern "C" int _ZN7android13SensorManagerC1Ev();
extern "C" int _ZN7android13SensorManagerC1Ev(){
    return _ZN7android13SensorManagerC1Ev();
}

// android::SensorManager::createEventQueue()
extern "C" int _ZN7android13SensorManager16createEventQueueEv();
extern "C" int _ZN7android13SensorManager16createEventQueueEv(){
    return _ZN7android13SensorManager16createEventQueueEv();
}

extern "C" {
void *CRYPTO_malloc(int num, const char *file, int line) {
    return malloc(num);
}

}
/***********************************************************************************/
// AT_DISTRIBUTOR //
/***********************************************************************************/

extern "C" int IsSupportBrcmLogicalUart = NULL;
extern "C" int IsSupportSwitchPhysicalUart = NULL;
extern "C" int IsSupportQualcommLogicalUart = NULL;
extern "C" int IsDuosRil = NULL;

