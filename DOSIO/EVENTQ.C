/*
 * Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: eventq.c 1.3 1995/09/07 15:19:39 sam Exp $
 * $Locker: sam $
 *
 * Event queue for mouse and keyboard events - events can be added to queue
 * from both real-mode and protected mode
 */
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <dos.h>
#include <conio.h>

#include "fw.h"
#include "syshost.h"
#include "segregs.h"
#include "realsupt.h"
#include "farmem.h"
#include "eventq.h"

static char rscid[] = "$Id: eventq.c 1.3 1995/09/07 15:19:39 sam Exp $";

#define MAX_EVENTS 50

dosio_real_memory _DOSEventMemory;
int _DOSEventQEnabled = 0;

void BR_PUBLIC_ENTRY DOSEventBegin(void)
{
	int s = sizeof(dosio_event_queue) + sizeof(dosio_event) * MAX_EVENTS;

	/*
	 * Put event queue in a real mode buffer
	 */
	if(_RealAllocate(&_DOSEventMemory, s)) {
		BR_ERROR0("Could not allocate real mode buffer (E)\n");
	}

// 	_RealLock(&_DOSEventMemory, s);

	FarBlockFill(_DOSEventMemory.pm_off, _DOSEventMemory.pm_seg, 0, s);
	FarWordWrite(_DOSEventMemory.pm_off + offsetof(dosio_event_queue,total), _DOSEventMemory.pm_seg, MAX_EVENTS);

	_DOSEventQEnabled = 1;

#ifdef __X32z__
	_disable();
	_enable();
#endif
}

void BR_PUBLIC_ENTRY DOSEventEnd(void)
{
	/*
	 * Free up event queue
	 */
	_DOSEventQEnabled = 0;
	_RealFree(&_DOSEventMemory);
}

#define EVENTQ_GET(field) 		FarWordRead(_DOSEventMemory.pm_off + offsetof(dosio_event_queue,field), _DOSEventMemory.pm_seg)
#define EVENTQ_SET(field,value) FarWordWrite(_DOSEventMemory.pm_off + offsetof(dosio_event_queue,field), _DOSEventMemory.pm_seg,(value))

br_uint_16 BR_PUBLIC_ENTRY DOSEventWait(dosio_event *event, int block)
{
	int t;

	if(block) {
		/*
		 * Busy wait on an incoming event
		 */
		while(EVENTQ_GET(head) == EVENTQ_GET(tail))
			;
	} else {
		if(EVENTQ_GET(head) == EVENTQ_GET(tail))
			return 0;
	}

	t = EVENTQ_GET(tail);

	/*
	 * Extract event from queue
	 */
	FarBlockRead(
		_DOSEventMemory.pm_off + offsetof(dosio_event_queue,slots) +
		sizeof(*event) * t,
		_DOSEventMemory.pm_seg,
		(unsigned char *)event,
		sizeof(*event));

	/*
	 * Bump tail pointer over event
	 */
	t += 1;

	if( t >= EVENTQ_GET(total))
		t = 0;

	EVENTQ_SET(tail, t);

	EVENTQ_SET(count, EVENTQ_GET(count)-1);

	return 1;
}

