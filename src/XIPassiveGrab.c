/*
 * Copyright © 2009 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include <stdint.h>
#include <X11/Xlibint.h>
#include <X11/extensions/XI2proto.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/extutil.h>
#include "XIint.h"

static int
_XIPassiveGrabDevice(Display* dpy, int deviceid, int grabtype, int detail,
                     Window grab_window, Cursor cursor,
                     int grab_mode, int paired_device_mode,
                     Bool owner_events, XIDeviceEventMask *mask,
                     int num_modifiers, int *modifiers_inout)
{
    xXIPassiveGrabDeviceReq *req;
    xXIPassiveGrabDeviceReply reply;
    int len = 0;
    char *buff;

    XExtDisplayInfo *extinfo = XInput_find_display(dpy);

    LockDisplay(dpy);
    if (_XiCheckExtInit(dpy, Dont_Check, extinfo) == -1)
	return -1;

    GetReq(XIPassiveGrabDevice, req);
    req->reqType = extinfo->codes->major_opcode;
    req->ReqType = X_XIPassiveGrabDevice;
    req->deviceid = deviceid;
    req->grab_mode = grab_mode;
    req->paired_device_mode = paired_device_mode;
    req->owner_events = owner_events;
    req->grab_window = grab_window;
    req->cursor = cursor;
    req->detail = detail;
    req->num_modifiers = num_modifiers;
    req->mask_len = (mask->mask_len + 3)/4;
    req->grab_type = grabtype;

    len = req->mask_len + num_modifiers;
    SetReqLen(req, len, len);

    buff = calloc(4, req->mask_len);
    memcpy(buff, mask->mask, mask->mask_len);
    Data32(dpy, buff, req->mask_len * 4);
    Data32(dpy, modifiers_inout, (num_modifiers * 4));

    free(buff);

    if (_XReply(dpy, (xReply *)&reply, 0, xTrue))
    {
	UnlockDisplay(dpy);
	SyncHandle();
	return -1;
    }

    _XRead(dpy, (char*)modifiers_inout, reply.num_modifiers * 4);

    UnlockDisplay(dpy);
    SyncHandle();
    return reply.num_modifiers;
}

int
XIGrabButton(Display* dpy, int deviceid, int button,
             Window grab_window, Cursor cursor,
             int grab_mode, int paired_device_mode,
             Bool owner_events, XIDeviceEventMask *mask,
             int num_modifiers, int *modifiers_inout)
{
    return _XIPassiveGrabDevice(dpy, deviceid, GrabtypeButton, button,
                                grab_window, cursor, grab_mode,
                                paired_device_mode, owner_events, mask,
                                num_modifiers, modifiers_inout);
}

int
XIGrabKeysym(Display* dpy, int deviceid, int keysym,
             Window grab_window, int grab_mode, int paired_device_mode,
             Bool owner_events, XIDeviceEventMask *mask,
             int num_modifiers, int *modifiers_inout)
{
    return _XIPassiveGrabDevice(dpy, deviceid, GrabtypeKeysym, keysym,
                                grab_window, None, grab_mode, paired_device_mode,
                                owner_events, mask, num_modifiers,
                                modifiers_inout);
}

static int
_XIPassiveUngrabDevice(Display* dpy, int deviceid, int grabtype, int detail,
                       Window grab_window, int num_modifiers, int *modifiers)
{
    xXIPassiveUngrabDeviceReq *req;

    XExtDisplayInfo *extinfo = XInput_find_display(dpy);

    LockDisplay(dpy);
    if (_XiCheckExtInit(dpy, Dont_Check, extinfo) == -1)
	return -1;

    GetReq(XIPassiveUngrabDevice, req);
    req->reqType = extinfo->codes->major_opcode;
    req->ReqType = X_XIPassiveUngrabDevice;
    req->deviceid = deviceid;
    req->grab_window = grab_window;
    req->detail = detail;
    req->num_modifiers = num_modifiers;
    req->grab_type = grabtype;

    SetReqLen(req, num_modifiers, num_modifiers);
    Data32(dpy, modifiers, (num_modifiers * 4));

    UnlockDisplay(dpy);
    SyncHandle();
    return Success;
}

int
XIUngrabButton(Display* display, int deviceid, int button,Window grab_window,
               int num_modifiers, int *modifiers)
{
    return _XIPassiveUngrabDevice(display, deviceid, GrabtypeButton, button,
                                  grab_window, num_modifiers, modifiers);
}

int
XIUngrabKeysym(Display* display, int deviceid, int keysym, Window grab_window,
               int num_modifiers, int *modifiers)
{
    return _XIPassiveUngrabDevice(display, deviceid, GrabtypeKeysym, keysym,
                                  grab_window, num_modifiers, modifiers);
}


