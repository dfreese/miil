#ifndef PROCESSING_H
#define PROCESSING_H

#include <deque>
#include <vector>
#include <miil/EventRaw.h>
#include <miil/EventCal.h>

class SystemConfiguration;

int DecodePacketByteStream(
        const std::deque<char>::iterator begin,
        const std::deque<char>::iterator end,
        SystemConfiguration const * const system_config,
        std::vector<EventRaw> & events);

int RawEventToEventCal(
        const EventRaw & rawevent,
        EventCal & event,
        SystemConfiguration const * const system_config);

int CalculateXYandEnergy(
        const EventRaw & rawevent,
        SystemConfiguration const * const system_config,
        float & x,
        float & y,
        float & energy,
        int & apd);

bool InEnergyWindow(const EventCal & event, float low, float high);

float EventCalTimeDiff(
        const EventCal & arg1,
        const EventCal & arg2,
        float uv_period_ns,
        float ct_period_ns);

bool EventCalLessThan(
        const EventCal & arg1,
        const EventCal & arg2,
        float uv_period_ns,
        float ct_period_ns);

bool EventCalLessThanOnlyCt(const EventCal & arg1, const EventCal & arg2);


#endif // PROCESSING_H